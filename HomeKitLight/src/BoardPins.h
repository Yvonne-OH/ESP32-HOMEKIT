#pragma once
/*
 * 微雪 ESP32-C6-LCD-1.3 板级引脚定义
 * 依据官方原理图: ESP32C6-1.3.pdf
 *
 * 电源链路: VBUS(Type-C) -> ME6217C33M5G LDO -> VCC3.3
 *           数字 VDD3V3 经磁珠 L1 隔离去耦
 * 复位/启动: RST = CHIP_PU(EN); BOOT = GPIO9(按下到 GND, 10K 上拉)
 * USB: D-=GPIO12, D+=GPIO13 (CC1/CC2 各 5.1K 下拉, device 角色)
 */

// ---- 板载 WS2812B RGB 灯 ----
#define PIN_RGB_LED   8        // DIN

// ---- 1.3寸 ST7789 240x240 LCD (12pin 排线) ----
// 排线定义: 1 GND / 2 LEDK(BL, 经 Q1 SI2302 驱动, 高电平点亮)
//          3 LEDA(3V3) / 4 VDD(3V3) / 5,6 GND / 7 D/C / 8 CS
//          9 SCL / 10 SDA / 11 RES / 12 GND
#define PIN_LCD_CS    14
#define PIN_LCD_DC    15
#define PIN_LCD_RES   21
#define PIN_LCD_RST   21       // 同 PIN_LCD_RES（代码中沿用 RST 命名）
#define PIN_LCD_BL    22

// ---- TF (Micro-SD) 卡座 ----
// 卡座引脚: 2 CS=GPIO4 / 3 CMD(MOSI)=GPIO6 / 5 CLK=GPIO7 / 7 D0(MISO)=GPIO5
//           9 脚 CD 直接接地(始终识别插入); 1/8 悬空; CS 有 10K 上拉
#define PIN_SD_CS     4
#define PIN_SD_MISO   5

// ---- SPI 总线（LCD 与 TF 卡共享, 靠 CS 分时访问）----
#define PIN_SPI_SCLK  7
#define PIN_SPI_MOSI  6

// ---- 按键 ----
#define PIN_BTN_BOOT  9        // BOOT 键, 低电平有效

// ---- 扩展排针 ----
// P5: 1 GND / 2 3V3 / 3 GPIO1 / 4 GPIO2 / 5 GPIO3
#define PIN_P5_1      1
#define PIN_P5_2      2
#define PIN_P5_3      3
// P6: 1 GPIO12(USB_D-) / 2 GPIO13(USB_D+) / 3 GPIO16(U0TXD)
//     4 GPIO17(U0RXD) / 5 GPIO20 / 6 GPIO23
#define PIN_P6_5      20
#define PIN_P6_6      23

// ---- 不可用引脚 ----
// GPIO18/19: 接片内 Flash (SDIO_CMD/CLK), 用户不可用
// GPIO16/17: 复用 UART0 TX/RX (CDCOnBoot=cdc 时串口走 USB, 二者才可用于外设)
