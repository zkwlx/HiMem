package com.himem

object HiMemNative {

    external fun setDebug(enable: Int)

    /**
     * 初始化 himem，包括创建 .himem 日志文件，初始化信号处理、xhook等
     *
     * @param dumpDir .himem 文件的父目录
     * @param mmapSizeThreshold mmap 阈值，超过阈值时触发监控逻辑
     */
    external fun init(dumpDir: String, mmapSizeThreshold: Long)

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

    init {
        System.loadLibrary("himem-native")
    }
}