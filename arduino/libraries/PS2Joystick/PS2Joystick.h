#ifndef PS2Joystick_h
#define PS2Joystick_h

#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif

class PS2Joystick
{
public:
    PS2Joystick(int, int, int);
    char direction(); // method to read out the current direction
    bool pressed(); // method to check if the center button was pressed
private:
    int VRx; // pin for VRx
    int VRy; // pin for VRy
    int SW; // pin for SW
};
#endif