// PT Motorsport AU - CAN shift light project - (c) 2023 Peter Nowell
// Released under the GPLv3 license
// PT-CAN VERSION (based on the A90 GR Supra)
// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// Released under the GPLv3 license to match the rest of the Adafruit NeoPixel library

#include <Adafruit_NeoPixel.h> // Library for NeoPixel Communication
#include <SPI.h>               // Library for using SPI Communication 
#include <mcp2515.h>           // Library for using CAN Communication

// ---------------- Change your Pattern Start, END and Flash RPM Here ----------------
int STARTRPM = 4500;
int ENDRPM = 6800;
int FLASHRPM = 7200;
// -----------------------------------------------------------------------------------

// How many NeoPixels are attached to the Arduino? If you add more LEDs, add the colours below too
#define NUMPIXELS 8 

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN 4 

// ---------------------CHANGE YOUR LED COLOURS HERE ---------------------------------
const uint32_t LEDcolour[NUMPIXELS] =    //Set color for each individual LED (Red, Green, Blue)
{
  Adafruit_NeoPixel::Color(0, 120, 0), //First LED green
  Adafruit_NeoPixel::Color(0, 120, 0),
  Adafruit_NeoPixel::Color(0, 120, 0),
  Adafruit_NeoPixel::Color(0, 120, 0),
  Adafruit_NeoPixel::Color(255, 125, 0), //Orange
  Adafruit_NeoPixel::Color(255, 125, 0),
  Adafruit_NeoPixel::Color(255, 0, 0), //red
  Adafruit_NeoPixel::Color(255, 0, 0), //8th LED red
};
// ------------------------------------------------------------------------------------

int ON_LEDS = 0;
int RPM = 0;
int step;

struct can_frame canMsg1; // RPM


MCP2515 mcp2515(10); // SPI CS Pin 10 - MCP2515 CS Pin

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

unsigned int ShiftLightsOn[NUMPIXELS]; // define how many lights should be on

void setup() {
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.clear();
  pixels.fill(pixels.Color(10, 0, 0)); // Faint red
  pixels.show();

  SPI.begin();
  
  Serial.begin(9600);
  
  mcp2515.reset();      
  mcp2515.setBitrate(CAN_500KBPS,MCP_8MHZ); // Sets CAN at speed and clock freq | Default = (CAN_500KBPS,MCP_8MHZ)
  mcp2515.setNormalMode();                   // Sets CAN at normal mode

  pinMode(5, OUTPUT);

  step = (ENDRPM - STARTRPM) / NUMPIXELS; //maps the range of RPM the LEDs will cover from the STARTRPM and ENDRPM

  for (int i = 0; i < NUMPIXELS; i++) {
    ShiftLightsOn[i] = STARTRPM + (step * i);
  }
}
void loop() {
  pixels.clear();
  if (mcp2515.readMessage(&canMsg1) == MCP2515::ERROR_OK) {

    //CAN Message 1, dettermine the CAN ID and Bytes of the message
    if(canMsg1.can_id == 0x0A5) { // CAN ID
    
      int x1 = canMsg1.data[0]; //byte 0
      int x2 = canMsg1.data[1]; //byte 1
      int RPM = (x2 + (x1 << 8)) / 4; //put them together (Swap x2 and x1 to change endianness of message) and divide the value by 4

      // Print to serial (uncomment for debugging)
      // Serial.println("RPM");   
      // Serial.println(RPM); 

      if (RPM < ENDRPM) {
        // Normal operating range
        for (int i = 0; i < NUMPIXELS; i++) {
          if (RPM > ShiftLightsOn[i]) {
            pixels.setPixelColor(i, LEDcolour[i]);
          } else {
            pixels.setPixelColor(i, pixels.Color(0, 0, 0));
          }
        }
        pixels.show();
      }
      else if (RPM >= ENDRPM && RPM < FLASHRPM) {
        // Above ENDRPM but under FlashRPM
        pixels.fill(pixels.Color(120, 0, 0)); // Fill all LEDS
        pixels.show();
        delay(30);
      }
      else if (RPM >= FLASHRPM) {
        // Over FLASHRPM - flash flash
        pixels.fill(pixels.Color(0, 0, 0)); // overrev flash colour 1
        pixels.show();
        delay(30);
        pixels.fill(pixels.Color(120, 120, 120)); // overrev flash colour 2
        pixels.show();
        delay(30);
      }
    }
  }
}
