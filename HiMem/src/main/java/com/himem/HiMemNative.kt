package com.himem

import android.os.Build

object HiMemNative {

    init {
        System.loadLibrary("himem-native")
    }

    /**
     * 判断系统是否支持
     */
    fun isOSSupport(): Boolean {
        // 目前只支持 Android 9.0
        return Build.VERSION.SDK_INT == 28
    }

    external fun setDebug(enable: Int)

    /**
     * 初始化 himem，包括创建 .himem 日志文件，初始化信号处理、xhook等
     *
     * @param dumpDir .himem 文件的父目录
     * @param mmapSizeThreshold mmap 阈值，超过阈值时触发监控逻辑
     * @param flushThreshold 日志回写磁盘的阈值，超过阈值 fflush
     */
    external fun init(dumpDir: String, mmapSizeThreshold: Long, flushThreshold: Long)

    /**
     * 结束监控，包括取消 xhook、关闭日志文件等
     */
    external fun deInit()

    /**
     * 手动将内存中的日志刷到磁盘
     */
    external fun memFlush()

    /**
     * 采用 dl_iterate_phdr() 回调的方式触发新 so 的 hook（有些 so 是运行时加载的）
     */
    external fun refreshHookForDl()

}