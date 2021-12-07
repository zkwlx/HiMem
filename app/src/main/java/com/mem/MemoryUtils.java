package com.mem;

import android.os.Process;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStreamReader;

public class MemoryUtils {

    /**
     * 获取当前进程文件描述符数量
     */
    public static int getFDCount() {
        int pid = Process.myPid();
        int count = -1;
        try {
            String cmd = "ls /proc/" + pid + "/fd";
            java.lang.Process p = Runtime.getRuntime().exec(cmd);
            BufferedReader in = new BufferedReader(new InputStreamReader(p.getInputStream()));
            while (in.readLine() != null) {
                count++;
            }
            in.close();
        } catch (IOException ignore) {
        }
        return count;
    }

    /**
     * 获取当前进程的 VmSize，单位 MB
     *
     * @return
     */
    public static long getVmSizeMB() {
        int pid = Process.myPid();
        try (BufferedReader pidReader = new BufferedReader(new FileReader("/proc/" + pid + "/stat"), 1024)) {
            String line = pidReader.readLine();
            if (line == null) {
                return -1;
            }
            String[] results = line.split(" ");
            if (results.length > 22) {
                return Long.parseLong(results[22]) / 1024 / 1024;
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        return -1;
    }

    /**
     * 获取 largeHeap=true 时的最大堆限制
     *
     * @return
     */
    public static String getLargeHeapSize() {
        StringBuilder result = new StringBuilder();
        try {
            java.lang.Process p = Runtime.getRuntime().exec("getprop dalvik.vm.heapsize");
            BufferedReader in = new BufferedReader(new InputStreamReader(p.getInputStream()));
            String line;
            while ((line = in.readLine()) != null) {
                result.append(line).append("\n");
            }
            in.close();
        } catch (IOException ignore) {
        }
        return result.toString();
    }

}

