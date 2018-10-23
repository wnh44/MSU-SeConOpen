/*
 * Library to use Lidar, Ultrasonic, and Color Sensors
 */

#ifndef AllSensors_h
#define AllSensors_h

#include <SPI.h>
#include <Wire.h>
#include "Arduino.h"
#include "Adafruit_TCS34725.h"

class AllSensors {
    private:
    void setupliDAR1();
    void setupliDAR2();
    void setupUltraSonic();
    int trigPin;
    int echoPin;
    Adafruit_TCS34725 tcs;
    void setupColorSensor();

    public:

    AllSensors(bool liDAR1, bool liDAR2, int trigPin, int echoPin, bool colorSensor);
    float scanliDAR();
    float scanUltraSonic();
    String getHexColor();
    String getObstacleColor();
    float *getRGBColorArray();

};

#endif