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

loadedColors = [[ 25,184, 158], [179, 192, 125], [ 78, 208, 54], [111, 241, 94]]
colors = []
for color in loadedColors:
    temp = {}
    temp['lower'] = (color[0] - color[0]*percentDifference, color[1] - color[1]*percentDifference, color[2] - color[2]*percentDifference)
    temp['upper'] = (color[0] + color[0]*percentDifference, color[1] + color[1]*percentDifference, color[2] + color[2]*percentDifference)
    colors.append(temp)



# Takes frame and gets color
while (True):
    # Gets frame from camera feed
    ret, frame = camera.read()

    # Resize frame so it can be processed quicker
    frame = imutils.resize(frame, width=frameWidth, height=frameWidth)

    # Convert to HSV colorspace
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

    masks = []
    for color in colors:
        mask = cv2.inRange(hsv, color['lower'], color['upper'])
        masks.append(mask)



    center = [int(frame.shape[1]/2), int(frame.shape[0]/2)]

    # Gets color from middle pixel
    color = hsv[center[1], center[0]].tolist()
    print(color)

    # Add color to list
    colors.append(color)

    # Break out of loop if all 4 colors are gotten
    if (len(colors) == 4):
        break

    # Sleeps for x seconds so the robot can be moved to next side
    time.sleep(15)

# Converts it to JSON format
output = json.dumps(colors)

# Saves it to a file
with open('colorCalibration.json', 'w') as f:
    f.write(output)


camera.release()


