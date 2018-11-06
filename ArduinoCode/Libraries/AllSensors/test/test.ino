/*
 * Testing program that implements two lidars, two ultrasonic sensors, and one color sensor
 */

#include <AllSensors.h>
AllSensors *sal;

void setup() {

    //Create instance of AllSensors with inputs:
    //( bool liDAR1 On?, bool liDAR2 On?, int Ultrasonic1 TrigPin, int Ultrasonic1 EchoPin,
    //  int Ultrasonic2 TrigPin, int Ultrasonic2 EchoPin, bool ColorSensor On?)

    sal = &AllSensors(true, true, 10, 11, 12, 13, false);
    Serial.println("Ultrasonic1\tLiDAR2\tUltrasonic2\tLidar1\tColor");
}

void loop() {
    delay(100);

    //Call functions to get values of each sensor
    Serial.print(sal->scanUltraSonic1());
    Serial.print("\t\t");
    Serial.print(sal->scanliDAR2());
    Serial.print("\t");
    Serial.print(sal->scanUltraSonic2());
    Serial.print("\t\t");
    Serial.println(sal->scanliDAR1());

    //The color sensor messes with the ultrasonic sensors sometimes

//  Serial.print("\t");
//  Serial.println(sal->getHexColor());
}