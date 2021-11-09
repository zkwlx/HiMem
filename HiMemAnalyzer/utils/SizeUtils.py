#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# @Time    : 2020/11/25 下午5:06
# @Author  : zkw
# @File    : SizeUtils.py

import math


def convertSize(size_bytes: int) -> str:
    size_bytes = abs(size_bytes)
    if size_bytes == 0:
        return "0 B"
    size_name = ("B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB")
    i = int(math.floor(math.log(size_bytes, 1024)))
    p = math.pow(1024, i)
    s = round(size_bytes / p, 2)
    return "%s %s" % (s, size_name[i])
