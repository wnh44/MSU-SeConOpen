#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include <motorController.h>

using namespace std;

// Constructor
motorController::motorController(int shieldAddress, int numberOfWheels){
    // Initiates the motorshield
    motorShield = Adafruit_MotorShield(shieldAddress);

    currentSpeed = 0;

    // Calls the correct setup function based on number of wheels provided
    if (numberOfWheels == 2){
        setupTwoWheels();
        this->numberOfWheels = 2;
    }else if (numberOfWheels == 4){
        setupFourWheels();
        this->numberOfWheels = 4;
    }

    // ADD input validation with error handling
}

// Sets up just 2 motors
void motorController::setupTwoWheels(){
    // Grabs all 4 motors
    leftMotor = motorShield.getMotor(1);
    rightMotor = motorShield.getMotor(2);

    // Adds them all to array to loop over
    activeMotors[0] = leftMotor;
    activeMotors[1] = rightMotor;

    // Starts with the default frequency
    motorShield.begin();

    // Sets start speed and turns motor on
    for (int i = 0; i < 2; i++){
        activeMotors[i]->setSpeed(currentSpeed);
        activeMotors[i]->run(FORWARD);
        activeMotors[i]->run(RELEASE);
    }
}

// Sets up all four motors
void motorController::setupFourWheels(){
    // Grabs all 4 motors
    frontLeftMotor = motorShield.getMotor(1);
    frontRightMotor = motorShield.getMotor(2);
    backLeftMotor = motorShield.getMotor(3);
    backRightMotor = motorShield.getMotor(4);

    // Adds them all to array to loop over
    activeMotors[0] = frontLeftMotor;
    activeMotors[1] = frontRightMotor;
    activeMotors[2] = backLeftMotor;
    activeMotors[3] = backRightMotor;
    
    // Starts with the default frequency
    motorShield.begin();

    // Sets start speed and turns motor on
    for (int i = 0; i < 4; i++){
        activeMotors[i]->setSpeed(currentSpeed);
        activeMotors[i]->run(FORWARD);
        activeMotors[i]->run(RELEASE);
    }
}

// Changes speed of one motor                           
void motorController::changeSpeed(Adafruit_DCMotor *motor, int speed){
    // Input validation
    if (speed > 100 || speed < 0){
        return;
    }
    
    // Updates object var 
    currentSpeed = speed;

    // Convert speed to 0-255 range for motors
    speed = speed * 2.5;

    // Set speed and make sure its moving
    motor->setSpeed(speed);
}

// Changes speed of all motors
void motorController::changeSpeed(int speed){
    // Input validation
    if (speed > 100 || speed < 0){
        return;
    }
    
    // Updates object var 
    currentSpeed = speed;

    for (int i = 0; i < numberOfWheels; i++){
        changeSpeed(activeMotors[i], speed);
    }
}

// Makes all motors go forward
void motorController::goForward(int speed){
    for (int i = 0; i < numberOfWheels; i++){
        changeSpeed(activeMotors[i], speed);
        activeMotors[i]->run(FORWARD);
    }
}

// Makes all motors go backward
void motorController::goBackward(int speed){
    for (int i = 0; i < numberOfWheels; i++){
        changeSpeed(activeMotors[i], speed);
        activeMotors[i]->run(BACKWARD);
    }
}

// Turns right
void motorController::turnRight(int method, int speed){
    if (numberOfWheels == 2){
        switch (method){
            case twist:
                changeSpeed(speed);
                activeMotors[0]->run(FORWARD);
                activeMotors[1]->run(BACKWARD);
                break;
            case forward:
                changeSpeed(speed);
                activeMotors[0]->run(FORWARD);
                activeMotors[1]->run(RELEASE);
                break;
            case backward:
                changeSpeed(speed);
                activeMotors[0]->run(RELEASE);
                activeMotors[1]->run(BACKWARD);
                break;
        }
    }

    if (numberOfWheels == 4){
        switch (method){
            case twist:

                break;
            case forward:

                break;
            case backward:

                break;
        }
    }

}

// Turns left
void motorController::turnLeft(int method, int speed){
    if (numberOfWheels == 2){
        switch (method){
            case twist:
                changeSpeed(speed);
                activeMotors[0]->run(BACKWARD);
                activeMotors[1]->run(FORWARD);
                break;
            case forward:
                changeSpeed(speed);
                activeMotors[0]->run(RELEASE);
                activeMotors[1]->run(FORWARD);
                break;
            case backward:
                changeSpeed(speed);
                activeMotors[0]->run(BACKWARD);
                activeMotors[1]->run(RELEASE);
                break;
        }
    }

    if (numberOfWheels == 4){
        switch (method){
            case twist:

                break;
            case forward:

                break;
            case backward:

                break;
        }
    }

}



// Stops all motors
void motorController::stop(){
    for (int i = 0; i < numberOfWheels; i++){
        changeSpeed(activeMotors[i], 0);
        activeMotors[i]->run(RELEASE);
    }
}



