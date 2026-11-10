#include "RgbLed.h"

uint8_t RgbLed::gamma8(uint8_t x) {
  return (uint8_t)(powf(x / 255.0f, 2.2f) * 255.0f + 0.5f);
}

void RgbLed::begin(uint8_t pin) {
  _pin = pin;
  setColor(0, 0, 0);
}

void RgbLed::setColor(uint8_t r, uint8_t g, uint8_t b) {
  _r = r; _g = g; _b = b;
  neopixelWrite(_pin, gamma8(r), gamma8(g), gamma8(b));
}
