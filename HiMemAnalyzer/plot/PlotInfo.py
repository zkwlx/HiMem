#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# @Time    : 2020/11/24 下午4:21
# @Author  : zkw
# @File    : PlotInfo.py

class PlotInfo:
    """
    一份日志的全局信息，和日志文件是一对一关系
    """

    def __init__(self):
        # 日志文件名
        self.fileName = ""
        # 日志路径
        self.filePath = ""
        # 日志总条数
        self.count = 0
