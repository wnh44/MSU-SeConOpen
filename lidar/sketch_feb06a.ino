//OLED Screen Setup//////////////////////////
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);
//////////////////////////////////////////

//Ultrasonic Setup
const int trigPin = 9;
const int echoPin = 10;
long duration;
int distance;
/////////////////////////////////////////////////

//LiDAR Setup
volatile float liDARval = 0;
volatile float liDARvalb = 0;
volatile float total = 0;
/////////////////////////////////////////////////

void setup() {
    Serial1.begin(115200); // HW Serial for TFmini
    //Serial.begin(115200); // Serial output through USB to computer
    Serial2.begin(115200); //For second TFmini
    delay (100); // Give a little time for things to start
    
    Serial.println("Sending start code:");
    // Set to Standard Output mode
    Serial1.write(0x42);
    Serial1.write(0x57);
    Serial1.write(0x02);
    Serial1.write(0x00);
    Serial1.write(0x00);
    Serial1.write(0x00);
    Serial1.write(0x01);
    Serial1.write(0x06);

    
    Serial2.write(0x42);
    Serial2.write(0x57);
    Serial2.write(0x02);
    Serial2.write(0x00);
    Serial2.write(0x00);
    Serial2.write(0x00);
    Serial2.write(0x01);
    Serial2.write(0x06);

    ///Display setup
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    delay(2000);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);


    ///Ultrasonic setup
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    Serial.begin(9600);
}

void loop() {
  //////LiDAR Sensors////////////////////////////////////////////////////////////////////////////////////////
  delay(100);  // Don't want to read too often as TFmini samples at 100Hz
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
    delay(50);
    
    while(Serial2.available()>=9) // When at least 9 bytes of data available (expected number of bytes for 1 signal), then read
    {
      if((0x59 == Serial2.read()) && (0x59 == Serial2.read())) // byte 1 and byte 2
      {
        unsigned int t1b = Serial2.read(); // byte 3 = Dist_L
        unsigned int t2b = Serial2.read(); // byte 4 = Dist_H
        t2b <<= 8;
        t2b += t1b;
        liDARvalb = t2b;
        t1b = Serial2.read(); // byte 5 = Strength_L
        t2b = Serial2.read(); // byte 6 = Strength_H
        t2b <<= 8;
        t2b += t1b;
        for(int i=0; i<3; i++)Serial2.read(); // byte 7, 8, 9 are ignored
      }
    }
  //////Ultrasonic Sensors////////////////////////////////////////////////////////////////////////////////////////
  
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034/2;
  
  //////Calculations and Output////////////////////////////////////////////////////////////////////////////////////////

  //calculate more accurate distance
  //liDARval = liDARval * 1.164 - 2.192;
  //liDARvalb = liDARvalb * 1.164 - 2.192;
  
  display.clearDisplay();
  display.setCursor(0, 0);
  //Serial.print(liDARval);
  display.print("1) ");
  display.print(liDARval);
  display.print("\n");
  //Serial.print("\t");
  //Serial.print(liDARvalb);
  display.print("2) ");
  display.print(liDARvalb);
  display.print("\n");
  //Serial.print("\tcentimeters\n");
  total = liDARval + liDARvalb;
  
  /**liDARval *= 0.0328084;
  Serial.print(liDARval);
  Serial.print("\t");
  liDARvalb *= 0.0328084;
  Serial.print(liDARvalb);
  Serial.print("\tfeet\n");
  
  liDARval *= 12;
  Serial.print(liDARval);
  Serial.print("\t");
  liDARvalb *= 12;
  Serial.print(liDARvalb);
  Serial.print("\tinches\n");
  **/
  //Serial.print(total);
  //Serial.print("\ttotal centimeters\n");
  display.print("Total: ");
  display.print(total);
  display.print("\n");

  display.print("Ultrasonic: ");
  display.print(distance);
  display.print("\n");
  display.display();
  
}

