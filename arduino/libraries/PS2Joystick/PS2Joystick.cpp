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

    // initial value for debounceTime set to 300ms
    debounceTime = 300;
}

void PS2Joystick::setDebounceTime(unsigned int time)
{
    debounceTime = time;
}

// method to check directions
char PS2Joystick::directions()
{
    char direction = JOYSTICK_CENTERED;

    return direction;
}

// method to check if center button is pressed
bool PS2Joystick::isPressed()
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