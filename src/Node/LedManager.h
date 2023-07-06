#include <FastLED.h>  // https://kit.alexgyver.ru/tutorials/address-strip/

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
}