#https://www.pyimagesearch.com/2015/09/14/ball-tracking-with-opencv/


import cv2
import numpy as np
import imutils

camera = cv2.VideoCapture(0)

def getImage():
    # read is the easiest way to get a full image out of a VideoCapture object.
    retval, im = camera.read()
    return im


# Adjusts to light, gets rid of first x frames
for i in range(15):
    getImage()

while (True):
    ret, frame = camera.read()

    blueLower = (0, 20, 80)
    blueUpper = (50, 80, 150)
    greenLower = (35, 120, 30)
    greenUpper = (120, 255, 255)

    
    # Resize frame so it can be processed quicker
    frame = imutils.resize(frame, width=800)

    # Blur to reduce extra noise
    blurred = cv2.GaussianBlur(frame, (11, 11), 0)

    # Convert to HSV colorspace
    hsv = cv2.cvtColor(blurred, cv2.COLOR_BGR2HSV)

    # Gets the places in image between the color two bounds
        # Then removes any extra small blobs
    mask = cv2.inRange(hsv, greenLower, greenUpper)
    # mask = cv2.erode(mask, None, iterations=2)
    # mask = cv2.dilate(mask, None, iterations=2)






    cv2.imshow('frame', mask)

    # Takes a picture and saves and closes when pressing 's'
    if cv2.waitKey(1) & 0xFF == ord('s'):
        saveLocation = "/Users/patrickbell/Desktop/savedCapture.jpg"
        cv2.imwrite(saveLocation, getImage())
        break


# Closes all windows opened
camera.release()
camera.destroyAllWindows()