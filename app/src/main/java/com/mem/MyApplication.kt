package com.mem

import android.app.Application
import com.mem.hook.MemNative

class MyApplication : Application() {

    override fun onCreate() {
        super.onCreate()
        MemTest.initNative()

        MemNative.init()
    }
}