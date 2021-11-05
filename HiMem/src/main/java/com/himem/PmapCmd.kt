package com.himem

import java.io.File
import java.io.FileReader

/**
 * 移植自：https://cs.android.com/android/platform/superproject/+/master:external/toybox/toys/other/pmap.c
 *
 * @author zhoukewen
 * @since 2021/10/15 下午4:03
 */
object PmapCmd {

    private val anySpace = Regex("\\s+")

    fun pmap(pid: Int): String {
        var titleArray: Array<String> = emptyArray()
        var detailCount = 0
        var pss = 0
        var dirty = 0
        var swap = 0
        val result = StringBuilder()
        FileReader(File("/proc/$pid/smaps")).useLines {
            it.forEach { line ->
                if (Character.isUpperCase(line[0])) {
                    when {
                        line.startsWith("Pss:") -> {
                            pss = line.split(anySpace)[1].toInt()
                            detailCount++
                        }
                        line.startsWith("Private_Dirty:") -> {
                            dirty = line.split(anySpace)[1].toInt()
                            detailCount++
                        }
                        line.startsWith("Swap:") -> {
                            swap = line.split(anySpace)[1].toInt()
                            detailCount++
                        }
                    }
                    if (detailCount == 3) {
                        // start_address,lengthKB,pssKB,dirtyKB,swapKB,mode,name
                        val content =
                            "${titleArray[0]},${titleArray[1]},$pss,$dirty,$swap,${titleArray[2]},${titleArray[3]}"
                        result.append(content)
                        detailCount = 0
                    }
                } else {
                    titleArray = parseTitle(line)
                }
            }
        }
        return result.toString()
    }

    private val manySpace = Regex("\\s{3,}")

    /**
     * 6fcea000-6ff80000 rw-p 00000000 00:00 0                                  [anon:dalvik-/apex/com.android.art/javalib/boot.art]
     *
     * @return [start, lengthKB, mode, name]
     */
    private fun parseTitle(line: String): Array<String> {
        val result = arrayOf("", "", "", "")
        val str = line.split(manySpace)
        //first = 6fcea000-6ff80000 rw-p 00000000 00:00 0
        val first = str[0].split(' ')
        //name = [anon:dalvik-/apex/com.android.art/javalib/boot.art]
        val name = if (str.size == 2) {
            str[1].trim() + "\n"
        } else {
            "[anon]\n"
        }
        result[3] = name
        //address = 6fcea000-6ff80000
        val address = first[0]
        val startEnd = address.split('-')
        result[0] = startEnd[0]
        result[1] =
            ((startEnd[1].toLong(16) - startEnd[0].toLong(16)) / 1024).toString()
        val mode = first[1]
        result[2] = mode

        return result
    }

}