#pragma once
#include <Arduino.h>
#include <HTTPClient.h>

/*
 * 联网时钟 (NTP) + 天气 (wttr.in 文本接口)
 * WiFi 连接成功后调用 begin()，主循环调用 tick() 刷新时间显示。
 * 天气由独立 FreeRTOS 任务每 15 分钟拉取一次（HTTP 明文，无需 API Key）。
 */

// ---- 位置配置（默认杭州，按需修改）----
#define WTW_CITY_NAME  "Hangzhou"
#define WTW_LAT        30.25f
#define WTW_LON        120.17f
#define WTW_TZ_INFO    "CST-8"          // UTC+8
#define WTW_NTP_1      "ntp.aliyun.com"
#define WTW_NTP_2      "ntp1.aliyun.com"
#define WTW_NTP_3      "pool.ntp.org"

// 回调: 时间/天气有更新时通知 UI
typedef void (*TimeWeatherCb)(const char *clockStr, const char *weatherStr);

class TimeWeather {
public:
  void begin(TimeWeatherCb cb);     // WiFi 连接成功后调用（重复调用自动跳过）
  void tick();                      // 主循环调用，每秒刷新时钟
  const char *weatherText() const { return _weather; }

private:
  static void taskEntry(void *arg); // FreeRTOS 天气任务入口
  bool fetchWeather();              // 拉取一次天气, 成功返回 true

  TimeWeatherCb _cb = nullptr;
  char _weather[48] = "Weather loading...";
  char _clock[12] = "--:--";
  time_t _lastSec = -1;
};
