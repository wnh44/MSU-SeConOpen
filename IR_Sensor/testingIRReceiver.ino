//first, install IRremote-Due from
//https://github.com/enternoescape/Arduino-IRremote-Due

#include <IRremote2.h>

//unused except for code array
const byte ledPins[3] = {11,12,13};
unsigned long prevMicro = 0;
byte code[3] = {0,0,0};
const float pause = 1562.5;
byte inTransmission = 0;

/*
Start Pulse     16 HIGH
                8 LOW
Address         (5) 1:1 HIGH:LOW
Data            (3)
End Pulse        1 HIGH

0- 1:1 HIGH:LOW
1- 1:3 HIGH:LOW
 */

int RECV_PIN = 2;
int IR_PIN = 13;

IRrecv irrecv(RECV_PIN);

decode_results results;

void setup()
{
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
    for (int i = 0; i < 3; i++)
    pinMode(ledPins[i],OUTPUT);

  //make the random code and display it
  randomSeed(analogRead(0));

  Serial.print("The code is: ");
  for (int i = 0; i < 3; i++)
  {
    code[i] = random(2);
    Serial.print(code[i]);
  }
  Serial.println();
}
///////All This came in the example/////////////////////////////////////////////////////////

// Dumps out the decode_results structure.
// Call this after IRrecv::decode()
// void * to work around compiler issue
//void dump(void *v) {
//  decode_results *results = (decode_results *)v
void dump(decode_results *results) {
  int count = results->rawlen;
  if (results->decode_type == UNKNOWN) {
    Serial.print("Unknown encoding: ");
  } 
  else if (results->decode_type == NEC) {
    Serial.print("Decoded NEC: ");
  } 
  else if (results->decode_type == SONY) {
    Serial.print("Decoded SONY: ");
  } 
  else if (results->decode_type == RC5) {
    Serial.print("Decoded RC5: ");
  } 
  else if (results->decode_type == RC6) {
    Serial.print("Decoded RC6: ");
  }
  else if (results->decode_type == PANASONIC) {	
    Serial.print("Decoded PANASONIC - Address: ");
    Serial.print(results->panasonicAddress,HEX);
    Serial.print(" Value: ");
  }
  else if (results->decode_type == JVC) {
     Serial.print("Decoded JVC: ");
  }
  else if (results->decode_type == SAMSUNG) {
     Serial.print("Decoded SAMSUNG: ");
  }
  else if (results->decode_type == SAMSUNG2) {	
    Serial.print("Decoded SAMSUNG2 - Address: ");
    Serial.print(results->panasonicAddress,HEX);
    Serial.print(" Value: ");
  }
  Serial.print(results->value, HEX);
  Serial.print(" (");
  Serial.print(results->bits, DEC);
  Serial.println(" bits)");
  Serial.print("Raw (");
  Serial.print(count, DEC);
  Serial.print("): ");

  for (int i = 0; i < count; i++) {
    if ((i % 2) == 1) {
      Serial.print(results->rawbuf[i]*USECPERTICK, DEC);
    } 
    else {
      Serial.print(-(int)results->rawbuf[i]*USECPERTICK, DEC);
    }
    Serial.print(" ");
  }
  Serial.println("");
}

/////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
  
  //receive IR signal
  if (irrecv.decode(&results)) {
    Serial.println(results.value, HEX);
    dump(&results);
    irrecv.resume(); // Receive the next value
  }

  //send IR signal (hasn't worked because using same board I think) (maybe also because I don't know how to send it correctly)
  else {
    //Start blast
    digitalWrite(IR_PIN, HIGH);
    delay(9);
  
    //start quiet
    digitalWrite(IR_PIN, LOW);
    delay(4.5);
  
    //five zeros
    for (int i = 0; i < 5; i++) {
      digitalWrite(IR_PIN, HIGH);
      delayMicroseconds(560);
      digitalWrite(IR_PIN, LOW);
      delayMicroseconds(560);
    }
  
    //the code
    for (int i = 0; i < 3; i++) {
      if (code[i]==0) {
        digitalWrite(IR_PIN, HIGH);
        delayMicroseconds(560);
        digitalWrite(IR_PIN, LOW);
        delayMicroseconds(560);
      }
      else {
        digitalWrite(IR_PIN, HIGH);
        delayMicroseconds(560);
        digitalWrite(IR_PIN, LOW);
        delayMicroseconds(1690);
      }
    }
  
    //end burst
    digitalWrite(IR_PIN, HIGH);
    delayMicroseconds(560);
    digitalWrite(IR_PIN, LOW);
    delay(1000);
  }
}
/////////////Spencer's Code - Not Used/////////////////////////////////////////////////////////////

void blast() {
  unsigned long currMicro = micros(); 
  //unsigned long currMicro = millis(); //DEBUGGING ONLY
  
  bool inTransmission = 1;
  
  // Transmission in progress
  if (inTransmission == 1)
  {
    // In start HIGH pulse
    if ( currMicro - prevMicro == 0 )
    {
      digitalWrite(ledPins[2], HIGH);
      digitalWrite(ledPins[1], HIGH);
      digitalWrite(ledPins[0], HIGH);
    }
    // In start LOW pulse, Give 15 uS span
    else if ( (currMicro - prevMicro >= 16 * pause) && (currMicro - prevMicro < (16 * pause)+15) )
      digitalWrite(ledPins[0], LOW);
    
    // In address pulse 1 HIGH
    else if ( (currMicro - prevMicro >= (24 * pause)) && (currMicro - prevMicro < ((24 * pause)+15)) )
    {
      digitalWrite(ledPins[1], LOW);
      digitalWrite(ledPins[0], HIGH);
    }
    // In address pulse 1 LOW
    else if ( (currMicro - prevMicro >= (25 * pause)) && (currMicro - prevMicro < (25 * pause)+15) )
      digitalWrite(ledPins[0], LOW);
    // In address pulse 2 HIGH
    else if ( (currMicro - prevMicro >= 26 * pause) && (currMicro - prevMicro < (26 * pause)+15) )
      digitalWrite(ledPins[0], HIGH);
    // In address pulse 2 LOW
    else if ( (currMicro - prevMicro >= 27 * pause) && (currMicro - prevMicro < (27 * pause)+15) )
      digitalWrite(ledPins[0], LOW);
    // In address pulse 3 HIGH
    else if ( (currMicro - prevMicro >= 28 * pause) && (currMicro - prevMicro < (28 * pause)+15) )
      digitalWrite(ledPins[0], HIGH);
    // In address pulse 3 LOW
    else if ( (currMicro - prevMicro >= 29 * pause) && (currMicro - prevMicro < (29 * pause)+15) )
      digitalWrite(ledPins[0], LOW);
    // In address pulse 4 HIGH
    else if ( (currMicro - prevMicro >= 30 * pause) && (currMicro - prevMicro < (30 * pause)+15) )
      digitalWrite(ledPins[0], HIGH);
    // In address pulse 4 LOW
    else if ( (currMicro - prevMicro >= 31 * pause) && (currMicro - prevMicro < (31 * pause)+15) )
      digitalWrite(ledPins[0], LOW);
    // In address pulse 5 HIGH
    else if ( (currMicro - prevMicro >= 32 * pause) && (currMicro - prevMicro < (32 * pause)+15) )
      digitalWrite(ledPins[0], HIGH);
    // In address pulse 5 LOW
    else if ( (currMicro - prevMicro >= 33 * pause) && (currMicro - prevMicro < (33 * pause)+15) )
      digitalWrite(ledPins[0], LOW);

    // In data pulse 1 HIGH
    else if ( (currMicro - prevMicro >= 34 * pause) && (currMicro - prevMicro < (34 * pause)+15) )
      digitalWrite(ledPins[0], HIGH);
    // In data pulse 1 LOW
    else if ( (currMicro - prevMicro >= 35 * pause) && (currMicro - prevMicro < ((35) * pause)+15) )
      digitalWrite(ledPins[0], LOW);

    // In data pulse 2 HIGH
    else if ( (currMicro - prevMicro >= (36 + 2*code[0])* pause) && (currMicro - prevMicro < ((36 + 2*code[0])* pause)+15 ) )
      digitalWrite(ledPins[0], HIGH);
    // In data pulse 2 LOW
    else if ( (currMicro - prevMicro >= (37 + 2*code[0])* pause) && (currMicro - prevMicro < ((37 + 2*code[0])* pause)+15 ) )
      digitalWrite(ledPins[0], LOW);

    // In data pulse 3 HIGH
    else if ( (currMicro - prevMicro >= (38 + 2*code[0] + 2*code[1])* pause) && (currMicro - prevMicro < ((38 + 2*code[0] + 2*code[1])* pause)+15 ) )
      digitalWrite(ledPins[0], HIGH);
    // In data pulse 3 LOW
    else if ( (currMicro - prevMicro >= (39 + 2*code[0] + 2*code[1])* pause) && (currMicro - prevMicro < ((39 + 2*code[0] + 2*code[1])* pause)+15 ) )
      digitalWrite(ledPins[0], LOW);

    // In Exit pulse
    else if ( (currMicro - prevMicro >= (40 + 2*code[0] + 2*code[1] + 2*code[2])* pause) && (currMicro - prevMicro < ((40 + 2*code[0] + 2*code[1] + 2*code[2])* pause)+15 ) )
    {
      digitalWrite(ledPins[1], HIGH);
      digitalWrite(ledPins[0], HIGH);
    }
    
    // Terminate transmission
    else if ( (currMicro - prevMicro >= (41 + 2*code[0] + 2*code[1] + 2*code[2])* pause) && (currMicro - prevMicro < ((45 + 2*code[0] + 2*code[1] + 2*code[2])* pause)+15 ) )
    {
      inTransmission = 0;
      digitalWrite(ledPins[2], LOW);
      digitalWrite(ledPins[1], LOW);
      digitalWrite(ledPins[0], LOW);
    }
  }  
}

