#pragma once
#include <Arduino.h>
#include "src/BoardPins.h"

/*
 * TF (Micro-SD) 卡驱动（微雪 ESP32-C6-LCD-1.3）
 * 与 LCD 共享 SPI 总线: CLK=GPIO7, MOSI=GPIO6, MISO=GPIO5, CS=GPIO4
 * 注意: 需先调用 LcdDisplay::begin() 初始化共享 SPI 总线
 */
class SdCard {
public:
  bool begin(uint8_t csPin = PIN_SD_CS);    // 挂载 TF 卡
  bool mounted() const { return _mounted; }
  uint32_t sizeMB() const { return _sizeMB; }

  void printInfo(Print &out = Serial);                       // 串口打印卡信息
  void listDir(const char *path = "/", int maxEntries = 20, Print &out = Serial);

private:
  bool _mounted = false;
  uint32_t _sizeMB = 0;
  uint8_t _cs = PIN_SD_CS;
};
