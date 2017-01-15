/*
    Library to check pressed direction of an analogue PS2 Joystick. It also checks if the center button is pressed.

    Created 2017 by Markus Rudel 
*/

#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif
#include "PS2Joystick.h"

// parametrized constructor
PS2Joystick::PS2Joystick(int X, int Y, int Switch)
{
    SWx = X;
    SWy = Y;
    SW = Switch;

    pinMode(SW, INPUT);
}

// method to check directions
char PS2Joystick::directions()
{
    return '-';
}

// method to check if center button is pressed
bool PS2Joystick::pressed()
{
    buttonVal = digitalRead(SW);
    if (buttonVal == HIGH)
    {
        return false;
    }
    else
    {
        return true;
    }
}