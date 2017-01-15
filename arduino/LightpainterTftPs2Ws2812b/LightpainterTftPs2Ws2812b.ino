/***************************************************
  This is a library for the Adafruit 1.8" SPI display.

  This library works with the Adafruit 1.8" TFT Breakout w/SD card
  ----> http://www.adafruit.com/products/358
  The 1.8" TFT shield
  ----> https://www.adafruit.com/product/802
  The 1.44" TFT breakout
  ----> https://www.adafruit.com/product/2088
  as well as Adafruit raw 1.8" TFT display
  ----> http://www.adafruit.com/products/618

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
#include <SD.h>

// TFT display and SD card will share the hardware SPI interface.
// Hardware SPI pins are specific to the Arduino board type and
// cannot be remapped to alternate pins.  For Arduino Uno,
// Duemilanove, etc., pin 11 = MOSI, pin 12 = MISO, pin 13 = SCK.
#define TFT_CS  10  // Chip select line for TFT display
#define TFT_RST  9  // Reset line for TFT (or see below...)
#define TFT_DC   8  // Data/command line for TFT

#define SD_CS    4  // Chip select line for SD card

// PS2 Joystick
int Xin= A0; // X Input Pin
int Yin = A1; // Y Input Pin
int KEYin = 3; // Push Button

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
int ionce = 1;
char PressKey;
//char KeepKey;
File root;
int yStep = 0;
int yPos = 0;
int yRowHeight = 10;
int yStart = 23;

void setup(void) {
  pinMode (KEYin, INPUT);
  Serial.begin(9600);
  
  // Use this initializer if you're using a 1.8" TFT
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST7735_BLACK);
  String iMsg;
  tft.setTextColor(ST7735_BLUE);
  tft.setTextSize(1);
  iMsg =  initCard();
  tft.setCursor(0, 110);
  tft.print(iMsg);

  menu();
  delay(500);
}


void menu() {
  if (ionce == 1) {
    yStep = 0;
    yPos = yStart;
    tft.drawRect(0, yStart, 127, yRowHeight, ST7735_GREEN);

    ionce++;
  } else {
    tft.fillScreen(ST7735_BLACK);

  }

  //tft.stroke(255,255,255);

  // set the fill color to grey
  // tft.fill(0,0,0);

  // draw a rectangle in the center of screen
  tft.drawRect(0, 0, 127, 20, ST7735_WHITE);
  tft.setTextColor(ST7735_YELLOW);
  tft.setTextSize(1);
  // tft.stroke(0, 255, 0);


  tft.setCursor(0, 10);
  tft.print(" Menu? (<,>,Select)");
  tft.setCursor(0, 25);
  tft.print(" 1)List Files");
  tft.setCursor(0, 35);
  tft.print(" 2)Display Parrot");
  tft.setCursor(0, 45);
  tft.print(" 3)Display Image2");
  tft.setCursor(0, 55);
  tft.print(" 4)Display Image3");
  tft.setCursor(0, 65);
  tft.print(" 5)Rotate Image4");
  tft.setCursor(0, 75);
  tft.print(" 6)Police/Ambulance");
  tft.setCursor(0, 85);
  tft.print(" 7)Siren On/Off");
  //  tft.setCursor(0, 75);
  //  tft.print(" 6) Show Menu :\n ");

}

String initCard() {
  if (!SD.begin(SD_CS)) {
    return " SdCard failed";
  }
  return " SdCard init...OK!";
}

void loop() {
  int xVal, yVal, buttonVal;
  
  xVal = analogRead (Xin);
  yVal = analogRead (Yin);
  buttonVal = digitalRead (KEYin);
  char PressKey;
  
  Serial.print("X = ");
  Serial.println (xVal, DEC);
  
  Serial.print ("Y = ");
  Serial.println (yVal, DEC);
  
  Serial.print("Button is ");
  if (buttonVal == HIGH){
    Serial.println ("not pressed");
  }
  else{
    Serial.println ("PRESSED");
    PressKey = 'p'; //enter
  }
    
  delay (100);

  if ((xVal > 500) && (yVal < 430))
  {
    // left top corner, menu requested
    PressKey = 'm'; //menu
  }

  if (xVal > 500)
  {
    PressKey = 'u'; //up
  } else if (xVal < 430)
  {
    PressKey = 'd'; //down
  }

  if (yVal > 500)
  {
    PressKey = 'r'; //right
  } else if (yVal < 430)
  {
    PressKey = 'l'; //left
  }

  
  
  if ((PressKey != 'e'))
  {
    // tft.fillScreen(ST7735_BLACK);
    switch (PressKey) {
      case 'u' :   //up
        {
          tft.drawRect(0, yPos, 127, yRowHeight, ST7735_BLACK);

          if (yStep == 0) {
            yStep = 6;
          } else {
            yStep--;
          }
          yPos = yStart + yRowHeight * yStep;
          tft.drawRect(0, yPos, 127, yRowHeight, ST7735_GREEN);
          break;
        }
      case 'd' :   //down
        tft.drawRect(0, yPos, 127, yRowHeight, ST7735_BLACK);
        if (yStep == 6 ) {
          yStep = 0;
        } else {
          yStep++;
        }
        yPos = yStart + yRowHeight * yStep;
        tft.drawRect(0, yPos, 127, yRowHeight, ST7735_GREEN);
        break; {

        }
      case 'p' :   //enter
        {

          switch (yStep) {
            case 0:
              {
                tft.fillScreen(ST7735_BLACK);
                root = SD.open("/");
                printDirectory(root, 0);
                break;
              }
            case 1:
              {
                tft.fillScreen(ST7735_BLACK);
               tft.setRotation(0);
                bmpDraw("parrot.bmp", 0, 0);

                delay(1500);
                break; 
              }
            case 2:
              {
                tft.fillScreen(ST7735_BLACK);
                tft.setRotation(0);
                bmpDraw("logo.bmp", 0, 0);

                delay(1000);
                break;
              }
            case 3:
              {
                tft.fillScreen(ST7735_BLACK);
             tft.setRotation(0);
                bmpDraw("lgjoe_h.bmp", 0, 0);

                delay(1500);
                break;
              }
            case 4:
              {
                tft.fillScreen(ST7735_BLACK);
                tft.setRotation(1);
                bmpDraw("lgjoe128.bmp", 0, 0);

                delay(1500);
                break;
              }
            case 5:
              {
                yStep = 5 ;
                tft.drawRect(0, yPos, 127, yRowHeight, ST7735_GREEN);
                break;
              }
            case 6:
              {
                yStep = 6 ;
                tft.drawRect(0, yPos, 127, yRowHeight, ST7735_GREEN);
                break;
              }
          }
          break;
        }

      case 'm':  //menu
        {
          menu();
          tft.drawRect(0, yPos, 127, yRowHeight, ST7735_BLACK);
          yStep = 0;
          yPos = yStart;
          tft.drawRect(0, yPos, 127, yRowHeight, ST7735_GREEN);
          break;
        }
      default:
        break;
    }
  } else {
    //
  }
  delay(20);

}



void printDirectory(File dir, int numTabs) {
  // Begin at the start of the directory
  dir.rewindDirectory();
  tft.setCursor(0, 10);
  tft.println("List Files:");
  tft.drawLine(0, 20, tft.width() / 2, 20, ST7735_YELLOW);
  tft.println("");
  while (true) {
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      //Serial.println("**nomorefiles**");
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      tft.print('\t');   // we'll have a nice indentation
    }
    // Print the 8.3 name
    tft.print(entry.name());
    // Recurse for directories, otherwise print the file size
    if (entry.isDirectory()) {
      tft.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      tft.print("\t\t");
      tft.println(entry.size(), DEC);
    }
    entry.close();
  }
}

// This function opens a Windows Bitmap (BMP) file and
// displays it at the given coordinates.  It's sped up
// by reading many pixels worth of data at a time
// (rather than pixel by pixel).  Increasing the buffer
// size takes more of the Arduino's precious RAM but
// makes loading a little faster.  20 pixels seems a
// good balance.

#define BUFFPIXEL 20

void bmpDraw(char *filename, uint8_t x, uint8_t y) {

  File     bmpFile;
  int      bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3 * BUFFPIXEL]; // pixel buffer (R+G+B per pixel)
  uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  int      w, h, row, col;
  uint8_t  r, g, b;
  uint32_t pos = 0, startTime = millis();

  if ((x >= tft.width()) || (y >= tft.height())) return;

  Serial.println();
  Serial.print("Loading image '");
  Serial.print(filename);
  Serial.println('\'');

  // Open requested file on SD card
  if ((bmpFile = SD.open(filename)) == NULL) {
    Serial.print("File not found");
    return;
  }

  // Parse BMP header
  if (read16(bmpFile) == 0x4D42) { // BMP signature
    Serial.print("File size: "); Serial.println(read32(bmpFile));
    (void)read32(bmpFile); // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    Serial.print("Image Offset: "); Serial.println(bmpImageoffset, DEC);
    // Read DIB header
    Serial.print("Header size: "); Serial.println(read32(bmpFile));
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if (read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
      Serial.print("Bit Depth: "); Serial.println(bmpDepth);
      if ((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed

        goodBmp = true; // Supported BMP format -- proceed!
        Serial.print("Image size: ");
        Serial.print(bmpWidth);
        Serial.print('x');
        Serial.println(bmpHeight);

        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * 3 + 3) & ~3;

        // If bmpHeight is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if (bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip      = false;
        }

        // Crop area to be loaded
        w = bmpWidth;
        h = bmpHeight;
        if ((x + w - 1) >= tft.width())  w = tft.width()  - x;
        if ((y + h - 1) >= tft.height()) h = tft.height() - y;

        // Set TFT address window to clipped image bounds
        tft.setAddrWindow(x, y, x + w - 1, y + h - 1);

        for (row = 0; row < h; row++) { // For each scanline...

          // Seek to start of scan line.  It might seem labor-
          // intensive to be doing this on every line, but this
          // method covers a lot of gritty details like cropping
          // and scanline padding.  Also, the seek only takes
          // place if the file position actually needs to change
          // (avoids a lot of cluster math in SD library).
          if (flip) // Bitmap is stored bottom-to-top order (normal BMP)
            pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
          else     // Bitmap is stored top-to-bottom
            pos = bmpImageoffset + row * rowSize;
          if (bmpFile.position() != pos) { // Need seek?
            bmpFile.seek(pos);
            buffidx = sizeof(sdbuffer); // Force buffer reload
          }

          for (col = 0; col < w; col++) { // For each pixel...
            // Time to read more pixel data?
            if (buffidx >= sizeof(sdbuffer)) { // Indeed
              bmpFile.read(sdbuffer, sizeof(sdbuffer));
              buffidx = 0; // Set index to beginning
            }

            // Convert pixel from BMP to TFT format, push to display
            b = sdbuffer[buffidx++];
            g = sdbuffer[buffidx++];
            r = sdbuffer[buffidx++];
            tft.pushColor(tft.Color565(r, g, b));
          } // end pixel
        } // end scanline
        Serial.print("Loaded in ");
        Serial.print(millis() - startTime);
        Serial.println(" ms");
      } // end goodBmp
    }
  }

  bmpFile.close();
  if (!goodBmp) Serial.println("BMP format not recognized.");
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(File f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}
