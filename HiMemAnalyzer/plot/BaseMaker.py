#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# @Time    : 2020/11/24 下午7:23
# @Author  : zkw
# @File    : BaseMaker.py
from bokeh.plotting import Figure

from plot.PlotInfo import PlotInfo


class BaseMaker:

    def make(self, info: PlotInfo, eventList: list) -> Figure:
        pass
