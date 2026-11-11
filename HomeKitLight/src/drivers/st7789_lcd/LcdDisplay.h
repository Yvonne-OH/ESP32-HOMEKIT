#pragma once
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include "src/BoardPins.h"

/*
 * ST7789 240x240 界面驱动（微雪 ESP32-C6-LCD-1.3）
 * 两种完整布局, 由状态机切换, 元素不叠加:
 *   PROVISIONING — 配网页: 二维码卡片 + 配对码 + 提示
 *   HOME         — 主界面: 标题栏(左名称/右时间) + 颜色圆 + 开关/天气行 + 底部状态栏
 */
class LcdDisplay {
public:
  void begin();

  void showBoot(const char *msg);        // 开机提示页 (整屏)
  void drawProvisioning(const char *qrText);   // 配网页: 二维码铺满全屏
  void drawHome();                       // 进入主界面 (整屏框架)

  // 以下仅在 HOME 状态下生效, 避免污染配网页
  void drawClock(const char *clockStr);              // 标题栏右侧时间
  void drawLightState(uint8_t r, uint8_t g, uint8_t b, bool on, int level);
  void drawWeather(const char *weatherStr);          // 信息行右侧
  void drawStatusLine(const char *txt, uint16_t color = 0x7BEF);

private:
  enum UiState { UI_BOOT, UI_PROVISIONING, UI_HOME };
  void drawTitleBar(const char *clockStr);           // clockStr 可为 null

  UiState _state = UI_BOOT;
  Adafruit_ST7789 _tft = Adafruit_ST7789(&SPI, PIN_LCD_CS, PIN_LCD_DC, PIN_LCD_RST);
};
