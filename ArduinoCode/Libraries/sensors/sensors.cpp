#include "sensors.h"
using namespace std;

sensors::sensors(bool liDAR1, bool liDAR2, int trigPin, int echoPin) {
    this->trigPin = trigPin;
    this->echoPin = echoPin;
    if (liDAR1) {
        setupliDAR1();
    }
    if (liDAR2) {
        setupliDAR2();
    }
    if (trigPin != 0 && echoPin != 0) {
        setupUltraSonic();
    }
}

void sensors::setupUltraSonic() {
    if (this->trigPin == 0 || this->echoPin == 0) {
        Serial.println("Cannot setup UltraSonic Sensor with given pins");
    }
    //Ultrasonic setup
    pinMode(this->trigPin, OUTPUT);
    pinMode(this->echoPin, INPUT);
    Serial.begin(9600);
}

void sensors::setupliDAR1() {
    Serial1.begin(115200);      // HW Serial for TFmini
    delay(100);
    // Set to Standard Output mode
    Serial1.write(0x42);
    Serial1.write(0x57);
    Serial1.write(0x02);
    Serial1.write(0x00);
    Serial1.write(0x00);
    Serial1.write(0x00);
    Serial1.write(0x01);
    Serial1.write(0x06);
}

void sensors::setupliDAR2() {
    Serial2.begin(115200);      // HW Serial for TFmini
    delay(100);
    // Set to Standard Output mode
    Serial2.write(0x42);
    Serial2.write(0x57);
    Serial2.write(0x02);
    Serial2.write(0x00);
    Serial2.write(0x00);
    Serial2.write(0x00);
    Serial2.write(0x01);
    Serial2.write(0x06);
}

float sensors::scanliDAR() {
    float liDARval = 0;
    while(Serial1.available()>=9)
    {
        if((0x59 == Serial1.read()) && (0x59 == Serial1.read())) // byte 1 and byte 2
        {
            unsigned int t1 = Serial1.read(); // byte 3 = Dist_L
            unsigned int t2 = Serial1.read(); // byte 4 = Dist_H
            t2 <<= 8;
            t2 += t1;
            liDARval = t2;
            t1 = Serial1.read(); // byte 5 = Strength_L
            t2 = Serial1.read(); // byte 6 = Strength_H
            t2 <<= 8;
            t2 += t1;
            for(int i=0; i<3; i++)Serial1.read(); // byte 7, 8, 9 are ignored
        }
    }
    return liDARval;
}

float sensors::scanUltraSonic() {
    long duration;
    float distance = 0;
    digitalWrite(this->trigPin, LOW);           //Set to low for 2 ms
    delayMicroseconds(2);

    digitalWrite(this->trigPin, HIGH);          //Set to high for 10 ms, then back to low
    delayMicroseconds(10);
    digitalWrite(this->trigPin, LOW);

    duration = pulseIn(this->echoPin, HIGH);    //Save the duration as the length of the pulse where echopin reads high
    distance = duration * 0.034/2;        //distance is a function of the duration
    return distance;
}