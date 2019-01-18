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

# Width of frame
frameWidth = 512

# Starts the camera feed
camera = PiCamera()
camera.resolution = (frameWidth, frameWidth)
camera.framerate = 32
rawCapture = PiRGBArray(camera)

# allow the camera to warmup
time.sleep(1)

# Stores all 4 colors needed for tracking
colors = []


# Takes frame and gets color
for frame in camera.capture_continuous(rawCapture, format="bgr", use_video_port=True):
    frame = frame.array

    # Resize frame so it can be processed quicker
    frame = imutils.resize(frame, width=frameWidth, height=frameWidth)

    # Convert to HSV colorspace
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

    # Gets color from middle pixel
    color = hsv[int(frameWidth/2), int(frameWidth/2)].tolist()
    print(color)

    # Add color to list
    colors.append(color)

    # Break out of loop if all 4 colors are gotten
    if (len(colors) == 4):
        break

    # Clears the frame buffer
    rawCapture.truncate(0)

    # Sleeps for x seconds so the robot can be moved to next side
    time.sleep(7)

# Converts it to JSON format
output = json.dumps(colors)

# Saves it to a file
with open('colorCalibration.json', 'w') as f:
    f.write(output)




