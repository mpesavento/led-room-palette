//************************************************
// uses the pro-mini wearable device
// arduino pro mini on a custom shield from Chromatech
// Use "Pro Trinket 5V/16 MHz USB" board
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
}


// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { water_colors,  rainbow, confetti, sinelon, juggle, bpm };

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
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow

}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void water_colors() {
  currentPalette = OceanColors_p;

  static uint8_t paletteIndex = 0;
  for(int i = 0; i < NUM_LEDS; i++) {   
    // fade everything out
    fadeToBlackBy(leds, NUM_LEDS, 50); // dim by 50/256

    CRGB color = ColorFromPalette( currentPalette, gHue++, 255);
    leds[i] = color;

    if (gHue>240) gHue = 0;
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
