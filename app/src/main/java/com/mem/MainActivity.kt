package com.mem

import android.os.Bundle
import android.os.Process
import android.util.Log
import android.view.View
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import com.himem.HiMemNative
import java.io.File
import java.io.FileWriter
import kotlin.system.measureNanoTime

class MainActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        findViewById<TextView>(R.id.sample_text).text = "PID:${Process.myPid()}"

        val default = Thread.currentThread().uncaughtExceptionHandler
        Thread.currentThread().uncaughtExceptionHandler =
            Thread.UncaughtExceptionHandler { thread, exception ->
                Log.e("zkw", "FATAL Crash :$exception")
            }
        MemTest.initNative()
    }


    fun onMmapClick(view: View) {
        MemTest.mmapSmall()
        MemTest.stringFromJNI()
        File(externalCacheDir, "pmap.txt").writeText(ProcUtils.getPmap())
        File(externalCacheDir, "maps.txt").writeText(ProcUtils.getPidMaps())
        File(externalCacheDir, "smaps.txt").writeText(ProcUtils.getSmaps())
        val path = File(externalCacheDir, "pppmap.csv").absolutePath
        val pid = Process.myPid()
        val content = HiMemNative.pmap(pid)
        val output = FileWriter(path)
        output.use {
            it.write(content)
        }

//        repeat(7000) {
//            Log.i("zkw", "------> $it")
//            Thread(Thread.currentThread().threadGroup, {
//                MemTest.mmapSmall()
//                Thread.sleep(1000000000)
//            }, "xxx$it", 1 * 1024 * 1024).start()
//        }
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
        MemTest.asprintfTest()
    }

    var dumpCount = 0

    fun onDumpClick(view: View) {
//        dumpCount++
        HiMemNative.flushFile()
//        File(externalCacheDir, "maps_$dumpCount.txt").writeText(ProcUtils.getPidMaps())
//        MemTest.stringTest()
    }

    fun onStackTraceTest(view: View) {
        var s:String = ""
        val nanoDuration = measureNanoTime {
            repeat(20) {
                val stacks = Thread.currentThread().stackTrace
                for (ele in stacks) {
                     s = ele.toString()
                }
            }
        }
        Log.i("zkw", "java stack:  -> $nanoDuration ns")
    }

    fun onNativeStackTraceTest(view: View) {
        HiMemNative.flushFile()
    }
}
