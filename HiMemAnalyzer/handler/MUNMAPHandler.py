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
        if segmentList[0] != "munmap":
            return None
        event = Event()
        event.type = segmentList[0]
        event.address = int(segmentList[1])
        event.length = int(segmentList[2])

        if event in self.uniqueEvents:
            print("munmap, 重复 event:" + str(event))
            return None
        else:
            self.uniqueEvents.append(event)
            return event
