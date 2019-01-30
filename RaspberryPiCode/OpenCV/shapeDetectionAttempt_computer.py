#####################################################################
# Name: trackColorLeftCenterRight.py
# 
# Authors:  Patrick Bell
# Creation Date: 10-26-18
# Update Date:
# 
# Description: Tracks a predetermined color (green) and outs to the screen
# if it is to the left, right, or in the center of the screen.
# It can take a sys argument of a HSV color.
#####################################################################


# Current problem: Color contours overlap. Maybe try bitwising all them 
# together so the contours dont overlap

import cv2
import numpy as np
import imutils
import sys
import json
import time

# Starts the camera feed
camera = cv2.VideoCapture(0)

# Gets base color from command line, else uses hardcoded
if (len(sys.argv) == 4):
    baseColor = (int(sys.argv[1]), int(sys.argv[2]), int(sys.argv[3]))
else:
    baseColor = (67, 125, 90)


g_lowerColorRange = (0,0,0)
g_upperColorRange = (0,0,0)
percentDifference = 0.6

# Sets up color ranges based on base given color
g_lowerColorRange = (baseColor[0] - baseColor[0]*percentDifference, baseColor[1] - baseColor[1]*percentDifference, baseColor[2] - baseColor[2]*percentDifference)
g_upperColorRange = (baseColor[0] + baseColor[0]*percentDifference, baseColor[1] + baseColor[1]*percentDifference, baseColor[2] + baseColor[2]*percentDifference)

# Creates the vars to avoid error in functions
hsv = 0
frame = 0
mask = 0
frameWidth = 800
frameHeight = 400

# Loads all 4 colors from JSON file
def getColorsFromJSON(fileLocation):
    loadedColors = []
    with open(fileLocation, 'r') as f:
        loadedColors = json.loads(f.readline().rstrip('\n'))
    colors = []

    #Adds lower up upper bounds
    for color in loadedColors:
        temp = {}
        temp['lower'] = (color[0] - color[0]*percentDifference, color[1] - color[1]*percentDifference, color[2] - color[2]*percentDifference)
        temp['upper'] = (color[0] + color[0]*percentDifference, color[1] + color[1]*percentDifference, color[2] + color[2]*percentDifference)
        colors.append(temp)

    return colors


# Updates the color & upper color ranges when clicking on hsv
def updateColorRangeWhenClick(event, x, y, flags, param):
    global g_lowerColorRange, g_upperColorRange, percentDifference
    # If it wasn't a left click, then break here
    if event != cv2.EVENT_LBUTTONDOWN:
        return

    color = hsv[y, x]
    g_lowerColorRange = (color[0] - color[0]*percentDifference, color[1] - color[1]*percentDifference, color[2] - color[2]*percentDifference)
    g_upperColorRange = (color[0] + color[0]*percentDifference, color[1] + color[1]*percentDifference, color[2] + color[2]*percentDifference)
    print("Color: ", color)

# Returns the center of object and the enclosing circle x, y and radius of object
def getObjectSpecs(mask):
    image, contours, hierarchy = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    center = None

    # only proceed if at least one contour was found
    if len(contours) > 0:
        # find the largest contour in the mask, then use
        # it to compute the minimum enclosing circle and
        # centroid
        try:
            largestContour = max(contours, key=cv2.contourArea)
            ((x, y), radius) = cv2.minEnclosingCircle(largestContour)
            M = cv2.moments(largestContour)
            center = (int(M["m10"] / M["m00"]), int(M["m01"] / M["m00"]))

            # Ignores object if shape is above halfway point
            if (center[1] > frameHeight*.33):
                return None


            # approxShape = detectShape(largestContour)
            # print("Approx Shape: " + approxShape)
            return {"center" : center, "x" : x, "y" : y,"radius" : radius}
        except:
            return None

# Loops through all contours and labels/outlines the shapes
def identifyAndLabelAllShapes(mask, frame):
    image, contours, hierarchy = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    largestContour = None
    largestArea = 0

    areaArray = []
    for i, c in enumerate(contours):
        area = cv2.contourArea(c)
        areaArray.append(area)

    sorteddata = sorted(zip(areaArray, contours), key=lambda x: x[0], reverse=True)
    numberOfContoursAllowed = 2
    largestContours = []
    for i in range(numberOfContoursAllowed):
        try:
            largestContours.append(sorteddata[i][1])
        except:
            None


    for contour in largestContours:
        try:
            ((x, y), radius) = cv2.minEnclosingCircle(contour)
            M = cv2.moments(contour)
            center = (int(M["m10"] / M["m00"]), int(M["m01"] / M["m00"]))
            
            # Only uses object if below halfway
            if (center[1] > frameHeight*.33):
                approxShape, aspectRatio = detectShape(contour)
                specs = {"center" : center, "x" : x, "y" : y,"radius" : radius, "shape" : approxShape}
                
                cv2.putText(frame, specs["shape"], (int(specs["x"])+ int(specs["radius"]), int(specs["y"])), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 2)
                cv2.putText(frame, str(aspectRatio), (int(specs["x"])+ int(specs["radius"]), int(specs["y"])+ 20), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 2)
                cv2.drawContours(frame, [contour], -1, (255,255,255), 2)

                # Calculates are of contour and saves if largest and block/circle
                area = cv2.contourArea(contour)
                if (area > largestArea and (approxShape == "Block" or approxShape == "Circle")):
                    largestArea = area
                    largestContour = contour
        except:
            None

    return (largestContour, largestArea)
# Detects the shape of the contour
def detectShape(contour):
    # Source: https://www.pyimagesearch.com/2016/02/08/opencv-shape-detection/
    # initialize the shape name and approximate the contour
    shape = "unidentified"
    peri = cv2.arcLength(contour, True)
    approx = cv2.approxPolyDP(contour, 0.04 * peri, True)
    aspectRatio = 0

    # if the shape is a triangle, it will have 3 vertices
    if len(approx) == 3:
        shape = "triangle"

    # if the shape has 4 vertices, it is either a square or
    # a rectangle
    elif len(approx) == 4:
        # compute the bounding box of the contour and use the
        # bounding box to compute the aspect ratio
        (x, y, w, h) = cv2.boundingRect(approx)
        aspectRatio = w / float(h)

        # a square will have an aspect ratio that is approximately
        # equal to one, otherwise, the shape is a rectangle
        if aspectRatio < 0.35:
            shape = "Corner Post"
        elif aspectRatio >= 0.35 and aspectRatio <= 0.85:
            shape = "Center Post"
        elif aspectRatio > 0.85:
            shape = "Block"


    # otherwise, we assume the shape is a circle
    else:
        shape = "circle"

    # return the name of the shape
    return shape, aspectRatio



# Names the windows
cv2.namedWindow("frame")
# cv2.namedWindow("hsv")
# cv2.namedWindow("mask")
cv2.setMouseCallback("frame", updateColorRangeWhenClick)

# Current area of screen of object being tracked
currentPosition = None

# colorSavedFile = 'RaspberryPiCode/OpenCV/colorCalibration.json'
colorSavedFile = 'colorCalibration.json'
colors = getColorsFromJSON(colorSavedFile)


while (True):
    startTime = time.time()
    ret, frame = camera.read()
    
    # Resize frame so it can be processed quicker
    frame = imutils.resize(frame, height=frameHeight)

    # Blur to reduce extra noise
    blurred = cv2.GaussianBlur(frame, (11, 11), 0)

    # Convert to HSV colorspace
    hsv = cv2.cvtColor(blurred, cv2.COLOR_BGR2HSV)

    largestContourAndArea = (0,0)

    # Gets the places in image between the color two bounds
        # Then removes any extra small blobs
    for color in colors:
        mask = cv2.inRange(hsv, color['lower'], color['upper'])
        # mask = cv2.erode(mask, None, iterations=2)
        # mask = cv2.dilate(mask, None, iterations=2)
        objectSpecs = getObjectSpecs(mask)
        contourAndArea = identifyAndLabelAllShapes(mask, frame)

        # If given contour is larger than current largest, overwrite
        if (contourAndArea[1] > largestContourAndArea[1]):
            largestContourAndArea = contourAndArea
    
    # Outlines largest contour that is a shape or ball
    if (largestContourAndArea[1] != 0):
        cv2.drawContours(frame, [largestContourAndArea[0]], -1, (0,0,0), 2)

    # cv2.imshow('hsv', hsv)
    # cv2.imshow('mask', mask)
    cv2.imshow('frame', frame)

    # Takes a picture and saves and closes when pressing 's'
    if cv2.waitKey(1) & 0xFF == ord('s'):
        break

    # TODO - Change to make it only print if the position changes from left/right/center
        # - Maybe store variable with current left/right/center

    if (objectSpecs != None):
        # Tells if object is left, right, or center of screen
        if ((int(objectSpecs["x"]) - int(objectSpecs["radius"])) <= frameWidth/2 and (int(objectSpecs["x"]) + int(objectSpecs["radius"])) >= frameWidth/2):
            if (currentPosition != "center"):
                currentPosition = "center"
                print("Its in the center")
        elif ((int(objectSpecs["x"]) - int(objectSpecs["radius"])) <= frameWidth/2 and (int(objectSpecs["x"]) + int(objectSpecs["radius"])) <= frameWidth/2):
            if (currentPosition != "left"):
                currentPosition = "left"
                print("Its on the left")
        elif ((int(objectSpecs["x"]) - int(objectSpecs["radius"])) >= frameWidth/2 and (int(objectSpecs["x"]) + int(objectSpecs["radius"])) >= frameWidth/2):
            if (currentPosition != "right"):
                currentPosition = "right"
                print("Its on the right")

    totalTime = time.time() - startTime

    # print("Frame time: ", totalTime)


# Closes all windows opened
camera.release()
#camera.destroyAllWindows()