import RPi.GPIO as GPIO
import subprocess

def button_callback1(channel):
    print("Running Main.")
    subprocess.call("python shapeDetectionNavigation_overlapColors_pi2.py 0 h 3", shell=True)

def button_callback2(channel):
    print("Running Calibration.")
    subprocess.call("python colorCalibrationManual_pi.py", shell=True)

GPIO.setwarnings(False)
GPIO.setmode(GPIO.BOARD)
GPIO.setup(10, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
GPIO.setup(12, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
GPIO.add_event_detect(10,GPIO.RISING, callback=button_callback1)
GPIO.add_event_detect(12,GPIO.RISING, callback=button_callback2)

message = input("Press enter to quit\n\n")  # Run until someone presses enter
GPIO.cleanup()
