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

import cv2
import numpy as np
import imutils
import sys

# Starts the camera feed
camera = cv2.VideoCapture(0)

# Color range for inRange function to use
# Stored as BGR, not RGB for HSV

# Gets base color from command line, else uses hardcoded
if (len(sys.argv) == 4):
    baseColor = (int(sys.argv[1]), int(sys.argv[2]), int(sys.argv[3]))
else:
    baseColor = (67, 125, 90)



g_lowerColorRange = (0,0,0)
g_upperColorRange = (0,0,0)
percentDifference = 0.3

# Sets up color ranges based on base given color
g_lowerColorRange = (baseColor[0] - baseColor[0]*percentDifference, baseColor[1] - baseColor[1]*percentDifference, baseColor[2] - baseColor[2]*percentDifference)
g_upperColorRange = (baseColor[0] + baseColor[0]*percentDifference, baseColor[1] + baseColor[1]*percentDifference, baseColor[2] + baseColor[2]*percentDifference)

# Creates the vars to avoid error in functions
hsv = 0
frame = 0
mask = 0
frameWidth = 800

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
    # print("Lower: ", g_lowerColorRange)
    # print("Upper: ", g_upperColorRange)

# Gets all the contours for that mask
def getContours(mask):
    im2, contours, hierarchy = cv2.findContours(mask,cv2.RETR_TREE,cv2.CHAIN_APPROX_SIMPLE)
    # print(len(contours))
    if (len(contours) > 1):
        cv2.drawContours(frame, contours, 0, (0,255,0), 3)
        cv2.drawContours(frame, contours, 1, (255,0,0), 3)

# Returns the center of object and the enclosing circle x, y and radius of object
def getObjectSpecs(mask):
    cnts = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    cnts = cnts[0] if imutils.is_cv2() else cnts[1]
    center = None

    # only proceed if at least one contour was found
    if len(cnts) > 0:
        # find the largest contour in the mask, then use
        # it to compute the minimum enclosing circle and
        # centroid
        c = max(cnts, key=cv2.contourArea)
        ((x, y), radius) = cv2.minEnclosingCircle(c)
        M = cv2.moments(c)
        center = (int(M["m10"] / M["m00"]), int(M["m01"] / M["m00"]))
        return {"center" : center, "x" : x, "y" : y,"radius" : radius}


# Names the windows
cv2.namedWindow("frame")
cv2.namedWindow("hsv")
cv2.namedWindow("mask")
cv2.setMouseCallback("frame", updateColorRangeWhenClick)

# Current area of screen of object being tracked
currentPosition = None


while (True):
    ret, frame = camera.read()
    
    # Resize frame so it can be processed quicker
    frame = imutils.resize(frame, width=frameWidth)

    # Blur to reduce extra noise
    blurred = cv2.GaussianBlur(frame, (11, 11), 0)

    # Convert to HSV colorspace
    hsv = cv2.cvtColor(blurred, cv2.COLOR_BGR2HSV)

    # Gets the places in image between the color two bounds
        # Then removes any extra small blobs
    mask = cv2.inRange(hsv, g_lowerColorRange, g_upperColorRange)
    mask = cv2.erode(mask, None, iterations=2)
    mask = cv2.dilate(mask, None, iterations=2)
    objectSpecs = getObjectSpecs(mask)
    if (objectSpecs != None):
        # print("Center: ", objectSpecs["center"])
        cv2.circle(frame, (int(objectSpecs["x"]), int(objectSpecs["y"])), int(objectSpecs["radius"]), (0, 255, 255), 2)


    cv2.imshow('hsv', hsv)
    cv2.imshow('mask', mask)
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



# Closes all windows opened
camera.release()
#camera.destroyAllWindows()