volatile float liDARval = 0;
volatile float liDARvalb = 0;
volatile float total = 0;

void setup() {
    Serial1.begin(115200); // HW Serial for TFmini
    Serial.begin(115200); // Serial output through USB to computer
    Serial2.begin(115200); //For second TFmini
    delay (100); // Give a little time for things to start
    
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
    
    Serial.println("Message sent. Starting loop.");
}

void loop() {
  delay(300);  // Don't want to read too often as TFmini samples at 100Hz
  // Data Format for Benewake TFmini
  // ===============================
  // 9 bytes total per message:
  // 1) 0x59
  // 2) 0x59
  // 3) Dist_L (low 8bit)
  // 4) Dist_H (high 8bit)
  // 5) Strength_L (low 8bit)
  // 6) Strength_H (high 8bit)
  // 7) Reserved bytes
  // 8) Original signal quality degree
  // 9) Checksum parity bit (low 8bit), Checksum = Byte1 + Byte2 +...+Byte8. This is only a low 8bit though
    while(Serial1.available()>=9) // When at least 9 bytes of data available (expected number of bytes for 1 signal), then read
    {
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

  //calculate more accurate distance
  liDARval = liDARval * 1.164 - 2.192;
  liDARvalb = liDARvalb * 1.164 - 2.192;
  
  Serial.print(liDARval);
  Serial.print("\t");
  Serial.print(liDARvalb);
  Serial.print("\tcentimeters\n");
  total = liDARval + liDARvalb;
  
  liDARval *= 0.0328084;
  Serial.print(liDARval);
  Serial.print("\t");
  liDARvalb *= 0.0328084;
  Serial.print(liDARvalb);
  Serial.print("\tfeet\n");
  
  liDARval *= 12;
  Serial.print(liDARval);
  Serial.print("\t");
  liDARvalb *= 12;
  Serial.print(liDARvalb);
  Serial.print("\tinches\n");
  
  Serial.print(total);
  Serial.print("\ttotal centimeters\n");
}


