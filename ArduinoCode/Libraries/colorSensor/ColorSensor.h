/*
  ColorSensor.h - Custom library for the Adafruit_TCS34725
    color sensor. The wiring diagram can be found at 
    dfrobot.com.

    Make sure the Adafruit_TCS34725 library is installed.
        
*/
#ifndef ColorSensor_h
#define ColorSensor_h

#include "Arduino.h"
#include <Wire.h>
#include "Adafruit_TCS34725.h"


class ColorSensor{
    private:
    Adafruit_TCS34725 tcs;

    public:
    void setup();
    String getHexColor();
    String getObstacleColor();
    float* getRGBColorArray();
};

#endif