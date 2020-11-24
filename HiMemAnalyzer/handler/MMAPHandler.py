#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# @Time    : 2020/11/24 下午2:47
# @Author  : zkw
# @File    : MMAPHandler.py
from typing import Optional

from data.Event import Event
from handler.BaseHandler import BaseHandler
from handler.BaseHandler import SEP_OR


class MMAPHandler(BaseHandler):

    def __init__(self):
        super().__init__()
        self.uniqueEvents = []

    def handle(self, segmentList: list) -> Optional[Event]:
        if segmentList[0] != "mmap":
            return None
        event = Event()
        event.type = segmentList[0]
        event.address = int(segmentList[1])
        event.length = int(segmentList[2])
        event.protect = int(segmentList[3])
        event.flag = int(segmentList[4])
        if len(segmentList) == 6:
            event.stack = str(segmentList[5]).replace(SEP_OR, "\n")

        if event in self.uniqueEvents:
            print("mmap, 重复 event:" + str(event))
            return None
        else:
            self.uniqueEvents.append(event)
            return event
