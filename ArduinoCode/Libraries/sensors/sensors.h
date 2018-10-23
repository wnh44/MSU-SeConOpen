#include <SPI.h>
#include <Wire.h>

class sensors {
    private:
    void setupliDAR1();
    void setupliDAR2();
    void setupUltraSonic();
    int trigPin;
    int echoPin;

    public:

    sensors(bool liDAR1, bool liDAR2, int trigPin, int echoPin);
    float scanliDAR();
    float scanUltraSonic();

};