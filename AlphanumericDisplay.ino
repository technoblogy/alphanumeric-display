/* Alphanumeric Display - see http://www.technoblogy.com/show?2ULE

   David Johnson-Davies - www.technoblogy.com - 16th December 2019
   
   CC BY 4.0
   Licensed under a Creative Commons Attribution 4.0 International license: 
   http://creativecommons.org/licenses/by/4.0/
*/

#include <Wire.h>

// Globals
const int DisplaySize = 8;         // Number of alphanumeric characters
const int I2CAddress = 0x70;
const int Brightness = 8;          // Can be between 0 and 15
const int Scrollspeed = 250;       // Milliseconds per character

// Define the 64 characters from space (ASCII code 32) to '_' (ASCII code 95)
const uint16_t Segs[64] = {
0x0, 0x6, 0x220, 0x12CE, 0x12ED, 0xC24, 0x235D, 0x400,
0x2400, 0x900, 0x3FC0, 0x12C0, 0x800, 0xC0, 0x0, 0xC00,
0xC3F, 0x6, 0xDB, 0x8F, 0xE6, 0x2069, 0xFD, 0x7,
0xFF, 0xEF, 0x1200, 0xA00, 0x2400, 0xC8, 0x900, 0x1083,
0x2BB, 0xF7, 0x128F, 0x39, 0x120F, 0xF9, 0x71, 0xBD,
0xF6, 0x1200, 0x1E, 0x2470, 0x38, 0x536, 0x2136, 0x3F,
0xF3, 0x203F, 0x20F3, 0xED, 0x1201, 0x3E, 0xC30, 0x2836,
0x2D00, 0x1500, 0xC09, 0x39, 0x2100, 0xF, 0xC03, 0x8 };

class AlphaDisplay : public Print {
  public:
    void init();
    size_t write(uint8_t);
    void clear();
  private:
    void send(uint8_t);
    uint8_t cur = 0;
    char buf[DisplaySize];
};

// Initialise the display
void AlphaDisplay::init () {
  Wire.begin();
  Wire.beginTransmission(I2CAddress);
  Wire.write(0x21);                // Normal operation mode
  Wire.endTransmission(false);
  Wire.beginTransmission(I2CAddress);
  Wire.write(0xE0 + Brightness);   // Set brightness
  Wire.endTransmission(false);
  clear();                         // Clear display
  Wire.beginTransmission(I2CAddress);
  Wire.write(0x81);                // Display on
  Wire.endTransmission();
}

// Send character to display as two bytes; top bit set = decimal point
void AlphaDisplay::send (uint8_t x) {
  uint16_t dp = 0;
  if (x & 0x80) { dp = 0x4000; x = x & 0x7F; }
  if (x >= 0x60) x = x - 0x20;
  uint16_t segments = Segs[x - 32] | dp; 
  Wire.write(segments);
  Wire.write(segments >> 8);
}

// Clear display
void AlphaDisplay::clear () {
  Wire.beginTransmission(I2CAddress);
  for (int i=0; i<(2*DisplaySize+1); i++) Wire.write(0);
  Wire.endTransmission();
  cur = 0;
}

// Write to the current cursor position 0 to 7 and handle scrolling
size_t AlphaDisplay::write (uint8_t c) {
  if (c == 13) c = ' ';            // Carriage return displayed as space
  if (c == '.') {
    c = buf[cur-1] | 0x80;
    Wire.beginTransmission(I2CAddress);
    Wire.write((cur-1) * 2);
    send(c);
    Wire.endTransmission();
    buf[cur-1] = c;
  } else if (c >= 32) {            // Printing character
    if (cur == DisplaySize) {      // Scroll display left
      Wire.beginTransmission(I2CAddress);
      Wire.write(0);
      for (int i=0; i<7; i++) {
        uint8_t d = buf[i+1];
        send(d);
        buf[i] = d;
      }
      Wire.endTransmission();
      cur--;
    }
    Wire.beginTransmission(I2CAddress);
    Wire.write(cur * 2);
    send(c);
    Wire.endTransmission();
    buf[cur] = c;
    cur++;
    if (cur == DisplaySize) delay(Scrollspeed);
  } else if (c == 12) {            // Clear display
    clear();
  }                                // Other characters don't print
  return 1;
}

// Demo **********************************************

AlphaDisplay Display; // Make an instance of AlphaDisplay

void setup() {
  Display.init();
  Display.println("The quick brown fox jumps over the lazy dog.");
  Display.print("The value of pi = ");
  Display.println(acos(-1.0));
}

void loop() {
}
