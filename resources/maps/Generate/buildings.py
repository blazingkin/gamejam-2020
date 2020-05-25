from PIL import Image
import random
import sys
import math

import checks

def placeHouse(im, location):
    for x in range (-10, 11):
        for y in range (-10, 11):
            if (location[0] + x < 0 or location[0] + x > im.size[0] - 1 or location[1] + y < 0 or location[1] + y > im.size[1] - 1):
                continue
            r, g, b, a = im.getpixel((location[0] + x, location[1] + y))
            if checks.checkBlack(r, g, b, a) or checks.checkBlue(r, g, b, a) or checks.checkRed(r, g, b, a):
                continue
            im.putpixel((location[0] + x, location[1] + y), green)
    for x in range (-5, 6):
        for y in range (-5, 6):
            im.putpixel((location[0] + x, location[1] + y), blue)
    im.putpixel(location, red)
    for x in range (-20, 21):
        for y in range (-20, 21):
            if (location[0] + x < 0 or location[0] + x > im.size[0] - 1 or location[1] + y < 0 or location[1] + y > im.size[1] - 1):
                continue
            r, g, b, a = im.getpixel((location[0] + x, location[1] + y))
            if (r == 0 and b == 0 and g != 255 and g != 0):
                im.putpixel((location[0] + x, location[1] + y), (r, g + 5 * (28 - (int)(math.sqrt((x * x) + (y * y)))), b, a))

if len(sys.argv) != 3:
    print("Usage: buildings.py fileSuffix numHouses")

roads = Image.open("roadGradient_"+ sys.argv[1] + ".png")
rgba_roads = roads.convert('RGBA')

red = (255, 0, 0 , 255)
green = (0, 255, 0, 255)
blue = (0, 0, 255, 255)


houses = 0
max_houses = int(sys.argv[2])

while houses < max_houses:
    pos = (random.randint(5, rgba_roads.size[0] - 6), random.randint(5, rgba_roads.size[1] - 6))
    r, g, b, a = rgba_roads.getpixel(pos)
    if checks.checkGreen(r, g, b, a) or checks.checkBlack(r, g, b, a) or checks.checkBlue(r, g, b, a) or checks.checkRed(r, g, b, a):
        continue
    if (random.randint(0, 100) > ((g / 255) * 100)):
        placeHouse(rgba_roads, pos)
        houses += 1

rgba_roads.save("buildings_" + sys.argv[1] + ".png");
