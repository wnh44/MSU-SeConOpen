#include <motorController.h>
#include <AllSensors.h>

AllSensors *sensors;
motorController *motors;

String currentMessage;


// Parses a given word from the given string
String getXword(int wordNum, String command, String delimiter){
  int currentPlace = 0; // The first index of the current word we are reading
  int currentWord = 0;  // The index of the current word we are reading

  // Loops through and gets each word between the delimiter
  for (int i = 0; i < command.length(); i++){
    // If it hits a delimiter, return the word if its the number word we want
      // Or increment our word counters and keep searching
    if (command.substring(i,i+1) == delimiter){
      if (currentWord == wordNum){
        return command.substring(currentPlace, i);
      }else {
        currentPlace = i+1;
        currentWord++;
      }
    }
    
    // Returns the word if it is the last word and doesn't have a space after
    if (i == (command.length()-1) && (currentWord) == wordNum){
      return command.substring(currentPlace, i+1);
    }
  }

  // Returns blank if the word isn't found
  return "BLANK";
}


void parseCommand(String command){
  String firstWord = getXword(0, command, " ");

  // All the get commands for reading sensors
  if (firstWord == "GET"){
    String secondWord = getXword(1, command, " ");
    if (secondWord == "ultrasonic"){
       float ultrasonic_reading = sensors->scanUltraSonic2();
       Serial.print("Getting ultrasonic reading: ");
       Serial.println(ultrasonic_reading);
    }

    else if (secondWord == "color"){
       String color_reading = sensors->getHexColor();
       Serial.print("Getting color reading: ");
       Serial.println(color_reading);
    }

    else if (secondWord == "lidar"){
      float lidar_reading = sensors->scanliDAR1();
      Serial.print("Getting lidar1 reading: ");
      Serial.println(lidar_reading);
    }

    else{
      Serial.println("Did not issue correct GET command");
    }

    // All the commands for controlling motors
  } else if (firstWord == "GO"){
    String secondWord = getXword(1, command, " ");

    if (secondWord == "forward") {
      // NEED TO FIX PARSING OF INTS, NOT WORKING??????
      String motor_speed_string = getXword(2, command, " ");
      int motor_speed = motor_speed_string.toInt();
      Serial.println("Going forward at speed " + motor_speed_string);
      motors->goForward(motor_speed);
    }else if (secondWord == "backward") {
      String motor_speed_string = getXword(2, command, " ");
      int motor_speed = motor_speed_string.toInt();
      Serial.println("Going backward at speed " + motor_speed_string);
      motors->goBackward(motor_speed);
    }else if (secondWord == "stop") {
      Serial.println("Stopping");
      motors->stop();
    }else if (secondWord == "right") {
      String motor_speed_string = getXword(2, command, " ");
      int motor_speed = motor_speed_string.toInt();
      Serial.println("Going right at speed " + motor_speed_string);
      motors->turnRight(twist, motor_speed);
    }else if (secondWord == "left") {
      String motor_speed_string = getXword(2, command, " ");
      int motor_speed = motor_speed_string.toInt();
      Serial.println("Going left at speed " + motor_speed_string);
      motors->turnLeft(twist, motor_speed);
    }
  }
  
  else{
    Serial.println("Parsed: " + getXword(1, command, " "));
  }
  return;
}






void setup() {
  Serial.begin(9600);
  sensors = &AllSensors(true, true, 10, 11, 0, 0, false);
  motors = &motorController(0x60, 2);
  
  // Weird glitch, wont turn later on if dont do this to activate motors???
  motors->turnRight(twist, 100);
  delay(1);
  motors->turnLeft(twist, 100);
  delay(1);
  motors->stop();
  
}

void loop() {
  while(Serial.available()){
    char t = Serial.read();
    if (t == '@'){
      parseCommand(currentMessage);
      currentMessage = "";
    }
    else if (t != ""){
      currentMessage.concat(t);
    }
  }
  

}
