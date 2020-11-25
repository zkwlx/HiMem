#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# @Time    : 2020/11/25 下午7:04
# @Author  : zkw
# @File    : Log.py
from himem import DEBUG_LOG


def i(content: str):
    if DEBUG_LOG:
        print(content)
