#include <PS2Joystick.h>

PS2Joystick joystick;

void setup()
{
    Serial.begin (9600);
    // read start value right after reset/start
    // so its best to leave the joystick as is, but you can also hold it in a position to 
    // set a new default center
    unsigned int startX = analogRead(A0);
    unsigned int startY = analogRead(A1);
    // Arduino Uno Pins A0 for SWX, A1 for SWY and D3 for SW
    joystick = PS2Joystick(A0, A1, 3, startX, startY); // initialize an instance of the class
}

void loop()
{
    /*
        Directions are defined as follows:
        #define JOYSTICK_CENTERED 'c'
        #define JOYSTICK_UP 'u'
        #define JOYSTICK_DOWN 'd'
        #define JOYSTICK_LEFT 'l'
        #define JOYSTICK_RIGHT 'r'
        #define JOYSTICK_LEFT_UP '1'
        #define JOYSTICK_LEFT_DOWN '4'
        #define JOYSTICK_RIGHT_UP '2'
        #define JOYSTICK_RIGHT_DOWN '3'
    */
    Serial.print("Joystick points to: ");
    Serial.println(joystick.direction());
    Serial.print("Joystick is currently centered: ");
    Serial.println(joystick.isCentered() ? "true" : "false");
    Serial.print("Joystick is pressed: ");
    Serial.println(joystick.isPressed() ? "true" : "false");
    delay(500);
}