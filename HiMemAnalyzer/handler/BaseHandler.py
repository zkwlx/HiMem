#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# @Time    : 2020/11/24 下午2:46
# @Author  : zkw
# @File    : BaseHandler.py
from data.Event import Event

SEP_EQ = "[]"
SEP_OR = "|"

# uniqueAddress = set()
"""
去重逻辑暂时关闭，端上sdk 已经可以较好的去重了，剩下的重复基本上都是 Thread.nativeCreate 的堆栈
用于 mmap/munmap 事件去重的集合，去重逻辑与端上 sdk 类似：
mmap新增、munmap删减，在此基础上 mmap 忽略重复事件
"""


class BaseHandler:

    def handle(self, segmentList: list) -> Event:
        pass
