#ifndef FONT_H
#define FONT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ssd1306.h"
void SSD1306_DrawChar(uint8_t x,uint8_t y,uint8_t ch);
extern const unsigned char F6x8[][6];

#ifdef __cplusplus
}
#endif
#endif