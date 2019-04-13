import cv2
import numpy as np
import imutils
import sys
import json
import time
import serial
import sys



# Sets up serial
serialPath = "/dev/ttyACM" + sys.argv[1]
g_SER=serial.Serial(serialPath,9600)  # Change ACM number as found from ls /dev/tty/ACM*
g_SER.baudrate=9600


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

time.sleep(2)

received = writeAndReadToSerial("conveyor forward")
time.sleep(4.75)
received = writeAndReadToSerial("conveyor backward")
time.sleep(4.6)
received = writeAndReadToSerial("conveyor stop")
