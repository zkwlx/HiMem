package com.himem

import android.util.Log

object HiMemNative {

    external fun setDebug(enable: Int)

    external fun init(dumpDir: String, mmapSizeThreshold: Long)

    external fun deInit()

    external fun memDump()

    fun onMmap(addr: Long, length: Long, prot: Int, flags: Int, fd: Int, offset: Long) {
        Log.i(
            "zkw",
            "mmap > addr:$addr, \nstack:${stackTraceToString(Thread.currentThread().stackTrace)}"
        )
    }

    fun onMunmap(addr: Long, length: Long) {
        Log.i(
            "zkw",
            "munmap > addr:$addr"
        )
    }

    private fun stackTraceToString(elements: Array<StackTraceElement?>): String {
        val stackString = StringBuilder()
        if (elements.isEmpty()) {
            return stackString.toString()
        }
        for (e in elements) {
            if (e == null) {
                continue
            }
            stackString.append("\n").append(e.toString())
        }
        stackString.append("\n")
        return stackString.toString()
    }

    // Used to load the 'native-lib' library on application startup.
    init {
        System.loadLibrary("himem-native")
    }
}