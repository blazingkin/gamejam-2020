def checkBlack(r, g, b, a):
    if (r == 0 and b == 0 and g == 0 and a == 255):
        return True
    return False

def checkWhite(r, g, b, a):
    if (r == 255 and b == 255 and g == 255 and a == 255):
        return True
    return False

def checkBlue(r, g, b, a):
    if (r == 0 and b == 255 and g == 0 and a == 255):
        return True
    return False

def checkGreen(r, g, b, a):
    if (r == 0 and b == 0 and g == 255 and a == 255):
        return True
    return False

def checkRed(r, g, b, a):
    if (r == 255 and b == 0 and g == 0 and a == 255):
        return True
    return False
