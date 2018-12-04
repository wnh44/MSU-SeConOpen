#####################################################################
# Name: serialCommunication.py
# 
# Authors:  Patrick Bell
# Creation Date: 10-26-18
# Update Date:
# 
# Description:
#####################################################################


import serial
import RPi.GPIO as GPIO
import time
import sys

serialPath = "/dev/ttyACM" + sys.argv[1]

# Sets up serial device
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

keepGoing = True

while(keepGoing):
    dataToSend = raw_input("> ")
    if (dataToSend == "quit"):
        keepGoing = False
    received = writeAndReadToSerial(dataToSend)
    print("Received: " + received)
