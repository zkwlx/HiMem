#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# @Time    : 2020/11/24 下午2:47
# @Author  : zkw
# @File    : MMAPHandler.py
from typing import Optional

from data.Event import Event
from handler.BaseHandler import BaseHandler
from handler.BaseHandler import SEP_OR


class AllocHandler(BaseHandler):

    def __init__(self):
        super().__init__()

    def handle(self, segmentList: list) -> Optional[Event]:
        """
        alloc[]3121610752[]5117979[]/system/lib/libjpeg.so(jinit_master_decompress)|/system/lib/libjpeg.so(jpeg_start_decompress)|/system/lib/libhwui.so(None)|/system/lib/libhwui.so(_ZN7SkCodec9getPixelsERK11SkImageInfoPvjPKNS_7OptionsE)|/system/lib/libhwui.so(None)|/system/lib/libhwui.so(_ZN14SkAndroidCodec16getAndroidPixelsERK11SkImageInfoPvjPKNS_14AndroidOptionsE)|/system/lib/libandroid_runtime.so(None)|/system/lib/libandroid_runtime.so(None)|
        alloc_映射的虚拟地址_大小_堆栈
        :param segmentList:
        :return:
        """
        if segmentList[0] != "alloc" or len(segmentList) < 4:
            return None
        event = Event()
        event.type = segmentList[0]
        event.address = int(segmentList[1])
        event.length = int(segmentList[2])
        event.stack = str(segmentList[3]).replace(SEP_OR, "\n")
        # 去重逻辑暂时关闭，端上sdk 已经可以较好的去重了，剩下的重复基本上都是 Thread.nativeCreate 的堆栈
        # if event.address in uniqueAddress:
        #     Log.i("mmap, 重复 address event:" + str(event))
        #     return None
        # else:
        #     uniqueAddress.add(event.address)
        #     return event
        return event
