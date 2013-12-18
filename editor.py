#!/usr/bin/env python

import sys
import string

from Tkinter import *

import subprocess

tkroot = Tk()

class Container:
    pass

cont = Container()

gutter = 20
pixel = 32
XSIZE, YSIZE = 5, 9
LEFT, TOP = 100, 700 # for transforming coordinates returned from bedstead

cont.canvas = Canvas(tkroot,
                     width=2 * (XSIZE*pixel) + 3*gutter,
                     height=YSIZE*pixel + 2*gutter,
                     bg='white')
cont.bitmap = [0] * YSIZE
cont.oldbitmap = cont.bitmap[:]
cont.pixels = [[None]*XSIZE for y in range(YSIZE)]
cont.polygons = []

for x in range(XSIZE+1):
    cont.canvas.create_line(gutter + x*pixel, gutter,
                            gutter + x*pixel, gutter + YSIZE*pixel)
for y in range(YSIZE+1):
    cont.canvas.create_line(gutter, gutter + y*pixel,
                            gutter + XSIZE*pixel, gutter + y*pixel)

dragging = None

def getpixel(x, y):
    assert x >= 0 and x < XSIZE and y >= 0 and y < YSIZE
    bit = 1 << (XSIZE-1 - x)
    return cont.bitmap[y] & bit

def setpixel(x, y, state):
    assert x >= 0 and x < XSIZE and y >= 0 and y < YSIZE
    bit = 1 << (XSIZE-1 - x)
    if state and not (cont.bitmap[y] & bit):
        cont.bitmap[y] |= bit
        cont.pixels[y][x] = cont.canvas.create_rectangle(
            gutter + x*pixel, gutter + y*pixel,
            gutter + (x+1)*pixel, gutter + (y+1)*pixel,
            fill='black')
    elif not state and (cont.bitmap[y] & bit):
        cont.bitmap[y] &= ~bit
        cont.canvas.delete(cont.pixels[y][x])
        cont.pixels[y][x] = None

def regenerate():
    if cont.oldbitmap == cont.bitmap:
        return

    cont.oldbitmap = cont.bitmap[:]

    for pg in cont.polygons:
        cont.canvas.delete(pg)
    cont.polygons = []

    data = subprocess.check_output(["./bedstead"] + map(str, cont.bitmap))
    paths = []
    path = None
    for line in data.splitlines():
        words = line.split()
        if len(words) >= 3 and words[2] in ["m","l"]:
            x = int((float(words[0])-LEFT)*pixel*0.01 + 2*gutter + XSIZE*pixel)
            y = int((TOP - float(words[1]))*pixel*0.01 + gutter) 
            if words[2] == "m":
                path = []
                paths.append(path)
            path.append([x,y])

    # The output from 'bedstead' will be a set of disjoint paths,
    # in the Postscript style (going one way around the outside of
    # filled areas, and the other way around internal holes in
    # those areas). Python/Tk doesn't know how to fill an
    # arbitrary path in that representation, so instead we must
    # convert into a set of individual Tk polygons (convex shapes
    # with a single closed outline) and display them in the right
    # order with the right colour.
    #
    # A neat way to arrange this is to compute the area enclosed
    # by each polygon, essentially by integration: for each line
    # segment (x0,y0)-(x1,y1), sum the y difference (y1-y0) times
    # the average x value, which gives the area between that line
    # segment and the corresponding segment of the x-axis. After
    # we go all the way round an outline in this way, we'll have
    # precisely the area enclosed by the outline, no matter how
    # many times it doubles back on itself (because every piece of
    # x-axis has been cancelled out by an outline going back the
    # other way). Furthermore, the sign of the integral we've
    # computed tells us whether the outline goes one way or the
    # other around the area.
    #
    # So then we sort our paths into descending order of the
    # absolute value of its computed area (guaranteeing that any
    # path contained inside another appears after it, since it
    # must enclose a strictly smaller area) and fill each one with
    # a colour based on the area's sign.
    #
    # This strategy depends critically on 'bedstead' having given
    # us sensible paths in the first place: it wouldn't handle an
    # _arbitrary_ PostScript path, with loops allowed to overlap
    # and intersect rather than being neatly nested.
    pathswithmetadata = []
    for path in paths:
        area = 0
        for i in range(len(path)):
            x0, y0 = path[i-1]
            x1, y1 = path[i]
            area += (y1-y0) * (x0+x1)/2
        pathswithmetadata.append([abs(area),
                                 ('black' if area>0 else 'white'),
                                  path])
    pathswithmetadata.sort(reverse=True)

    for _, colour, path in pathswithmetadata:
        if len(path) > 1 and path[0] == path[-1]:
            del path[-1]
            args = sum(path, []) # x,y,x,y,...,x.y
            pg = cont.canvas.create_polygon(*args, fill=colour)
            cont.polygons.append(pg)

def click(event):
    for dragstartx in gutter, 2*gutter + XSIZE*pixel:
        x = (event.x - dragstartx) / pixel
        y = (event.y - gutter) / pixel
        if x >= 0 and x < XSIZE and y >= 0 and y < YSIZE:
            cont.dragstartx = dragstartx
            cont.dragstate = not getpixel(x,y)
            setpixel(x, y, cont.dragstate)
            regenerate()
            break

def drag(event):
    x = (event.x - cont.dragstartx) / pixel
    y = (event.y - gutter) / pixel
    if x >= 0 and x < XSIZE and y >= 0 and y < YSIZE:
        setpixel(x, y, cont.dragstate)
        regenerate()
        return

def key(event):
    if event.char in (' '):
        bm = ",".join(map(lambda n: "%03o" % n, cont.bitmap))
        print " {{%s}, 0x }," % bm
    elif event.char in ('c','C'):
        for y in range(YSIZE):
            for x in range(XSIZE):
                setpixel(x, y, 0)
        regenerate()
    elif event.char in ('q','Q','\x11'):
        sys.exit(0)

cont.canvas.bind("<Button-1>", click)
cont.canvas.bind("<B1-Motion>", drag)
tkroot.bind("<Key>", key)
cont.canvas.pack()

mainloop()
