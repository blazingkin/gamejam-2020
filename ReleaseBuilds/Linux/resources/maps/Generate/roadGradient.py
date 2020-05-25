from PIL import Image
import random
import sys
import math

import checks

def placeSpawner(im, location):
    for x in range (-5, 6):
        for y in range (-5, 6):
            im.putpixel((location[0]+x, location[1] + y), blue)
    im.putpixel(location, (255, 0, 255, 255))

def getGradientColor(location, offset):
    x = location[0] - offset[0]
    y = location[1] - offset[1]
    distance = math.sqrt((x * x) + (y * y))
    if distance > (max_road_dist * (2/3)):
        o_e = 255
        o_s = 200
        i_e = max_road_dist
        i_s = max_road_dist * (2/3)
    elif distance < (max_road_dist * (1/3)):
        o_e = 100
        o_s = 0
        i_e = max_road_dist * (1/3)
        i_s = 0
    else:
        o_e = 200
        o_s = 100
        i_e = max_road_dist * (2/3)
        i_s = max_road_dist * (1/3)

    s = (o_e - o_s) / (i_e - i_s)
    c = o_s + s * (distance - i_s)
    
    return (0, (int)(c), 0, 255)

if len(sys.argv) != 3:
    print("Usage: roadGradient.py fileSuffix maxDistFromRoad")

roads = Image.open("../roads_"+ sys.argv[1] + ".png")
rgba_roads = roads.convert('RGBA')

max_road_dist = int(sys.argv[2])

red = (255, 0, 0 , 255)
green = (0, 255, 0, 255)
blue = (0, 0, 255, 255)

for x in range (roads.size[0]):
    for y in range(roads.size[1]):
        r, g, b, a = rgba_roads.getpixel((x,y))
        isBlack = checks.checkBlack(r, g, b, a)
        if x < 5 or x > roads.size[0] - 6 or y < 5 or y > roads.size[1] - 6:
            if not isBlack:
                rgba_roads.putpixel((x,y), green)
            continue
        if isBlack:
            for i in range (-5, 6):
                for j in range (-5, 6):
                    r2, g2, b2, a2 = rgba_roads.getpixel((x + i, y + j))
                    if not checks.checkBlack(r2, g2, b2, a2):
                        rgba_roads.putpixel((x + i, y + j), green);

count = 0
for x in range (roads.size[0]):
    for y in range(roads.size[1]):
        r, g, b, a = rgba_roads.getpixel((x,y))
        if checks.checkWhite(r,g,b,a):
            radius = 5
            finished = False
            checked = False
            while not finished:
                for i in range (-radius, radius + 1):
                    if x + i < 5 or x + i > roads.size[0] - 6:
                        continue
                    r2, g2, b2, a2 = rgba_roads.getpixel((x + i, y))
                    checked = True
                    if checks.checkBlack(r2, g2, b2, a2):
                        rgba_roads.putpixel((x,y), getGradientColor((x, y), (x + i, y)))
                        finished = True
                    if x - i < 5 or x - i > roads.size[0] - 6:
                        continue
                    checked = True
                    r2, g2, b2, a2 = rgba_roads.getpixel((x - i, y))
                    if checks.checkBlack(r2, g2, b2, a2):
                        rgba_roads.putpixel((x,y), getGradientColor((x, y), (x - i, y)))
                        finished = True

                for j in range (-radius, radius + 1):
                    if y + j < 5 or y + j > roads.size[1] - 6:
                        continue
                    r2, g2, b2, a2 = rgba_roads.getpixel((x, y + j))
                    checked = True
                    if checks.checkBlack(r2, g2, b2, a2):
                        rgba_roads.putpixel((x,y), getGradientColor((x, y), (x, y + j)))
                        finished = True
                    if y - j < 5 or y - j > roads.size[1] - 6:
                        continue
                    checked = True
                    r2, g2, b2, a2 = rgba_roads.getpixel((x, y - j))
                    if checks.checkBlack(r2, g2, b2, a2):
                        rgba_roads.putpixel((x,y), getGradientColor((x, y), (x, y - j)))
                        finished = True
                radius += 1
                if not checked or radius > max_road_dist:
                    finished = True
                checked = False
            count += 1
            if count % 100 == 0:
                print(count)



rgba_roads.save("roadGradient_" + sys.argv[1] + ".png");
