#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# @Time    : 2020/11/24 下午7:24
# @Author  : zkw
# @File    : LinePlotMaker.py
from bokeh.plotting import figure, Figure

from plot.BaseMaker import BaseMaker
from plot.PlotInfo import PlotInfo
from utils.SizeUtils import convertSize


class FlatEvent:

    def __init__(self):
        self.typeList = []
        self.addressList = []
        self.lengthList = []
        self.protectList = []
        self.flagList = []
        self.stackList = []
        self.numberList = []
        self.fdList = []
        self.totalLengthList = []
        self.totalLengthStrList = []


class LinePlotMaker(BaseMaker):

    def make(self, info: PlotInfo, eventList: list) -> Figure:
        flat = self.flatEventList(eventList)
        return self.makeLinePlot(info, flat)

    def flatEventList(self, eventList: list) -> FlatEvent:
        flat = FlatEvent()
        totalLength = 0
        for index, event in enumerate(eventList):
            flat.typeList.append(event.type)
            flat.addressList.append(hex(event.address))
            flat.numberList.append(index)
            flat.lengthList.append(convertSize(event.length))
            if event.type == "mmap":
                totalLength += event.length
            elif event.type == "munmap":
                totalLength -= event.length
            flat.totalLengthList.append(totalLength)
            flat.totalLengthStrList.append(convertSize(totalLength))
            flat.protectList.append(event.protectStr)
            flat.flagList.append(event.flagStr)
            if event.fd == -1:
                flat.fdList.append("-1")
            else:
                flat.fdList.append(event.fdLink)
            flat.stackList.append(event.stack)
        return flat

    def makeLinePlot(self, info: PlotInfo, flat: FlatEvent) -> Figure:
        hoverToolHtml = """
                   <div>
                       <div>
                           <span style="font-size: 5px; font-weight: bold;">@typeList</span>
                           <span style="font-size: 5px; font-weight: bold;">address:</span>
                           <span style="font-size: 6px;">@addressList</span>
                       </div>
                       <div>
                           <span style="font-size: 5px; font-weight: bold;">total length:</span>
                           <span style="font-size: 6px;">@totalLengthStrList</span>
                       </div>
                       <div>
                           <span style="font-size: 5px; font-weight: bold;">length:</span>
                           <span style="font-size: 6px;">@lengthList</span>
                       </div>
                       <div>
                           <span style="font-size: 5px; font-weight: bold;">prot:</span>
                           <span style="font-size: 6px;">@protectList</span>
                       </div>
                       <div>
                           <span style="font-size: 5px; font-weight: bold;">flag:</span>
                           <span style="font-size: 6px;">@flagList</span>
                       </div>
                       <div>
                           <span style="font-size: 5px; font-weight: bold;">fd:</span>
                           <span style="font-size: 6px;">@fdList</span>
                       </div>
                       <div>
                           <span style="font-size: 5px; font-weight: bold;">stack:</span>
                           <span style="font-size: 6px; white-space: pre-wrap;">\n@stackList</span>
                       </div>
                   </div>
                   """
        title = "内存 mmap 监控趋势图，总次数：%d 次，日志文件：%s" % (info.count, info.fileName)

        graph = figure(plot_width=2100, plot_height=1000, title=title,
                       x_axis_label="日志索引", y_axis_label="内存总 mmap 量（字节）", tooltips=hoverToolHtml)

        data = dict(x=flat.numberList, y=flat.totalLengthList, typeList=flat.typeList, addressList=flat.addressList,
                    protectList=flat.protectList, flagList=flat.flagList, stackList=flat.stackList,
                    lengthList=flat.lengthList, fdList=flat.fdList, totalLengthStrList=flat.totalLengthStrList)
        graph.line(source=data, x="x", y="y", line_width=2)
        # graph.circle(source=data, x="x", y="y", fill_color="white", size=5)

        return graph
