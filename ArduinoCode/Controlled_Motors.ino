/*
This is an example file of using the Arduino Motor Shield

For use with the Adafruit Motor Shield v2
---->  http://www.adafruit.com/products/1438
*/

#include <Wire.h>
#include <Adafruit_MotorShield.h>

/*** GLOBAL VARIABLES ***/
Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x61); // our shield's address

Adafruit_DCMotor *rightMotor = AFMS.getMotor(1);
Adafruit_DCMotor *leftMotor = AFMS.getMotor(2);

bool pathChange = true;
int path = 0;
int motorSpeed = 200;
bool speedChange = true;

bool heartbeat = false;

// So the directions make sense
enum directions
  {
  Stop,
  forward,        // (F , F) forward
  
                  // (F , B) Twist right
  right,          // (F , R) Forward right
                  // (R , B) Back right
                  
                  // (B , F) Twist left
  left,           // (R , F) Forward left
                  // (B , R) Back left
                  
  backward        // (B , B) backward
  };

/*** INITIALIZATION ***/
void setup() {
  Serial.begin(9600);
  Serial.println("DC Motor test!");

  // Set heartbeat pin
  pinMode(13, OUTPUT);
  digitalWrite(13, heartbeat);

  setupTimer();
  
  AFMS.begin();  // default frequency of 1.6KHz
  //AFMS.begin(1000);  // specified 1.0KHz

  leftMotor -> setSpeed(0);
  leftMotor -> run(FORWARD);
  leftMotor -> run(RELEASE);
  
  rightMotor -> setSpeed(0);
  rightMotor -> run(FORWARD);
  rightMotor -> run(RELEASE);
}

// Hardware timer
ISR(TIMER1_COMPA_vect)
{
  // heartbeat true on Short, false on Long
  if(heartbeat)
  {
    digitalWrite(13, HIGH);
    heartbeat = false;
  }
  else
  {
    digitalWrite(13, LOW);
  }
}

// Call in init method!!!!
void setupTimer()
{
    cli();
    
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;

    // Sets time delay between timer ticks
    OCR1A = 3000; //((16000000/1024) * 0.2) -1; 0.2 seconds
    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS12) | (1<< CS10);
    TIMSK1 |= (1 << OCIE1A);

    sei();
}

/*** RUNNING LOOP ***/
void loop() {
  // get serial input
  SerialEvent();

  SetPath();
  SetSpeed();
}

void SetSpeed()
{
  // no need to set to the same speed
  if (!speedChange)
    return;

  leftMotor -> setSpeed(motorSpeed);
  rightMotor -> setSpeed(motorSpeed);
  speedChange = false;
}

void SetPath()
{
  // no need to set to the same state
  if (!pathChange)
    return;

  // Give motors a small break, expecially when changing direction
  leftMotor -> setSpeed(0);
  rightMotor -> setSpeed(0);
  delay(100);

  // interprets the path input
  switch (path)
  {
    case Stop:
      leftMotor -> run(RELEASE);
      rightMotor -> run(RELEASE);
      break;
      
    case forward:
      leftMotor -> run(FORWARD);
      rightMotor -> run(FORWARD);
      break;
      
    case right:
      leftMotor -> run(FORWARD);
      rightMotor -> run (RELEASE);
      break;
      
    case left:
      leftMotor -> run(RELEASE);
      rightMotor -> run (FORWARD);
      break;
      
    case backward:
      leftMotor -> run (BACKWARD);
      rightMotor -> run (BACKWARD);
      break;
      
    default:
      break;
  }

  // Set motors back to speed
  leftMotor -> setSpeed(motorSpeed);
  rightMotor -> setSpeed(motorSpeed);
  pathChange = false;
  speedChange = false;
}

// Run each loop iteration, checks for serial input
void SerialEvent()
{
  while (Serial.available())
  {
    // indicate that serial data was recieved
    heartbeat = true;
    int newPath = path;

    // interpret input
    char inChar = (char)Serial.read();
    switch (inChar)
    {
      case 'w':
        newPath = forward;
        break;
      case 'd':
        newPath = right;
        break;
      case 's':
        newPath = backward;
        break;
      case 'a':
        newPath = left;
        break;
      case '-':
        motorSpeed = motorSpeed - 20;
        speedChange = true;
        break;
      case '+':
        motorSpeed = motorSpeed + 20;
        speedChange = true;
        break;
      default:
        newPath = Stop;
        break;
    }
    // flag if direction change
    if (newPath != path)
    {
      pathChange = true;
      // update path if it has been changed
      path = newPath;
    }
      
  }
}

