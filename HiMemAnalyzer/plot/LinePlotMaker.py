#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# @Time    : 2020/11/24 下午7:24
# @Author  : zkw
# @File    : LinePlotMaker.py
from bokeh.plotting import figure, Figure

from data.Event import Event
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
    """
    线图，反映 App 在一段时间内的 mmap/munmap 整体趋势
    """

    def make(self, info: PlotInfo, eventList: list) -> Figure:
        flat = self.flatEventList(eventList)
        return self.makeLinePlot(info, flat)

    def flatEventList(self, eventList: list) -> FlatEvent:
        flat = FlatEvent()
        totalLength = 0
        allocList = []
        for index, event in enumerate(eventList):
            flat.typeList.append(event.type)
            flat.addressList.append(hex(event.address))
            flat.numberList.append(index)
            if event.type == "mmap":
                totalLength += event.length
            elif event.type == "munmap":
                totalLength -= event.length
            elif event.type == "alloc":
                allocList.append(event)
                totalLength += event.length
            elif event.type == "free":
                allocEvent = self.popRecentlyAlloc(event, allocList)
                if allocEvent is not None:
                    totalLength -= allocEvent.length
                    event.length = allocEvent.length
            flat.totalLengthList.append(totalLength)
            flat.totalLengthStrList.append(convertSize(totalLength))
            flat.lengthList.append(convertSize(event.length))
            flat.protectList.append(event.protectStr)
            flat.flagList.append(event.flagStr)
            if event.fd == -1:
                flat.fdList.append("-1")
            else:
                flat.fdList.append(event.fdLink)
            flat.stackList.append(event.stack)
        return flat

    def popRecentlyAlloc(self, freeEvent: Event, allocList: list) -> Event:
        """
        从 allocList 中找到并移除最近的 freeEvent.address 对应的 alloc 事件
        :param freeEvent:
        :param allocList:
        :return:
        """
        deletedEvent = None
        for event in allocList[::-1]:
            if event.address == freeEvent.address:
                deletedEvent = event
                break
        if deletedEvent is not None:
            allocList.remove(deletedEvent)
        else:
            print("[popRecentlyAlloc] 未找到相同地址的 free 事件，address:%x" % freeEvent.address)
        return deletedEvent

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
                           <span style="width: 800; font-size: 6px; white-space: pre-wrap;">\n@stackList</span>
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
