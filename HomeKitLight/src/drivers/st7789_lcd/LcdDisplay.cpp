#include "LcdDisplay.h"
#include "src/BoardPins.h"

void LcdDisplay::begin() {
  pinMode(PIN_LCD_BL, OUTPUT);
  digitalWrite(PIN_LCD_BL, HIGH);                       // 开背光
  SPI.begin(PIN_SPI_SCLK, PIN_SD_MISO, PIN_SPI_MOSI, PIN_LCD_CS);
  _tft.init(240, 240);
  _tft.setRotation(0);
  _tft.fillScreen(ST77XX_BLACK);
  showTitle();
}

void LcdDisplay::showTitle() {
  _tft.fillRect(0, 0, 240, 28, 0x00185A);               // 深蓝标题栏
  _tft.setTextColor(ST77XX_WHITE);
  _tft.setTextSize(2);
  _tft.setCursor(12, 8);
  _tft.print("C6 Light");
}

void LcdDisplay::showBoot(const char *msg) {
  _tft.fillScreen(ST77XX_BLACK);
  showTitle();
  drawStatusLine(msg);
}

void LcdDisplay::drawLightState(uint8_t r, uint8_t g, uint8_t b,
                                bool on, int level, float hue, float sat) {
  // 颜色预览圆
  if (on && (r + g + b) > 0)
    _tft.fillCircle(120, 108, 56, _tft.color565(r, g, b));
  else {
    _tft.fillCircle(120, 108, 56, 0x2108);              // 熄灭: 暗灰
    _tft.drawCircle(120, 108, 56, 0x7BEF);              // 加边框
  }

  // 开关 + 亮度
  _tft.fillRect(0, 176, 240, 24, ST77XX_BLACK);
  _tft.setTextSize(2);
  _tft.setCursor(16, 178);
  if (on) {
    _tft.setTextColor(ST77XX_GREEN);
    _tft.printf("ON  %d%%", level);
  } else {
    _tft.setTextColor(0x7BEF);
    _tft.print("OFF");
  }

  // 色相/饱和度
  _tft.fillRect(0, 202, 240, 20, ST77XX_BLACK);
  _tft.setTextSize(1);
  _tft.setCursor(16, 206);
  _tft.setTextColor(0x5A69);
  _tft.printf("H:%.0f  S:%.0f", hue, sat);
}

void LcdDisplay::drawStatusLine(const char *txt, uint16_t color) {
  _tft.fillRect(0, 224, 240, 16, ST77XX_BLACK);
  _tft.setTextSize(1);
  _tft.setTextColor(color);
  _tft.setCursor(16, 228);
  _tft.print(txt);
}
