//
// Created by zkw on 20-11-16.
//

#include "mmap_tracer.h"
#include "clooper/looper.h"
#include <map>
#include <cerrno>

extern "C" {
#include "log.h"
}

/**
 * log 模式：每次 mmap 或 munmap 都保存一条日志（可以是文件），之后出报表分析曲线，用于观察分配趋势
 */
#define MODE_LOG 1
/**
 * dump 模式：在内存里统一维护，mmap 时增，munmap 时减，在某个时间点保存到文件，用于观察当前进程内存分配状态
 */
#define MODE_DUMP 2

#define ADD_MMAP_INFO 5
#define DELETE_MMAP_INFO 6
#define DO_DUMP_FOR_MODE_DUMP 1

#define INFO_MMAP "mmap"
#define INFO_MUNMMAP "munmmap"
#define INFO_DIV "="

using namespace std;

message_looper_t *looper;

FILE *dumpFile = nullptr;
size_t totalSize;

int modeFlag = MODE_LOG;

// 几种处理模式：
// 一、每次 mmap 或 munmap 都保存一条日志（可以是文件），之后出报表分析曲线，用于观察分配趋势
// 二、同上，不过阈值调高，只打印到终端，用于观察大内存分配堆栈
// 三、在内存里统一维护，mmap 时增，munmap 时减，在某个时间点保存到文件，用于观察当前进程内存分配状态

void writeLine(char *line, size_t size) {
    static int writeCount = 0;
    static const int FLUSH_THRESHOLD = 10;
    fwrite(line, sizeof(char), size / sizeof(char), dumpFile);
    if (++writeCount > FLUSH_THRESHOLD) {
        fflush(dumpFile);
        writeCount = 0;
    }
}

/**
 *  MODE_LOG，log 模式：每次 mmap 或 munmap 都保存一条日志（可以是文件），之后出报表分析曲线，用于观察分配趋势
 * @param msg
 */
void handleForModeLog(message_t *msg) {
    char *content;
    size_t size = 0;
    //TODO 可能会有相同 address 多次 mmap 或 munmmap 的情况，考虑通过 map 去重
    if (msg->what == ADD_MMAP_INFO) {
        auto *info = reinterpret_cast<mmap_info *>(msg->data);
        // 样例：mmap=0xee6891=104800=prot=flag=java/lang/String.get|com/zhihu/A.mmm|xxx
        size = asprintf(&content, "%s%s%x%s%d%s%d%s%d%s%s\n", INFO_MMAP, INFO_DIV,
                        info->address, INFO_DIV, info->length, INFO_DIV, info->prot, INFO_DIV,
                        info->flag, INFO_DIV, info->stack.c_str());
    } else if (msg->what == DELETE_MMAP_INFO) {
        auto *info = reinterpret_cast<munmap_info *>(msg->data);
        // 样例：munmmap=0xee6891=104800
        size = asprintf(&content, "%s%s%x%s%d\n", INFO_MUNMMAP, INFO_DIV,
                        info->address, INFO_DIV, info->length);
    }
    if (size > 0) {
        writeLine(content, size);
        free(content);
    }
}

/**
 * MODE_DUMP dump 模式：在内存里统一维护，mmap 时增，munmap 时减，在某个时间点保存到文件，用于观察当前进程内存分配状态
 * @param msg
 */
void handleForModeDump(message_t *msg) {
    static map<uintptr_t, mmap_info> mmap_infos;
    if (msg->what == ADD_MMAP_INFO) { // 添加一条 mmap 信息
        auto *info = reinterpret_cast<mmap_info *>(msg->data);
        pair<map<uintptr_t, mmap_info>::iterator, bool> result;
        result = mmap_infos.insert(pair<uintptr_t, mmap_info>(info->address, *info));
        if (result.second) {
            totalSize += info->length;
        } else {
            LOGW("mmap info insert failed. addr:%u", info->address);
        }
    } else if (msg->what == DELETE_MMAP_INFO) { // 尝试删除一条 mmap 信息
        auto *info = reinterpret_cast<munmap_info *>(msg->data);
        if (mmap_infos.erase(info->address)) {
            totalSize -= info->length;
        } else {
            LOGI("mmap info delete failed. address:%u", info->address);
        }
    } else if (msg->what == DO_DUMP_FOR_MODE_DUMP) { // 将内存中的 mmap 信息保存到文件
        map<uintptr_t, mmap_info>::iterator iter;
        size_t total = 0;
        for (iter = mmap_infos.begin(); iter != mmap_infos.end(); iter++) {
            total += iter->second.length;
            char *content;
            size_t size = asprintf(&content, "--- addr:%x, length:%d, prot:%d, stack:\n%s\n",
                                   iter->second.address, iter->second.length, iter->second.prot,
                                   iter->second.stack.c_str());
            writeLine(content, size);
            free(content);
        }
        char *end;
        size_t size = asprintf(&end, "=== total:%d, totalSize:%d", total, totalSize);
        writeLine(end, size);
        free(end);
        fflush(dumpFile);
    }
}


void handle(message_t *msg) {
    if (modeFlag == MODE_LOG) {
        handleForModeLog(msg);
    } else if (modeFlag == MODE_DUMP) {
        handleForModeDump(msg);
    }
}

void createFile(char *dumpDir) {
    if (dumpFile == nullptr) {
        struct timeval stamp{};
        gettimeofday(&stamp, nullptr);
        char *filePath;
        asprintf(&filePath, "%s/himem_%d_%ld.log", dumpDir, modeFlag, stamp.tv_sec);
        dumpFile = fopen(filePath, "ae");
        if (dumpFile == nullptr) {
            LOGE("文件打开错误：%s，错误原因：%s", filePath, strerror(errno));
        }
    }
}

void tracerStart(char *dumpDir) {
    looper = looperCreate(handle);
    if (looper == nullptr) {
        LOGI("looperCreate fail!!!!!!!!!");
        return;
    }
    switch (looperStart(looper)) {
        case LOOPER_START_SUCCESS:
            createFile(dumpDir);
            break;
        case LOOPER_START_THREAD_ERROR:
            LOGI("looperStart thread create fail.");
            looperDestroy(&looper);
            break;
        case LOOPER_START_REPEAT_ERROR:
            LOGI("looperStart looper is started");
            break;
        case LOOPER_IS_NULL:
            LOGI("looperStart looper is NULL");
            break;
        default:
            break;
    }
}

void postOnMmap(mmap_info *data) {
    looperPost(looper, ADD_MMAP_INFO, data, sizeof(mmap_info));
}

void postOnMunmap(munmap_info *data) {
    looperPost(looper, DELETE_MMAP_INFO, data, sizeof(munmap_info));
}

void dumpToFile() {
    looperPost(looper, DO_DUMP_FOR_MODE_DUMP, nullptr, 0);
}

void tracerDestroy() {
    if (dumpFile != nullptr) {
        fflush(dumpFile);
        if (fclose(dumpFile)) {
            LOGI("%s", "文件关闭成功");
        }
        dumpFile = nullptr;
    }
    looperDestroy(&looper);
}
