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


class AggEvent:
    def __init__(self, expand: bool):
        # (length, protect, flag)
        self.mask = ()
        self.protStr = ""
        self.flagStr = ""
        self.perLengthStr = ""
        self.stack = ""
        self.totalLength = 0
        self.count = 0
        self.addressSet = set()
        self.fdSet = set()
        self.expand = expand

    def __eq__(self, other):
        # 先判断堆栈是否一致，再判断掩码是否完全一致，掩码由（length, protect, flag）组成
        return self.isSameStack(other) and self.mask == other.mask

    def isSameStack(self, other) -> bool:
        if self.stack == "" and other.stack == "":
            return True
        elif self.expand:
            # 展开模式，需要完全匹配 stack
            return self.stack == other.stack
        else:
            # 非展开模式，只匹配 stack 的前缀，目前只判断 Cursor 和 Thread 两大类
            return (self.stack.startswith(PREFIX_CURSOR) and other.stack.startswith(PREFIX_CURSOR)) or \
                   (self.stack.startswith(PREFIX_THREAD) and other.stack.startswith(PREFIX_THREAD))


class FlatEvent:
    """
    平铺开的事件类，保存每个需要展示的字段的横向平铺列表
    """

    def __init__(self):
        # 编号
        self.numberList = []
        # 当前归类的总大小
        self.totalLengthList = []
        # 当前归类的总大小 可读格式
        self.totalLengthStr = []
        # 当前归类的堆栈
        self.stackList = []
        # 当前归类的掩码（length, protect, flag）
        self.maskList = []
        # 当前归类所包含的所有地址集合
        self.addressSetList = []
        # 当前归类总大小占总体大小的百分比
        self.proportionList = []
        # 当前归类的 mmap 条数
        self.countList = []
        # 当前归类的每条 mmap 的大小 可读格式
        self.perLengthStrList = []
        # 当前归类的每条 mmap 时 prot 参数 可读格式
        self.protStrList = []
        # 当前归类的每条 mmap 时 flag 参数 可读格式
        self.flagStrList = []
        # 当前归类的 fd 汇总
        self.fdStrList = []


def cmpAgg(event1: AggEvent, event2: AggEvent) -> int:
    if event1.totalLength > event2.totalLength:
        return 1
    elif event1.totalLength < event2.totalLength:
        return -1
    else:
        return 0


class BarPlotMaker(BaseMaker):
    """

    """

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
                                  <span style="font-size: 5px; font-weight: bold;">per length:</span>
                                  <span style="font-size: 5px;">@perLengthStrList</span>
                              </div>
                              <div>
                                  <span style="font-size: 5px; font-weight: bold;">protect:</span>
                                  <span style="font-size: 5px;">@protStrList</span>
                              </div>
                              <div>
                                  <span style="font-size: 5px; font-weight: bold;">flag:</span>
                                  <span style="font-size: 5px;">@flagStrList</span>
                              </div>
                              <div>
                                  <span style="font-size: 5px; font-weight: bold;">mmap count:</span>
                                  <span style="font-size: 5px;">@countList</span>
                              </div>
                              <div>
                                  <span style="font-size: 5px; font-weight: bold;">fd list:</span>
                                  <span style="font-size: 5px; white-space: pre-wrap;">\n@fdStrList</span>
                              </div>
                              <div>
                                  <span style="font-size: 5px; font-weight: bold;">stack:</span>
                                  <span style="font-size: 5px; white-space: pre-wrap;">\n@stackList</span>
                              </div>
                          </div>
                          """
        title = "内存 mmap 监控占比图，总次数：%d 次，聚类后：%d 类，日志文件：%s" % (info.count, len(flat.numberList), info.fileName)
        plot_height = 1000
        if self.isExpand:
            plot_height *= 2
        p = figure(plot_width=1000, plot_height=int(plot_height), title=title, x_axis_label="虚拟内存大小（字节）",
                   y_axis_label="排名", tooltips=hoverToolHtml)
        data = dict(x=flat.totalLengthList, y=flat.numberList, totalLengthStrList=flat.totalLengthStr,
                    proportionList=flat.proportionList, maskList=flat.maskList, countList=flat.countList,
                    stackList=flat.stackList, protStrList=flat.protStrList, flagStrList=flat.flagStrList,
                    perLengthStrList=flat.perLengthStrList, fdStrList=flat.fdStrList)
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
                self.aggItemGrowth(aggItem, event)
            else:
                self.aggItemGrowth(aggEvent, event)
                aggEvent.protStr = event.protectStr
                aggEvent.flagStr = event.flagStr
                aggEvent.perLengthStr = convertSize(event.length)
                aggEventList.append(aggEvent)

        # 将聚合到的列表展开到不同维度的 list
        flat = FlatEvent()
        aggEventList.sort(key=functools.cmp_to_key(cmpAgg))
        for index, aggItem in enumerate(aggEventList):
            flat.numberList.append(index)
            flat.maskList.append(aggItem.mask)
            flat.addressSetList.append(aggItem.addressSet)
            if not self.isExpand and aggItem.stack != "":  # 非展开模式，只保留堆栈第一行
                try:
                    i = aggItem.stack.index("\n")
                    flat.stackList.append(aggItem.stack[0:i])
                except ValueError:
                    print(aggItem.stack)
            else:
                flat.stackList.append(aggItem.stack)
            flat.totalLengthStr.append(convertSize(aggItem.totalLength))
            flat.totalLengthList.append(aggItem.totalLength)
            flat.countList.append(aggItem.count)
            flat.protStrList.append(aggItem.protStr)
            flat.flagStrList.append(aggItem.flagStr)
            flat.fdStrList.append("\n".join(aggItem.fdSet))
            flat.perLengthStrList.append(aggItem.perLengthStr)
            proportion = aggItem.totalLength / overallLength
            flat.proportionList.append("%.2f%%" % (proportion * 100))

        return flat

    def aggItemGrowth(self, aggItem: AggEvent, event: Event):
        aggItem.count += 1
        aggItem.totalLength += event.length
        aggItem.addressSet.add(event.address)
        aggItem.fdSet.add(event.fdLink)

    def adjustForMunmap(self, eventList: list) -> list:
        """
        调整原始 eventList，减去 munmap 对应的 mmap，剩下的都是未 munmap 的所有 mmap 事件
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
        删除 newEventList 中被 munmap 掉的 event，只要 address 一致就删除
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
            print("[removeForMunmaped] 未找到相同地址的 mmap 事件，address:%x" % munmapEvent.address)
