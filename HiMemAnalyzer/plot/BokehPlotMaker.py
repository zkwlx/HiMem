#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# @Time    : 2020/11/24 下午3:18
# @Author  : zkw
# @File    : BokehPlotMaker.py

from bokeh.layouts import column
from bokeh.plotting import show, output_file

from plot.BarPlotMaker import BarPlotMaker
from plot.LinePlotMaker import LinePlotMaker
from plot.PlotInfo import PlotInfo


class BokehPlotMaker:

    def __init__(self, ):
        super().__init__()
        self.makerList = [LinePlotMaker()]
        # self.makerList = [BarPlotMaker(False)]
        # self.makerList = [LinePlotMaker(), BarPlotMaker(True), BarPlotMaker(False)]

    def make(self, info: PlotInfo, eventList: list):
        output_file(info.filePath + ".html")
        plotList = []
        for maker in self.makerList:
            plotList.append(maker.make(info, eventList))
        # 展示所有 Plot
        show(column(plotList))
