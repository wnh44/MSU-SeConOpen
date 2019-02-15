import cv2
import numpy as np
import imutils
import sys
import json
import time
import serial

# -------------------- Pi Stuff Setup --------------------
# import RPi.GPIO as GPIO

# from picamera.array import PiRGBArray
# from picamera import PiCamera

# # Sets up serial
# serialPath = "/dev/ttyACM" + sys.argv[1]
# g_SER=serial.Serial(serialPath,9600)  # Change ACM number as found from ls /dev/tty/ACM*
# g_SER.baudrate=9600

# Starts the camera feed
# camera = PiCamera()
# camera.resolution = (frameWidth, frameWidth)
# camera.framerate = 32
# rawCapture = PiRGBArray(camera)

# # allow the camera to warmup
# time.sleep(1)

# ---------------------------------------------------------

allColors = []
frameWidth = 600
frameHeight = 500
hsv = None

# Updates the color & upper color ranges when clicking on hsv
def updateColorRangeWhenClick(event, x, y, flags, param):
    global allColors, hsv
    # If it wasn't a left click, then break here
    if event != cv2.EVENT_LBUTTONDOWN:
        return

    color = hsv[y, x].tolist()
    allColors.append(color)
    print(color)


# Gets camera
camera = cv2.VideoCapture(0)

# allow the camera to warmup
time.sleep(1)

# Stores all 4 colors needed for tracking
allColors = []

# Width of frame
frameWidth = 500


cv2.namedWindow("frame")
cv2.setMouseCallback("frame", updateColorRangeWhenClick)

# Takes frame and gets color
while (True):
    # Gets frame from camera feed
    ret, frame = camera.read()

    # Resize frame so it can be processed quicker
    frame = cv2.resize(frame,(frameWidth, frameHeight))

    # Convert to HSV colorspace
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

    # Closes when pressing 's'
    if cv2.waitKey(1) & 0xFF == ord('s'):
        break

    cv2.imshow('frame', frame)


# Converts it to JSON format
output = json.dumps(allColors)

# Saves it to a file
with open('colorCalibration.json', 'w') as f:
    f.write(output)


camera.release()
