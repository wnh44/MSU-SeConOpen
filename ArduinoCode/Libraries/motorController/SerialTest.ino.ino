#include <motorController.h>



motorController *mC;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  mC = &motorController(0x61, 4);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Forward");
  mC->goForward(80);
  delay(1000);
  mC->changeSpeed(100);
  delay(1000);
  mC->stop();
  delay(2000);
  Serial.println("Backward");
  mC->goBackward(100);
  delay(2000);
  mC->stop();
  delay(2000);
  
  Serial.println("Turn Right Twist");
  mC->turnRight(twist, 100);
  delay(2000);
  mC->stop();
  delay(2000);
  Serial.println("Turn Right Forward");
  mC->turnRight(forward, 100);
  delay(2000);
  mC->stop();
  delay(2000);
  Serial.println("Turn Right Backward");
  mC->turnRight(backward, 100);
  delay(2000);
  mC->stop();
  delay(2000);

  Serial.println("Turn Left Twist");
  mC->turnLeft(twist, 100);
  delay(2000);
  mC->stop();
  delay(2000);
  Serial.println("Turn Left Forward");
  mC->turnLeft(forward, 100);
  delay(2000);
  mC->stop();
  delay(2000);
  Serial.println("Turn Left Backward");
  mC->turnLeft(backward, 100);
  delay(2000);
  mC->stop();
  delay(2000);

  
  
}
