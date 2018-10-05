import serial
import RPi.GPIO as GPIO
import time

# Sets up serial device
ser=serial.Serial("/dev/ttyACM0",9600)  #change ACM number as found from ls /dev/tty/ACM*
ser.baudrate=9600

while True:
	# Gets input from user
	command = raw_input("> ")
	print("Sending: " + command)

	# If input is 'quit', it closes the connection
	if (command == 'quit'):
		ser.close()
		break

	# Writes to serial
	ser.write(command)