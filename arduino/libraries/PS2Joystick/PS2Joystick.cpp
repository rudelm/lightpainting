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
    VRx = X;
    VRy = Y;
    SW = Switch;

    pinMode(SW, INPUT);

    // initial value for debounceTime set to 300ms
    debounceTime = 300;

    // inital threshold value for read Joystick movement values
    threshold = 10;

    calibrateCenter();
}

void PS2Joystick::setDebounceTime(unsigned int time)
{
    debounceTime = time;
}

void PS2Joystick::calibrateCenter()
{
    // get the default centered values when the joystick is left alone during initialization
    centeredX = analogRead(VRx);
    centeredY = analogRead(VRy);
}

bool PS2Joystick::isCentered()
{
    int xVal, yVal;

    xVal = analogRead (VRx);
    yVal = analogRead (VRy);

    if ( (xVal < (xVal - threshold)) && (xVal > (xVal + threshold))
    && ( (yVal < (yVal - threshold)) && (yVal > (yVal + threshold))))
    {
        return true;
    }
    else
    {
        return false;
    }

}

// method to check directions
char PS2Joystick::direction()
{
    int xVal, yVal;

    char direction = JOYSTICK_CENTERED;
    xVal = analogRead (VRx);
    yVal = analogRead (VRy);

    if(!isCentered() && ((millis() - lastValue) >= debounceTime))
    {
        if (xVal < (centeredX - threshold)) direction = JOYSTICK_LEFT;
        if (xVal > (centeredX + threshold)) direction = JOYSTICK_RIGHT;
        if (yVal < (centeredX - threshold)) direction = JOYSTICK_UP;
        if (yVal > (centeredX + threshold)) direction = JOYSTICK_DOWN;
        lastValue = millis();
    }

    return direction;
}

// method to check if center button is pressed
bool PS2Joystick::isPressed()
{
    int buttonVal = digitalRead(SW);
    if (buttonVal == HIGH)
    {
        return false;
    }
    else
    {
        return true;
    }
}