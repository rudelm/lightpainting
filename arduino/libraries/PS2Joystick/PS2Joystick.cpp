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

PS2Joystick::PS2Joystick()
{
    // empty constructor
}

// parametrized constructor
PS2Joystick::PS2Joystick(int X, int Y, int Switch, unsigned int startX, unsigned int startY)
{
    VRx = X;
    VRy = Y;
    SW = Switch;

    pinMode(SW, INPUT);

    // initial value for debounceTime set to 300ms
    debounceTime = 300;

    // inital threshold value for read Joystick movement values
    threshold = 10;

    centerX = startX;
    centerY = startY;
}

void PS2Joystick::setDebounceTime(unsigned int time)
{
    debounceTime = time;
}

bool PS2Joystick::isCentered()
{
    int xVal, yVal;
    bool xCentered = true;
    bool yCentered = true;

    xVal = analogRead (VRx);
    yVal = analogRead (VRy);

    // check x coordinates first
    if ((xVal < (centerX - threshold)) || (xVal > (centerX + threshold)))
    {
        xCentered = false;
    }
    // check y coordinates second
    if ((yVal < (centerY - threshold)) || (yVal > (centerY + threshold)))
    {
        yCentered = false;
    }

    if (xCentered && yCentered)
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
        if (xVal < (centerX - threshold)) direction = JOYSTICK_LEFT;
        if (xVal > (centerX + threshold)) direction = JOYSTICK_RIGHT;
        if (yVal < (centerX - threshold)) direction = JOYSTICK_UP;
        if (yVal > (centerX + threshold)) direction = JOYSTICK_DOWN;

        // diagonal directions
        if ((xVal < (centerX - threshold)) && (yVal < (centerX - threshold))) direction = JOYSTICK_LEFT_UP;
        if ((xVal < (centerX - threshold)) && (yVal > (centerX + threshold))) direction = JOYSTICK_LEFT_DOWN;
        if ((xVal > (centerX + threshold)) && (yVal < (centerX - threshold))) direction = JOYSTICK_RIGHT_UP;
        if ((xVal > (centerX + threshold)) && (yVal > (centerX + threshold))) direction = JOYSTICK_RIGHT_DOWN;
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