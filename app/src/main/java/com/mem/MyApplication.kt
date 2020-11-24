package com.mem

import android.app.Application
import android.widget.Toast
import com.himem.HiMemNative
import java.io.File

class MyApplication : Application() {

    override fun onCreate() {
        super.onCreate()
        MemTest.initNative()

        val file: File? = externalCacheDir
        val root = File(file!!.absolutePath, "HiMem/")
        Toast.makeText(this, "HiMem 日志存放在 " + root.absolutePath + " 目录下", Toast.LENGTH_LONG).show()
        root.mkdirs()
        HiMemNative.init(root.absolutePath, 0)
    }
}