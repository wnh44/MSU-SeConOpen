import cv2
import numpy as np
import imutils
import sys
import serial
import RPi.GPIO as GPIO
import time
import json

from picamera.array import PiRGBArray
from picamera import PiCamera

# allow the camera to warmup
time.sleep(1)

# Stores all 4 colors needed for tracking
allColors = []

# Width of frame
frameWidth = 500
percentDifference = 0.6

loadedColors = [[ 25,184, 158], [179, 192, 125], [ 78, 208, 54], [111, 241, 94]]
colors = []
for color in loadedColors:
    temp = {}
    temp['lower'] = (color[0] - color[0]*percentDifference, color[1] - color[1]*percentDifference, color[2] - color[2]*percentDifference)
    temp['upper'] = (color[0] + color[0]*percentDifference, color[1] + color[1]*percentDifference, color[2] + color[2]*percentDifference)
    colors.append(temp)

# Starts the camera feed
camera = PiCamera()
camera.resolution = (frameWidth, frameWidth)
camera.framerate = 32
rawCapture = PiRGBArray(camera)

colorNumber = 0

# Takes frame and gets color
for frame in camera.capture_continuous(rawCapture, format="bgr", use_video_port=True):
    frame = frame.array

    # Resize frame so it can be processed quicker
    frame = imutils.resize(frame, width=frameWidth, height=frameWidth)

    # Convert to HSV colorspace
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

    masks = []
    for individualColor in colors:
        mask = cv2.inRange(hsv, individualColor['lower'], individualColor['upper'])
        masks.append(mask)

    # Combines all masks 
    mask = cv2.bitwise_or(masks[0], masks[1])
    mask = cv2.bitwise_or(mask, masks[2])
    mask = cv2.bitwise_or(mask, masks[3])

    cnts = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    contours = cnts[0] if imutils.is_cv2() else cnts[1]

    largestContour = max(contours, key=cv2.contourArea)
    ((x, y), radius) = cv2.minEnclosingCircle(largestContour)
    M = cv2.moments(largestContour)
    center = (int(M["m10"] / M["m00"]), int(M["m01"] / M["m00"]))
    
    newColor = hsv[center[1], center[0]].tolist()
    colorNumber += 1
    cv2.putText(frame, "Here", (int(center[1]), int(center[0])), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 2)
    cv2.imwrite( "color" + str(colorNumber) + ".jpg", frame)

    # center = [int(frame.shape[1]/2), int(frame.shape[0]/2)]

    # Gets color from middle pixel
    # color = hsv[center[1], center[0]].tolist()
    print(newColor)

    # Add color to list
    allColors.append(newColor)

    # Break out of loop if all 4 colors are gotten
    if (len(allColors) == 4):
        break

    # Sleeps for x seconds so the robot can be moved to next side
    time.sleep(10)
    rawCapture.truncate(0)


# Converts it to JSON format
output = json.dumps(allColors)

# Saves it to a file
with open('colorCalibration.json', 'w') as f:
    f.write(output)



# Use this to convert to brighter??
#https://progtpoint.blogspot.com/2017/02/today-im-going-to-show-you-how-to.html
