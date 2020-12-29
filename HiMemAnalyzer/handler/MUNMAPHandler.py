#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# @Time    : 2020/11/24 下午2:47
# @Author  : zkw
# @File    : MUNMAPHandler.py
from typing import Optional

from data.Event import Event
from handler.BaseHandler import BaseHandler


class MUNMAPHandler(BaseHandler):

    def __init__(self):
        super().__init__()
        self.uniqueEvents = []

    def handle(self, segmentList) -> Optional[Event]:
        if len(segmentList) < 3:
            return None
        if segmentList[0] != "munmap":
            return None
        event = Event()
        event.type = segmentList[0]
        event.address = int(segmentList[1])
        event.length = int(segmentList[2])

        # 去重逻辑暂时关闭，端上sdk 已经可以较好的去重了，剩下的重复基本上都是 Thread.nativeCreate 的堆栈
        # if event.address in uniqueAddress:
        #     uniqueAddress.remove(event.address)
        # else:
        #     # munmap 貌似有重复，但经过实验量很少，所以不忽略事件只打印日志，
        #     Log.i("munmap, but no mmap address:" + str(event.address))
        return event
