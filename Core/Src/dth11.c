#include <stdio.h>
#include "main.h"
#include "dth11.h"

#define DHT11_GPIO_PORT GPIOC
#define DHT11_GPIO_PIN  GPIO_PIN_3

extern TIM_HandleTypeDef htim2;

// 微秒级延时函数 (基于定时器实现)
static void DHT11_Delay_us(uint16_t us)
{
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    while (__HAL_TIM_GET_COUNTER(&htim2) < us);
}

// 设置GPIO为输出模式
static void DHT11_Set_Output(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStruct);
}

// 设置GPIO为输入模式
static void DHT11_Set_Input(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP; // 使用内部上拉
    HAL_GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStruct);
}

// 初始化DHT11 (启动定时器)
void DHT11_Init(void)
{
    HAL_TIM_Base_Start(&htim2); // 启动定时器，仅需一次
}

// 读取一个字节 (8位)
static uint8_t DHT11_Read_Byte(void)
{
    uint8_t i, byte = 0;
    for (i = 0; i < 8; i++)
    {
        // 等待低电平结束
        while (HAL_GPIO_ReadPin(DHT11_GPIO_PORT, DHT11_GPIO_PIN) == GPIO_PIN_RESET);
        // 延时40us后读取电平，高电平持续时间长则为1，短则为0
        DHT11_Delay_us(40);
        byte <<= 1;
        if (HAL_GPIO_ReadPin(DHT11_GPIO_PORT, DHT11_GPIO_PIN) == GPIO_PIN_SET)
        {
            byte |= 1;
        }
        // 等待高电平结束
        while (HAL_GPIO_ReadPin(DHT11_GPIO_PORT, DHT11_GPIO_PIN) == GPIO_PIN_SET);
    }
    return byte;
}

// 读取温湿度数据
// 返回值: 0-成功, 1-失败
uint8_t DHT11_Read_Data(uint8_t *temp, uint8_t *humi)
{
    uint8_t buf[5];
    uint8_t i;

    // 1. 主机发送起始信号
    DHT11_Set_Output();
    HAL_GPIO_WritePin(DHT11_GPIO_PORT, DHT11_GPIO_PIN, GPIO_PIN_RESET);
    HAL_Delay(18); // 拉低至少18ms
    HAL_GPIO_WritePin(DHT11_GPIO_PORT, DHT11_GPIO_PIN, GPIO_PIN_SET);
    DHT11_Delay_us(30); // 拉高20-40us
    DHT11_Set_Input();  // 立即切换为输入模式

    // 2. 检测DHT11响应
    if (HAL_GPIO_ReadPin(DHT11_GPIO_PORT, DHT11_GPIO_PIN) == GPIO_PIN_RESET)
    {
        // 等待DHT11拉低80us结束
        while (HAL_GPIO_ReadPin(DHT11_GPIO_PORT, DHT11_GPIO_PIN) == GPIO_PIN_RESET);
        // 等待DHT11拉高80us结束
        while (HAL_GPIO_ReadPin(DHT11_GPIO_PORT, DHT11_GPIO_PIN) == GPIO_PIN_SET);
    }
    else
    {
        return 1; // 未检测到响应
    }

    // 3. 读取40位数据
    for (i = 0; i < 5; i++)
    {
        buf[i] = DHT11_Read_Byte();
    }

    // 4. 校验数据
    if (buf[4] == (buf[0] + buf[1] + buf[2] + buf[3]))
    {
        *humi = buf[0]; // 湿度整数部分
        *temp = buf[2]; // 温度整数部分
        return 0;       // 成功
    }

    return 1; // 校验失败
}