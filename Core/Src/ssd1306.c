#include "memory.h"
#include "ssd1306.h"
#include "font.h"

extern I2C_HandleTypeDef hi2c1;

uint8_t ssd1306_buf[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

static void SSD1306_WriteCmd(uint8_t cmd)
{
    HAL_I2C_Mem_Write(&hi2c1, SSD1306_ADDR << 1, 0x00, I2C_MEMADD_SIZE_8BIT, &cmd, 1, 100);
}

static void SSD1306_WriteData(uint8_t data)
{
    HAL_I2C_Mem_Write(&hi2c1, SSD1306_ADDR << 1, 0x40, I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
}

void SSD1306_Init(void)
{
    HAL_Delay(100);
    SSD1306_WriteCmd(0xAE); // 关闭显示
    SSD1306_WriteCmd(0xD5); // 时钟分频
    SSD1306_WriteCmd(0x80);
    SSD1306_WriteCmd(0xA8); // 复用率
    SSD1306_WriteCmd(0x3F); // 64行
    SSD1306_WriteCmd(0xD3); // 偏移
    SSD1306_WriteCmd(0x00);
    SSD1306_WriteCmd(0x40); // 起始行
    SSD1306_WriteCmd(0x8D); // 电荷泵
    SSD1306_WriteCmd(0x14);
    SSD1306_WriteCmd(0x20); // 地址模式
    SSD1306_WriteCmd(0x00); // 水平模式
    SSD1306_WriteCmd(0xA1); // 段映射
    SSD1306_WriteCmd(0xC8); // COM方向
    SSD1306_WriteCmd(0xDA);
    SSD1306_WriteCmd(0x12);
    SSD1306_WriteCmd(0x81); // 对比度
    SSD1306_WriteCmd(0xCF);
    SSD1306_WriteCmd(0xD9);
    SSD1306_WriteCmd(0xF1);
    SSD1306_WriteCmd(0xDB);
    SSD1306_WriteCmd(0x30);
    SSD1306_WriteCmd(0xA4);
    SSD1306_WriteCmd(0xA6); // 正常显示
    SSD1306_WriteCmd(0xAF); // 打开屏幕

    SSD1306_Clear();
    SSD1306_Refresh();
}

void SSD1306_Clear(void)
{
    memset(ssd1306_buf, 0x00, sizeof(ssd1306_buf));
}

void SSD1306_Refresh(void)
{
    SSD1306_WriteCmd(0x21);
    SSD1306_WriteCmd(0x00);
    SSD1306_WriteCmd(SSD1306_WIDTH - 1);
    SSD1306_WriteCmd(0x22);
    SSD1306_WriteCmd(0x00);
    SSD1306_WriteCmd(7);

    for(uint8_t page = 0; page < 8; page++)
    {
        SSD1306_WriteCmd(0xB0 + page);
        SSD1306_WriteCmd(0x00);
        SSD1306_WriteCmd(0x10);
        for(uint8_t i = 0; i < SSD1306_WIDTH; i++)
        {
            SSD1306_WriteData(ssd1306_buf[page * SSD1306_WIDTH + i]);
        }
    }
}

// 画点 color=1点亮，0熄灭
void SSD1306_DrawPixel(uint8_t x, uint8_t y, uint8_t color)
{
    if(x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) return;
    if(color)
        ssd1306_buf[x + (y / 8) * SSD1306_WIDTH] |= (1 << (y % 8));
    else
        ssd1306_buf[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
}

// 6*8小字显示
void SSD1306_ShowString(uint8_t x, uint8_t y, char *str)
{
    uint8_t i = 0;
    while(str[i])
    {
        SSD1306_DrawChar(x, y, str[i]);
        x += 6;
        if(x > SSD1306_WIDTH -6)
        {
            x = 0;
            y +=8;
        }
        i++;
    }
}

uint8_t I2C_Scan(void)
{
  uint8_t i;
  for(i=0;i<128;i++)
  {
    if(HAL_I2C_IsDeviceReady(&hi2c1, i << 1, 1, 10) == HAL_OK)
    {
        return i;
    }
  }
  return 0xFF;
}
