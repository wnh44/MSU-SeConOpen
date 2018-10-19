/*
  ColorSensor.cpp - Custom library for the Adafruit_TCS34725
    color sensor. The wiring diagram can be found at 
    dfrobot.com.

    Make sure the Adafruit_TCS34725 library is installed.
*/
#include "Arduino.h"
#include "ColorSensor.h"
#include <Wire.h>
#include "Adafruit_TCS34725.h"

// Sets up device w/ I2C
void ColorSensor::setup(){
    // Connects to the I2C device
    tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

    // If it doesn't connect correctly, print error message and loop endlessly
    if (!tcs.begin()){
        Serial.println("Color sensor not setup correctly");
        while(1);
    }
}


// Returns string of the color in hex
String ColorSensor::getHexColor(){
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


// Converts hex to rbg and tells if red, green, blue or yellow
String ColorSensor::getObstacleColor(){
    // Gets hex string and converts to hex
    String hexColor = getHexColor();
    unsigned long red = strtoul(hexColor.substring(0,2).c_str(), NULL, 16);
    unsigned long green = strtoul(hexColor.substring(2,4).c_str(), NULL, 16);
    unsigned long blue = strtoul(hexColor.substring(4,6).c_str(), NULL, 16);
    
    // Checks for each color
    if (red > 110 && green < 80 && blue < 80){
        return "red";
    }else if (green > 110 && blue < 80 && red < 80){
        return "green";
    }else if (blue > 110 && red < 50 && green < 95){
        return "blue";
    }else if (red > 90 && green > 90 && blue < 60){
        return "yellow";
    }else {
        return "unknown";
    }
}


float *ColorSensor::getRGBColorArray(){
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
