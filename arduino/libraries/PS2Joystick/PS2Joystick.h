#ifndef PS2Joystick_h
#define PS2Joystick_h

#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif

#define JOYSTICK_CENTERED 'c'
#define JOYSTICK_UP 'u'
#define JOYSTICK_DOWN 'd'
#define JOYSTICK_LEFT 'l'
#define JOYSTSICK_RIGHT 'r'
#define JOYSTICK_LEFT_UP '1'
#define JOYSTICK_LEFT_DOWN '4'
#define JOYSTICK_RIGHT_UP '2'
#define JOYSTICK_RIGHT_DOWN '3'

class PS2Joystick
{
public:
    PS2Joystick(int, int, int);
    char direction(); // method to read out the current direction
    bool isPressed(); // method to check if the center button was pressed
    void setDebounceTime(unsigned int debounceTime); // method to set read out debounce time
private:
    int VRx; // pin for VRx
    int VRy; // pin for VRy
    int SW; // pin for SW
    unsigned int debounceTime;
    long lastValue; // last recorded time of millis() call
};
#endif