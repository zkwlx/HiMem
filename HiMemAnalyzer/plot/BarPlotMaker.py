#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# @Time    : 2020/11/25 上午11:11
# @Author  : zkw
# @File    : BarPlotMaker.py
import functools

from bokeh.plotting import Figure, figure

from data.Event import Event
from plot.BaseMaker import BaseMaker
from plot.PlotInfo import PlotInfo
from utils.SizeUtils import convertSize

"""
主要是针对 Thread.nativeCreate 和 CursorWindow.nativeCreate 分类是否展开
如果不展开，且堆栈前缀是上面这两种直接归为一类
"""
PREFIX_CURSOR = "android/database/CursorWindow.nativeCreate"
PREFIX_THREAD = "java/lang/Thread.nativeCreate"


# TODO thread 472, cursor 215
# TODO thread 1765801984, cursor 506634240
# threads=0
# cursors=0
# for event in newEventList:
#     if str(event.stack).startswith(PREFIX_CURSOR):
#         threads += event.length
#     elif str(event.stack).startswith(PREFIX_THREAD):
#         cursors += event.length

class AggEvent:
    def __init__(self, expand: bool):
        # (length, protect, flag)
        self.mask = ()
        self.stack = ""
        self.totalLength = 0
        self.count = 0
        self.addressList = []
        self.expand = expand

    def __eq__(self, other):
        if self.stack == "" and other.stack == "":
            # 如果堆栈为空，则比较掩码，掩码由 (length, protect, flag) 组成
            return self.mask == other.mask
        elif self.expand:
            # 展开模式，需要完全匹配栈
            return self.stack == other.stack
        else:
            # 非展开模式，几个常见的类别归为一类
            # 暂时只判断 stack 的 prefix
            return (self.stack.startswith(PREFIX_CURSOR) and other.stack.startswith(PREFIX_CURSOR)) or \
                   (self.stack.startswith(PREFIX_THREAD) and other.stack.startswith(PREFIX_THREAD))


class FlatEvent:
    def __init__(self):
        self.numberList = []
        self.totalLengthList = []
        self.totalLengthStr = []
        self.stackList = []
        self.maskList = []
        self.addressSetList = []
        self.proportionList = []
        self.countList = []


def cmpAgg(event1: AggEvent, event2: AggEvent) -> int:
    if event1.totalLength > event2.totalLength:
        return 1
    elif event1.totalLength < event2.totalLength:
        return -1
    else:
        return 0


class BarPlotMaker(BaseMaker):

    def __init__(self, expand: bool = True):
        """
        主要是针对 Thread.nativeCreate 和 CursorWindow.nativeCreate 分类是否展开
        如果不展开，且堆栈前缀是上面这两种直接归为一类
        :param expand: bool
        """
        self.isExpand = expand

    def make(self, info: PlotInfo, eventList: list) -> Figure:
        newEventList = self.adjustForMunmap(eventList)
        flat = self.flatEventList(newEventList)
        hoverToolHtml = """
                          <div>
                              <div>
                                  <span style="font-size: 5px; font-weight: bold;">proportion:</span>
                                  <span style="font-size: 5px;">@proportionList</span>
                              </div>
                              <div>
                                  <span style="font-size: 5px; font-weight: bold;">total length:</span>
                                  <span style="font-size: 5px;">@totalLengthStrList</span>
                              </div>
                              <div>
                                  <span style="font-size: 5px; font-weight: bold;">length\\prot\\flag:</span>
                                  <span style="font-size: 5px;">[@maskList]</span>
                              </div>
                              <div>
                                  <span style="font-size: 5px; font-weight: bold;">mmap count:</span>
                                  <span style="font-size: 5px;">@countList</span>
                              </div>
                              <div>
                                  <span style="font-size: 5px; font-weight: bold;">stack:</span>
                                  <span style="font-size: 5px; white-space: pre-wrap;">\n@stackList</span>
                              </div>
                          </div>
                          """
        title = "内存 mmap 监控占比图，总次数：%d 次，聚类后：%d 类，日志文件：%s" % (info.count, len(flat.numberList), info.fileName)
        p = figure(plot_width=1000, plot_height=1800, title=title, x_axis_label="虚拟内存大小（字节）", y_axis_label="排名",
                   tooltips=hoverToolHtml)
        data = dict(x=flat.totalLengthList, y=flat.numberList, totalLengthStrList=flat.totalLengthStr,
                    proportionList=flat.proportionList, maskList=flat.maskList, countList=flat.countList,
                    stackList=flat.stackList)
        p.hbar(source=data, y="y", height=0.5, left=0, right="x", color="navy")

        return p

    def flatEventList(self, newEventList: list) -> FlatEvent:
        aggEventList = []
        overallLength = 0
        # 遍历 eventList，创建聚类 aggEventList
        for event in newEventList:
            aggEvent = AggEvent(self.isExpand)
            aggEvent.mask = (event.length, event.protect, event.flag)
            aggEvent.stack = event.stack
            overallLength += event.length
            if aggEvent in aggEventList:
                index = aggEventList.index(aggEvent)
                aggItem = aggEventList[index]
                aggItem.count += 1
                aggItem.totalLength += event.length
                aggItem.addressList.append(event.address)
            else:
                aggEvent.count += 1
                aggEvent.totalLength += event.length
                aggEvent.addressList.append(event.address)
                aggEventList.append(aggEvent)

        flat = FlatEvent()
        aggEventList.sort(key=functools.cmp_to_key(cmpAgg))
        for index, aggItem in enumerate(aggEventList):
            flat.numberList.append(index)
            flat.maskList.append(aggItem.mask)
            flat.addressSetList.append(aggItem.addressList)
            if not self.isExpand and aggItem.stack != "":  # 非展开模式，只保留堆栈第一行
                i = aggItem.stack.index("\n")
                flat.stackList.append(aggItem.stack[0:i])
            else:
                flat.stackList.append(aggItem.stack)
            flat.totalLengthStr.append(convertSize(aggItem.totalLength))
            flat.totalLengthList.append(aggItem.totalLength)
            flat.countList.append(aggItem.count)
            proportion = aggItem.totalLength / overallLength
            flat.proportionList.append("%.2f%%" % (proportion * 100))

        return flat

    def adjustForMunmap(self, eventList: list) -> list:
        """
        调整原始 eventList，减去 munmap 对应的 mmap
        :param eventList:
        :return:
        """
        newEventList = []
        for event in eventList:
            if event.type == "mmap":
                newEventList.append(event)
            elif event.type == "munmap":
                self.removeForMunmaped(event, newEventList)
        return newEventList

    def removeForMunmaped(self, munmapEvent: Event, newEventList: list):
        """
        删除 newEventList 中被 munmap 掉的 event
        :param munmapEvent:
        :param newEventList:
        :return:
        """
        deletedEvent = None
        # 倒序遍历，找到并删除最近的同地址 mmap 事件
        for event in newEventList[::-1]:
            if munmapEvent.address == event.address:
                deletedEvent = event
                break
        if deletedEvent is not None:
            newEventList.remove(deletedEvent)
        else:
            print("未找到相同地址的 mmap 事件，address:%x" % munmapEvent.address)
