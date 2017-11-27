#include <IRLibAll.h>

IRrecvPCI myReciever(12);

void setup() {
  Serial.begin(9600);
  myReciever.enableIRIn();
  //myReciever.blink13(true);
}

void loop() {
  if (myReciever.getResults())
  {
    Serial.println(recvGlobal.recvLength, DEC);
    for (int i = 1; i < recvGlobal.recvLength; i++)
    {
      Serial.print(recvGlobal.recvBuffer[i], DEC);
      Serial.print(", ");
    }
    Serial.println();
    myReciever.enableIRIn();
  }
}
