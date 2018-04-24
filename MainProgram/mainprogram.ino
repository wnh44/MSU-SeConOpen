/***********************************************************************************************
 * 
 * This program is intended for the 2018 IEEE Southeast Conference Hardware Competition
 * written by the Mississippi State Open Team
 *
 * Kumar Zaveri, Spencer Barnes, and (80%) Will Herrington
 *
 * All digital devices (lidar, ultrasonic, OLED, IR reader) should be connected to 5V
 * Limit switches connected from ground to pins with coded pullup
 *
 * //////////////////////////////////////////////////////////////////////////////////////////////
 *
 * This program has two modes for the robot: Competition and Demo
 *
 * It starts out in demo, if a limit switch is pressed the robot will move in the direction
 * opposite of the button. The upper left and right switches stop movement
 *
 * If both upper limit switches are pressed together, the robot enters competition mode
 *
 * Designed for the course, the program will try to get the robot to the three objective buttons,
 * navigating the ramp and avoiding the laser water. Arduino must be reset to get back to demo mode
 *
 ***********************************************************************************************/


//OLED Screen Setup////////////////////////////////////////////////////////////////////////////
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 22                   //Connect OLED RST pin to pin 22 on arduino
Adafruit_SSD1306 display(OLED_RESET);   //Initialize display object

//Ultrasonic Setup/////////////////////////////////////////////////////////////////////////////
const int trigPinFront = 8;          //Connect front trig to arduino pin 8
const int echoPinFront = 9;          //Connect front echo to arduino pin 9
const int trigPinLeft = 48;          //etcetera
const int echoPinLeft = 49;
const int trigPinBack = 35;
const int echoPinBack = 34;
const int trigPinRight = 28;
const int echoPinRight = 26;

long duration;                       //Initialized variables
float distance;

//LiDAR Setup///////////////////////////////////////////////////////////////////////////////////

//Initialized variables
volatile float liDARvalright = 0;
volatile float liDARvalleft = 0;

//for IR input/output//////////////////////////////////////////////////////////////////////////
#include "IRLibAll.h"

IRrecvPCI myReceiver(2);                //Connect IRreceive pin (leftmost when facing you) to pin 2 on arduino
String displaycode = "";                //empty string for OLED display of code
int code[] = {0, 0, 0, 0, 0, 0, 0, 0};  //array of zeroes

//Limit switch Setup/////////////////////////////////////////////////////////////////////////////

//Connect limit swtiches from ground to pin (pullup enabled later in setup)
const int buttonfrontpin = 50;
const int buttonleftpin = 52;
const int buttonbackpin = 53;
const int buttonrightpin = 24;
const int buttontopleftpin = 40;
const int buttontoprightpin = 32;

//MOTOR Setup///////////////////////////////////////////////////////////////////////////////////
#include <Wire.h>
#include <Adafruit_MotorShield.h>
Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x61);     //Create motor shield object
Adafruit_DCMotor *motorFL = AFMS.getMotor(1);               //create four motor objects
Adafruit_DCMotor *motorFR = AFMS.getMotor(2);
Adafruit_DCMotor *motorBR = AFMS.getMotor(3);
Adafruit_DCMotor *motorBL = AFMS.getMotor(4);

//Sensor and Output functions///////////////////////////////////////////////////////////////////

String IRRead(void) {

/* IRRead uses IR reader mounted on bottom of robot to read the code transmitted from IR LED in the course
 * to get the correct sequence of buttons for the course
 *
 * IR signal is sent using its own protocol
 */

    if (myReceiver.getResults()) {
        if ((recvGlobal.recvLength == 20) &&                        //sent code should be 20 pieces long (not bits, idk what to call it)
            (abs(recvGlobal.recvBuffer[1] - 9000) < 500) &&         //first piece should be around 9000
            (abs(recvGlobal.recvBuffer[2] - 4500) < 500) &&         //second should be around 4500
            (recvGlobal.recvBuffer[19] - 500 < 200)) {              //next to last should be around 500
            for (int i = 4, j = 0; i < 19; i +=2, j++) {            //every other piece is what we're looking at, so skip 3 start at 4
                if (abs(recvGlobal.recvBuffer[i] - 560) < 500) {        //560 is a zero
                    code[j] = 0;
                }
                else if (abs(recvGlobal.recvBuffer[i] - 1690) < 500) {  //1690 is a one
                    code[j] = 1;
                }
            }
            Serial.print("Code: ");                                 //prints the code to serial line for debugging
            for (int i = 0; i < 8; i++) {                           //all eight pieces
                Serial.print(code[i]);
                Serial.print(", ");
            }
            Serial.print("\n\n");

            //if the first five pieces are zeroes, last three are course code
            if (code[0] == 0 && code[1] == 0 && code[2] == 0 && code[3] == 0 && code[4] == 0) {
                if (code[5] == 0 && code[6] == 0 && code[7] == 0) {
                    displaycode = "000";
                }
                else if (code[5] == 0 && code[6] == 0 && code[7] == 1) {
                    displaycode = "001";
                }
                else if (code[5] == 0 && code[6] == 1 && code[7] == 0) {
                    displaycode = "010";
                }
                else if (code[5] == 0 && code[6] == 1 && code[7] == 1) {
                    displaycode = "011";
                }
                else if (code[5] == 1 && code[6] == 0 && code[7] == 0) {
                    displaycode = "100";
                }
                else if (code[5] == 1 && code[6] == 0 && code[7] == 1) {
                    displaycode = "101";
                }
                else if (code[5] == 1 && code[6] == 1 && code[7] == 0) {
                    displaycode = "110";
                }
                else if (code[5] == 1 && code[6] == 1 && code[7] == 1) {
                    displaycode = "111";
                }
                myReceiver.enableIRIn();        //reenable IR receiver
                return displaycode;
            }

            //if all eight pieces are ones, code hasn't been sent yet, but bot is in correct position
            else if (code[0] == code[1] == code[2] == code[3] == code[4] == code[5] == code[6] == code[7] == 1) {
                displayText("In position");     //displays on OLED
                myReceiver.enableIRIn();        //reenable IR receiver
                return "null";
            }
            myReceiver.enableIRIn();            //reenable IR receiver
        }
        else {
            myReceiver.enableIRIn();            //reenable IR receiver (listen, I was tired and confused I'm not sure how many of these are necessary)
            return "null";
        }
        myReceiver.enableIRIn();      //reenable receiver
    }
    else {
        return "null";
    }
}
void displayText(String stringa) {
/* Displays a string on the OLED */

    display.clearDisplay();
    display.setCursor(0, 0);
    display.print(stringa);
    display.display();
}
void DisplayUltrasonic() {
/* Displays the ultrasonic sensor data on the OLED and serial monitor*/

    display.clearDisplay();         //Clears the display
    display.setCursor(0, 0);

    display.print("Front: ");  //Ultrasonic sensor
    display.print(UltrasonicRead("front"));
    display.print("\n");
    display.print("Back: ");  //Ultrasonic sensor
    display.print(UltrasonicRead("back"));
    display.print("\n");
    display.print("Left: ");  //Ultrasonic sensor
    display.print(UltrasonicRead("left"));
    display.print("\n");
    display.print("Right: ");  //Ultrasonic sensor
    display.print(UltrasonicRead("right"));
    display.display();
    Serial.print("Ultrasonic distance (right): ");
    Serial.println(distance);
}
void DisplayLidar() {
/* Displays the lidar sensor data on the OLED and serial monitor*/

    display.clearDisplay();         //Clears the display
    display.setCursor(0, 0);

    display.print("1) ");           //First liDAR sensor
    display.print(liDARvalright);
    display.print("\n");

    display.print("2) ");           //Second liDAR sensor
    display.print(liDARvalleft);
    display.print("\n");

    display.display();              //Actually displays all of the above

    Serial.print("liDARvalright: ");
    Serial.println(liDARvalright);
    Serial.print("liDARvalleft: ");
    Serial.println(liDARvalleft);
}
float UltrasonicRead(String direction) {
/*Reads from one ultrasonic sensor, chosen with the direction string parameter */

    int trigPin = 0;
    int echoPin = 0;
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
    distance = distance * 0.393701;       //distance converted to inches
    return distance;
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
            liDARvalright = t2;
            liDARvalright = liDARvalright * 0.393701;
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
            liDARvalleft = t2b;
            liDARvalleft = liDARvalleft * 0.393701;
            t1b = Serial2.read(); // byte 5 = Strength_L
            t2b = Serial2.read(); // byte 6 = Strength_H
            t2b <<= 8;
            t2b += t1b;
            for(int i=0; i<3; i++)Serial2.read(); // byte 7, 8, 9 are ignored
        }
    }
}

//Movement functions/////////////////////////////////////////////////////////////////////////////

void Move_Forward(int spd) {
    motorFL->run(FORWARD);
    motorFR->run(FORWARD);
    motorBL->run(FORWARD);
    motorBR->run(FORWARD);
    motorFL->setSpeed(spd);
    motorFR->setSpeed(spd);
    motorBL->setSpeed(spd);
    motorBR->setSpeed(spd);
}
void Move_Backward(int spd) {
    motorFL->run(BACKWARD);
    motorFR->run(BACKWARD);
    motorBL->run(BACKWARD);
    motorBR->run(BACKWARD);
    motorFL->setSpeed(spd);
    motorFR->setSpeed(spd);
    motorBL->setSpeed(spd);
    motorBR->setSpeed(spd);
}
void Move_Left(int spd) {
    motorFL->run(BACKWARD);
    motorFR->run(FORWARD);
    motorBL->run(FORWARD);
    motorBR->run(BACKWARD);
    motorFL->setSpeed(spd);
    motorFR->setSpeed(spd);
    motorBL->setSpeed(spd);
    motorBR->setSpeed(spd);
}
void Move_Right(int spd) {
    motorFL->run(FORWARD);
    motorFR->run(BACKWARD);
    motorBL->run(BACKWARD);
    motorBR->run(FORWARD);
    motorFL->setSpeed(spd);
    motorFR->setSpeed(spd);
    motorBL->setSpeed(spd);
    motorBR->setSpeed(spd);
}
void Move_Stop() {
    motorFL->setSpeed(0);
    motorFR->setSpeed(0);
    motorBL->setSpeed(0);
    motorBR->setSpeed(0);
    motorFL->run(RELEASE);
    motorFR->run(RELEASE);
    motorBL->run(RELEASE);
    motorBR->run(RELEASE);
}

//State Machine///////////////////////////////////////////////////////////////////////////////////

typedef enum {
    //Competition states
    STATE_IR_READ,
    STATE_A_BUTTON,
    STATE_A_CENTER,
    STATE_RAMP_DOWN,
    STATE_B_BUTTON,
    STATE_TREASURE,
    STATE_B_CENTER,
    STATE_RAMP_UP,
    STATE_C_BUTTON,

    //Demo mode states
    STATE_FORWARD_PRESSED,
    STATE_FORWARD_RELEASED,
    STATE_LEFT_PRESSED,
    STATE_LEFT_RELEASED,
    STATE_RIGHT_PRESSED,
    STATE_RIGHT_RELEASED,
    STATE_BACK_PRESSED,
    STATE_BACK_RELEASED,
    STATE_STILL

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
        "STATE_C_BUTTON - Press correct button C",
        "STATE_FORWARD_PRESSED: Back button pressed, moving forward",
        "STATE_FORWARD_RELEASED: Back button pressed, moving forward",
        "STATE_LEFT_PRESSED: Right button pressed, moving left",
        "STATE_LEFT_RELEASED: Right button pressed, moving left",
        "STATE_RIGHT_PRESSED: Left button pressed, moving right",
        "STATE_RIGHT_RELEASED: Left button pressed, moving right",
        "STATE_BACK_PRESSED: Front button pressed, moving back",
        "STATE_BACK_RELEASED: Front button pressed, moving back",
        "STATE_STILL: No buttons pressed, staying still"
};

void print_state(String e_state_string) {
/* function prints the state_names string for the current state, only when it has changed */

    static String e_last_state = "null";
    if (e_state_string != e_last_state) {
        e_last_state = e_state_string;
        Serial.println(e_state_string);
        displayText(e_state_string);
    }
}
void update_state(void) {
/* This is the function that performs all the actual work */

    static state_t e_state = STATE_STILL;                       //initialized to state_still

    //IR_Read variables
    static bool irreadflag = 0;
    static int irreadnumber = 0;
    static String irreads[] = {"null","null","null","null"};
    static String irreadcurrent = "null";
    static int coursecode = 0;
    static int irtimeout = 0;

    //A_Center variables
    float centermargin = 3.0;

    //Ramp variables
    float coursewidth = 35.0;
    static int timeout = 0;

    //B_Button variables
    float bbuttondistance = 37.0;
    float bmargin = 5.0;

    switch(e_state) {

        //gets the IR code for the course
        case STATE_IR_READ:

            irtimeout++;
            displayText("Scanning for IR code");
            irreadcurrent = IRRead();
            if (irreadcurrent != "null" && irreadcurrent != "") {
                irreads[irreadnumber] = irreadcurrent;
                irreadnumber++;
                if (irreadnumber == 4) {
                    irreadnumber = 0;
                }

                //For debugging/////////////////
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
                ///////////////////////////////

            }

            //if two of the four codes match
            if ((irreads[0] != "null" && irreads[0] != "11111111" && irreads[0] == irreads[1]) ||
                (irreads[0] != "null" && irreads[0] != "11111111" && irreads[0] == irreads[2]) ||
                (irreads[0] != "null" && irreads[0] != "11111111" && irreads[0] == irreads[3]) ||
                (irreads[1] != "null" && irreads[1] != "11111111" && irreads[1] == irreads[2]) ||
                (irreads[1] != "null" && irreads[1] != "11111111" && irreads[1] == irreads[3]) ||
                (irreads[2] != "null" && irreads[2] != "11111111" && irreads[2] == irreads[3])) {
                irreadflag = 1;
                Serial.println("Flag set");
            }

            //or if the timeout has happened
            if (irtimeout > 100) {
                irreadflag = 1;
                irreadcurrent = "000";
            }

            if (irreadflag == 1) {
                Serial.println("Deciding which code");
                irreadflag = 0;
                if (irreadcurrent == "000") {
                    coursecode = 1;
                }
                else if (irreadcurrent == "001") {
                    coursecode = 2;
                }
                else if (irreadcurrent == "010") {
                    coursecode = 3;
                }
                else if (irreadcurrent == "011") {
                    coursecode = 4;
                }
                else if (irreadcurrent == "100") {
                    coursecode = 5;
                }
                else if (irreadcurrent == "101") {
                    coursecode = 6;
                }
                else if (irreadcurrent == "110") {
                    coursecode = 7;
                }
                else if (irreadcurrent == "111") {
                    coursecode = 8;
                }

                //displays the code on the OLED
                display.clearDisplay();
                //display.setTextSize(4);
                display.setCursor(0, 0);
                display.print((String) coursecode);
                display.display();
                //delay(2000);
                e_state = STATE_A_BUTTON;               //next state
                Serial.println(state_names[e_state]);
                //Serial.println("moving on");
                //displayText(state_names[e_state]);
            }
            break;

        //presses the first button, A
        case STATE_A_BUTTON:

            //move to the back wall
            if (digitalRead(buttonbackpin) == LOW) {
                Move_Backward(255);
                Serial.println("Moving backward");
                //displayText("Moving backward");
            }

            else {
                Move_Stop();
                if (displaycode[0] == '0') {

                    //move to left button
                    if (digitalRead(buttontopleftpin) == LOW) {
                        Move_Left(255);
                        Serial.println("Moving left");
                    }
                    else {
                        //if top left button is pressed, button is clicked
                        Move_Stop();
                        e_state = STATE_A_CENTER;               //next state
                        Serial.println(state_names[e_state]);

                        //this should get it back near the middle, centered later
                        Move_Right(255);
                        delay(6000);
                        Move_Stop();
                    }
                }
                else {

                    //move to right button
                    if (digitalRead(buttontoprightpin) == LOW) {
                        Move_Right(255);
                        Serial.println("Moving right");
                    }
                    else {
                        //if top right button is pressed, button is clicked
                        Move_Stop();
                        e_state = STATE_A_CENTER;
                        Serial.println(state_names[e_state]);

                        //this should get it back near the middle
                        Move_Left(255);
                        delay(6000);
                        Move_Stop();
                    }
                }
            }
            break;

        //centers the robot in the upper level for going down the ramp
        case STATE_A_CENTER:

            //move to the back wall
            if (digitalRead(buttonbackpin) == LOW) {
                Move_Backward(200);
                Serial.println("Moving backward");
            }
            else {
                Move_Stop();

                //moves to where ultrasonic sensors should read button posts
                Move_Forward(255);
                delay(1000);
                Move_Stop();

                //PrintUltrasonic();        //debugging
                //if ultrasonics say robot is not centered
                if ((UltrasonicRead("left") - UltrasonicRead("right")) > centermargin) {
                    Move_Left(255);
                    Serial.println("US not even, moving left");
                    Serial.print("Left: ");
                    Serial.println(UltrasonicRead("left"));
                    Serial.print("Right: ");
                    Serial.println(UltrasonicRead("right"));
                }
                else if ((UltrasonicRead("right") - UltrasonicRead("left")) > centermargin) {
                    Move_Right(255);
                    Serial.println("US not even, moving right");
                }
                else {
                    //robot should be centered
                    Move_Stop();
                    e_state = STATE_RAMP_DOWN;                  //next state
                    Serial.println(state_names[e_state]);
                    delay(2000);

                    //begins movement toward ramp
                    Serial.println("Moving forward");
                    Move_Forward(255);
                    delay(1000);

                    //tilts (better for getting down without touching water
                    Serial.println("Tilting");
                    Move_Stop();
                    motorFR->run(FORWARD);
                    motorBR->run(FORWARD);
                    motorFR->setSpeed(255);
                    motorBR->setSpeed(255);
                    delay(800);

                    //moves down the ramp
                    Serial.println("Moving down the ramp");
                    Move_Forward(255);
                    delay(5000);
                    Move_Stop();
                }
            }
            break;

        //already has moved down the ramp, this state stops the bot at the bottom
        case STATE_RAMP_DOWN:
            timeout++;

            //keep moving forward until:
            Move_Forward(255);
            Serial.println("Moving forward");

            //ultrasonics are reading the walls or timeout has happened
            if ((UltrasonicRead("left") + UltrasonicRead("right") < coursewidth)
                || (timeout == 200)) {
                Serial.println("Ultrasonics say we are down");
                e_state = STATE_B_BUTTON;                           //next state
                Serial.println(state_names[e_state]);
                Move_Stop();
            }
            break;

        //pressent second button, B
        case STATE_B_BUTTON:

            //this stuff tries to use sensors to get there, I did not have time
            /*
              if ((UltrasonicRead("front") - bbuttondistance) > bmargin) {
                Move_Forward(255);
              }
              else {
                Move_Stop();
                if (UltrasonicRead("left") > 2) {
                  Move_Left(255);
                }
                else {
                  Move_Stop();
                  Move_Right(255);
                  if (UltrasonicRead("right") - 18.0 > 3.0) {
                    Move_Stop();
                    e_state = STATE_TREASURE;
                    displayText(state_names[e_state]);
                  }
                }
              }*/

            //This stuff just moves the bot for certain amounts of time to try to get to the button and the chest
            //Basically everything past this point is an unimplemented mess
            if (displaycode[1] == '1') {
                Move_Forward(255);
                Serial.println("Forward 3 seconds");
                delay(3000);
                Move_Right(255);
                Serial.println("Right 5s");
                delay(5000);
                e_state = STATE_TREASURE;
                Serial.println(state_names[e_state]);
                Move_Left(255);
                Serial.println("Left 2s");
                delay(2000);
                Move_Forward(255);
                Serial.println("Forward 2s");
                delay(2000);
                Move_Right(255);
                Serial.println("Right 7s");
                delay(7000);
                Move_Left(255);
                Serial.println("Left 4s");
                delay(4000);
                Move_Stop();
                e_state = STATE_TREASURE;
            }
            else{
                Move_Forward(255);
                Serial.println("Forward 3 seconds");
                delay(3000);
                Move_Left(255);
                Serial.println("Right 5s");
                delay(5000);
                e_state = STATE_TREASURE;
                Serial.println(state_names[e_state]);
                Move_Right(255);
                Serial.println("Left 2s");
                delay(2000);
                Move_Forward(255);
                Serial.println("Forward 2s");
                delay(2000);
                Move_Left(255);
                Serial.println("Right 7s");
                delay(7000);
                Move_Right(255);
                Serial.println("Left 4s");
                delay(4000);
                Move_Stop();
                e_state = STATE_TREASURE;
            }
            break;
        case STATE_TREASURE:
            /*if (UltrasonicRead("right") - UltrasonicRead("left") > 3.0) {
              Move_Right(255);
            }
            else if (UltrasonicRead("left") - UltrasonicRead("right") > 3.0) {
              Move_Left(255);
            }
            else {
              Move_Forward(255);
              delay(2000);
              Move_Stop();
              e_state = STATE_B_CENTER;
              //displayText(state_names[e_state]);
            }*/
            Move_Forward(255);
            Serial.println("Forward 4s");
            delay(4000);
            Move_Left(255);
            Serial.println("left 1s");
            delay(1000);
            Move_Backward(255);
            Serial.println("backward 2s");
            delay(2000);
            Move_Right(255);
            Serial.println("Right 1s");
            delay(1000);
            break;
        case STATE_B_CENTER:
            //if (UltrasonicRead("left")f
            /*
            while ((UltrasonicRead("left") - UltrasonicRead("right")) > centermargin) {
              Move_Left();
            }
            while ((UltrasonicRead("right") - UltrasonicRead("left")) > centermargin) {
              Move_Right();
            }
            LidarRead();
            while ((liDARvalright - liDARvalleft) > centermargin) {
              Move_Left();
              LidarRead();
            }
            LidarRead();
            while ((liDARvalleft - liDARvalright) > centermargin) {
              Move_Right();
              LidarRead();
            }
            e_state = STATE_RAMP_UP;
          */
            e_state = STATE_RAMP_UP;
            Serial.println(state_names[e_state]);
            //displayText(state_names[e_state]);
            break;
        case STATE_RAMP_UP:
            Move_Backward(255);
            displayText("Backward 6s");
            delay(6000);
            e_state = STATE_C_BUTTON;
            Serial.println(state_names[e_state]);/*
      if (UltrasonicRead("left") < coursewidth && UltrasonicRead("right") < coursewidth) {
        e_state = STATE_C_BUTTON;
      }
      if (digitalRead(8) == LOW) {
        e_state = STATE_C_BUTTON;
        displayText(state_names[e_state]);
      }*/
            break;
        case STATE_C_BUTTON:
            if (digitalRead(8) == HIGH) {
                e_state = STATE_IR_READ;
                Serial.println(state_names[e_state]);
                //displayText(state_names[e_state]);
            }
            break;

        //These are the states for the demo mode////////////////////////////////////////////////////////
        case STATE_FORWARD_PRESSED:
            Move_Forward(255);
            if (digitalRead(buttontopleftpin) == HIGH || digitalRead(buttontoprightpin) == HIGH) {
                Move_Stop();
                e_state = STATE_STILL;
            }
            else if (digitalRead(buttonleftpin) == HIGH) {
                e_state = STATE_RIGHT_PRESSED;
            }
            else if (digitalRead(buttonrightpin) == HIGH) {
                e_state = STATE_LEFT_PRESSED;
            }
            else if (digitalRead(buttonfrontpin) == HIGH) {
                e_state = STATE_BACK_PRESSED;
            }
            else if (digitalRead(buttonbackpin) == HIGH) {
                e_state = STATE_FORWARD_PRESSED;
            }
            else {
                e_state = STATE_FORWARD_RELEASED;
            }
            break;
        case STATE_FORWARD_RELEASED:
            Move_Forward(255);
            if (digitalRead(buttontopleftpin) == HIGH || digitalRead(buttontoprightpin) == HIGH) {
                Move_Stop();
                e_state = STATE_STILL;
            }
            else if (digitalRead(buttonleftpin) == HIGH) {
                e_state = STATE_RIGHT_PRESSED;
            }
            else if (digitalRead(buttonrightpin) == HIGH) {
                e_state = STATE_LEFT_PRESSED;
            }
            else if (digitalRead(buttonfrontpin) == HIGH) {
                e_state = STATE_BACK_PRESSED;
            }
            else if (digitalRead(buttonbackpin) == HIGH) {
                e_state = STATE_FORWARD_PRESSED;
            }
            else {
                e_state = STATE_FORWARD_RELEASED;
            }
            break;
        case STATE_BACK_PRESSED:
            Move_Backward(255);
            if (digitalRead(buttontopleftpin) == HIGH || digitalRead(buttontoprightpin) == HIGH) {
                Move_Stop();
                e_state = STATE_STILL;
            }
            else if (digitalRead(buttonleftpin) == HIGH) {
                e_state = STATE_RIGHT_PRESSED;
            }
            else if (digitalRead(buttonrightpin) == HIGH) {
                e_state = STATE_LEFT_PRESSED;
            }
            else if (digitalRead(buttonfrontpin) == HIGH) {
                e_state = STATE_BACK_PRESSED;
            }
            else if (digitalRead(buttonbackpin) == HIGH) {
                e_state = STATE_FORWARD_PRESSED;
            }
            else {
                e_state = STATE_BACK_RELEASED;
            }
            break;
        case STATE_BACK_RELEASED:
            Move_Backward(255);
            if (digitalRead(buttontopleftpin) == HIGH || digitalRead(buttontoprightpin) == HIGH) {
                Move_Stop();
                e_state = STATE_STILL;
            }
            else if (digitalRead(buttonleftpin) == HIGH) {
                e_state = STATE_RIGHT_PRESSED;
            }
            else if (digitalRead(buttonrightpin) == HIGH) {
                e_state = STATE_LEFT_PRESSED;
            }
            else if (digitalRead(buttonfrontpin) == HIGH) {
                e_state = STATE_BACK_PRESSED;
            }
            else if (digitalRead(buttonbackpin) == HIGH) {
                e_state = STATE_FORWARD_PRESSED;
            }
            else {
                e_state = STATE_BACK_RELEASED;
            }
            break;
        case STATE_RIGHT_PRESSED:
            Move_Right(255);
            if (digitalRead(buttontopleftpin) == HIGH || digitalRead(buttontoprightpin) == HIGH) {
                Move_Stop();
                e_state = STATE_STILL;
            }
            else if (digitalRead(buttonleftpin) == HIGH) {
                e_state = STATE_RIGHT_PRESSED;
            }
            else if (digitalRead(buttonrightpin) == HIGH) {
                e_state = STATE_LEFT_PRESSED;
            }
            else if (digitalRead(buttonfrontpin) == HIGH) {
                e_state = STATE_BACK_PRESSED;
            }
            else if (digitalRead(buttonbackpin) == HIGH) {
                e_state = STATE_FORWARD_PRESSED;
            }
            else {
                e_state = STATE_RIGHT_RELEASED;
            }
            break;
        case STATE_RIGHT_RELEASED:
            Move_Right(255);
            if (digitalRead(buttontopleftpin) == HIGH || digitalRead(buttontoprightpin) == HIGH) {
                Move_Stop();
                e_state = STATE_STILL;
            }
            else if (digitalRead(buttonleftpin) == HIGH) {
                e_state = STATE_RIGHT_PRESSED;
            }
            else if (digitalRead(buttonrightpin) == HIGH) {
                e_state = STATE_LEFT_PRESSED;
            }
            else if (digitalRead(buttonfrontpin) == HIGH) {
                e_state = STATE_BACK_PRESSED;
            }
            else if (digitalRead(buttonbackpin) == HIGH) {
                e_state = STATE_FORWARD_PRESSED;
            }
            else {
                e_state = STATE_RIGHT_RELEASED;
            }
            break;
        case STATE_LEFT_PRESSED:
            Move_Left(255);
            if (digitalRead(buttontopleftpin) == HIGH || digitalRead(buttontoprightpin) == HIGH) {
                Move_Stop();
                e_state = STATE_STILL;
            }
            else if (digitalRead(buttonleftpin) == HIGH) {
                e_state = STATE_RIGHT_PRESSED;
            }
            else if (digitalRead(buttonrightpin) == HIGH) {
                e_state = STATE_LEFT_PRESSED;
            }
            else if (digitalRead(buttonfrontpin) == HIGH) {
                e_state = STATE_BACK_PRESSED;
            }
            else if (digitalRead(buttonbackpin) == HIGH) {
                e_state = STATE_FORWARD_PRESSED;
            }
            else {
                e_state = STATE_LEFT_RELEASED;
            }
            break;
        case STATE_LEFT_RELEASED:
            Move_Left(255);
            if (digitalRead(buttontopleftpin) == HIGH || digitalRead(buttontoprightpin) == HIGH) {
                Move_Stop();
                e_state = STATE_STILL;
            }
            else if (digitalRead(buttonleftpin) == HIGH) {
                e_state = STATE_RIGHT_PRESSED;
            }
            else if (digitalRead(buttonrightpin) == HIGH) {
                e_state = STATE_LEFT_PRESSED;
            }
            else if (digitalRead(buttonfrontpin) == HIGH) {
                e_state = STATE_BACK_PRESSED;
            }
            else if (digitalRead(buttonbackpin) == HIGH) {
                e_state = STATE_FORWARD_PRESSED;
            }
            else {
                e_state = STATE_LEFT_RELEASED;
            }
            break;
        case STATE_STILL:
            Move_Stop();
            if (digitalRead(buttontopleftpin) == HIGH && digitalRead(buttontoprightpin) == HIGH) {
                e_state = STATE_IR_READ;
            }
            else if (digitalRead(buttontopleftpin) == HIGH || digitalRead(buttontoprightpin) == HIGH) {
                e_state = STATE_STILL;
            }
            else if (digitalRead(buttonleftpin) == HIGH) {
                e_state = STATE_RIGHT_PRESSED;
            }
            else if (digitalRead(buttonrightpin) == HIGH) {
                e_state = STATE_LEFT_PRESSED;
            }
            else if (digitalRead(buttonfrontpin) == HIGH) {
                e_state = STATE_BACK_PRESSED;
            }
            else if (digitalRead(buttonbackpin) == HIGH) {
                e_state = STATE_FORWARD_PRESSED;
            }
            else {
                e_state = STATE_STILL;
            }
            break;
        default:
            e_state = STATE_STILL;
    }
    print_state(state_names[e_state]);
}

void setup() {
    Serial.begin(9600);          //Serial for IR and ultrasonic
    Serial1.begin(115200);       //HW Serial for TFmini
    Serial2.begin(115200);       //For second TFmini
    delay (2000);                //Give a little time for things to start

    // Set lidars to Standard Output mode
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
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);      //address 0x3C might have to change
    delay(2000);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);


    ///Ultrasonic setup
    pinMode(trigPinFront, OUTPUT);
    pinMode(echoPinFront, INPUT);
    pinMode(trigPinBack, OUTPUT);
    pinMode(echoPinBack, INPUT);
    pinMode(trigPinLeft, OUTPUT);
    pinMode(echoPinLeft, INPUT);
    pinMode(trigPinRight, OUTPUT);
    pinMode(echoPinRight, INPUT);

    ///IR Receiver Setup
    myReceiver.enableIRIn();                                //Enable receiver

    ///Limit switch button setup
    pinMode(buttonfrontpin, INPUT_PULLUP);
    pinMode(buttonrightpin, INPUT_PULLUP);
    pinMode(buttonleftpin, INPUT_PULLUP);
    pinMode(buttonbackpin, INPUT_PULLUP);
    pinMode(buttontopleftpin, INPUT_PULLUP);
    pinMode(buttontoprightpin, INPUT_PULLUP);

    ///MOTORS setup
    AFMS.begin();                           // reate with the default frequency 1.6KHz
    motorFR->setSpeed(0);
    motorFL->setSpeed(0);
    motorBL->setSpeed(0);
    motorBR->setSpeed(0);

    motorFL->run(FORWARD);
    motorFR->run(FORWARD);
    motorBL->run(FORWARD);
    motorBR->run(FORWARD);

    motorFL->run(RELEASE);
    motorFR->run(RELEASE);
    motorBL->run(RELEASE);
    motorBR->run(RELEASE);
    Move_Stop();
}

void loop() {
    delay(100);               // Don't want to read too often as TFmini samples at 100Hz

    update_state();
}


