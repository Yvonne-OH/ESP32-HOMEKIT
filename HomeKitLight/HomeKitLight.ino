/*
 * Waveshare ESP32-C6-LCD-1.3  —  Apple HomeKit 彩色灯
 *
 * 结构：
 *   HomeKitLight.ino                  入口: HomeKit 配件定义与业务逻辑
 *   src/BoardPins.h                   板级引脚定义
 *   src/drivers/rgb_led/              WS2812 RGB 灯驱动
 *   src/drivers/st7789_lcd/           1.3寸 LCD 驱动（时钟/天气/灯状态显示）
 *   src/drivers/sd_card/              TF 卡驱动（挂载/信息/列目录）
 *   src/net/time_weather/             NTP 时钟 + wttr.in 天气
 *
 * 首次使用：
 *  1. 串口监视器 115200 发送 'W' 回车，按提示输入 WiFi SSID/密码
 *  2. 按 BOOT 键呼出配对二维码（全屏），再按退出；配对成功自动退出
 *  3. iPhone 家庭 App -> 添加配件 -> 更多选项 -> C6 Light
 *     配对码: 466-37-726
 */

#include "src/BoardPins.h"
#include "src/drivers/rgb_led/RgbLed.h"
#include "src/drivers/st7789_lcd/LcdDisplay.h"
#include "src/drivers/sd_card/SdCard.h"
#include "src/net/time_weather/TimeWeather.h"

#include "HomeSpan.h"

#define DEVICE_NAME "C6 Light"
#define QR_SETUP_ID "C6LT"    // HomeKit 二维码 SetupID (4位)
#define SETUP_CODE  "46637726" // 配对码，须与串口 'S' 命令设置一致

RgbLed     rgb;
LcdDisplay lcd;
SdCard     sd;
TimeWeather netTime;

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
  static ColorBulb *instance;
  SpanCharacteristic *power;   // 开关
  SpanCharacteristic *level;   // 亮度 5-100%
  SpanCharacteristic *hue;     // 色相 0-360
  SpanCharacteristic *sat;     // 饱和度 0-100

  ColorBulb() : Service::LightBulb() {
    instance = this;
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
    lcd.drawLightState(r, g, b, power->getVal(), (int)level->getVal());
    LOG1("RGB(%d,%d,%d) P=%d L=%d\n",
         r, g, b, (int)power->getVal(), (int)level->getVal());
  }
};

ColorBulb *ColorBulb::instance = nullptr;

// 时钟/天气数据回调 -> 刷到屏幕
void onNetData(const char *clockStr, const char *weatherStr) {
  lcd.drawClock(clockStr);
  lcd.drawWeather(weatherStr);
}

// ------------------- 配网页 (按 BOOT 呼出/退出, 配对成功自动退出) -------------------

bool provisioningOn = false;

void enterProvisioning() {
  // HomeKit 标准配对二维码: X-HM://00 + 8位配对码 + 4位 SetupID
  char qr[32];
  snprintf(qr, sizeof(qr), "X-HM://00%s%s", SETUP_CODE, QR_SETUP_ID);
  lcd.drawProvisioning(qr);          // 二维码铺满全屏
  provisioningOn = true;
  Serial.println("[UI] provisioning screen ON");
}

void exitProvisioning() {
  if (!provisioningOn) return;
  lcd.drawHome();
  if (ColorBulb::instance)
    ColorBulb::instance->apply();
  lcd.drawStatusLine(WiFi.localIP().toString().c_str(), ST77XX_GREEN);
  provisioningOn = false;
  Serial.println("[UI] provisioning screen OFF");
}

// BOOT 键 (GPIO9, 低电平按下): 切换配网页
void uiButtonTick() {
  static bool lastLevel = HIGH;
  static uint32_t lastMs = 0;
  bool level = digitalRead(PIN_BTN_BOOT);
  if (level != lastLevel && millis() - lastMs > 50) {   // 消抖
    lastMs = millis();
    lastLevel = level;
    if (level == LOW)                                   // 按下瞬间触发
      provisioningOn ? exitProvisioning() : enterProvisioning();
  }
}

// 配对状态回调: 配对成功自动退出配网页
void onPair(bool isPaired) {
  if (isPaired) exitProvisioning();
}

void onConnect(int status) {
  if (status > 0) {
    if (!provisioningOn) {
      lcd.drawHome();                  // 整屏切换到主界面
      if (ColorBulb::instance)
        ColorBulb::instance->apply();  // 圆形色块 + 开关/亮度
      lcd.drawStatusLine(WiFi.localIP().toString().c_str(), ST77XX_GREEN);
    }
    netTime.begin(onNetData);          // 启动 NTP + 天气
  } else if (!provisioningOn) {
    lcd.drawStatusLine("WiFi disconnected", ST77XX_RED);
  }
}

// ------------------- 设置与主循环 -------------------

void setup() {
  Serial.begin(115200);
  pinMode(PIN_BTN_BOOT, INPUT_PULLUP);   // BOOT 键

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
  homeSpan.setPairCallback(onPair);      // 配对成功 -> 自动退出配网页
  homeSpan.setQRID(QR_SETUP_ID);         // 二维码 SetupID
  homeSpan.begin(Category::Lighting, DEVICE_NAME);

  new SpanAccessory();
  new Service::AccessoryInformation();
  new Characteristic::Identify();
  new ColorBulb();

  // 开机进入主界面 (按 BOOT 键可呼出配对二维码)
  lcd.drawHome();
  ColorBulb::instance->apply();
  lcd.drawStatusLine("Connecting WiFi...", 0x7BEF);

  Serial.printf("\n=== %s (HomeKit) ===\n", DEVICE_NAME);
}

void loop() {
  homeSpan.poll();
  netTime.tick();      // 每秒刷新时钟
  uiButtonTick();      // BOOT 键: 呼出/退出配网页
}
