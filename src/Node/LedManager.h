#include <FastLED.h>  // https://kit.alexgyver.ru/tutorials/address-strip/

// for future
// 
// https://github.com/AlexGyver/WS2812_FX/blob/master/%D0%BF%D1%80%D0%BE%D1%88%D0%B8%D0%B2%D0%BA%D0%B8/WS2812_FX_rndChange_light_fixed/LED_EFFECT_FUNCTIONS.ino
//
//  https://github.com/GyverLibs/VolAnalyzer
// 

#include "CRGB_COLORS.h"

extern CRGB CRGBColors[];

#ifndef LED_PIN
#define LED_PIN 5  // GPIO5 -> D1
#endif

#ifndef LED_NUM
#define LED_NUM 20
#endif

CRGB leds[LED_NUM];

class ColorRGB {

public:

  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;

  ColorRGB(uint8_t _r, uint8_t _g, uint8_t _b) {
    r = _r;
    g = _g;
    b = _b;
  }
};

namespace LED_MANAGER {

// service functions

int adjacent_cw(int i) {
  int r;
  if (i < LED_NUM - 1) {
    r = i + 1;
  } else {
    r = 0;
  }
  return r;
}

int adjacent_ccw(int i) {
  int r;
  if (i > 0) {
    r = i - 1;
  } else {
    r = LED_NUM - 1;
  }
  return r;
}

// for dynamic animation (loading)

ColorRGB foregroundColor = ColorRGB(0, 0, 0);
ColorRGB backgroundColor = ColorRGB(0, 0, 0);
int nowIndex = 0;
int length = 3;
int step = 1;

uint8_t ihue = 0;
uint8_t thissat = 255;

void (*ledFunction)() = nullptr;

void setUp() {

  pinMode(LED_PIN, OUTPUT);

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, LED_NUM);

  FastLED.setBrightness(255);

  FastLED.clearData();  // false

  FastLED.show();
}

void setBrightness(uint8_t scale) {
  FastLED.setBrightness(scale);
}

CFastLED getFastLED() {
  return FastLED;
}

void initLoadAnimation(ColorRGB _foregroundcolor, ColorRGB _backgroundColor, int _startIndex, int _length, int _step = 1) {
  foregroundColor = _foregroundcolor;
  backgroundColor = _backgroundColor;

  nowIndex = _startIndex;
  length = _length;
  step = _step;

  return;
}

void tickAnimation() {
  int tempIndex(0);

  for (int i = 0; i < length; i++) {
    tempIndex = nowIndex - i;
    leds[tempIndex < 0 ? LED_NUM - (-tempIndex) : tempIndex].setRGB(backgroundColor.r, backgroundColor.g, backgroundColor.b);
    leds[(nowIndex + i) % LED_NUM].setRGB(foregroundColor.r, foregroundColor.g, foregroundColor.b);
  }

  nowIndex = (nowIndex + step) % LED_NUM;

  FastLED.show();

  return;
}

void showColor(ColorRGB _color) {
  FastLED.showColor(CRGB(_color.r, _color.g, _color.b));

  return;
}

void tickStaticColor() {
  CRGB color = CRGBColors[constrain(random(0, CRGB_COLORS_SIZE + 1), 0, CRGB_COLORS_SIZE - 1)];
  FastLED.showColor(color);
}

void rainbow_fade() {

  ++ihue;

  for (int index = 0; index < LED_NUM; ++index) {
    leds[index] = CHSV(ihue, thissat, 255);
  }

  LEDS.show();
}

int8_t bouncedirection = 0;
uint8_t idex = 0;
uint8_t thishue = 24;
bool forward = true;

void color_bounceFADE() {

  if (bouncedirection == 0) {
    idex = idex + 1;
    if (idex == LED_NUM) {
      bouncedirection = 1;
      idex = idex - 1;
    }
  }
  if (bouncedirection == 1) {
    idex = idex - 1;
    if (idex == 0) {
      bouncedirection = 0;
    }
  }

  if (forward) {
    ++thishue;
    if (thishue == 255) { forward = false; }
  } else {
    --thishue;
    if (thishue == 0) { forward = true; }
  }

  int iL1 = adjacent_cw(idex);
  int iL2 = adjacent_cw(iL1);
  int iL3 = adjacent_cw(iL2);
  int iR1 = adjacent_ccw(idex);
  int iR2 = adjacent_ccw(iR1);
  int iR3 = adjacent_ccw(iR2);

  for (int i = 0; i < LED_NUM; i++) {
    if (i == idex) {
      leds[i] = CHSV(thishue, thissat, 255);
    } else if (i == iL1) {
      leds[i] = CHSV(thishue, thissat, 150);
    } else if (i == iL2) {
      leds[i] = CHSV(thishue, thissat, 80);
    } else if (i == iL3) {
      leds[i] = CHSV(thishue, thissat, 20);
    } else if (i == iR1) {
      leds[i] = CHSV(thishue, thissat, 150);
    } else if (i == iR2) {
      leds[i] = CHSV(thishue, thissat, 80);
    } else if (i == iR3) {
      leds[i] = CHSV(thishue, thissat, 20);
    } else {
      leds[i] = CHSV(0, 0, 0);
    }
  }

  LEDS.show();
}

}