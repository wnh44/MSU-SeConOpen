#https://www.pyimagesearch.com/2015/09/14/ball-tracking-with-opencv/
#https://docs.opencv.org/3.1.0/d4/d73/tutorial_py_contours_begin.html

import cv2
import numpy as np
import imutils
from picamera.array import PiRGBArray
from picamera import PiCamera

#camera = cv2.VideoCapture(0)
camera = PiCamera()
rawCapture = PiRGBArray(camera)

# Color range for inRange function to use
# Stored as BGR, not RGB for HSV
g_lowerColorRange = (0,0,0)
g_upperColorRange = (0,0,0)
percentDifference = 0.3
hsv = 0
frame = 0
mask = 0


# Saves still of image
def getImage():
    # read is the easiest way to get a full image out of a VideoCapture object.
    #retval, im = camera.read()
    camera.capture(rawCapture, format="bgr")
    im = rawCapture.array
    return im

# Updates the color & upper color ranges when clicking on hsv
def updateColorRangeWhenClick(event, x, y, flags, param):
    global g_lowerColorRange, g_upperColorRange
    # If it wasn't a left click, then break here
    if event != cv2.EVENT_LBUTTONDOWN:
        return

    color = hsv[y, x]
    g_lowerColorRange = (color[0] - color[0]*percentDifference, color[1] - color[1]*percentDifference, color[2] - color[2]*percentDifference)
    g_upperColorRange = (color[0] + color[0]*percentDifference, color[1] + color[1]*percentDifference, color[2] + color[2]*percentDifference)
    print("Color: ", color)
    #print("Lower: ", g_lowerColorRange)
    #print("Upper: ", g_upperColorRange)


def getContours(mask):
    im2, contours, hierarchy = cv2.findContours(mask,cv2.RETR_TREE,cv2.CHAIN_APPROX_SIMPLE)
    print(len(contours))
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


while (True):
    # ret, frame = camera.read()
    camera.capture(rawCapture, format="bgr")
    frame = rawCapture.array
    
    # Resize frame so it can be processed quicker
    frame = imutils.resize(frame, width=800)

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
    # getContours(mask)
    if (objectSpecs != None):
        # print("Center: ", objectSpecs["center"])
        cv2.circle(frame, (int(objectSpecs["x"]), int(objectSpecs["y"])), int(objectSpecs["radius"]), (0, 255, 255), 2)


    cv2.imshow('hsv', hsv)
    cv2.imshow('mask', mask)
    cv2.imshow('frame', frame)

    # Takes a picture and saves and closes when pressing 's'
    if cv2.waitKey(1) & 0xFF == ord('s'):
        saveLocation = "/Users/patrickbell/Desktop/savedCapture.jpg"
        cv2.imwrite(saveLocation, getImage())
        break

    rawCapture.truncate(0)


# Closes all windows opened
camera.release()
camera.destroyAllWindows()