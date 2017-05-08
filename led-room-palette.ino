//************************************************
// uses the pro-mini wearable device
// arduino pro mini on a custom shield from Chromatech
// Use "Pro Trinket 5V/16 MHz USB" board
// Programmer: USBtinyISP
//
// Make sure have Additional Board Manager URI installed in Preferences:
//    https://adafruit.github.io/arduino-board-index/package_adafruit_index.json
//
// In Boards Manager, select Contributed, and install Adafruit AVR boards by Adafruit
//
//************************************************
#include <FastLED.h>
#include "btn.h"


#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN   9
#define CLOCK_PIN  8
#define CHIPSET    APA102
#define COLOR_ORDER BGR  // most of the 10mm black APA102

#define FRAMES_PER_SECOND  30

#define SWITCH_PIN 10
#define LONG_PRESS_TIME 1000

#define BRIGHTNESS_PIN  A0

#define NUM_LEDS  55

CRGBPalette16 currentPalette;
TBlendType currentBlending;

CRGB leds[NUM_LEDS];
Btn btn(SWITCH_PIN);


uint8_t num_leds = NUM_LEDS;

void setup() {
  // Leds
  FastLED.addLeds<CHIPSET, DATA_PIN, CLOCK_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalSMD5050 );
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 2000);

  // Switch
  pinMode(SWITCH_PIN, INPUT_PULLUP); 

  // set initial palette
  currentPalette = OceanColors_p;
  currentBlending = LINEARBLEND;
//  currentBlending = NOBLEND;

}


// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
//SimplePatternList gPatterns = { water_colors,  rainbow_shlomo, confetti, sinelon, juggle, bpm };
SimplePatternList gPatterns = { water_colors,  forest_colors, cloud_colors, lava_colors, heat_colors, party_colors};

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current

uint8_t gHue = 0; // rotating "base color" used by many of the patterns
bool buttonDown = false;

void loop() {
  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();


  uint16_t brightness = analogRead(BRIGHTNESS_PIN);
  FastLED.setBrightness(map(brightness, 0, 1023, 0, 255));

  btn.poll(NULL, NULL);
  // if held down, only want to switch when released
  if (btn.pressed() && !buttonDown) {
    nextPattern();
    buttonDown=true;
  }
  else if (!btn.pressed()) {
    buttonDown=false;
  }
  
  FastLED.show();
  delayToSyncFrameRate(FRAMES_PER_SECOND);

  // do periodic update
  EVERY_N_MILLISECONDS( 50 ) { gHue++; } // slowly cycle the "base color" through the rainbow

}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}


// palette names:
// OceanColors_p, CloudColors_p, LavalColors_p, ForestColors_p, PartyColors_p, HeatColors_p
// PartyColors_p is everything but the greens

void water_colors() {
  currentPalette = OceanColors_p;
  currentBlending = LINEARBLEND;
  FillLEDsFromPaletteColors(gHue);
}

void forest_colors() {
  currentPalette = ForestColors_p;
  currentBlending = LINEARBLEND;
  FillLEDsFromPaletteColors(gHue);
}

void lava_colors() {
  currentPalette = LavaColors_p;
  currentBlending = LINEARBLEND;
  FillLEDsFromPaletteColors(gHue);
}

void party_colors() {
  currentPalette = PartyColors_p;
  currentBlending = LINEARBLEND;
  FillLEDsFromPaletteColors(gHue);
}

void cloud_colors() {
  currentPalette = CloudColors_p;
  currentBlending = LINEARBLEND;
  FillLEDsFromPaletteColors(gHue);
}

void heat_colors() {
  currentPalette = HeatColors_p;
  currentBlending = LINEARBLEND;
  FillLEDsFromPaletteColors(gHue);
}


void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, 255, currentBlending);
        // the shift along color index changes the frequency of the pattern oscillations
        colorIndex += 1; // from PaletteTrace.ino
    }
}


void rainbow_shlomo(bool sparkle) {
  static int offset = 0;
  fill_rainbow(leds, num_leds, offset, 7);
  if (sparkle && random8() < 100) {
    leds[ random16(num_leds) ] += CRGB::White;
  }
  offset++;
}

void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16(13,0,NUM_LEDS);
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16(i+7,0,NUM_LEDS)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

// delayToSyncFrameRate - delay how many milliseconds are needed
//   to maintain a stable frame rate.
static void delayToSyncFrameRate( uint8_t framesPerSecond) {
  static uint32_t msprev = 0;
  uint32_t mscur = millis();
  uint16_t msdelta = mscur - msprev;
  uint16_t mstargetdelta = 1000 / framesPerSecond;
  // Serial.print("frame dt: ");
  // Serial.println(msdelta);
  if ( msdelta < mstargetdelta) {
    FastLED.delay( mstargetdelta - msdelta);
  }
  msprev = mscur;
}
