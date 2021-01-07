package com.himem

import android.os.Build

object HiMemNative {

    /**
     * 监控 mmap/munmap
     */
    const val MMAP_MODE = 1

    /**
     * 监控 malloc/calloc/relloc/free
     */
    const val ALLOC_MODE = 2

    init {
        System.loadLibrary("himem-native")
    }

    /**
     * 判断系统是否支持
     */
    fun isOSSupport(): Boolean {
        // 目前只支持 Android 9.0/8.1/8.0/7.1.2/7.1.1
        return Build.VERSION.SDK_INT == 28
                || Build.VERSION.SDK_INT == 27
                || Build.VERSION.SDK_INT == 26
                || Build.VERSION.SDK_INT == 25
    }

    /**
     * 是否打开 Debug 日志
     */
    fun setLogDebug(enable: Int) {
        setDebug(enable)
    }

    /**
     * 初始化 himem，包括创建 .himem 日志文件，初始化信号处理、xhook等
     *
     * @param dumpDir .himem 文件的父目录
     * @param mmapSizeThreshold mmap 阈值，超过阈值时触发监控逻辑
     * @param flushThreshold 日志回写磁盘的阈值，超过阈值 fflush
     * @param mode 监控模式，一次启动只支持一种模式，目前提供 [MMAP_MODE] 和 [ALLOC_MODE]，意味着监控不同的内存分配函数。默认使用 [MMAP_MODE]
     */
    @JvmOverloads
    fun initAndStart(
        dumpDir: String,
        mmapSizeThreshold: Long,
        flushThreshold: Long,
        mode: Int = MMAP_MODE
    ) {
        init(dumpDir, mmapSizeThreshold, flushThreshold, mode)
    }

    /**
     * 结束监控，包括取消 xhook、关闭日志文件等
     */
    fun destroy() {
        deInit()
    }

    /**
     * 手动将内存中的日志刷到磁盘
     */
    fun flushFile() {
        memFlush()
    }

    /**
     * 采用 dl_iterate_phdr() 回调的方式触发新 so 的 hook（有些 so 是运行时加载的）
     */
    fun refreshHook() {
        refreshHookForDl()
    }

    private external fun setDebug(enable: Int)

    private external fun init(
        dumpDir: String,
        mmapSizeThreshold: Long,
        flushThreshold: Long,
        mode: Int
    )

    private external fun deInit()

    private external fun memFlush()

    private external fun refreshHookForDl()

}