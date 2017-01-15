# lightpainting

## Introduction
This repo contains all interesting lightpainting projects I've tried to create. Currently I'm working on a lightpainting pixel stick which allows me to paint bmp images into a long exposure photo.

### Projects to orientate on
* [Pixelstick](http://www.thepixelstick.com/order.html) - commercial solution

### Alternatives
* Replace the TFT with a touchscreen
* [Arduino LCD KeyPad Shield](https://www.dfrobot.com/wiki/index.php/Arduino_LCD_KeyPad_Shield_(SKU:_DFR0009))
* [1.8" TFT shield with KeyPad and MicroSD Card reader](http://www.exp-tech.de/adafruit-1-8-18-bit-color-tft-shield-w-microsd-and-joystick)
* Use a Raspberry Pi instead of an Arduino
* Use an [ESP8266 over Wifi](https://www.youtube.com/watch?v=7Dv70ci-MOw) to control the WS2812B LEDs

## Requirements
### Hardware
* at least an Arduino Uno
* 1.8" TFT with 160*120px resolution
* PS2 Joystick controller
* [MicroSD/SD-Card Reader](https://learn.adafruit.com/adafruit-micro-sd-breakout-board-card-tutorial/intro) (possibly already integrated into the TFT shield)
* WS2812B LED stripes (ideally a string of 160LEDs to fit the same number of pixels on the TFT)
* An USB Powerbank capable of delivering more than 2A

#### 1.8" TFT with 128*160px LCD, SD Card reader and ST7735S controller
* http://www.angelfire.com/oh3/ebjoew/LCDDisplay.html - explanation and schematics of the boards pins
* http://www.elecfreaks.com/wiki/index.php?title=1.8%22_TFT_LCD:_TFT01-1.8 - mechanic dimensions
* http://www.tweaking4all.com/hardware/arduino/sainsmart-arduino-color-display/ - excellent code examples which show hows the TFT and SD reader can work together

#### PS2 Joystick controller
* [Example Sketch and Pin setup](http://henrysbench.capnfatz.com/henrys-bench/arduino-sensors-and-input/arduino-ps2-joystick-tutorial-keyes-ky-023-deek-robot/)

### Software
* Arduino IDE
* [ADAFRUIT_GFX Library](https://github.com/adafruit/Adafruit-GFX-Library) (installed via Arduino IDE) - to ease the use of the TFT
* SD Library (at least version 1.1 otherwise FAT32 formatted SD cards aren't recognized) - Library to access the SD card readere
* [ADAFRUIT_ST7735 Library](https://github.com/adafruit/Adafruit-ST7735-Library) (latest from github) - to interface the TFT screen
* [TFT_ST7735](https://github.com/Bodmer/TFT_ST7735) (altest from github) - faster library with better font support
* Neopixel Library (installed via Arduino IDE) - for controlling the WS2812B LEDs

### Functionality
What should be possible after we are finished with this project?
* Read a BMP with a maximum height of 160px and various widths
* List stored files from the SD card
* Select file from list to display
* Setup parameters (like brightness of LED strip and TFT, speed of drawing columns)

## License
coming soon
