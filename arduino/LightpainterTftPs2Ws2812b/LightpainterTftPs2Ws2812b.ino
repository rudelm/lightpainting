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
#include <Adafruit_NeoPixel.h>


// ADAFRUIT NEOPIXEL LIGHT PAINTER SKETCH: Reads 24-bit BMP image from
// SD card, plays back on NeoPixel strip for long-exposure photography.

// Requires SdFat library for Arduino:
// http://code.google.com/p/sdfatlib/

// As written, uses a momentary pushbutton connected between pin A1 and
// GND to trigger playback.  An analog potentiometer connected to A0 sets
// the brightness at startup and the playback speed each time the trigger
// is tapped.  BRIGHTNESS IS SET ONLY AT STARTUP; can't adjust this in
// realtime, not fast enough.  To change brightness, set dial and tap reset.
// Then set dial for playback speed.

// This is a 'plain vanilla' example with no UI or anything -- it always
// reads a fixed set of files at startup (frame000.bmp - frameNNN.bmp in
// root folder), outputs frameNNN.tmp for each (warning: doesn't ask, just
// overwrites), then plays back from the file(s) each time button is tapped
// (repeating in loop if held).  More advanced applications could add a UI
// (e.g. 16x2 LCD shield), but THAT IS NOT IMPLEMENTED HERE, you will need
// to get clever and rip up some of this code for such.

// This is well-tailored to the Arduino Uno or similar boards.  It may work
// with the Arduino Leonardo *IF* your SD shield or breakout board uses the
// 6-pin ICSP header for SPI rather than pins 11-13.  This WILL NOT WORK
// with 'soft' SPI on the Arduino Mega (too slow).  Also, even with 'hard'
// SPI, this DOES NOT RUN ANY FASTER on the Mega -- a common misconception.

// Adafruit invests time and resources providing this open source code,
// please support Adafruit and open-source hardware by purchasing
// products from Adafruit!

// Written by Phil Burgess / Paint Your Dragon for Adafruit Industries.
// BSD license, all text above must be included in any redistribution.
#include <SdFat.h>
#include <avr/pgmspace.h>
#include "./gamma.h"

// TFT display and SD card will share the hardware SPI interface.
// Hardware SPI pins are specific to the Arduino board type and
// cannot be remapped to alternate pins.  For Arduino Uno,
// Duemilanove, etc., pin 11 = MOSI, pin 12 = MISO, pin 13 = SCK.
#define TFT_CS  10  // Chip select line for TFT display
#define TFT_RST  9  // Reset line for TFT (or see below...)
#define TFT_DC   8  // Data/command line for TFT

#define SD_CS    4  // Chip select line for SD card

Adafruit_ST7735 display = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Arduino Uno Pins A0 for SWX, A1 for SWY and D3 for SW
#define X_AXIS   A0 // X-Axis of the joystick
#define Y_AXIS   A1 // Y-Axis of the joystick
#define BUTTON   3  // Center button of the joystick

int startDelay = 0;

volatile boolean up = false;
volatile boolean down = false;
volatile boolean select = false;
volatile boolean previousUp = false;
volatile boolean previousDown = false;
volatile boolean previousSelect = false;

// TFT Menu
int menuitem = 1;
int page = 1;

// CONFIGURABLE STUFF --------------------------------------------------------

#define N_LEDS       160 // Max value is 170 (fits one SD card block)
#define CARD_SELECT    4 // SD card select pin (some shields use #4, not 10)
#define LED_PIN        6 // NeoPixels connect here
#define SPEED         A0 // Speed-setting dial
#define BRIGHTNESS    A0 // Brightness-setting dial
#define TRIGGER        3 // Playback trigger pin
#define CURRENT_MAX 3500 // Max current from power supply (mA)
// The software does its best to limit the LED brightness to a level that's
// manageable by the power supply.  144 NeoPixels at full brightness can
// draw about 10 Amps(!), while the UBEC (buck converter) sold by Adafruit
// can provide up to 3A continuous, or up to 5A for VERY BRIEF intervals.
// The default CURRENT_MAX setting is a little above the continuous value,
// figuring light painting tends to run for short periods and/or that not
// every scanline will demand equal current.  For extremely long or bright
// images or longer strips, this may exceed the UBEC's capabilities, in
// which case it shuts down (will need to disconnect battery).  If you
// encounter this situation, set CURRENT_MAX to 3000.  Alternately, a more
// powerful UBEC can be substituted (RC hobby shops may have these),
// setting CURRENT_MAX to suit.

// Define ENCODERSTEPS to use rotary encoder rather than timer to advance
// each line.  The encoder MUST be on the T1 pin...this is digital pin 5
// on the Arduino Uno...check datasheet/pinout ref for other board types.
//#define ENCODERSTEPS 8 // # of steps needed to advance 1 line

// NON-CONFIGURABLE STUFF ----------------------------------------------------

#define OVERHEAD 150 // Extra microseconds for loop processing, etc.

uint8_t           sdBuf[512],  // One SD block (also for NeoPixel color data)
                  pinMask;     // NeoPixel pin bitmask
uint16_t          maxLPS,      // Max playback lines/sec
                  nFrames = 0, // Total # of image files
                  frame   = 0; // Current image # being painted
uint32_t          firstBlock,  // First block # in temp working file
                  nBlocks;     // Number of blocks in file
SdFat             sd;          // SD filesystem
volatile uint8_t *port;        // NeoPixel PORT register

boolean bmpProcess(char *inName, char *outName, uint8_t *brightness);

// INITIALIZATION ------------------------------------------------------------

void setup(void) {
  // read start value right after reset/start
  // so its best to leave the joystick as is, but you can also hold it in a position to 
  // set a new default center
  unsigned int startX = analogRead(A0);
  unsigned int startY = analogRead(A1);

  SPI.begin();
  display.initR(INITR_BLACKTAB);
  display.setRotation(2);
  display.setTextWrap(true);
  display.setTextColor(ST7735_WHITE);

  loadScreen();

  uint8_t  b, startupTrigger, minBrightness;
  char     infile[13], outfile[13];
  boolean  found;
  uint16_t i, n;
  SdFile   tmp;
  uint32_t lastBlock;


  digitalWrite(TRIGGER, HIGH);           // Enable pullup on trigger button
  startupTrigger = digitalRead(TRIGGER); // Poll startup trigger ASAP
  Serial.begin(57600);
  pinMode(LED_PIN, OUTPUT);              // Enable NeoPixel output
  digitalWrite(LED_PIN, LOW);            // Default logic state = low
  port    = portOutputRegister(digitalPinToPort(LED_PIN));
  pinMask = digitalPinToBitMask(LED_PIN);
  memset(sdBuf, 0, N_LEDS * 3);          // Clear LED buffer
  show();                                // Init LEDs to 'off' state
#ifdef ENCODERSTEPS
  digitalWrite(5, HIGH);                 // Enable pullup on encoder pin
#endif

  Serial.print(F("Initializing SD card..."));
  if(!sd.begin(CARD_SELECT, SPI_FULL_SPEED)) {
    error(F("failed. Things to check:\n"
            "* is a card is inserted?\n"
            "* Is your wiring correct?\n"
            "* did you edit CARD_SELECT to match the SD shield or module?"));
  }
  Serial.println(F("OK"));

#if 0
  if(!volume.init(&card)) {
    error(F("Could not find FAT16/FAT32 partition.\n"
            "Make sure the card is formatted."));
  }
  root.openRoot(&volume);
#endif

  // This simple application always reads the files 'frameNNN.bmp' in
  // the root directory; there's no file selection mechanism or UI.

  // If button is held at startup, the processing step is skipped, just
  // goes right to playback of the prior converted file(s) (if present).
  if(startupTrigger == HIGH) { // No button press

    // Two passes are made over the input images.  First pass counts the
    // files and estimates the max brightness level that the power supply
    // can sustain...
    minBrightness = 255;
    do {
      sprintf(infile, "frame%03d.bmp", nFrames);
      b = 255; // Assume frame at full brightness to start...
      // ...it's then modified by the bmpProcess() function here to the
      // actual brightness limit the UBEC can sustain for *this image*.
      if(found = bmpProcess(infile, NULL, &b)) {
        nFrames++;
        // Keep track of the minimum 'b' value returned for *all images*.
        // This will later be applied to all, in order that multi-frame
        // animations can have consistent brightness throughout.
        if(b < minBrightness) minBrightness = b;
      }
    } while(found && (nFrames < 1000));

    Serial.print(nFrames);
    Serial.print(F(" frames\nbrightness = "));
    Serial.println(minBrightness);

    // Read dial, setting brightness between 1 (almost but not quite off)
    // and the previously-estimated safe max.
    minBrightness = map(analogRead(BRIGHTNESS), 0, 1023, 1, minBrightness);
  
    // Second pass now applies brightness adjustment while converting
    // the image(s) from BMP to a raw representation of NeoPixel data
    // (this outputs the file(s) 'frameNNN.tmp' -- any existing file
    // by that name will simply be clobbered, IT DOES NOT ASK).
    for(i=0; i<nFrames; i++) {
      sprintf(infile , "frame%03d.bmp", i);
      sprintf(outfile, "frame%03d.tmp", i);
      b = minBrightness; // Reset b to safe limit on each loop iteration
      bmpProcess(infile, outfile, &b);
    }
    while(digitalRead(TRIGGER) == LOW); // Wait for button release

  } else { // Button held -- use existing data

    do { // Scan for files to get nFrames
      sprintf(infile, "frame%03d.tmp", nFrames);
      if(found = tmp.open(infile, O_RDONLY)) {
        if(tmp.contiguousRange(&firstBlock, &lastBlock)) {
          nFrames++;
        }
        tmp.close();
      }
    } while(found);

  } // end startupTrigger test

#ifdef ENCODERSTEPS
  // To use a rotary encoder rather than timer, connect one output
  // of encoder to T1 pin (digital pin 5 on Arduino Uno).  A small
  // capacitor across the encoder pins may help for debouncing.
  TCCR1A = _BV(WGM11) | _BV(WGM10);
  TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS12) | _BV(CS11);
#else
  // Prepare for playback from file; make a full read pass through the
  // file to estimate block read time (+5% margin) and max playback
  // lines/sec.  Not all SD cards perform the same.  This makes sure a
  // reasonable speed limit is used.
  for(i=0; i<nFrames; i++) { // Scan all files
    sprintf(infile, "frame%03d.tmp", i);
    tmp.open(infile, O_RDONLY);
    tmp.contiguousRange(&firstBlock, &lastBlock);
    nBlocks = tmp.fileSize() / 512;
    tmp.close();
    n = (uint16_t)(1000000L /                         // 1 uSec /
      (((benchmark(firstBlock, nBlocks) * 21) / 20) + // time + 5% +
       (N_LEDS * 30L) + OVERHEAD));                   // 30 uSec/pixel
    if(n > maxLPS) maxLPS = n;
  }
  if(maxLPS > 400) maxLPS = 400; // NeoPixel PWM rate is ~400 Hz
  Serial.print(F("Max lines/sec: "));
  Serial.println(maxLPS);

  // Set up Timer1 for 64:1 prescale (250 KHz clock source),
  // fast PWM mode, no PWM out.
  TCCR1A = _BV(WGM11) | _BV(WGM10);
  TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS11) | _BV(CS10);
  // Timer1 interrupt is not used; instead, overflow bit is tightly
  // polled.  Very infrequently a block read may inexplicably take longer
  // than normal...using an interrupt, the code would clobber itself.
  // By polling instead, worst case is just a minor glitch where the
  // corresponding row will be a little thicker than normal.
#endif

  // Timer0 interrupt is disabled for smoother playback.
  // This means delay(), millis(), etc. won't work after this.
  TIMSK0 = 0;

  clearScreen();
}

// Startup error handler; doesn't return, doesn't run loop(), just stops.
static void error(const __FlashStringHelper *ptr) {
  display.fillScreen(ST7735_BLACK);
  display.println(ptr);
  Serial.println(ptr); // Show message
  for(;;);             // and hang
}

void loop() {
  char     infile[13];
  // Get existing contiguous tempfile info
  sprintf(infile, "frame%03d.tmp", frame);
      
  drawMenu();

  checkIfDownButtonIsPressed();
  checkIfUpButtonIsPressed();
  checkIfSelectButtonIsPressed();

  if (up && page == 1 ) {
    up = false;
    menuitem--;
    if (menuitem==0)
    {
      menuitem=3;
    }      
  } else if (up && page == 2 ) {
    up = false;
    startDelay--;
  }


  if (down && page == 1) {
    down = false;
    menuitem++;
    if (menuitem==4) 
    {
      menuitem=1;
    }      
  } else if (down && page == 2 ) {
    down = false;
    startDelay++;
  }


  if (select) {
    select = false;
    
    if (page == 1 && menuitem==2) 
    {
      displayFile(infile);
    }

    if(page == 1 && menuitem ==3)
    {
      //bmpDraw(infile, 0, 0);
    }


    else if (page == 1 && menuitem==1) {
      page=2;
     }
    else if (page == 2) {
      page=1;
     }
   }
}

// LCD Menu code

void checkIfDownButtonIsPressed() {
  if (analogRead(Y_AXIS) >= 420) {
    down = true;
  } else {
    down = false;
  }
  previousDown = down;
}

void checkIfUpButtonIsPressed() {
  if (analogRead(Y_AXIS) <= 400) {
    up = true;
  } else {
    up = false;
  }
  previousUp = up;
}

void checkIfSelectButtonIsPressed() {
  if (digitalRead(BUTTON) == LOW) {
    select = true;
  } else {
    select = false;
  }
  previousSelect = select;
}

void drawMenu() {
  if (page==1) 
  {    
    display.setTextSize(1);
    if (previousSelect || previousUp || previousDown) {
      clearScreen();  
    }
    display.setTextColor(ST7735_WHITE, ST7735_BLACK);
    display.setCursor(15, 0);
    display.print(F("MAIN MENU"));
    display.drawFastHLine(0,10,83,ST7735_WHITE);
    display.setCursor(0, 15);
   
    if (menuitem==1) 
    { 
      display.setTextColor(ST7735_BLACK, ST7735_WHITE);
    }
    else 
    {
      display.setTextColor(ST7735_WHITE, ST7735_BLACK);
    }
    display.print(F(">Contrast"));
    display.setCursor(0, 25);
   
    if (menuitem==2) 
    {
      display.setTextColor(ST7735_BLACK, ST7735_WHITE);
    }
    else 
    {
      display.setTextColor(ST7735_WHITE, ST7735_BLACK);
    }    
    display.print(F(">Play Frame #: "));
    display.print(frame);
    
    if (menuitem==3) 
    { 
      display.setTextColor(ST7735_BLACK, ST7735_WHITE);
    }
    else 
    {
      display.setTextColor(ST7735_WHITE, ST7735_BLACK);
    }  
    display.setCursor(0, 35);
    display.print(F(">Bitmap preview"));
  }
    
 
  else if (page==2) 
  {
    
    display.setTextSize(1);
    if (previousSelect || previousUp || previousDown) {
      clearScreen();  
    }
    display.setTextColor(ST7735_WHITE, ST7735_BLACK);
    display.setCursor(15, 0);
    display.print(F("Start Delay in ms"));
    display.drawFastHLine(0,10,83,ST7735_WHITE);
    display.setCursor(5, 15);
    display.print(F("Value"));
    display.setTextSize(2);
    display.setCursor(5, 25);
    display.print(startDelay);
 
    display.setTextSize(2);
  }
}

// NeoPixel display code

void displayFile(char *infile) {
  uint32_t block    = 0;     // Current block # within file
  boolean  stopFlag = false; // If set, stop playback loop
  uint32_t lastBlock;

  SdFile   tmp;
  
  if(!tmp.open(infile, O_RDONLY)) {
    error(F("Could not open NeoPixel tempfile for input"));
  }
  if(!tmp.contiguousRange(&firstBlock, &lastBlock)) {
    error(F("NeoPixel tempfile is not contiguous"));
  }
  // Number of blocks needs to be calculated from file size, not the
  // range values.  The contiguous file creation and range functions
  // work on cluster boundaries, not necessarily the actual file size.
  nBlocks = tmp.fileSize() / 512;

  tmp.close(); // File handle is no longer accessed, just block reads

  // Stage first block, but don't display yet -- the loop below
  // does that only when Timer1 overflows.
  sd.card()->readBlock(firstBlock, sdBuf);
  // readBlock is used rather than readStart/readData/readEnd as
  // the duration between block reads may exceed the SD timeout.

  while(digitalRead(TRIGGER) == HIGH);   // Wait for trigger button

#ifdef ENCODERSTEPS
  // Set up for rotary encoder
  TCNT1 = 0;
  OCR1A = ENCODERSTEPS;
#else
  // Set up timer based on dial input
  uint32_t linesPerSec = map(analogRead(SPEED), 0, 1023, 10, maxLPS);
  // Serial.println(linesPerSec);
  OCR1A = (F_CPU / 64) / linesPerSec;          // Timer1 interval
#endif

  for(;;) {
    while(!(TIFR1 & _BV(TOV1)));               // Wait for Timer1 overflow
    TIFR1 |= _BV(TOV1);                        // Clear overflow bit

    show();                                    // Display current line
    if(stopFlag) break;                        // Break when done

    if(++block >= nBlocks) {                   // Past last block?
      if(digitalRead(TRIGGER) == HIGH) {       // Trigger released?
        memset(sdBuf, 0, N_LEDS * 3);          // LEDs off on next pass
        stopFlag = true;                       // Stop playback on next pass
        continue;
      }                                        // Else trigger still held
      block = 0;                               // Loop back to start
    }
    sd.card()->readBlock(block + firstBlock, sdBuf); // Load next pixel row
  }

  if(++frame >= nFrames) frame = 0;
}


// BMP->NEOPIXEL FILE CONVERSION ---------------------------------------------

#define BMP_BLUE  0 // BMP and NeoPixels have R/G/B color
#define BMP_GREEN 1 // components in different orders.
#define BMP_RED   2 // (BMP = BGR, Neo = GRB)
#define NEO_GREEN 0
#define NEO_RED   1
#define NEO_BLUE  2

// Convert file from 24-bit Windows BMP format to raw NeoPixel datastream.
// Conversion is bottom-to-top (see notes below)...for horizontal light
// painting, the image is NOT rotated here (the per-pixel file seeking this
// requires takes FOREVER on the Arduino).  Instead, such images should be
// rotated counterclockwise (in Photoshop or other editor) prior to moving
// to SD card.  As currently written, both the input and output files need
// to be in the same directory.  Brightness is set during conversion; there
// aren't enough cycles to do this in realtime during playback.  To change
// brightness, re-process image file using new brightness value.
boolean bmpProcess(
  char    *inName,
  char    *outName,
  uint8_t *brightness) {

  SdFile    inFile,              // Windows BMP file for input
            outFile;             // NeoPixel temp file for output
  boolean   ok        = false,   // 'true' on valid BMP & output file
            flip      = false;   // 'true' if image stored top-to-bottom
  int       bmpWidth,            // BMP width in pixels
            bmpHeight,           // BMP height in pixels
            bmpStartCol,         // First BMP column to process (crop/center)
            columns,             // Number of columns to process (crop/center)
            row,                 // Current image row (Y)
            column;              // and column (X)
  uint8_t  *ditherRow,           // 16-element dither array for current row
            pixel[3],            // For reordering color data, BGR to GRB
            b = 0,               // 1 + *brightness
            d,                   // Dither value for row/column
            color,               // Color component index (R/G/B)
            raw,                 // 'Raw' R/G/B color value
            corr,                // Gamma-corrected R/G/B
           *ledPtr,              // Pointer into sdBuf (output)
           *ledStartPtr;         // First LED column to process (crop/center)
  uint16_t  b16;                 // 16-bit dup of b
  uint32_t  bmpImageoffset,      // Start of image data in BMP file
            lineMax   = 0L,      // Cumulative brightness of brightest line
            rowSize,             // BMP row size (bytes) w/32-bit alignment
            sum,                 // Sum of pixels in row
            startTime = millis();

  if(brightness)           b = 1 + *brightness; // Wraps around, fun with maths
  else if(NULL == outName) return false; // MUST pass brightness for power est.

  Serial.print(F("Reading file '"));
  Serial.print(inName);
  Serial.print(F("'..."));
  if(!inFile.open(inName, O_RDONLY)) {
    Serial.println(F("error"));
    return false;
  }

  if(inFile.read(sdBuf, 34)             &&    // Load header
    (*(uint16_t *)&sdBuf[ 0] == 0x4D42) &&    // BMP signature
    (*(uint16_t *)&sdBuf[26] == 1)      &&    // Planes: must be 1
    (*(uint16_t *)&sdBuf[28] == 24)     &&    // Bits per pixel: must be 24
    (*(uint32_t *)&sdBuf[30] == 0)) {         // Compression: must be 0 (none)
    // Supported BMP format -- proceed!
    bmpImageoffset = *(uint32_t *)&sdBuf[10]; // Start of image data
    bmpWidth       = *(uint32_t *)&sdBuf[18]; // Image dimensions
    bmpHeight      = *(uint32_t *)&sdBuf[22];
    // That's some nonportable, endian-dependent code right there.

    Serial.print(bmpWidth);
    Serial.write('x');
    Serial.print(bmpHeight);
    Serial.println(F(" pixels"));

    if(outName) { // Doing conversion?  Need outFile.
      // Delete existing outFile file (if any)
      (void)sd.remove(outName);
      Serial.print(F("Creating contiguous file..."));
      // NeoPixel working file is always 512 bytes (one SD block) per row
      if(outFile.createContiguous(sd.vwd(), outName, 512L * bmpHeight)) {
        uint32_t lastBlock;
        outFile.contiguousRange(&firstBlock, &lastBlock);
        // Once we have the first block index, the file handle
        // is no longer needed -- raw block writes are used.
        outFile.close();
        nBlocks = bmpHeight; // See note in setup() re: block calcs
        ok      = true;      // outFile is good; proceed
        Serial.println(F("OK"));
      } else {
        Serial.println(F("error"));
      }
    } else ok = true; // outFile not needed; proceed

    if(ok) { // Valid BMP and contig file (if needed) are ready
      Serial.print(F("Processing..."));

      rowSize = ((bmpWidth * 3) + 3) & ~3; // 32-bit line boundary
      b16     = (int)b;

      if(bmpHeight < 0) {       // If bmpHeight is negative,
        bmpHeight = -bmpHeight; // image is in top-down order.
        flip      = true;       // Rare, but happens.
      }

      if(bmpWidth >= N_LEDS) { // BMP matches LED bar width, or crop image
        bmpStartCol = (bmpWidth - N_LEDS) / 2;
        ledStartPtr = sdBuf;
        columns     = N_LEDS;
      } else {                 // Center narrow image within LED bar
        bmpStartCol = 0;
        ledStartPtr = &sdBuf[((N_LEDS - bmpWidth) / 2) * 3];
        columns     = bmpWidth;
        memset(sdBuf, 0, N_LEDS * 3); // Clear left/right pixels
      }

      for(row=0; row<bmpHeight; row++) { // For each row in image...
        Serial.write('.');
        // Image is converted from bottom to top.  This is on purpose!
        // The ground (physical ground, not the electrical kind) provides
        // a uniform point of reference for multi-frame vertical painting...
        // could then use something like a leaf switch to trigger playback,
        // lifting the light bar like making giant soap bubbles.

        // Seek to first pixel to load for this row...
        inFile.seekSet(
          bmpImageoffset + (bmpStartCol * 3) + (rowSize * (flip ?
          (bmpHeight - 1 - row) : // Image is stored top-to-bottom
          row)));                 // Image stored bottom-to-top
        if(!inFile.read(ledStartPtr, columns * 3))  // Load row
          Serial.println(F("Read error"));

        sum       = 0L;
        ditherRow = (uint8_t *)&dither[row & 0x0F]; // Dither values for row
        ledPtr    = ledStartPtr;
        for(column=0; column<columns; column++) {   // For each column...
          if(b) { // Scale brightness, reorder R/G/B
            pixel[NEO_BLUE]  = (ledPtr[BMP_BLUE]  * b16) >> 8;
            pixel[NEO_GREEN] = (ledPtr[BMP_GREEN] * b16) >> 8;
            pixel[NEO_RED]   = (ledPtr[BMP_RED]   * b16) >> 8;
          } else { // Full brightness, reorder R/G/B
            pixel[NEO_BLUE]  = ledPtr[BMP_BLUE];
            pixel[NEO_GREEN] = ledPtr[BMP_GREEN];
            pixel[NEO_RED]   = ledPtr[BMP_RED];
          }

          d = pgm_read_byte(&ditherRow[column & 0x0F]); // Dither probability
          for(color=0; color<3; color++) {              // 3 color bytes...
            raw  = pixel[color];                        // 'Raw' G/R/B
            corr = pgm_read_byte(&gamma[raw]);          // Gamma-corrected
            if(pgm_read_byte(&bump[raw]) > d) corr++;   // Dither up?
            *ledPtr++ = corr;                           // Store back in sdBuf
            sum      += corr;                           // Total brightness
          } // Next color byte
        } // Next column

        if(outName) {
          if(!sd.card()->writeBlock(firstBlock + row, (uint8_t *)sdBuf))
            Serial.println(F("Write error"));
        }
        if(sum > lineMax) lineMax = sum;

      } // Next row
      Serial.println(F("OK"));

      if(brightness) {
        lineMax = (lineMax * 20) / 255; // Est current @ ~20 mA/LED
        if(lineMax > CURRENT_MAX) {
          // Estimate suitable brightness based on CURRENT_MAX
          *brightness = (*brightness * (uint32_t)CURRENT_MAX) / lineMax;
        } // Else no recommended change
      }

      Serial.print(F("Processed in "));
      Serial.print(millis() - startTime);
      Serial.println(F(" ms"));

    } // end 'ok' check
  } else { // end BMP header check
    Serial.println(F("BMP format not recognized."));
  }

  inFile.close();
  return ok; // 'false' on various file open/create errors
}

// MISC UTILITY FUNCTIONS ----------------------------------------------------

// Estimate maximum block-read time for card (microseconds)
static uint32_t benchmark(uint32_t block, uint32_t n) {
  uint32_t t, maxTime = 0L;

  do {
    t = micros();
    sd.card()->readBlock(block++, sdBuf);
    if((t = (micros() - t)) > maxTime) maxTime = t;
  } while(--n);

  return maxTime;
}

// NEOPIXEL FUNCTIONS --------------------------------------------------------

// The normal NeoPixel library isn't used by this project.  SD I/O and
// NeoPixels need to occupy the same buffer, there isn't quite an elegant
// way to do this with the existing library that avoids refreshing a longer
// strip than necessary.  Instead, just the core update function for 800 KHz
// pixels on 16 MHz AVR is replicated here; not handling every permutation.

static void show(void) {
  volatile uint16_t
    i   = N_LEDS * 3; // Loop counter
  volatile uint8_t
   *ptr = sdBuf,      // Pointer to next byte
    b   = *ptr++,     // Current byte value
    hi,               // PORT w/output bit set high
    lo,               // PORT w/output bit set low
    next,
    bit = 8;

  noInterrupts();
  hi   = *port |  pinMask;
  lo   = *port & ~pinMask;
  next = lo;

  asm volatile(
   "head20_%=:"                "\n\t"
    "st   %a[port],  %[hi]"    "\n\t"
    "sbrc %[byte],  7"         "\n\t"
     "mov  %[next], %[hi]"     "\n\t"
    "dec  %[bit]"              "\n\t"
    "st   %a[port],  %[next]"  "\n\t"
    "mov  %[next] ,  %[lo]"    "\n\t"
    "breq nextbyte20_%="       "\n\t"
    "rol  %[byte]"             "\n\t"
    "rjmp .+0"                 "\n\t"
    "nop"                      "\n\t"
    "st   %a[port],  %[lo]"    "\n\t"
    "nop"                      "\n\t"
    "rjmp .+0"                 "\n\t"
    "rjmp head20_%="           "\n\t"
   "nextbyte20_%=:"            "\n\t"
    "ldi  %[bit]  ,  8"        "\n\t"
    "ld   %[byte] ,  %a[ptr]+" "\n\t"
    "st   %a[port], %[lo]"     "\n\t"
    "nop"                      "\n\t"
    "sbiw %[count], 1"         "\n\t"
     "brne head20_%="          "\n"
    : [port]  "+e" (port),
      [byte]  "+r" (b),
      [bit]   "+r" (bit),
      [next]  "+r" (next),
      [count] "+w" (i)
    : [ptr]    "e" (ptr),
      [hi]     "r" (hi),
      [lo]     "r" (lo));

  interrupts();
  // There's no explicit 50 uS delay here as with most NeoPixel code;
  // SD card block read provides ample time for latch!
}

void clearScreen() {
  display.fillScreen(ST7735_BLACK);
  display.setCursor(0, 0);
}

void loadScreen() {
  display.fillScreen(ST7735_BLACK);
  display.println("Lightpainting Stick is booting.");
  Serial.println("Lightpainting Stick is booting.");
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
  SdFile   tmp;
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

  if ((x >= display.width()) || (y >= display.height())) return;

  Serial.println();
  Serial.print(F("Loading image '"));
  Serial.print(filename);
  Serial.println('\'');

  // Open requested file on SD card
  //if ((bmpFile = tmp.open(filename)) == NULL) {
  //  Serial.print("File not found");
  //  return;
  //}
  
  // Parse BMP header
  if (read16(bmpFile) == 0x4D42) { // BMP signature
    Serial.print(F("File size: ")); Serial.println(read32(bmpFile));
    (void)read32(bmpFile); // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    Serial.print(F("Image Offset: ")); Serial.println(bmpImageoffset, DEC);
    // Read DIB header
    Serial.print(F("Header size: ")); Serial.println(read32(bmpFile));
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if (read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
      Serial.print(F("Bit Depth: ")); Serial.println(bmpDepth);
      if ((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed

        goodBmp = true; // Supported BMP format -- proceed!
        Serial.print(F("Image size: "));
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
        if ((x + w - 1) >= display.width())  w = display.width()  - x;
        if ((y + h - 1) >= display.height()) h = display.height() - y;

        // Set TFT address window to clipped image bounds
        display.setAddrWindow(x, y, x + w - 1, y + h - 1);

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
            display.pushColor(display.Color565(r, g, b));
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
