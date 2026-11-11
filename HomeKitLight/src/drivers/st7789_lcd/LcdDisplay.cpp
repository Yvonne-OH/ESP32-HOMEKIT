#include "LcdDisplay.h"
#include "src/BoardPins.h"
#include "qrcode.h"    // 本目录自带副本, 避免与 IDF 内置 qrcode.h 重名冲突

// ---- 布局常量 (240x240) ----
static const int16_t  TITLE_H   = 26;      // 标题栏高度
static const uint16_t COL_NAVY  = 0x00185A; // 标题栏深蓝
static const uint16_t COL_DIM   = 0x7BEF;   // 次要文字灰
static const uint16_t COL_BAR   = 0x08421;  // 底部状态栏深灰

void LcdDisplay::begin() {
  pinMode(PIN_LCD_BL, OUTPUT);
  digitalWrite(PIN_LCD_BL, HIGH);                       // 开背光
  SPI.begin(PIN_SPI_SCLK, PIN_SD_MISO, PIN_SPI_MOSI, PIN_LCD_CS);
  _tft.init(240, 240);
  _tft.setRotation(0);
  _tft.cp437(true);               // 扩展字符（度数符号 °）
  _tft.fillScreen(ST77XX_BLACK);
}

void LcdDisplay::drawTitleBar(const char *clockStr) {
  _tft.fillRect(0, 0, 240, TITLE_H, COL_NAVY);
  _tft.setTextSize(2);
  _tft.setTextColor(ST77XX_WHITE);
  _tft.setCursor(12, 7);
  _tft.print("C6 Light");
  if (clockStr && clockStr[0]) {
    // 右对齐时间
    _tft.setTextColor(ST77XX_CYAN);
    int16_t w = strlen(clockStr) * 12;
    _tft.setCursor(228 - w, 7);
    _tft.print(clockStr);
  }
}

void LcdDisplay::showBoot(const char *msg) {
  _state = UI_BOOT;
  _tft.fillScreen(ST77XX_BLACK);
  drawTitleBar(nullptr);
  _tft.setTextSize(1);
  _tft.setTextColor(COL_DIM);
  _tft.setCursor(16, 120);
  _tft.print(msg);
}

// ------------------- 配网页 (二维码铺满全屏, 按 BOOT 呼出/退出) -------------------

void LcdDisplay::drawProvisioning(const char *qrText) {
  _state = UI_PROVISIONING;
  _tft.fillScreen(ST77XX_WHITE);                          // 全屏白底=静区

  QRCode qrcode;
  uint8_t qdata[400];
  qrcode_initText(&qrcode, qdata, 3, ECC_LOW, qrText);    // 版本3: 29x29 模块
  int scale = 240 / qrcode.size;                          // 尽量铺满: 8px/模块
  int off = (240 - qrcode.size * scale) / 2;              // 居中
  for (int y = 0; y < qrcode.size; y++)
    for (int x = 0; x < qrcode.size; x++)
      if (qrcode_getModule(&qrcode, x, y))
        _tft.fillRect(off + x * scale, off + y * scale, scale, scale, ST77XX_BLACK);
}

// ------------------- 主界面 -------------------

void LcdDisplay::drawHome() {
  _state = UI_HOME;
  _tft.fillScreen(ST77XX_BLACK);
  drawTitleBar(nullptr);

  // 颜色预览圆占位 (由 drawLightState 填充)
  _tft.drawCircle(120, 100, 56, COL_DIM);

  // 信息行左: 开关+亮度占位
  _tft.setTextSize(2);
  _tft.setTextColor(COL_DIM);
  _tft.setCursor(16, 172);
  _tft.print("--");

  // 底部状态栏
  _tft.fillRect(0, 222, 240, 18, COL_BAR);
}

void LcdDisplay::drawClock(const char *clockStr) {
  if (_state != UI_HOME || !clockStr || !clockStr[0]) return;
  _tft.fillRect(150, 0, 90, TITLE_H, COL_NAVY);   // 只清标题栏右侧
  _tft.setTextSize(2);
  _tft.setTextColor(ST77XX_CYAN);
  _tft.setCursor(228 - strlen(clockStr) * 12, 7);
  _tft.print(clockStr);
}

void LcdDisplay::drawLightState(uint8_t r, uint8_t g, uint8_t b,
                                bool on, int level) {
  if (_state != UI_HOME) return;

  // 颜色预览圆
  if (on && (r + g + b) > 0)
    _tft.fillCircle(120, 100, 56, _tft.color565(r, g, b));
  else {
    _tft.fillCircle(120, 100, 56, 0x2108);              // 熄灭: 暗灰
    _tft.drawCircle(120, 100, 56, COL_DIM);             // 边框
  }

  // 信息行左: 开关 + 亮度 (单行, 不换行)
  _tft.fillRect(0, 166, 130, 28, ST77XX_BLACK);
  _tft.setTextSize(2);
  _tft.setCursor(16, 172);
  if (on) {
    _tft.setTextColor(ST77XX_GREEN);
    _tft.printf("ON %d%%", level);
  } else {
    _tft.setTextColor(COL_DIM);
    _tft.print("OFF");
  }
}

void LcdDisplay::drawWeather(const char *weatherStr) {
  if (_state != UI_HOME || !weatherStr || !weatherStr[0]) return;
  _tft.fillRect(130, 166, 110, 28, ST77XX_BLACK);       // 信息行右
  _tft.setTextSize(1);
  _tft.setTextColor(0xFD20);                            // 橙黄
  int16_t w = strlen(weatherStr) * 6;
  _tft.setCursor(228 - w, 182);                         // 右对齐, 底部基线
  _tft.print(weatherStr);
}

void LcdDisplay::drawStatusLine(const char *txt, uint16_t color) {
  _tft.fillRect(0, 222, 240, 18, _state == UI_HOME ? COL_BAR : ST77XX_BLACK);
  _tft.setTextSize(1);
  _tft.setTextColor(color);
  _tft.setCursor(16, 226);
  _tft.print(txt);
}
