#####################################################################
# Name: moveBasedOnColorLeftCenterRight.py
# 
# Authors:  Patrick Bell
# Creation Date: 10-26-18
# Update Date:
# 
# Description: Tracks a predetermined color (green) and outs to the screen
# if it is to the left, right, or in the center of the screen. It then sends 
# commands to arduino to move accordingly
#
# It can take a sys argument of the device number and a HSV color.
# https://www.pyimagesearch.com/2015/03/30/accessing-the-raspberry-pi-camera-with-opencv-and-python/
#####################################################################

import cv2
import numpy as np
import imutils
import sys
import serial
import RPi.GPIO as GPIO
import time

from picamera.array import PiRGBArray
from picamera import PiCamera


# Sets up serial
serialPath = "/dev/ttyACM" + sys.argv[1]
g_SER=serial.Serial(serialPath,9600)  # Change ACM number as found from ls /dev/tty/ACM*
g_SER.baudrate=9600


# Color range for inRange function to use
# Stored as BGR, not RGB for HSV

# Gets base color from command line, else uses hardcoded
if (len(sys.argv) == 5):
    baseColor = (int(sys.argv[2]), int(sys.argv[3]), int(sys.argv[4]))
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

# Gets all the contours for that mask
def getContours(mask):
    im2, contours, hierarchy = cv2.findContours(mask,cv2.RETR_TREE,cv2.CHAIN_APPROX_SIMPLE)
    if (len(contours) > 1):
        cv2.drawContours(frame, contours, 0, (0,255,0), 3)
        cv2.drawContours(frame, contours, 1, (255,0,0), 3)

# Returns the center of object and the enclosing circle x, y and radius of object
def getObjectSpecs(mask):
    cnts = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    cnts = cnts[0] if imutils.is_cv2() else cnts[1]
    center = None

    # only proceed if at least one contour was found
    # if len(cnts) > 0:
    # find the largest contour in the mask, then use
    # it to compute the minimum enclosing circle and
    # centroid
    try:
        c = max(cnts, key=cv2.contourArea)
        ((x, y), radius) = cv2.minEnclosingCircle(c)
        M = cv2.moments(c)
        center = (int(M["m10"] / M["m00"]), int(M["m01"] / M["m00"]))
        return {"center" : center, "x" : x, "y" : y,"radius" : radius}
    except:
        print("Error getting center")
        return None

# Reads from serial, returns the text
def readFromSerial():
	dataReceived = g_SER.readline()
	return dataReceived

# Writes data to serial
def writeToSerial(dataToSend):
    # If input is 'quit', it closes the connection
	if (dataToSend == 'quit'):
		closeSerialConnection()
		return

	if (dataToSend[-1] != '@'):
		dataToSend = dataToSend + '@'

	# Writes to serial
	g_SER.write(dataToSend)

# Closes the serial communication
def closeSerialConnection():
    g_SER.close()

# Writes to arduino and waits for a response
def writeAndReadToSerial(dataToSend):
    writeToSerial(dataToSend)
    dataReceived = readFromSerial()
    return dataReceived


# Starts the camera feed
camera = PiCamera()
camera.resolution = (frameWidth, frameWidth)
camera.framerate = 32
rawCapture = PiRGBArray(camera)

# allow the camera to warmup
time.sleep(1)

# Current area of screen of object being tracked
currentPosition = None

for frame in camera.capture_continuous(rawCapture, format="bgr", use_video_port=True):
    # start = time.time()
    frame = frame.array

    # Convert to HSV colorspace
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)        # hsv = cv2.cvtColor(blurred, cv2.COLOR_BGR2HSV)

    # Gets the places in image between the color two bounds
    mask = cv2.inRange(hsv, g_lowerColorRange, g_upperColorRange)

    objectSpecs = getObjectSpecs(mask)

    # Closes when pressing 's'
    if cv2.waitKey(1) & 0xFF == ord('s'):
        break

    pixelErrorDifference = frameWidth*0.1

    if (objectSpecs != None):
        # Tells if object is left, right, or center of screen
        if ((int(objectSpecs["x"]) - int(objectSpecs["radius"]))-pixelErrorDifference <= frameWidth/2 and (int(objectSpecs["x"]) + int(objectSpecs["radius"]))+pixelErrorDifference >= frameWidth/2):
            if (currentPosition != "center"):
                currentPosition = "center"
                print("Its in the center")
                received = writeAndReadToSerial("GO forward 80@")
        elif ((int(objectSpecs["x"]) - int(objectSpecs["radius"])) <= frameWidth/2 and (int(objectSpecs["x"]) + int(objectSpecs["radius"])) <= frameWidth/2):
            if (currentPosition != "right"):
                currentPosition = "right"
                print("Its on the right")
                received = writeAndReadToSerial("GO right 25@")
        elif ((int(objectSpecs["x"]) - int(objectSpecs["radius"])) >= frameWidth/2 and (int(objectSpecs["x"]) + int(objectSpecs["radius"])) >= frameWidth/2):
            if (currentPosition != "left"):
                currentPosition = "left"
                print("Its on the left")
                received = writeAndReadToSerial("GO left 25@")  

    if (objectSpecs == None):
        print("Its not found")
        received = writeAndReadToSerial("GO stop@")



    # print("Total Time:", time.time() - start)
    rawCapture.truncate(0)
    print("Finished Loop\n")




# Closes all windows opened
camera.release()