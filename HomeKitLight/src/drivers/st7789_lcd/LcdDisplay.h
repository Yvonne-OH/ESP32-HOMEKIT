#pragma once
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include "src/BoardPins.h"

/*
 * ST7789 240x240 LCD 驱动（微雪 ESP32-C6-LCD-1.3）
 * 与 TF 卡共享 SPI 总线（CLK/MOSI 固定，CS 各自独立）
 */
class LcdDisplay {
public:
  void begin();                       // 初始化 SPI 总线、屏幕与背光
  void showBoot(const char *msg);     // 开机提示页
  void showTitle();                   // 顶部标题栏

  // 灯状态页：颜色预览圆 + 开关/亮度 + 色相/饱和度
  void drawLightState(uint8_t r, uint8_t g, uint8_t b,
                      bool on, int level, float hue, float sat);

  // 底部状态行（WiFi / SD 等临时信息）
  void drawStatusLine(const char *txt, uint16_t color = 0x7BEF);

private:
  Adafruit_ST7789 _tft = Adafruit_ST7789(&SPI, PIN_LCD_CS, PIN_LCD_DC, PIN_LCD_RST);
};
