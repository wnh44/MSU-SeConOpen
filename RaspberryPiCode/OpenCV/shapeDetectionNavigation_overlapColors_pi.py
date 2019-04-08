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


# Current problem: Only consider bottom have object to be blocks
# If object goes over midpoint and has 4 verts, it must be center or corner post *******


import cv2
import numpy as np
import imutils
import sys
import json
import time
import serial
import RPi.GPIO as GPIO
import sys

from picamera.array import PiRGBArray
from picamera import PiCamera


# Sets up serial
serialPath = "/dev/ttyACM" + sys.argv[1]
g_SER=serial.Serial(serialPath,9600)  # Change ACM number as found from ls /dev/tty/ACM*
g_SER.baudrate=9600


# Creates the vars to avoid error in functions
hsv = 0
frame = 0
mask = 0
frameWidth = 300
frameHeight = 300

# Keeps track of time conveyor has been running
conveyorTime = 0
conveyorRunning = False

# If true, it looks for corner posts and goes home
goHome = False
cornerPostSearchTimer = 0
colorIndexToLookFor = 0
framesWithoutCornerPost = 0

ignoreYellow = True

# Overall time
overallTime = time.time()

# Starts the camera feed, starts output feed
camera = cv2.VideoCapture(0)
# outputVideo = cv2.VideoWriter('output.avi',cv2.VideoWriter_fourcc('D','I','V','X'), 20.0, (int(camera.get(3)),int(camera.get(4))))

# Gets base color from command line, else uses hardcoded
if (len(sys.argv) == 4):
    baseColor = (int(sys.argv[1]), int(sys.argv[2]), int(sys.argv[3]))
else:
    baseColor = (67, 125, 90)


g_lowerColorRange = (0,0,0)
g_upperColorRange = (0,0,0)
percentDifference = 0.5

# Sets up color ranges based on base given color
g_lowerColorRange = (baseColor[0] - baseColor[0]*percentDifference, baseColor[1] - baseColor[1]*percentDifference, baseColor[2] - baseColor[2]*percentDifference)
g_upperColorRange = (baseColor[0] + baseColor[0]*percentDifference, baseColor[1] + baseColor[1]*percentDifference, baseColor[2] + baseColor[2]*percentDifference)



# Loads all 4 colors from JSON file
def getColorsFromJSON(fileLocation):
    loadedColors = []
    with open(fileLocation, 'r') as f:
        loadedColors = json.loads(f.readline().rstrip('\n'))
    colors = []

    #Adds lower up upper bounds
    for color in loadedColors:
        temp = {}
        temp['lower'] = (color[0] - color[0]*percentDifference, color[1] - color[1]*percentDifference, color[2] - color[2]*percentDifference)
        temp['upper'] = (color[0] + color[0]*percentDifference, color[1] + color[1]*percentDifference, color[2] + color[2]*percentDifference)
        colors.append(temp)

    return colors


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

# Returns the center of object and the enclosing circle x, y and radius of object
def getObjectSpecs(largestContour):
    try:
        ((x, y), radius) = cv2.minEnclosingCircle(largestContour)
        M = cv2.moments(largestContour)
        center = (int(M["m10"] / M["m00"]), int(M["m01"] / M["m00"]))

        # Ignores object if shape is above halfway point
        if (center[1] < frameHeight*.33):
            return None


        # approxShape = detectShape(largestContour)
        # print("Approx Shape: " + approxShape)
        return {"center" : center, "x" : x, "y" : y,"radius" : radius}
    except Exception as e: 
        print("Error in getObjectSpecs:" + str(e))
        return None

# Gets color of object
def getColorOfObject(specs, masks, colorIndex):
    mask = masks[colorIndex]    

    if (specs == None):
        return False

    value = mask[specs["center"][1], specs["center"][0]]
    if (value == 255):
        return True
    else:
        return False
    
# Loops through all contours and labels/outlines the shapes
def identifyAndLabelAllShapes(mask, frame, masks):
    cnts = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    contours = cnts[0] if imutils.is_cv2() else cnts[1]

    largestContour = None
    largestArea = 0
    approxShape = None
    largestShape = None

    # Sorts contours by size
    sortedContours = sorted(contours, key=lambda x: cv2.contourArea(x))
    sortedContours.reverse()


    # Loops through first 8 contours (largest ones, avoids small annoying artifacts)
    for contour in sortedContours:
        try:
            ((x, y), radius) = cv2.minEnclosingCircle(contour)
            M = cv2.moments(contour)
            if (int(M["m10"]) == 0 or int(M["m00"]) == 0 or int(M["m01"]) == 0):
                continue
            # print(int(M["m10"]), int(M["m00"]), int(M["m01"]), int(M["m00"]))
            center = (int(M["m10"] / M["m00"]), int(M["m01"] / M["m00"]))
            approxShape = None
            
            # Only uses object if below halfway
            # if (center[1] > frameHeight*.4):
            approxShape, aspectRatio = detectShape(contour)
            # print("Got shape", approxShape)
            area = cv2.contourArea(contour)
            specs = {"center" : center, "x" : x, "y" : y,"radius" : radius, "shape" : approxShape}

            # If area of object is less than amount, ignore it, probably an artifcat
            if (area < 75):
                continue

            # If true, ignore yellow blocks and circles
            if ((approxShape == "Block" or approxShape == "Circle") and ignoreYellow):
                if (getColorOfObject(specs, masks, 1)):
                    continue


            cv2.putText(frame, specs["shape"], (int(specs["x"])-  int(specs["radius"]), int(specs["y"])), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 2)
            cv2.putText(frame, "Area: " + str(area)[:5], (int(specs["x"]), int(specs["y"]) - 20), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 2)                
            cv2.putText(frame, str(aspectRatio)[:5], (int(specs["x"]), int(specs["y"]) - 50), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 2)
            cv2.drawContours(frame, [contour], -1, (255,255,255), 2)

            # Calculates area of contour and saves if largest and block/circle
            if (area > largestArea and (approxShape == "Block" or approxShape == "Circle") and (center[1] > frameHeight*0.5)):
                largestArea = area
                largestContour = contour
                largestShape = approxShape
                print ("Replaced smaller area " + str(area) + " with larger area " + str(largestArea))

            
                
        except Exception as e: 
            print("Error in identifyAndLabelAllShapes line " + str(sys.exc_info()[-1].tb_lineno) + ":" + str(e))

    # print("Largest shape: ", approxShape)
    print("End frame")
    return (largestContour, largestArea, largestShape)

# Detects the shape of the contour
def detectShape(contour):
    # Source: https://www.pyimagesearch.com/2016/02/08/opencv-shape-detection/
    # initialize the shape name and approximate the contour
    

    shape = "unidentified"
    peri = cv2.arcLength(contour, True)
    approx = cv2.approxPolyDP(contour, 0.04 * peri, True)
    aspectRatio = 0
    
    ((x, y), radius) = cv2.minEnclosingCircle(contour)
    M = cv2.moments(contour)
    center = (int(M["m10"] / M["m00"]), int(M["m01"] / M["m00"]))
    (x, y, w, h) = cv2.boundingRect(approx)
    area = cv2.contourArea(contour)

    if (area < 75):
        continue



    # if the shape is a triangle, it will have 3 vertices
    # if len(approx) == 3:
    #     shape = "triangle"

    # if the shape has 4 vertices, it is either a square or
    # a rectangle
    if len(approx) <= 4:

        # compute the bounding box of the contour and use the
        # bounding box to compute the aspect ratio
        # (x, y, w, h) = cv2.boundingRect(approx)
        if (h == 0):
            h = 0.0001

        if (w == 0):
            w = 0.0001

        aspectRatio = w / float(h)

        # print("Specs", area, w, h, center, frameHeight, frameWidth)


        # a square will have an aspect ratio that is approximately
        # equal to one, otherwise, the shape is a rectangle - had 'and (area < 300)'
        if (area < 300 and ((w > 2*h) or (aspectRatio > 5) or (area < w*h/5))):
            shape = "Line"
        elif (aspectRatio < 0.4):
            shape = "Corner Post"
        # elif aspectRatio >= 0.35 and aspectRatio <= 0.85 and center[0]+h/2 < frameHeight/2 or area > 10000:
        elif (area > 5000 and center[1]-h/2 < frameHeight/2):
            # print("Center y:", center[1], "Height:", h, "Frame Height:", frameHeight, "Center[1]-h/2:", center[1]-h/2)
            shape = "Center Post"
        elif (aspectRatio > 0.40):    #Was 0.6
            shape = "Block"
            if (w > frameWidth*.33):
                shape = "Line"

    # otherwise, we assume the shape is a circle
    else:
        # if (center[0]+h/2 < frameHeight/2 or area > 10000):
        if (area > 5000 and center[1]-h/2 < frameHeight/2):
            # print("Center y:", center[1], "Height:", h, "Frame Height:", frameHeight, "Center[1]-h/2:", center[1]-h/2)
            shape = "Center Post"
        elif (w > 2*h or area < w*h/5):
            shape = "Line"
            print("Line w >2*h and area < w*h/5 and area: " + str(area) + " and w and h: " + str(w) + " " + str(h))
        else:
            shape = "Circle"
            if (w > frameWidth*.33):
                print("Line greater than 1/3 the screen")
                shape = "Line"
        



    # return the name of the shape
    return shape, aspectRatio

# Gets corner post
def getCornerPosts(mask, frame):
    # Gets and sorts contours
    cnts = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    contours = cnts[0] if imutils.is_cv2() else cnts[1]

    # Sorts contours by size
    sortedContours = sorted(contours, key=lambda x: cv2.contourArea(x))
    sortedContours.reverse()

    # All Corner Posts
    cornerPosts = []

        # Loops through first 8 contours (largest ones, avoids small annoying artifacts)
    for contour in sortedContours[:8]:
        try:
            ((x, y), radius) = cv2.minEnclosingCircle(contour)
            peri = cv2.arcLength(contour, True)
            approx = cv2.approxPolyDP(contour, 0.04 * peri, True)
            (x, y, w, h) = cv2.boundingRect(approx)
            M = cv2.moments(contour)
            center = (int(M["m10"] / M["m00"]), int(M["m01"] / M["m00"]))
            approxShape = None
            
            # Only uses object if above top third
            if (center[1] < frameHeight*.6):
                approxShape, aspectRatio = detectShape(contour)
                area = cv2.contourArea(contour)
                specs = {"center" : center, "x" : x, "y" : y,"radius" : radius, "shape" : approxShape}

                # If object is over 2/3 of screen height, you've found it
                if (specs['radius']*2 > frameHeight*2/3 and specs['center'][0] > 100 and specs['center'][0] < 200):
                    specs['arrived'] = True
                else:
                    specs['arrived'] = False

                # If object is over 1/3 of screen height, for use with non primary corner post navigation
                if (specs['radius']*2 > frameHeight*1/3 ): #and specs['center'][0] > 80 and specs['center'][0] < 220):
                    specs['closeEnough'] = True
                else:
                    specs['closeEnough'] = False


                # If area of object is less than amount, ignore it, probably an artifcat
                if (area < 75):
                    continue

                # Ignores external objects entirely in the second half
                if (center[1]+h/2 < frameHeight*0.5):
                    # print("Center y:", center[1], "Height:", h, "Frame Height:", frameHeight, "Center[1]-h/2:", center[1]-h/2)
                    continue

                cv2.putText(frame, specs["shape"], (int(specs["x"])+ int(specs["radius"]), int(specs["y"])), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 2)
                cv2.putText(frame, str(area)[:5], (int(specs["x"])+ int(specs["radius"]), int(specs["y"])+ 20), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 2)                
                # cv2.putText(frame, str(aspectRatio)[:5], (int(specs["x"])+ int(specs["radius"]), int(specs["y"])+ 20), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 2)
                cv2.drawContours(frame, [contour], -1, (255,255,255), 2)

                if (approxShape == "Corner Post"):
                    cornerPosts.append(specs)
                
        except Exception as e: 
            print("Error in getCornerPosts:" + str(e))
            # print("Error: " + str(Exception))

    return cornerPosts

# Returns specs of desired color corner post
    # Given specs of all corner posts from getCornerPosts(), all masks, and color number
    # If color isn't found, get next best one - TODO
def getSpecificCornerPost(cornerPostsSpecs, masks, colorIndex):
    desiredCornerPostSpecs = None

    mask = masks[colorIndex]

    # print("Num of corner posts found:", len(cornerPostsSpecs))
    

    for specs in cornerPostsSpecs:
        try:
            if (specs == None):
                continue

            value = mask[specs["center"][1], specs["center"][0]]
            # print("Value of mask", colorIndex, " is", value, "at", specs['y'], specs['x'])
            if (value == 255):
                # print("Color is on corner post positions:", specs)
                desiredCornerPostSpecs = specs
        except:
            continue
    
    return desiredCornerPostSpecs


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

# Navigates the robot
def navigate(objectSpecs, goHome=False, chillSideThreshold = False):
    global currentPosition
    if (not goHome):
        straightSpeed = "70"
        turnSpeed = "20"
        spinSpeed = "25"
    elif (goHome):
        straightSpeed = "100"
        turnSpeed = "20"
        spinSpeed = "50"

    # Side threshold, less picky for navigating to secondary corner posts
    if (chillSideThreshold):
        sideThreshold = 1.1
    else:
        sideThreshold = 0.75

    # Navigates
    if (objectSpecs != None):
        # Tells if object is left, right, or center of screen
        # If largest object is close to bottom of screen, collect
        # print("Y pos: " , objectSpecs["center"][1], "Frame height: " , frameHeight, "Frame height*0.8", frameHeight*0.8)
        if ((int(objectSpecs["x"]) - int(objectSpecs["radius"]*sideThreshold)) <= frameWidth/2 and (int(objectSpecs["x"]) + int(objectSpecs["radius"]*sideThreshold)) >= frameWidth/2 and objectSpecs['center'][1] > frameHeight*0.9):
            print("Attempting to collect...")
            received = writeAndReadToSerial("GO forward " + straightSpeed +"@")
            time.sleep(4)
            received = writeAndReadToSerial("conveyor start@")
            conveyorTime = time.time()
            conveyorRunning = True
        elif ((int(objectSpecs["x"]) - int(objectSpecs["radius"]*sideThreshold)) <= frameWidth/2 and (int(objectSpecs["x"]) + int(objectSpecs["radius"]*sideThreshold)) >= frameWidth/2):
            if (currentPosition != "center"):
                currentPosition = "center"
                print("Its in the center")
                received = writeAndReadToSerial("GO forward " + straightSpeed +"@")
        elif ((int(objectSpecs["x"]) - int(objectSpecs["radius"])) <= frameWidth/2 and (int(objectSpecs["x"]) + int(objectSpecs["radius"])) <= frameWidth/2):
            if (currentPosition != "left"):
                currentPosition = "left"
                print("Its on the left")
                received = writeAndReadToSerial("GO left " + turnSpeed + "@")
        elif ((int(objectSpecs["x"]) - int(objectSpecs["radius"])) >= frameWidth/2 and (int(objectSpecs["x"]) + int(objectSpecs["radius"])) >= frameWidth/2):
            if (currentPosition != "right"):
                currentPosition = "right"
                print("Its on the right")
                received = writeAndReadToSerial("GO right " + turnSpeed +"@") 
    else:
        print("No object detected...spinning")
        # received = writeAndReadToSerial("GO stop@") 
        currentPosition = "spinning"
        received = writeAndReadToSerial("GO left " + spinSpeed + "@")


# TRY: Checking each color mask individually. Make a duplicate of this file



# Names the windows
cv2.namedWindow("mask")
cv2.namedWindow("frame")
# cv2.namedWindow("hsv")
# cv2.setMouseCallback("frame", updateColorRangeWhenClick)

# Current area of screen of object being tracked
currentPosition = None

# colorSavedFile = 'RaspberryPiCode/OpenCV/colorCalibration.json'
colorSavedFile = 'colorCalibration.json'
colors = getColorsFromJSON(colorSavedFile)


# Starts the camera feed
camera = PiCamera()
camera.resolution = (frameWidth, frameWidth)
camera.framerate = 32
rawCapture = PiRGBArray(camera)

# allow the camera to warmup
time.sleep(1)
fpsTimes = []

for frame in camera.capture_continuous(rawCapture, format="bgr", use_video_port=True):
    startTime = time.time()
    frame = frame.array

    # After 120 seconds, go home
    if (time.time() - overallTime > 120):
        goHome = True

    
    # Resize frame so it can be processed quicker
    startT = time.time()
    # frame = imutils.resize(frame, height=frameHeight)
    frame = cv2.resize(frame,(frameWidth, frameHeight))
    # print("Frame resize time:", time.time()-startT)

    # Convert to HSV colorspace
    startT = time.time()
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    # print("HSV convert time:", time.time()-startT)

    largestContourAndAreaAndShape = (0,0,0)

    # Gets mask for each color
    startT = time.time()
    masks = []                 # Maybe take two samples of each color, then combine them into one frame, some 4 frames but broader range of colors
    for color in colors:
        mask = cv2.inRange(hsv, color['lower'], color['upper'])
        masks.append(mask)

    # Pre combines first 2 masks assuming both are red
    masks[0] = cv2.bitwise_or(masks[0], masks[1])
    del masks[1]

    # Combines all masks 
    mask = masks[0]
    for i in range(len(masks)):
        if (i==0): continue
        mask = cv2.bitwise_or(mask, masks[i])

    # If we are just getting balls
    if (not goHome):
        startT = time.time()
        # Gives (contour, area)
        largestContourAndAreaAndShape = identifyAndLabelAllShapes(mask, frame, masks)
        # print("Gets largest Contour", time.time()-startT)

        startT = time.time()
        objectSpecs = getObjectSpecs(largestContourAndAreaAndShape[0])
        # print("Gets specs of Contour", time.time()-startT)

        # Outlines largest contour that is a shape or ball
        if (largestContourAndAreaAndShape[1] != 0 and largestContourAndAreaAndShape[2] != None):
            # print("Largest area/shape", largestContourAndAreaAndShape[1], largestContourAndAreaAndShape[2])
            cv2.drawContours(frame, [largestContourAndAreaAndShape[0]], -1, (0,0,0), 2)
    
        #  Gets rid of shape if not identified some how
        if (largestContourAndAreaAndShape[2] == None):
            objectSpecs = None


    # If we want to go home, look are corner posts
    else:
        allVisibleCornerPosts = getCornerPosts(mask, frame)
        desiredCornerPost = getSpecificCornerPost(allVisibleCornerPosts, masks, colorIndexToLookFor)
        objectSpecs = desiredCornerPost

        # Adjust desired color based on time
        if (objectSpecs != None):
            if (colorIndexToLookFor == 0 and objectSpecs['arrived'] == True):
                print("FINAL PUSH HOME!")
                received = writeAndReadToSerial("GO forward 100@")
                time.sleep(4)
                print("You have reached your destination")
                break
            elif (colorIndexToLookFor != 0 and objectSpecs['closeEnough'] == True):
                print ("CORNER POST IS CLOSE ENOUGH, LOOKING FOR PRIMARY ONE NOW")
                colorIndexToLookFor -= 1 # Look for next obj
                # cornerPostSearchTimer = colorIndexToLookFor*10 # Decrease 10 seconds from total timer
                framesWithoutCornerPost = 0 #This will also resest colorIndex to look for

            print("Found corner post")
        # Checks timer
        else:
            # Change this to frames??????
            # if (cornerPostSearchTimer == 0):
            if (framesWithoutCornerPost == 0):
                # cornerPostSearchTimer = time.time()
                colorIndexToLookFor = 0
                print("LOOKING FOR MASK NUMBER", colorIndexToLookFor)
            # elif (time.time() - cornerPostSearchTimer > 30):
            elif (framesWithoutCornerPost > 280):
                colorIndexToLookFor = 2
                print("LOOKING FOR MASK NUMBER", colorIndexToLookFor)
            # elif (time.time() - cornerPostSearchTimer > 20):
            elif (framesWithoutCornerPost > 100):
                colorIndexToLookFor = 1
                print("LOOKING FOR MASK NUMBER", colorIndexToLookFor)
            framesWithoutCornerPost += 1

    # Displays video feed
    cv2.imshow('frame', frame)
    cv2.imshow('mask', mask)

    # Closes when pressing 's'
    if cv2.waitKey(1) & 0xFF == ord('s'):
        cv2.imwrite( "finalImage.jpg", frame)
        received = writeAndReadToSerial("GO stop@") 
        break

    # Navigates to object
    startT = time.time()
    if (goHome and colorIndexToLookFor != 0):
        chillThreshold = True
    else:
        chillThreshold = False

    navigate(objectSpecs, goHome, chillThreshold)
    
    startT = time.time()
    totalTime = time.time() - startTime
    fpsTimes.append(totalTime)
    rawCapture.truncate(0)

    if (conveyorRunning == True and time.time() - conveyorTime > 9.5):
        received = writeAndReadToSerial("conveyor stop@")
        conveyorTime = 0
        conveyorRunning = False


    if sum(fpsTimes) >= 1:
        print("FPS:", len(fpsTimes))
        fpsTimes = []

    

received = writeAndReadToSerial("GO stop@") 
# Closes all windows opened
# camera.release()
# outputVideo.release()
#camera.destroyAllWindows()