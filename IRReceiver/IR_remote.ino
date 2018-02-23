/*******************************************************************************************
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
 *******************************************************************************************/

 

//OLED Screen Setup////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4                      //connect RST pin on OLED to pin 4 on arduino

Adafruit_SSD1306 display(OLED_RESET);

//for IR input/output///////////////////////////////////////////////////////////////////////////////////////////////////////

#include "IRLibAll.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IRrecvPCI myReceiver(2);    //Connect IRreceive pin (leftmost when facing you) to pin 2 on arduino
IRdecode myDecoder;         //Creates IRdecoder object
IRsend mySender;            //Creates IRsender object
int8_t codeProtocol;                //8bit int for code protocal (eg. NEC, SONY, UNKNOWN)
uint32_t RcodeValue;                //32bit int for received code
uint8_t RcodeBits;                  //8bit int for length of received code
int j = 0;                          //incrementer variable
uint32_t sendcode;                  //32bit int for send code
String displaycode = "";            //empty string for OLED display of code



void setup() {
  //////////////////////////Display setup///////////////////////////////////////
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);              //Start the OLED display (0x3C might have to change with different setups)
  delay(2000);                                            //Delay for setup
  display.clearDisplay();                                 //Set starting settings
  display.setTextSize(3);                                      
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print("No code");
  display.display();                                      //Must display.display() at the end of every block you want to display to actually do it
  
  ////////////////////////////IR////////////////////////////////////////////////
  
  Serial.begin(9600);                                     //Begin serial at 9600 baud
  delay(2000);                                            //Delay for setup
  myReceiver.enableIRIn();                                //Enable receiver
  Serial.println(F("Ready to receive IR signals"));
  
  ///////////////////////////Random Seed///////////////////////////////////////////////////

  randomSeed(analogRead(12));                             //Sets the random seed to noise from unconnected pin 12 on arduino
  
}

void loop() {
  
  ////////////////////////Make Code to Send///////////////////////////////////////
  //sendcode = random(8);       //Random code
  
  sendcode = j;                 //Incrementing code each loop
  j++;                          //
  if (j==8) j = 0;              //
  
  Serial.println(sendcode, BIN);        //Displays code in Serial Monitor
  
  //////////////////////////////IR Receive//////////////////////////////////////////////
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
      
      display.clearDisplay();                       //Displays received code string
      display.setCursor(0, 0);
      display.print(displaycode);
      display.display();
    }

        
    myReceiver.enableIRIn();                    //Re-enables IR sensor
  }
  
  ////////////////////////////////IR Send///////////////////////////////////////
  delay(500);
  mySender.send(NEC, sendcode,0);           //Send with NEC protocol the sendcode (i don't know what the last parameter is for)
}




