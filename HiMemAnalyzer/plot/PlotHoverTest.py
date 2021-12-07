#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# @Time    : 2021/11/9 下午5:16
# @Author  : zkw
# @File    : PlotHoverTest.py

import numpy as np

from bokeh.plotting import figure, output_file, show

output_file("tools_hover_tooltip_image.html")

steps = np.array([np.linspace(0, 10, 10)] * 20)

data = dict(image=[steps],
            x=[0],
            y=[20],
            dw=[20],
            dh=[10])

TOOLTIPS = [
    ('name', "$name"),
    ('index', "$index"),
    ("value", "@image")
]

p = figure(x_range=(0, 35), y_range=(0, 35), tools='hover,wheel_zoom', tooltips=TOOLTIPS)
p.image(source=data, image='image', x='x', y='y', dw='dw', dh='dh', palette="Inferno256", name="Image Glyph")

show(p)
