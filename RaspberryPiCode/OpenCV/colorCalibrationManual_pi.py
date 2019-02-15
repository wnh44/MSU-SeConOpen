import cv2
import numpy as np
import imutils
import sys
import json
import time
import serial


allColors = []
frameWidth = 600
frameHeight = 500
hsv = None

# -------------------- Pi Stuff Setup --------------------
import RPi.GPIO as GPIO

from picamera.array import PiRGBArray
from picamera import PiCamera


# Starts the camera feed
camera = PiCamera()
camera.resolution = (frameWidth, frameWidth)
camera.framerate = 32
rawCapture = PiRGBArray(camera)

# allow the camera to warmup
time.sleep(1)

# ---------------------------------------------------------



# Updates the color & upper color ranges when clicking on hsv
def updateColorRangeWhenClick(event, x, y, flags, param):
    global allColors, hsv
    # If it wasn't a left click, then break here
    if event != cv2.EVENT_LBUTTONDOWN:
        return

    color = hsv[y, x].tolist()
    allColors.append(color)
    print(color)


# allow the camera to warmup
time.sleep(1)

# Stores all 4 colors needed for tracking
allColors = []


cv2.namedWindow("frame")
cv2.setMouseCallback("frame", updateColorRangeWhenClick)

# Takes frame and gets color
# Takes frame and gets color
for frame in camera.capture_continuous(rawCapture, format="bgr", use_video_port=True):
    frame = frame.array

    # Resize frame so it can be processed quicker
    frame = cv2.resize(frame,(frameWidth, frameHeight))

    # Convert to HSV colorspace
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

    # Closes when pressing 's'
    if cv2.waitKey(1) & 0xFF == ord('s'):
        break

    cv2.imshow('frame', frame)
    rawCapture.truncate(0)


# Converts it to JSON format
output = json.dumps(allColors)

# Saves it to a file
with open('colorCalibration.json', 'w') as f:
    f.write(output)


camera.release()
