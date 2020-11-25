#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# @Time    : 2020/11/24 下午2:39
# @Author  : zkw
# @File    : Event.py

class Event:
    """
    对应日志中的一行事件，保存事件所有信息
    """

    def __init__(self):
        self.type = ""
        self.address = 0
        self.length = 0
        self.protect = 0
        self.flag = 0
        self.stack = ""

    def __eq__(self, other):
        return self.address == other.address \
               and self.length == other.length \
               and self.protect == other.protect \
               and self.flag == other.flag \
               and self.type == other.type \
               and self.stack == other.stack

    def __str__(self):
        return "%x, %d, %d, %d, %s" % (self.address, self.length, self.protect, self.flag, self.stack)
