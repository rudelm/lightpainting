#include <PS2Joystick.h>

// Arduino Uno Pins A0 for SWX, A1 for SWY and D3 for SW
PS2Joystick joystick = PS2Joystick(A0, A1, 3); // initialize an instance of the class

void setup()
{
    Serial.begin (9600);
}

void loop()
{
    Serial.println("Joystick points to: " + joystick.directions());
    Serial.println("Joystick is pressed: " + joystick.isPressed());
    delay(500);
}