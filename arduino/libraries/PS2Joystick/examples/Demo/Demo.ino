#include <PS2Joystick.h>

PS2Joystick joystick;

void setup()
{
    Serial.begin (9600);
    unsigned int startX = analogRead(A0);
    unsigned int startY = analogRead(A1);
    // Arduino Uno Pins A0 for SWX, A1 for SWY and D3 for SW
    joystick = PS2Joystick(A0, A1, 3, startX, startY); // initialize an instance of the class
}

void loop()
{
    Serial.print("Joystick points to: ");
    Serial.println(joystick.direction());
    Serial.print("Joystick is currently centered: ");
    Serial.println(joystick.isCentered() ? "true" : "false");
    Serial.print("Joystick is pressed: ");
    Serial.println(joystick.isPressed() ? "true" : "false");
    delay(500);
}