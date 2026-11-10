/*
 * Waveshare ESP32-C6-LCD-1.3  —  Apple HomeKit 彩色灯
 *
 * 结构：
 *   HomeKitLight.ino                  入口: HomeKit 配件定义与业务逻辑
 *   src/BoardPins.h                   板级引脚定义
 *   src/drivers/rgb_led/              WS2812 RGB 灯驱动
 *   src/drivers/st7789_lcd/           1.3寸 LCD 驱动（状态显示）
 *   src/drivers/sd_card/              TF 卡驱动（挂载/信息/列目录）
 *
 * 首次使用：
 *  1. 串口监视器 115200 发送 'W' 回车，按提示输入 WiFi SSID/密码
 *  2. iPhone 家庭 App -> 添加配件 -> 更多选项 -> C6 Light
 *     配对码: 466-37-726
 */

#include "src/BoardPins.h"
#include "src/drivers/rgb_led/RgbLed.h"
#include "src/drivers/st7789_lcd/LcdDisplay.h"
#include "src/drivers/sd_card/SdCard.h"

#include "HomeSpan.h"

#define DEVICE_NAME "C6 Light"

RgbLed     rgb;
LcdDisplay lcd;
SdCard     sd;

// ------------------- HSV -> RGB（HomeKit 色彩模型转换） -------------------

void hsvToRgb(float h, float s, float v, uint8_t &r, uint8_t &g, uint8_t &b) {
  float V = v / 100.0f;
  if (s <= 0.0f) {
    r = g = b = (uint8_t)(V * 255.0f);
    return;
  }
  float H = fmodf(h, 360.0f) / 60.0f;
  int   i = (int)floorf(H);
  float f = H - i;
  float p = V * (1.0f - s / 100.0f);
  float q = V * (1.0f - s / 100.0f * f);
  float t = V * (1.0f - s / 100.0f * (1.0f - f));
  float R, G, B;
  switch (i % 6) {
    case 0: R = V; G = t; B = p; break;
    case 1: R = q; G = V; B = p; break;
    case 2: R = p; G = V; B = t; break;
    case 3: R = p; G = q; B = V; break;
    case 4: R = t; G = p; B = V; break;
    default: R = V; G = p; B = q; break;
  }
  r = (uint8_t)(R * 255.0f);
  g = (uint8_t)(G * 255.0f);
  b = (uint8_t)(B * 255.0f);
}

// ------------------- HomeKit 彩色灯配件 -------------------

struct ColorBulb : Service::LightBulb {
  SpanCharacteristic *power;   // 开关
  SpanCharacteristic *level;   // 亮度 5-100%
  SpanCharacteristic *hue;     // 色相 0-360
  SpanCharacteristic *sat;     // 饱和度 0-100

  ColorBulb() : Service::LightBulb() {
    power = new Characteristic::On(0, true);
    level = new Characteristic::Brightness(100, true);
    level->setRange(5, 100, 1);
    hue   = new Characteristic::Hue(0, true);
    sat   = new Characteristic::Saturation(100, true);
    apply();
  }

  boolean update() override {   // HomeKit 端特征值变化时触发
    apply();
    return true;
  }

  void apply() {
    uint8_t r, g, b;
    hsvToRgb(hue->getVal(), sat->getVal(),
             power->getVal() ? (float)level->getVal() : 0.0f, r, g, b);
    rgb.setColor(r, g, b);
    lcd.drawLightState(r, g, b, power->getVal(), (int)level->getVal(),
                       hue->getVal(), sat->getVal());
    LOG1("RGB(%d,%d,%d) P=%d L=%d H=%.0f S=%.0f\n",
         r, g, b, (int)power->getVal(), (int)level->getVal(),
         (double)hue->getVal(), (double)sat->getVal());
  }
};

void onConnect(int status) {
  if (status > 0) {
    char buf[64];
    snprintf(buf, sizeof(buf), "WiFi OK  %s", WiFi.localIP().toString().c_str());
    lcd.drawStatusLine(buf, ST77XX_GREEN);
  } else {
    lcd.drawStatusLine("WiFi LOST", ST77XX_RED);
  }
}

// ------------------- 设置与主循环 -------------------

void setup() {
  Serial.begin(115200);

  lcd.begin();
  rgb.begin(PIN_RGB_LED);

  // TF 卡（与 LCD 共享 SPI 总线，须在 lcd.begin() 之后挂载）
  lcd.showBoot("Mounting SD card...");
  if (sd.begin()) {
    char buf[40];
    snprintf(buf, sizeof(buf), "SD OK  %u MB", sd.sizeMB());
    lcd.drawStatusLine(buf, ST77XX_GREEN);
  } else {
    lcd.drawStatusLine("SD not found", 0x7BEF);
  }
  sd.printInfo();
  sd.listDir();

  lcd.showBoot("Starting HomeSpan...");

  homeSpan.setLogLevel(1);
  homeSpan.setConnectionCallback(onConnect);
  homeSpan.begin(Category::Lighting, DEVICE_NAME);

  new SpanAccessory();
  new Service::AccessoryInformation();
  new Characteristic::Identify();
  new ColorBulb();

  Serial.printf("\n=== %s (HomeKit) ===\n", DEVICE_NAME);
}

void loop() {
  homeSpan.poll();
}
