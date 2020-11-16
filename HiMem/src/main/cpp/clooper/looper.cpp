//
// Created by kewen on 2019-08-16.
//

#include "looper.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "../log.h"

#define LOG_TAG "adi_looper"

static message_t *fetch_message(message_queue_t *queue) {
    if (queue->size < 1) {
        return nullptr;
    }
    message_t *message = queue->head;
    queue->head = message->next;    //取出队列头的消息后，消息队列指向下一个
    (queue->size)--;
    return message;
}

static int delete_message(message_t *message) {
    if (message == nullptr) {
        LOGI("%s", "delete_message message is null.\n");
        return -1;
    }
    if (message->data != nullptr) {
        free(message->data);
        message->data = nullptr;
    }
    free(message);
    return 0;
}

static int clear_message_queue(message_queue_t *queue) {
    if (queue == nullptr) {
        return -1;
    }
    message_t *message;
    while ((queue->size) > 0) {
        message = queue->head;
        queue->head = message->next;    //取出队列头的消息后，消息队列指向下一个
        (queue->size)--;
        delete_message(message);
    }
    return 0;
}

/**
 * 主 looper 循环函数
 * @param arg
 * @return
 */
static void *message_loop(void *arg) {
    auto *looper = (message_looper_t *) arg;
    while (looper->is_looping) {
        pthread_mutex_lock(&(looper->queue_mutex));
        if ((looper->queue).size < 1) {
            pthread_cond_wait(&(looper->queue_cond), &(looper->queue_mutex));  //等待这个信号量
            pthread_mutex_unlock(&(looper->queue_mutex));  //下次进入判断队列是否有消息
            continue;
        }
        message_t *msg = fetch_message(&(looper->queue));
        pthread_mutex_unlock(&(looper->queue_mutex));

        if (msg == nullptr) {
            continue;
        }

        // 没有对共享变量如 is_looping queue 做改动，无需加锁
        looper->handler(msg);  //处理消息
        delete_message(msg);    //删除消息

    }
    LOGI("looper exit~");
    clear_message_queue(&(looper->queue));  //最后清空链表
    return nullptr;
}

///////////// 以下是对外接口 /////////////////

/**
 * 新建一个 Looper，并初始化数据
 * @param func
 * @return
 */
message_looper_t *looperCreate(CALLBACK_FUNC func) {
    if (func == nullptr) {
        LOGI("func is null.\n");
        return nullptr;
    }
    auto *looper = static_cast<message_looper_t *>(malloc(sizeof(message_looper_t)));
    if (looper == nullptr) {
        LOGI("looperCreate malloc fail.\n");
        return nullptr;
    }
    looper->is_looping = 0;
    pthread_mutex_init(&(looper->queue_mutex), nullptr);
    pthread_cond_init(&(looper->queue_cond), nullptr);
    looper->queue.head = nullptr;
    looper->queue.size = 0;
    looper->handler = func;
    return looper;
}

/**
 * 新建线程，并启动 loop
 * @param looper
 * @return
 */
int looperStart(message_looper_t *looper) {
    if (looper == nullptr) {
        LOGI("looperStart looper is null.");
        return LOOPER_IS_NULL;
    }

    if (looper->is_looping) {
        LOGI("[message_loop] looperStart message_loop had start.");
        return LOOPER_START_REPEAT_ERROR;
    }
    looper->is_looping = 1;   //标志创建了 looper线程
    if (pthread_create(&(looper->looper_thread), nullptr, message_loop, looper)) {
        LOGI("pthread_create message_loop fail.");
        looper->is_looping = 0;  //线程创建失败，loop标志为false
        return LOOPER_START_THREAD_ERROR;
    }
    return 0;
}

/**
 * 退出 loop，并退出线程
 * @param looper
 * @return
 */
int looperStop(message_looper_t *looper) {
    if (looper == nullptr) {
        LOGI("looperStop loop is null.");
        return LOOPER_IS_NULL;
    }
    looper->is_looping = 0; //标志线程结束
    looperPost(looper, MESSAGE_EXIT_LOOP, nullptr, 0);  //线程可能还在等待消息，push 一个消息，让它结束运行
    pthread_join(looper->looper_thread, nullptr);   //等待线程运行结束

    return 0;
}

/**
 * 退出 loop，并清理数据
 * @param looper
 * @return
 */
int looperDestroy(message_looper_t **looper) {
    if (*looper == nullptr) {
        LOGI("destroy_loop looper is null.");
        return LOOPER_IS_NULL;
    }
    if ((*looper)->is_looping) {
        LOGI("destroy_loop stop");
        looperStop(*looper); //结束线程
    }

    pthread_cond_destroy(&((*looper)->queue_cond));
    pthread_mutex_destroy(&((*looper)->queue_mutex));
    free(*looper);
    *looper = nullptr;

    return 0;
}

/**
 * 向 looper 中发送消息
 * @param looper
 * @param what
 * @param data
 * @param size
 * @return
 */
int looperPost(message_looper_t *looper, int what, const void *data, size_t size) {
    if (looper == nullptr) {
        LOGI("looperPost looper is null.");
        return LOOPER_IS_NULL;
    }
    pthread_mutex_lock(&(looper->queue_mutex));
    if ((looper->queue).size > MAX_MESSAGE_SIZE) {
        //当前队列中的消息太多了，还没来得及处理
        LOGI("queue.size(%d) more than %d.", (looper->queue).size,
             MAX_MESSAGE_SIZE);
//        pthread_mutex_unlock(&(looper->queue_mutex));
//        return -1;
    }
    pthread_mutex_unlock(&(looper->queue_mutex));

    auto *message = (message_t *) malloc(sizeof(message_t));
    message->what = what;
    message->next = nullptr;

    if (data != nullptr && size > 0) {
        message->data = malloc(size);
        memcpy(message->data, data, size);
        message->data_size = size;
    } else {
        message->data = nullptr;
        message->data_size = 0;
    }

    pthread_mutex_lock(&(looper->queue_mutex));
    if ((looper->queue).head == nullptr) {
        //头部为空时，直接指向新的消息
        (looper->queue).head = message;
        ((looper->queue).size)++;
        pthread_cond_signal(&(looper->queue_cond));
        pthread_mutex_unlock(&(looper->queue_mutex));
        return 0;
    }
    message_t *tmp = (looper->queue).head;
    while (tmp->next != nullptr) {
        //新的消息发到链表尾部
        tmp = tmp->next;
    }
    tmp->next = message;
    ((looper->queue).size)++;
    pthread_cond_signal(&(looper->queue_cond));
    pthread_mutex_unlock(&(looper->queue_mutex));
    return 0;
}