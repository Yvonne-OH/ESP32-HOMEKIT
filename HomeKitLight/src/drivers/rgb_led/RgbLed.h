#pragma once
#include <Arduino.h>

/*
 * WS2812B RGB 灯驱动
 * 输出 0-255 的 RGB，内部做 2.2 gamma 校正使低亮度过渡更自然
 */
class RgbLed {
public:
  void begin(uint8_t pin);
  void setColor(uint8_t r, uint8_t g, uint8_t b);   // 关灯传 (0,0,0)
  bool isOn() const { return _r || _g || _b; }
  uint8_t red() const { return _r; }
  uint8_t green() const { return _g; }
  uint8_t blue() const { return _b; }

private:
  static uint8_t gamma8(uint8_t x);
  uint8_t _pin = 8;
  uint8_t _r = 0, _g = 0, _b = 0;
};
