#ifndef DTH_H
#define DTH_H

#ifdef __cplusplus
extern "C" {
#endif

extern TIM_HandleTypeDef htim2;

// 函数声明
void DHT11_Init(void);
uint8_t DHT11_Read_Data(uint8_t *temp, uint8_t *humi);

#ifdef __cplusplus
}
#endif
#endif