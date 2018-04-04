/***********************************************************************************************
 * 
 * Another serial would have to be added to read to the computer instead of an 
 * OLED screen
 * 
 * Ultrasonic Vcc connected to 5V, Trig to Arduino pin 9, Echo to Arduino
 * pin 10, Gnd to arduino ground
 * 
 * liDAR: Black (leftmost when facing) connected to arduino ground, Red
 * connected to 5V, white connected to TXO->1 for communication from Arduino
 * to sensor (both connected to same pin), green connected to RX1 or RX2 
 * (one for each sensor)
 * 
 * OLED: SDA connected to Arduino SDA 20, SCL connected to Arduino SCL 21,
 * RST connected to Arduino pin 9, GND connected to arduino ground, 3.3V
 * connected to Arduino 3.3V, VIN connected to Arduino 5V
 * 
 * Power usage seems to necessitate connection to power through a USB cord
 * instead of an actual power cord
 * 
 * Testing ongoing, but liDARs freak out sometimes with different colors 
 * (specifically black) and Ultrasonic not quite as accurate
 *
 ***********************************************************************************************
 *
 * Just point the IR blaster at the sensor and it should pick it up
 * and display on the OLED
 * 
 * IR blaster should be connected to pin 9 on arduino with about 470 OHM
 * resister connecting it to ground
 * 
 * IR receiver pins are [Output, Ground, 5V] when it is facing you
 * 
 * OLED: SDA connected to SDA 20 on Mega, SCL connected to SCL 21,
 * RST connected to pin 4, GND connected to ground, 3.3V connected to
 * 3.3V, and VIN connected to 5V
 *
 ***********************************************************************************************/
 

//OLED Screen Setup//////////////////////////////////////
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4            //Connect OLED RST pin to pin 4 on arduino
Adafruit_SSD1306 display(OLED_RESET);

//Ultrasonic Setup///////////////////////////////////////
const int trigPin = 9;          //Connect US trig to arduino pin 9
const int echoPin = 10;         //Connect US echo to arduino pin 10

long duration;                  //Initialized variables
int distance;

//LiDAR Setup////////////////////////////////////////////
volatile float liDARval = 0;    //Initialized variables
volatile float liDARvalb = 0;
volatile float total = 0;

//for IR input/output////////////////////////////////////
#include "IRLibAll.h"

IRrecvPCI myReceiver(2);    //Connect IRreceive pin (leftmost when facing you) to pin 2 on arduino
IRdecode myDecoder;         //Creates IRdecoder object
IRsend mySender;            //Creates IRsender object
int8_t codeProtocol;                //8bit int for code protocal (eg. NEC, SONY, UNKNOWN)
uint32_t RcodeValue;                //32bit int for received code
uint8_t RcodeBits;                  //8bit int for length of received code
String displaycode = "";            //empty string for OLED display of code


/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
    Serial.begin(9600)          //Serial for IR and ultrasonic
    Serial1.begin(115200);      //HW Serial for TFmini
    Serial2.begin(115200);      //For second TFmini
    delay (2000);               //Give a little time for things to start
    
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
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);      //0x3C might have to change
    delay(2000);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);


    ///Ultrasonic setup
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    
    //IR Receiver Setup
    myReceiver.enableIRIn();                                //Enable receiver
 
}

void loop() {
  //////LiDAR Sensors////////////////////////////////////////////////////////////////////////////////////////
  delay(100);               // Don't want to read too often as TFmini samples at 100Hz

  LidarRead();
    
  //////Ultrasonic Sensors////////////////////////////////////////////////////////////////////////////////////////
  
  distance = UltrasonicRead();
  
  //////Calculations and Output////////////////////////////////////////////////////////////////////////////////////////
  
  DisplayLidarUltrasonic();
 
  //////////IR Sensor/////////////////////////////////////////////////////////////////////////////////////////////
 
  IRRead();                       //Calls IR Read function (defined below)
  DisplayIRCode();
  
}

void DisplayIRCode() {
 display.clearDisplay();         //Displays received code string
 display.setCursor(0, 0);
 display.print(displaycode);
 display.display();
}

int UltrasonicRead() {
 int distance = 0;
 long duration = 0;
 digitalWrite(trigPin, LOW);           //Set to low for 2 ms
 delayMicroseconds(2);

 digitalWrite(trigPin, HIGH);          //Set to high for 10 ms, then back to low
 delayMicroseconds(10);
 digitalWrite(trigPin, LOW);

 duration = pulseIn(echoPin, HIGH);    //Save the duration as the length of the pulse where echopin reads high
 distance = duration * 0.034/2;        //distance is a function of the duration
}

void DisplayLidarUltrasonic() {
 display.clearDisplay();         //Clears the display
 display.setCursor(0, 0);

 display.print("1) ");           //First liDAR sensor
 display.print(liDARval);
 display.print("\n");

 display.print("2) ");           //Second liDAR sensor
 display.print(liDARvalb);
 display.print("\n");

 total = liDARval + liDARvalb;

 display.print("Total: ");       //Total of both
 display.print(total);
 display.print("\n");

 display.print("Ultrasonic: ");  //Ultrasonic sensor
 display.print(distance);
 display.print("\n");


 display.display();              //Actually displays all of the above
}

void LidarRead() {
 //I just basically copied the below code from an example from somewhere
  while(Serial1.available()>=9)  {
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
}

void IRRead() {
 if (myReceiver.getResults()) {                //If the receiver reads something
    myDecoder.decode();                         //Decode it
    codeProtocol = myDecoder.protocolNum;       //Gets protocal (eg. NEC, UNKNOWN)
    if (codeProtocol!=UNKNOWN) {                //As long as it didnt read some weird data
      Serial.print(F("Received "));                 //Print "Received NEC (or sony) Value: 0b[received code in binary]
      Serial.print(Pnames(codeProtocol));           
      RcodeValue = myDecoder.value;                  
      RcodeBits = myDecoder.bits;                    
      Serial.print(F(" Value:0b"));
      Serial.println(RcodeValue, BIN);

      displaycode = String(RcodeValue, BIN);        //Makes received code a string
      while (displaycode.length() < 3) {            //Extends the string with 0s
        displaycode = "0" + displaycode;
      }
    } 
    myReceiver.enableIRIn();                    //Re-enables IR sensor
  }
}
