/*
 * Testing program that implements two lidars, two ultrasonic sensors, and one color sensor
 */

#include <AllSensors.h>
AllSensors *sal;

int incomingbyte = 0;
String color = "";

void setup() {

    //Create instance of AllSensors with inputs:
    //( bool liDAR1 On?, bool liDAR2 On?, int Ultrasonic1 TrigPin, int Ultrasonic1 EchoPin,
    //  int Ultrasonic2 TrigPin, int Ultrasonic2 EchoPin, bool ColorSensor On?)

    sal = &AllSensors(false, false, 10, 11, 12, 13, true);
    Serial.println("Ultrasonic1\tLiDAR2\tUltrasonic2\tLidar1\tColor");
    //Serial.begin(9600);
}

void loop() {
    delay(2000);

    //Call functions to get values of each sensor
//    Serial.print(sal->scanUltraSonic1());
//    Serial.print("\t\t");
//    Serial.print(sal->scanliDAR2());
//    Serial.print("\t");
//    Serial.print(sal->scanUltraSonic2());
//    Serial.print("\t\t");
//    Serial.println(sal->scanliDAR1());

    //The color sensor messes with the ultrasonic sensors sometimes

//  Serial.print("\t");
    if (Serial.available() > 0) {
        incomingbyte = Serial.read();
        if (incomingbyte == 99) {
            Serial.println("changing colors");
        }
    }

    color = sal->getHexColor();
    if ((color[0] == '6' || color[0] == '5') &&
        (color[0] == '6' || color[1] == '9' || color[1] == '8' || color[1] == 'a' || color[1] == 'b' || color[1] == 'c' || color[1] == 'd' || color[1] == 'e' || color[1] == 'f') &&
        (color[2] == '5' || color[2] == '4') &&
        (color[4] == '4' || color[4] == '5')) {
        Serial.print(color);
        Serial.println(" is carpet");
    }
    else if ((color[0] == '5' || color[0] == '4') &&
             (color[0] == '4' || color[1] == '0' || color[1] == '1' || color[1] == '2' || color[1] == '3' || color[1] == '4' || color[1] == '5' || color[1] == '6' || color[1] == '7') &&
             (color[2] == '4' || color[2] == '5') &&
             (color[4] == '4' || color[4] == '5')) {
        Serial.print(color);
        Serial.println(" is white tape");
    } else if ((color[0] == '7' || color[0] == '8') &&
               (color[2] == '4' || color[2] == '3') &&
               (color[4] == '4' || color[4] == '3')) {
        Serial.print(color);
        Serial.println(" is red tape");
    } else if ((color[0] == '5' || color[0] == '4') &&
               (color[2] == '6' || color[2] == '7')&&
               (color[4] == '4')) {
        Serial.print(color);
        Serial.println(" is green tape");
    } else if (color[0] == '4' && color[2] == '5' && color[4] == '6') {
        Serial.print(color);
        Serial.println(" is blue tape");
    }else if (color[0] == '6' && color[2] == '5' && color[4] == '3') {
        Serial.print(color);
        Serial.println(" is yellow tape");
    }
    else {
        Serial.print(color);
        Serial.println(" is unknown");
    }
}