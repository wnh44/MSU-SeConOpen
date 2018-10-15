#include <Wire.h>
#include <Adafruit_MotorShield.h>

// Maybe make a class for a motor, then have this motor controller just call that class??

class motorController {
    private:
    // Sets up 2 or 4 motors
    void setupTwoWheels();
    void setupFourWheels();
    int numberOfWheels;

    Adafruit_DCMotor *activeMotors[4];

    public:
    Adafruit_MotorShield motorShield;

    // 4 Wheel setup
    Adafruit_DCMotor *frontRightMotor;
    Adafruit_DCMotor *frontLeftMotor;
    Adafruit_DCMotor *backRightMotor;
    Adafruit_DCMotor *backLeftMotor;

    // 2 Wheel setup
    Adafruit_DCMotor *leftMotor;
    Adafruit_DCMotor *rightMotor;

    // Keeps track of current speed of all motors
    int currentSpeed;

    motorController(int shieldAddress, int numberOfWheels);
    void changeSpeed(Adafruit_DCMotor *motor, int speed);   // Should change speed of all motors instead of one motor?
    void changeSpeed(int speed);

    void goForward(int speed);
    void goBackward(int speed);

    // Setup enum for types of turns

    void turnRight(int method);
    void turnLeft(int method);

    void stop();
};

