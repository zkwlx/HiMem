#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# @Time    : 2020/11/24 下午2:46
# @Author  : zkw
# @File    : BaseHandler.py
from data.Event import Event

SEP_EQ = "[]"
SEP_OR = "|"


class BaseHandler:

    def handle(self, segmentList: list) -> Event:
        pass
