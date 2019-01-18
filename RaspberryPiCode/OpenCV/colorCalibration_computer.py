import cv2
import numpy as np
import imutils
import sys
import serial
import time
import json

# Gets camera
camera = cv2.VideoCapture(0)

# allow the camera to warmup
time.sleep(1)

# Stores all 4 colors needed for tracking
colors = []

# Width of frame
frameWidth = 500

# Takes frame and gets color
while (True):
    # Gets frame from camera feed
    ret, frame = camera.read()

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

    # Sleeps for x seconds so the robot can be moved to next side
    time.sleep(5)

# Converts it to JSON format
output = json.dumps(colors)

# Saves it to a file
with open('colorCalibaration.json', 'w') as f:
    f.write(output)


camera.release()


