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
    void setupUltraSonic1();
    void setupUltraSonic2();
    int trigPin1;
    int echoPin1;
    int trigPin2;
    int echoPin2;
    Adafruit_TCS34725 tcs;
    void setupColorSensor();

public:

    AllSensors(bool liDAR1, bool liDAR2, int trigPin1, int echoPin1, int trigPin2, int echoPin2, bool colorSensor);
    float scanliDAR1();
    float scanliDAR2();
    float scanUltraSonic1();
    float scanUltraSonic2();
    String getHexColor();
    String getObstacleColor();
    float *getRGBColorArray();

};

#endif