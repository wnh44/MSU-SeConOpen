#ifndef AllSensors_h
#define AllSensors_h

#include <SPI.h>
#include <Wire.h>

class AllSensors {
    private:
    void setupliDAR1();
    void setupliDAR2();
    void setupUltraSonic();
    int trigPin;
    int echoPin;

    public:

    AllSensors(bool liDAR1, bool liDAR2, int trigPin, int echoPin);
    float scanliDAR();
    float scanUltraSonic();

};