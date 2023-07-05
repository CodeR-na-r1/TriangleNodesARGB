#include <FastLED.h>  // https://kit.alexgyver.ru/tutorials/address-strip/

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

class LedManager {

  // for dynamic animation (loading)

  ColorRGB foregroundColor = ColorRGB(0, 0, 0);
  ColorRGB backgroundColor = ColorRGB(0, 0, 0);
  int nowIndex = 0;
  int length = 3;
  int step = 1;

public:

  LedManager() {

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
    this->foregroundColor = _foregroundcolor;
    this->backgroundColor = _backgroundColor;

    this->nowIndex = _startIndex;
    this->length = _length;
    this->step = _step;

    return;
  }

  void tickLoadAnimation() {
    int tempIndex(0);

    for (int i = 0; i < this->length; i++) {
      tempIndex = this->nowIndex - i;
      leds[tempIndex < 0 ? LED_NUM - (-tempIndex) : tempIndex].setRGB(this->backgroundColor.r, this->backgroundColor.g, this->backgroundColor.b);
      leds[(this->nowIndex + i) % LED_NUM].setRGB(this->foregroundColor.r, this->foregroundColor.g, this->foregroundColor.b);
    }

    this->nowIndex = (this->nowIndex + this->step) % LED_NUM;

    FastLED.show();

    return;
  }

  void showColor(ColorRGB _color) {
    FastLED.showColor(CRGB(_color.r, _color.g, _color.b));

    return;
  }
};