#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include <motorControl.h>
#include <stdexcept>

using namespace std;

motorController::motorController(int shieldAddress, int numberOfWheels){
    // Initiates the motorshield
    motorShield = Adafruit_MotorShield(shieldAddress);

    currentSpeed = 0;

    // Calls the correct setup function based on number of wheels provided
    if (numberOfWheels == 2){
        setupTwoWheels();
    }else if (numberOfWheels == 4){
        setupFourWheels();
    }else{
        throw invalid_argument("ERROR: motorController object only accepts 2 or 4 motors as input");
    }
}

// Sets up just 2 motors
void motorController::setupTwoWheels({
    // Grabs all 4 motors
    leftMotor = AFMS.getMotor(1);
    rightMotor = AFMS.getMotor(2);

    // Adds them all to array to loop over
    Adafruit_DCMotor motors[2] = [leftMotor, rightMotor];

    // Starts with the default frequency
    motorShield.begin();

    // Sets start speed and turns motor on
    for (var i = 0; i < 2; i++){
        motors[i]->setSpeed(currentSpeed);
        motors[i]->run(FORWARD);
        motors[i]->run(RELEASE);
    }
}


// Sets up all four motors
void motorController::setupFourWheels({
    // Grabs all 4 motors
    frontLeftMotor = AFMS.getMotor(1);
    frontRightMotor = AFMS.getMotor(2);
    backLeftMotor = AFMS.getMotor(3);
    backRightMotor = AFMS.getMotor(4);

    // Adds them all to array to loop over
    Adafruit_DCMotor motors[4] = [frontLeftMotor, frontRightMotor, backLeftMotor, backRightMotor];

    // Starts with the default frequency
    motorShield.begin();

    // Sets start speed and turns motor on
    for (var i = 0; i < 4; i++){
        motors[i]->setSpeed(currentSpeed);
        motors[i]->run(FORWARD);
        motors[i]->run(RELEASE);
    }
}

// Changes speed of motor                           
void changeMotorSpeed(Adafruit_DCMotor *motor, int speed){

    // Input validation
    if (speed > 100 || speed < 0){
        throw "Error: Speed must be between 0-100 inclusive";
        return;
    }
    
    // Updates object var 
    currentSpeed = speed;

    // Convert speed to 0-255 range for motors
    speed = speed * 2.5;

    // Set speed and make sure its moving
    motor->setSpeed(speed);
}

//void changeMotorDirection(enum)

// Have a 2 motor setup and a 4 motor setup, have the constructor take either 2 or 4, 
// then have the 2motorSetup and 4motorSetup be private functions called by that