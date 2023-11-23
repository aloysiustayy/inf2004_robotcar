#ifndef ultrasonic_h
#define ultrasonic_h
int getCode();
void setupUltrasonicPins(int trigPin, int echoPin);
int getCm(int trigPin, int echoPin);
int getInch(int trigPin, int echoPin);
#endif