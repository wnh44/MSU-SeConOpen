/*
 * Testing program that implements two lidars, two ultrasonic sensors, and one color sensor
 */

#include <AllSensors.h>
AllSensors *sal;

int incomingbyte = 0;
String color = "";
float *colorarray;

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
    Serial.println(sal->getObstacleColor());

    //Call functions to get values of each sensor
    Serial.print(sal->scanUltraSonic1());
    Serial.print("\t\t");
    Serial.print(sal->scanliDAR2());
    Serial.print("\t");
    Serial.print(sal->scanUltraSonic2());
    Serial.print("\t\t");
    Serial.println(sal->scanliDAR1());

    //The color sensor messes with the ultrasonic sensors sometimes
}
