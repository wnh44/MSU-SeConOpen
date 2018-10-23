/*
 * Testing program that implements one lidar and one ultrasonic sensor
 */

#include <AllSensors.h>
AllSensors *sal;

void setup() {
  sal = &AllSensors(true, false, 9, 10, false);
  
}

void loop() {
  delay(100);
  Serial.print(sal->scanUltraSonic());
  Serial.print("\t");
  Serial.println(sal->scanliDAR());
}
