//
// Created by zkw on 20-11-16.
//

#include "mmap_tracer.h"
#include "clooper/looper.h"
#include <map>
#include <set>
#include <cerrno>
#include <atomic>

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
#define INFO_MUNMMAP "munmap"
#define INFO_DIV "="

using namespace std;

FILE *dumpFile = nullptr;
atomic_size_t totalSize;
static const int FLUSH_THRESHOLD = 8 * 1024;

int modeFlag = MODE_LOG;

// 几种处理模式：
// 一、每次 mmap 或 munmap 都保存一条日志（可以是文件），之后出报表分析曲线，用于观察分配趋势
// 二、同上，不过阈值调高，只打印到终端，用于观察大内存分配堆栈
// 三、在内存里统一维护，mmap 时增，munmap 时减，在某个时间点保存到文件，用于观察当前进程内存分配状态

void writeLine(char *line, size_t size) {
    static atomic_size_t wroteSize(0);
    int sizeChar = sizeof(char);
    size_t count = fwrite(line, sizeChar, size / sizeChar, dumpFile);
    wroteSize += count * sizeChar;
    if (wroteSize > FLUSH_THRESHOLD) {
        LOGI("----------wrote: %d", wroteSize.load());
        fflush(dumpFile);
        wroteSize = 0;
    }
}

/**
 * MODE_DUMP dump 模式：在内存里统一维护，mmap 时增，munmap 时减，在某个时间点保存到文件，用于观察当前进程内存分配状态
 * @deprecated 旧实现，不使用
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
        size_t size = asprintf(&end, "=== total:%d, totalSize:%d", total, totalSize.load());
        writeLine(end, size);
        free(end);
        fflush(dumpFile);
    }
}


/**
 *  MODE_LOG，log 模式：每次 mmap 或 munmap 都保存一条日志（可以是文件），之后出报表分析曲线，用于观察分配趋势
 * @param msg
 */
void mmapForModeLog(mmap_info *data) {
    char *content;
    //TODO 可能会有相同 address 多次 mmap 或 munmmap 的情况，考虑通过 map 去重
    // 样例：mmap=0xee6891=104800=prot=flag=java/lang/String.get|com/zhihu/A.mmm|xxx
    size_t size = asprintf(&content, "%s[]%u[]%d[]%d[]%d[]%d[]%s[]%s\n", INFO_MMAP,
                           data->address, data->length, data->prot, data->flag, data->fd,
                           data->fdLink.c_str(), data->stack.c_str());
    if (size > 0) {
        writeLine(content, size);
        free(content);
    }
}

/**
 *  MODE_LOG，log 模式：每次 mmap 或 munmap 都保存一条日志（可以是文件），之后出报表分析曲线，用于观察分配趋势
 * @param msg
 */
void munmapForModeLog(munmap_info *data) {
    char *content;
    //TODO 可能会有相同 address 多次 mmap 或 munmmap 的情况，考虑通过 map 去重
    // 样例：munmmap=0xee6891=104800
    size_t size = asprintf(&content, "%s[]%u[]%d\n", INFO_MUNMMAP,
                           data->address, data->length);
    if (size > 0) {
        writeLine(content, size);
        free(content);
    }
}

void createFile(char *dumpDir) {
    if (dumpFile == nullptr) {
        struct timeval stamp{};
        gettimeofday(&stamp, nullptr);
        char *filePath;
        asprintf(&filePath, "%s/trace_%d_%ld.himem", dumpDir, modeFlag, stamp.tv_sec);
        dumpFile = fopen(filePath, "ae");
        if (dumpFile == nullptr) {
            LOGE("文件打开错误：%s，错误原因：%s", filePath, strerror(errno));
        }
    }
}

void tracerStart(char *dumpDir) {
    createFile(dumpDir);
}

void postOnMmap(mmap_info *data) {
    mmapForModeLog(data);
}

void postOnMunmap(munmap_info *data) {
    munmapForModeLog(data);
}

void dumpToFile() {
    if (dumpFile != nullptr) {
        fflush(dumpFile);
    }
}

void tracerDestroy() {
    if (dumpFile != nullptr) {
        fflush(dumpFile);
        if (fclose(dumpFile)) {
            LOGI("%s", "文件关闭成功");
        }
        dumpFile = nullptr;
    }
}
