#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# @Time    : 2020/11/24 下午2:47
# @Author  : zkw
# @File    : MMAPHandler.py
from typing import Optional

from data.Event import Event
from handler.BaseHandler import BaseHandler
from handler.BaseHandler import SEP_OR
from utils.StackUtils import stackAutoWrap


class CMask:
    def __init__(self, value: int, name: str):
        self.value = value
        self.name = name


PROT_LIST = [CMask(0x1, "PROT_READ"), CMask(0x2, "PROT_WRITE"), CMask(0x4, "PROT_EXEC"), CMask(0x8, "PROT_SEM"),
             CMask(0x0, "PROT_NONE"), CMask(0x01000000, "PROT_GROWSDOWN"), CMask(0x02000000, "PROT_GROWSUP")]

FLAG_LIST = [CMask(0x01, "MAP_SHARED"), CMask(0x02, "MAP_PRIVATE"), CMask(0x03, "MAP_SHARED_VALIDATE"),
             CMask(0x20, "MAP_ANONYMOUS"), CMask(0x10, "MAP_FIXED"), CMask(0x100000, "MAP_FIXED_NOREPLACE"),
             CMask(0x0100, "MAP_GROWSDOWN"), CMask(0x008000, "MAP_POPULATE"), CMask(0x010000, "MAP_NONBLOCK"),
             CMask(0x020000, "MAP_STACK"), CMask(0x080000, "MAP_SYNC"), CMask(0x4000000, "MAP_UNINITIALIZED"),
             CMask(0x2000, "MAP_LOCKED"), CMask(0x4000, "MAP_NORESERVE"), CMask(0x040000, "MAP_HUGETLB"),
             CMask(16 << 26, "MAP_HUGE_64KB"), CMask(19 << 26, "MAP_HUGE_512KB"), CMask(20 << 26, "MAP_HUGE_1MB"),
             CMask(21 << 26, "MAP_HUGE_2MB"), CMask(23 << 26, "MAP_HUGE_8MB"), CMask(24 << 26, "MAP_HUGE_16MB"),
             CMask(25 << 26, "MAP_HUGE_32MB"), CMask(28 << 26, "MAP_HUGE_256MB"), CMask(29 << 26, "MAP_HUGE_512MB"),
             CMask(30 << 26, "MAP_HUGE_1GB"), CMask(31 << 26, "MAP_HUGE_2GB"), CMask(34 << 26, "MAP_HUGE_16GB"),
             CMask(0x0800, "MAP_DENYWRITE"), CMask(0x1000, "MAP_EXECUTABLE")]


class MMAPHandler(BaseHandler):

    def __init__(self):
        super().__init__()

    def handle(self, segmentList: list) -> Optional[Event]:
        """
        mmap[]3189379072[]2483604[]1[]1[]236[]/system/framework/framework-res.apk[]android/content/res/ApkAssets.nativeLoad|android/content/res/ApkAssets.<init>|android/content/res/ApkAssets.loadFromPath|android/app/ResourcesManager.loadApkAssets|android/app/ResourcesManager.createAssetManager|android/app/ResourcesManager.createResourcesImpl|android/app/ResourcesManager.getOrCreateResources|android/app/ResourcesManager.getResources|android/app/ActivityThread.getTopLevelResources|android/app/ApplicationPackageManager.getResourcesForApplication|android/app/ApplicationPackageManager.getText|android/content/pm/PackageItemInfo.loadUnsafeLabel|android/content/pm/PackageItemInfo.loadLabel|com/zhihu/android/app/accounts/AppListUitls.getAllPkgs|com/zhihu/android/app/accounts/AppListUitls.lambda$uploadAppList$0|com/zhihu/android/app/accounts/-$$Lambda$AppListUitls$cwfNpYVcUGnNGRR6ypGII5eSxHk.subscribe|io/reactivex/internal/operators/observable/ObservableCreate.subscribeActual|io/reactivex/Observable.subscribe|io/reactivex/internal/operators/observable/ObservableFlatMap.subscribeActual|io/reactivex/Observable.subscribe|io/reactivex/internal/operators/observable/ObservableSubscribeOn$SubscribeTask.run|io/reactivex/Scheduler$DisposeTask.run|io/reactivex/internal/schedulers/ScheduledRunnable.run|io/reactivex/internal/schedulers/ScheduledRunnable.call|java/util/concurrent/FutureTask.run|java/util/concurrent/ScheduledThreadPoolExecutor$ScheduledFutureTask.run|java/util/concurrent/ThreadPoolExecutor.runWorker|java/util/concurrent/ThreadPoolExecutor$Worker.run|java/lang/Thread.run|
        函数名_映射的虚拟地址_大小_prot_flag_fd_fd对应的文件路径_堆栈
        :param segmentList:
        :return:
        """
        if segmentList[0] != "mmap" or len(segmentList) <= 6:
            return None
        event = Event()
        event.type = segmentList[0]
        event.address = int(segmentList[1])
        event.length = int(segmentList[2])
        event.protect = int(segmentList[3])
        event.protectStr = self.convertMaskToStr(event.protect, PROT_LIST)
        event.flag = int(segmentList[4])
        event.flagStr = self.convertMaskToStr(event.flag, FLAG_LIST)
        event.fd = int(segmentList[5])
        event.fdLink = str(segmentList[6])
        if len(segmentList) == 8:
            event.stack = stackAutoWrap(segmentList[7])
            # event.stack = str(segmentList[7]).replace(SEP_OR, "\n")
        return event
        # 去重逻辑暂时关闭，端上sdk 已经可以较好的去重了，剩下的重复基本上都是 Thread.nativeCreate 的堆栈
        # if event.address in uniqueAddress:
        #     Log.i("mmap, 重复 address event:" + str(event))
        #     return None
        # else:
        #     uniqueAddress.add(event.address)
        #     return event

    def convertMaskToStr(self, currentMask: int, maskList: list) -> str:
        result = ""
        if currentMask == 0x0:
            # 如果 currentMask 为0,则查找为0的名字，如果没有直接返回
            for mask in maskList:
                if mask.value == 0x0:
                    return mask.name
            return "%x" % currentMask

        for mask in maskList:
            # currentMask 不为 0 了，所以跳过 mask == 0 的情况
            if mask.value == 0x0:
                continue
            if currentMask & mask.value == mask.value:
                result += mask.name + "|"
        if result == "":
            return "%x" % currentMask
        else:
            return result[:-1]
