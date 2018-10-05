import serial
import RPi.GPIO as GPIO
import time

# Sets up serial device
ser=serial.Serial("/dev/ttyACM0",9600)  #change ACM number as found from ls /dev/tty/ACM*
ser.baudrate=9600

while True:
	# Reads input every 1 second(s) and prints to screen
	read_ser=ser.readline()
	print(read_ser)
	time.sleep(1)