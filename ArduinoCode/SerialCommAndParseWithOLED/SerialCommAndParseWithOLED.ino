#include <motorController.h>
#include <AllSensors.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Conveyorbelt and encoder setup
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

Adafruit_MotorShield conveyorMotorShield = Adafruit_MotorShield(0x60); //Sheild address
Adafruit_DCMotor *conveyorMotor = conveyorMotorShield.getMotor(2); // DC motor on M2

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool motorRunning = false;
bool stalled = false;
int stallDelay = 5; // Mutliples of 10 ms to delay before reacting
int pause = 100; // ms to wait before changing motor direction
int interruptPin = 2; // Can only be 2 or 3


AllSensors *sensors;
motorController *motors;

String currentMessage;


//Gets a whole word from command ---------------------------------------

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


// Parses and executes commands -----------------------------------------

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
      display.clearDisplay();
      display.setTextSize(1);             // Normal 1:1 pixel scale
      display.setTextColor(WHITE);        // Draw white text
      display.setCursor(0,0);             // Start at top-left corner
      display.println("FORWARD SPEED " + motor_speed_string);
      display.display();
      delay(2000);
      motors->goForward(motor_speed);
    }else if (secondWord == "backward") {
      String motor_speed_string = getXword(2, command, " ");
      int motor_speed = motor_speed_string.toInt();
      Serial.println("Going backward at speed " + motor_speed_string);
      display.clearDisplay();
      display.setTextSize(1);             // Normal 1:1 pixel scale
      display.setTextColor(WHITE);        // Draw white text
      display.setCursor(0,0);             // Start at top-left corner
      display.println("BACKWARD SPEED " + motor_speed_string);
      display.display();
      delay(2000);
      motors->goBackward(motor_speed);
    }else if (secondWord == "stop") {
      Serial.println("Stopping");
      display.clearDisplay();
      display.setTextSize(1);             // Normal 1:1 pixel scale
      display.setTextColor(WHITE);        // Draw white text
      display.setCursor(0,0);             // Start at top-left corner
      display.println("STOPPING");
      display.display();
      delay(2000);
      motors->stop();
    }else if (secondWord == "right") {
      String motor_speed_string = getXword(2, command, " ");
      int motor_speed = motor_speed_string.toInt();
      Serial.println("Going right at speed " + motor_speed_string);
      display.clearDisplay();
      display.setTextSize(1);             // Normal 1:1 pixel scale
      display.setTextColor(WHITE);        // Draw white text
      display.setCursor(0,0);             // Start at top-left corner
      display.println("RIGHT SPEED " + motor_speed_string);
      display.display();
      delay(2000);
      motors->turnRight(twist, motor_speed);
    }else if (secondWord == "left") {
      String motor_speed_string = getXword(2, command, " ");
      int motor_speed = motor_speed_string.toInt();
      Serial.println("Going left at speed " + motor_speed_string);
      display.clearDisplay();
      display.setTextSize(1);             // Normal 1:1 pixel scale
      display.setTextColor(WHITE);        // Draw white text
      display.setCursor(0,0);             // Start at top-left corner
      display.println("LEFT SPEED " + motor_speed_string);
      display.display();
      delay(2000);
      motors->turnLeft(twist, motor_speed);
    }else {
      Serial.println("Incorrect GO command");
      display.clearDisplay();
      display.setTextSize(1);             // Normal 1:1 pixel scale
      display.setTextColor(WHITE);        // Draw white text
      display.setCursor(0,0);             // Start at top-left corner
      display.println("INCORRECT COMMAND");
      display.display();
      delay(2000);
    }
  } else if (firstWord == "conveyor"){
    String secondWord = getXword(1, command, " ");

    if (secondWord == "start") {
      startMotor(200);
      Serial.println("Starting conveyor motor");
      display.clearDisplay();
      display.setTextSize(1);             // Normal 1:1 pixel scale
      display.setTextColor(WHITE);        // Draw white text
      display.setCursor(0,0);             // Start at top-left corner
      display.println("CONVEYOR MOTOR START");
      display.display();
      delay(2000);
    } else if (secondWord == "stop"){
      releaseMotor();
      Serial.println("Stopping conveyor motor");
      display.clearDisplay();
      display.setTextSize(1);             // Normal 1:1 pixel scale
      display.setTextColor(WHITE);        // Draw white text
      display.setCursor(0,0);             // Start at top-left corner
      display.println("CONVEYOR MOTOR STOP");
      display.display();
      delay(2000);
    }
  }
  
  else{
    Serial.println("Parsed: " + getXword(1, command, " "));
    display.clearDisplay();
    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(WHITE);        // Draw white text
    display.setCursor(0,0);             // Start at top-left corner
    display.println("Parsed: " + getXword(1, command, " "));
    display.display();
    delay(2000);
  }
  return;
}


// Conveyor Motor and encoder setup ------------------------------------------

// Timer interupt
// If encoder is not heard in timer span, release motor
ISR(TIMER1_COMPA_vect)
{
    // Kill motor if stall timer has timed out while motor is running
    if(motorRunning == true)
    {
        stalled = true;
        // Serial.println("Stalled");
    }
}
// Pin Interupt
// Reset stallShutoff counter if encoder spike is found
void stillSpinning()
{
    // Turn off interrupts
    cli();
    // Reset timer
    TCNT1 = 0;
    // Turn on interrupts
    sei();
}
// Initialize timer and pin interrupts
// Run in Setup()
void setupInterrupts()
{
    // Turn off interrupts
    cli();
    
    // Prep Timer interrupts
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    // Set timeout time
    OCR1A = 155 * stallDelay; //((16000000/1024) * 0.01 * stallDelay) -1;
    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS12) | (1<< CS10);
    TIMSK1 |= (1 << OCIE1A);
    // Prep Pin interrupts for the encoder
    pinMode(interruptPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt( interruptPin ), stillSpinning, FALLING);
    // Turn on interrupts
    sei();
}
void releaseMotor()
{
    // Serial.println("Stopping");
    stalled = false;
    motorRunning = false;
    // Ensure timers are on
    sei();
    // Stop motor
    conveyorMotor->setSpeed(0);
    delay( pause );
    
    // Run backwards
    conveyorMotor->run(BACKWARD);
    conveyorMotor->setSpeed(255);
    // Let run for some time
    for (int i = 0; i < 5; i++)
      delay( pause );
    // Stop motor
    conveyorMotor-> setSpeed(0);
    conveyorMotor->run(RELEASE);
    // Clear stall flags
    
}
void startMotor(int speed)
{
    // Serial.println("Starting");
    // Turn off interrupts
    cli();
    // Ensure stall timer is at 0
    TCNT1 = 0;
    // Set stall flags
    motorRunning = true;
    stalled = false;
    // Start interupts again
    sei();
    conveyorMotor->run(FORWARD);
    conveyorMotor->setSpeed( speed );
}















void setup() {
  Serial.begin(9600);
  sensors = &AllSensors(true, true, 10, 11, 0, 0, false);
  motors = &motorController(0x60, 2);

  
  conveyorMotorShield.begin();
//  setupInterrupts();
  
  // Weird glitch, wont turn later on if dont do this to activate motors???
  motors->turnRight(twist, 100);
  delay(1);
  motors->turnLeft(twist, 100);
  delay(1);
  motors->stop();

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();
  
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

  if (stalled == true){
        releaseMotor();
  }
  
}
