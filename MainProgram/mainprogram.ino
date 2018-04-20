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
const int trigPinFront = 9;          //Connect US trig to arduino pin 9
const int echoPinFront = 10;         //Connect US echo to arduino pin 10
const int trigPinLeft = 11;          
const int echoPinLeft = 12;         
const int trigPinBack = 13;         
const int echoPinBack = 14;         
const int trigPinRight = 15;          
const int echoPinRight = 16;         

long duration;                  //Initialized variables
float distance;

//LiDAR Setup////////////////////////////////////////////
volatile float liDARval = 0;    //Initialized variables
volatile float liDARvalb = 0;

//for IR input/output////////////////////////////////////
#include "IRLibAll.h"

IRrecvPCI myReceiver(2);    //Connect IRreceive pin (leftmost when facing you) to pin 2 on arduino
IRdecode myDecoder;         //Creates IRdecoder object
IRsend mySender;            //Creates IRsender object
int8_t codeProtocol;                //8bit int for code protocal (eg. NEC, SONY, UNKNOWN)
uint32_t RcodeValue;                //32bit int for received code
uint8_t RcodeBits;                  //8bit int for length of received code
String displaycode = "";            //empty string for OLED display of code

//Button Setup////////////////////////////////////////////
const int buttonPin = 8;
int buttonstate = 0;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
String IRRead(void) {
 if (myReceiver.getResults()) {                //If the receiver reads something
    myDecoder.decode();                         //Decode it
    codeProtocol = myDecoder.protocolNum;       //Gets protocal (eg. NEC, UNKNOWN)
    if (codeProtocol!=UNKNOWN) {                //As long as it didnt read some weird data
      Serial.print(F("Received "));                 //Print "Received NEC (or sony) Value: 0b[received code in binary]
      Serial.print(Pnames(codeProtocol));           
      RcodeValue = myDecoder.value;                  
      RcodeBits = myDecoder.bits;                    
      Serial.print(F(" Value:0x"));
      Serial.println(RcodeValue, HEX);

      displaycode = String(RcodeValue, HEX);        //Makes received code a string
      while (displaycode.length() < 3) {            //Extends the string with 0s
        displaycode = "0" + displaycode;
      }
    } 
    myReceiver.enableIRIn();                    //Re-enables IR sensor
    return displaycode;
  }
  else {
    return "null";
  }
}

void displayText(String stringa) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(stringa);
  display.display();
}

float UltrasonicRead(String direction) {
 if (direction == "front") {
  trigPin = trigPinFront;
  echoPin = echoPinFront;
 }
 else if (direction == "back") {
  trigPin = trigPinBack;
  echoPin = echoPinBack;
 }
 else if (direction == "left") {
  trigPin = trigPinLeft;
  echoPin = echoPinLeft;
 }
 else if (direction == "right") {
  trigPin = trigPinRight;
  echoPin = echoPinRight;
 }
 digitalWrite(trigPin, LOW);           //Set to low for 2 ms
 delayMicroseconds(2);

 digitalWrite(trigPin, HIGH);          //Set to high for 10 ms, then back to low
 delayMicroseconds(10);
 digitalWrite(trigPin, LOW);

 duration = pulseIn(echoPin, HIGH);    //Save the duration as the length of the pulse where echopin reads high
 distance = duration * 0.034/2;        //distance is a function of the duration
 return distance;
}

//State Machine///////////////////////////////////////////////////////////////////////////////////

typedef enum {
  STATE_IR_READ,
  STATE_A_BUTTON,
  STATE_A_CENTER,
  STATE_RAMP_DOWN,
  STATE_B_BUTTON,
  STATE_TREASURE,
  STATE_B_CENTER,
  STATE_RAMP_UP,
  STATE_C_BUTTON
} state_t;

String state_names[] = {
  "STATE_IR_READ - Read and display IR code",
  "STATE_A_BUTTON - Press correct button A",
  "STATE_A_CENTER - Center in upper area",
  "STATE_RAMP_DOWN - Move down ramp",
  "STATE_B_BUTTON - Press correct pressure plate B",
  "STATE_TREASURE - Push treasure box",
  "STATE_B_CENTER - Center in lower area",
  "STATE_RAMP_UP - Move up ramp",
  "STATE_C_BUTTON - Press correct button C"
};

void print_state(String e_state_string) {
  static String e_last_state = "null";
  if (e_state_string != e_last_state) {
    e_last_state = e_state_string;
    Serial.println(e_state_string);
  }
}

void update_state(void) {
  static state_t e_state = STATE_IR_READ;
  static bool irreadflag = 0;
  static int irreadnumber = 0;
  static String irreads[] = {"null","null","null","null"};
  static String irreadcurrent = "null";
  
  switch(e_state) {
    case STATE_IR_READ:
      irreadcurrent = IRRead();
      if (irreadcurrent != "null") {
        irreads[irreadnumber] = irreadcurrent;
        irreadnumber++;
        if (irreadnumber == 4) {
          irreadnumber = 0;
        }
      }
      /*
      Serial.print("irreads: [");
      for (int i = 0; i < 3; i++) {
        Serial.print("\"");
        Serial.print(irreads[i]);
        Serial.print("\"");
        Serial.print(", ");
      }
      Serial.print("\"");
      Serial.print(irreads[3]);
      Serial.print("\"]");
      Serial.print("\n");
      */
      if ((irreads[0] != "null" && irreads[0] == irreads[1]) ||
          (irreads[0] != "null" && irreads[0] == irreads[2]) ||
          (irreads[0] != "null" && irreads[0] == irreads[3]) ||
          (irreads[1] != "null" && irreads[1] == irreads[2]) ||
          (irreads[1] != "null" && irreads[1] == irreads[3]) ||
          (irreads[2] != "null" && irreads[2] == irreads[3])) {
        irreadflag = 1;
      }
      if (irreadflag == 1) {
        irreadflag = 0;
        displayText(irreadcurrent);
        delay(1000);
        e_state = STATE_A_BUTTON;
        for (int i = 0; i < 4; i++){
          irreads[i] = "null";
        }
      }
      break;
    case STATE_A_BUTTON:
      if (digitalRead(8) == HIGH) {
        e_state = STATE_A_CENTER;
      }
      break;
    case STATE_A_CENTER:
      
      if (digitalRead(8) == LOW) {
        e_state = STATE_RAMP_DOWN;
      }
      break;
    case STATE_RAMP_DOWN:
      if (digitalRead(8) == HIGH) {
        e_state = STATE_B_BUTTON;
      }
      break;
    case STATE_B_BUTTON:
      if (digitalRead(8) == LOW) {
        e_state = STATE_TREASURE;
      }
      break;
    case STATE_TREASURE:
      if (digitalRead(8) == HIGH) {
        e_state = STATE_B_CENTER;
      }
      break;
    case STATE_B_CENTER:
      if (digitalRead(8) == LOW) {
        e_state = STATE_RAMP_UP;
      }
      break;
    case STATE_RAMP_UP:
      if (digitalRead(8) == LOW) {
        e_state = STATE_C_BUTTON;
      }
      break;
    case STATE_C_BUTTON:
      if (digitalRead(8) == HIGH) {
        e_state = STATE_IR_READ;
      }
      break;
    default:
      e_state = STATE_IR_READ;
  }
  print_state(state_names[e_state]);
  displayText(state_names[e_state]);
}

void setup() {
    Serial.begin(9600);          //Serial for IR and ultrasonic
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

    pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {
  //////LiDAR Sensors////////////////////////////////////////////////////////////////////////////////////////
  //delay(100);               // Don't want to read too often as TFmini samples at 100Hz


  //There need to be states where it is reading certain things; it cannot read from the IR and the sensors simultaneously really
  //LidarRead();
  //UltrasonicRead();
  //DisplayLidarUltrasonic();
  //IRRead();                       //Calls IR Read function (defined below)
  //DisplayIRCode();
  update_state();
  delay(200);

}

void DisplayIRCode() {
 display.clearDisplay();         //Displays received code string
 display.setCursor(0, 0);
 display.print(displaycode);
 display.display();
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
