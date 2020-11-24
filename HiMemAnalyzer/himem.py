#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# @Time    : 2020/11/24 下午4:21
# @Author  : zkw
# @File    : himem.py


import os
import sys

from data.Event import Event
from handler.BaseHandler import SEP_EQ
from handler.MMAPHandler import MMAPHandler
from handler.MUNMAPHandler import MUNMAPHandler
from plot.BokehPlotMaker import BokehPlotMaker
from plot.PlotInfo import PlotInfo

LOG_PATH = ""
LOG_NAME = ""

handlerList = [MMAPHandler(), MUNMAPHandler()]


def handleTraceFile() -> list:
    """
    从原始 trace 文件中解析 Event 对象列表
    :return:
    """
    originEventList = []
    with open(LOG_PATH, "r", encoding="ISO-8859-1") as file:
        count = 0
        for line in file:
            count += 1
            event = handleLineFromFile(line)
            if event is not None:
                originEventList.append(event)
    return originEventList


def handleLineFromFile(line: str) -> Event:
    """
    将一行原始数据解析成 Event 对象
    :param line:
    :return:
    """
    segmentList = line.split(SEP_EQ)
    for h in handlerList:
        event = h.handle(segmentList)
        if event is not None:
            return event


def parseArgv(argv):
    if len(argv) != 1:
        print("usage: himem <log file>")
        sys.exit(2)
    global LOG_PATH, LOG_NAME
    LOG_PATH = os.path.expanduser(argv[0])
    LOG_NAME = os.path.basename(LOG_PATH)


def makePlot(eventList: list):
    info = PlotInfo()
    global LOG_PATH, LOG_NAME
    info.fileName = LOG_NAME
    info.filePath = LOG_PATH
    info.count = len(eventList)
    maker = BokehPlotMaker()
    maker.make(info, eventList)


def main():
    argv = sys.argv[1:]
    parseArgv(argv)
    print("开始解析...")
    originEventList = handleTraceFile()
    print("原始数据解析完毕，总共 %d 条" % len(originEventList))
    print("\n聚合完毕，生成图表...")
    makePlot(originEventList)
    print("解析完成！")


if __name__ == "__main__":
    main()
