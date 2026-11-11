#include "TimeWeather.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "HomeSpan.h"     // LOG1

void TimeWeather::begin(TimeWeatherCb cb) {
  if (_cb) return;                  // 已初始化（回调可能触发多次）
  _cb = cb;
  // NTP: 东八区
  configTzTime(WTW_TZ_INFO, WTW_NTP_1, WTW_NTP_2, WTW_NTP_3);

  // 独立任务拉天气: 8KB 栈（HTTPS 握手较吃栈）
  xTaskCreate(taskEntry, "weather", 8192, this, 5, nullptr);
}

void TimeWeather::tick() {
  struct tm t;
  if (!getLocalTime(&t, 0)) return;          // NTP 未同步时不刷
  if (t.tm_sec == _lastSec) return;          // 同一秒内不重复刷
  _lastSec = t.tm_sec;
  strftime(_clock, sizeof(_clock), "%H:%M", &t);
  if (_cb) _cb(_clock, _weather);
}

void TimeWeather::taskEntry(void *arg) {
  auto *self = (TimeWeather *)arg;
  for (;;) {
    bool ok = self->fetchWeather();
    // 失败 30 秒后重试, 成功 15 分钟后刷新
    vTaskDelay(pdMS_TO_TICKS(ok ? 15 * 60 * 1000 : 30 * 1000));
  }
}

bool TimeWeather::fetchWeather() {
  HTTPClient http;
  // wttr.in 文本接口: 响应即一行 ASCII, 如 "Cloudy +23°C"
  char url[160];
  snprintf(url, sizeof(url),
           "http://wttr.in/%s?format=%%25C%%20%%25t", WTW_CITY_NAME);

  if (!http.begin(url)) return false;
  http.setTimeout(10000);
  http.setUserAgent("curl/8.0");
  int code = http.GET();
  bool ok = false;
  if (code == 200) {
    String body = http.getString();
    body.trim();
    if (body.length() > 0 && body.length() < sizeof(_weather)) {
      strlcpy(_weather, body.c_str(), sizeof(_weather));
      if (_cb) _cb(_clock, _weather);
      LOG1("[Weather] %s\n", _weather);
      ok = true;
    }
  } else {
    LOG1("[Weather] HTTP %d\n", code);
  }
  http.end();
  return ok;
}
