#ifndef SSD1306_H
#define SSD1306_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

// 屏幕参数
#define SSD1306_ADDR       0x3C    // 在主函数中进行iIC地址扫描，如果扫描出来是0x3D，这里改成0x3D
#define SSD1306_WIDTH      128
#define SSD1306_HEIGHT     64


void SSD1306_Init(void);
void SSD1306_Clear(void);
void SSD1306_Refresh(void);
void SSD1306_ShowString(uint8_t x, uint8_t y, char *str);
void SSD1306_DrawPixel(uint8_t x, uint8_t y, uint8_t color);
uint8_t I2C_Scan(void);
#ifdef __cplusplus
}
#endif
#endif