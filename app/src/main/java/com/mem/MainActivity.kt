package com.mem

import android.os.Bundle
import android.util.Log
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import com.himem.HiMemNative
import java.io.File

class MainActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val default = Thread.currentThread().uncaughtExceptionHandler
        Thread.currentThread().uncaughtExceptionHandler =
            Thread.UncaughtExceptionHandler { thread, exception ->
                Log.e("zkw", "FATAL Crash :$exception")
            }

    }


    fun onButtonClick(view: View) {
//        MemTest.stringFromJNI()
//        File(externalCacheDir, "pmapBefore.txt").writeText(ProcUtils.getPmap())
        repeat(500) {
            Log.i("zkw", "------> $it")
            Thread(Thread.currentThread().threadGroup, {
//                MemTest.mmapSmall()
                Thread.sleep(1000)
            }, "xxx$it", 5 * 1024 * 1024).start()
        }
//        val length = 512L * 1024 * 1024
//        val file = File(externalCacheDir, "xxxxx.txt")
//        file.createNewFile()
//        FileChannel.open(
//            Paths.get(file.toURI()),
//            StandardOpenOption.READ, StandardOpenOption.WRITE
//        ).use { channel ->
//            val mapBuffer: MappedByteBuffer = channel.map(FileChannel.MapMode.READ_WRITE, 0, length)
//            for (i in 0 until length) {
//                mapBuffer.put(0.toByte())
//            }
//        }
//        Thread.sleep(1000)
//        File(externalCacheDir, "pmapAfter.txt").writeText(ProcUtils.getPmap())
    }

    fun onGcClick(view: View) {
//        Runtime.getRuntime().gc()
        MemTest.munmapSmall()
    }

    var dumpCount = 0

    fun onDumpClick(view: View) {
        dumpCount++
        HiMemNative.memDump()
//        File(externalCacheDir, "maps_$dumpCount.txt").writeText(ProcUtils.getPidMaps())
    }


}