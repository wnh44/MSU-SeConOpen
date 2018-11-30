#include "AllSensors.h"
using namespace std;

AllSensors::AllSensors(bool liDAR1, bool liDAR2, int trigPin1, int echoPin1, int trigPin2, int echoPin2, bool colorSensor) {
    this->trigPin1 = trigPin1;
    this->echoPin1 = echoPin1;
    this->trigPin2 = trigPin2;
    this->echoPin2 = echoPin2;
    if (liDAR1) {
        setupliDAR1();
    }
    if (liDAR2) {
        setupliDAR2();
    }
    if (trigPin1 != 0 && echoPin1 != 0) {
        setupUltraSonic1();
    }
    if (trigPin2 != 0 && echoPin2 != 0) {
        setupUltraSonic2();
    }
    if (colorSensor) {
        setupColorSensor();
    }

}

void AllSensors::setupColorSensor() {
    // Connects to the I2C device
    this->tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

    // If it doesn't connect correctly, print error message and loop endlessly
    if (!this->tcs.begin()){
        Serial.println("Color sensor not setup correctly");
        while(1);
    }
}

String AllSensors::getHexColor() {
    uint16_t clear, red, green, blue;
    // Turns on the LED
    tcs.setInterrupt(false);
    // takes 50ms to read
    delay(60);
    // Gets data from sensor and stores in the 4 variables
    tcs.getRawData(&red, &green, &blue, &clear);
    // Turns off the LED
    tcs.setInterrupt(true);

    // Calculates RGB values
    uint32_t sum = clear;
    float r, g, b;
    r = red; r /= sum;
    g = green; g /= sum;
    b = blue; b /= sum;
    r *= 256; g *= 256; b *= 256;

    // Converts the rgb value to hex
    String hexColor = String((int)r, HEX) + String((int)g, HEX) + String((int)b, HEX);

    // Returns the hex value
    return hexColor;
}

String AllSensors::getObstacleColor() {
    // Gets hex string and converts to hex
    String hexColor = getHexColor();
//    unsigned long red = strtoul(hexColor.substring(0,2).c_str(), NULL, 16);
//    unsigned long green = strtoul(hexColor.substring(2,4).c_str(), NULL, 16);
//    unsigned long blue = strtoul(hexColor.substring(4,6).c_str(), NULL, 16);

    // Checks for each color
//    if (red > 110 && green < 80 && blue < 80){
//        return "red";
//    }else if (green > 110 && blue < 80 && red < 80){
//        return "green";
//    }else if (blue > 110 && red < 50 && green < 95){
//        return "blue";
//    }else if (red > 90 && green > 90 && blue < 60){
//        return "yellow";
//    }else {
//        return "unknown";
//    }
    String returnstring = "";

    if ((hexColor[0] == '6' || hexColor[0] == '5') &&
        (hexColor[0] == '6' || hexColor[1] == '9' || hexColor[1] == '8' || hexColor[1] == 'a' || hexColor[1] == 'b' || hexColor[1] == 'c' || hexColor[1] == 'd' || hexColor[1] == 'e' || hexColor[1] == 'f') &&
        (hexColor[2] == '5' || hexColor[2] == '4') &&
        (hexColor[4] == '4' || hexColor[4] == '5')) {
        returnstring += hexColor;
        returnstring += " is carpet";
    }
    else if ((hexColor[0] == '5' || hexColor[0] == '4') &&
             (hexColor[0] == '4' || hexColor[1] == '0' || hexColor[1] == '1' || hexColor[1] == '2' || hexColor[1] == '3' || hexColor[1] == '4' || hexColor[1] == '5' || hexColor[1] == '6' || hexColor[1] == '7') &&
             (hexColor[2] == '4' || hexColor[2] == '5') &&
             (hexColor[4] == '4' || hexColor[4] == '5')) {
        returnstring += hexColor;
        returnstring += " is white tape";
    } else if ((hexColor[0] == '7' || hexColor[0] == '8') &&
               (hexColor[2] == '4' || hexColor[2] == '3') &&
               (hexColor[4] == '4' || hexColor[4] == '3')) {
        returnstring += hexColor;
        returnstring += " is red tape";
    } else if ((hexColor[0] == '5' || hexColor[0] == '4') &&
               (hexColor[2] == '6' || hexColor[2] == '7')&&
               (hexColor[4] == '4')) {
        returnstring += hexColor;
        returnstring += " is green tape";
    } else if (hexColor[0] == '4' && hexColor[2] == '5' && hexColor[4] == '6') {
        returnstring += hexColor;
        returnstring += " is blue tape";
    }else if (hexColor[0] == '6' && hexColor[2] == '5' && hexColor[4] == '3') {
        returnstring += hexColor;
        returnstring += " is yellow tape";
    }
    else {
        returnstring += hexColor;
        returnstring += " is unknown";
    }
    return returnstring;
}

float *AllSensors::getRGBColorArray() {
    uint16_t clear, red, green, blue;
    // Turns on the LED
    tcs.setInterrupt(false);
    // takes 50ms to read
    delay(60);
    // Gets data from sensor and stores in the 4 variables
    tcs.getRawData(&red, &green, &blue, &clear);
    // Turns off the LED
    tcs.setInterrupt(true);

    // Calculates RGB values
    uint32_t sum = clear;
    float r, g, b;
    r = red; r /= sum;
    g = green; g /= sum;
    b = blue; b /= sum;
    r *= 256; g *= 256; b *= 256;

    // Stores in a float array
    float *colorArray = new float[3];
    colorArray[0] = r;
    colorArray[1] = g;
    colorArray[2] = b;

    // Returns the pointer to the array
    // ***** Make sure to delete the pointer when done!!! *****
    return colorArray;
}

void AllSensors::setupUltraSonic1() {
    if (this->trigPin1 == 0 || this->echoPin1 == 0) {
        Serial.println("Cannot setup UltraSonic Sensor 1 with given pins");
    }
    //Ultrasonic setup
    pinMode(this->trigPin1, OUTPUT);
    pinMode(this->echoPin1, INPUT);
    Serial.begin(9600);
}

void AllSensors::setupUltraSonic2() {
    if (this->trigPin2 == 0 || this->echoPin2 == 0) {
        Serial.println("Cannot setup UltraSonic Sensor 2 with given pins");
    }
    //Ultrasonic setup
    pinMode(this->trigPin2, OUTPUT);
    pinMode(this->echoPin2, INPUT);
    Serial.begin(9600);
}

void AllSensors::setupliDAR1() {
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

void AllSensors::setupliDAR2() {
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

float AllSensors::scanliDAR1() {
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

float AllSensors::scanliDAR2() {
    float liDARval = 0;
    while(Serial2.available()>=9)
    {
        if((0x59 == Serial2.read()) && (0x59 == Serial2.read())) // byte 1 and byte 2
        {
            unsigned int t1 = Serial2.read(); // byte 3 = Dist_L
            unsigned int t2 = Serial2.read(); // byte 4 = Dist_H
            t2 <<= 8;
            t2 += t1;
            liDARval = t2;
            t1 = Serial2.read(); // byte 5 = Strength_L
            t2 = Serial2.read(); // byte 6 = Strength_H
            t2 <<= 8;
            t2 += t1;
            for(int i=0; i<3; i++)Serial2.read(); // byte 7, 8, 9 are ignored
        }
    }
    return liDARval;
}

float AllSensors::scanUltraSonic1() {
    long duration;
    float distance = 0;
    digitalWrite(this->trigPin1, LOW);           //Set to low for 2 ms
    delayMicroseconds(2);

    digitalWrite(this->trigPin1, HIGH);          //Set to high for 10 ms, then back to low
    delayMicroseconds(10);
    digitalWrite(this->trigPin1, LOW);

    duration = pulseIn(this->echoPin1, HIGH);    //Save the duration as the length of the pulse where echopin reads high
    distance = duration * 0.034/2;        //distance is a function of the duration
    return distance;
}

float AllSensors::scanUltraSonic2() {
    long duration;
    float distance = 0;
    digitalWrite(this->trigPin2, LOW);           //Set to low for 2 ms
    delayMicroseconds(2);

    digitalWrite(this->trigPin2, HIGH);          //Set to high for 10 ms, then back to low
    delayMicroseconds(10);
    digitalWrite(this->trigPin2, LOW);

    duration = pulseIn(this->echoPin2, HIGH);    //Save the duration as the length of the pulse where echopin reads high
    distance = duration * 0.034/2;        //distance is a function of the duration
    return distance;
}