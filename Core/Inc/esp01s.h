#ifndef ESP01S_H
#define ESP01S_H

#ifdef __cplusplus
extern "C" {    
#endif

#include "main.h"

// 外部引用 CubeMX 生成的句柄
extern UART_HandleTypeDef huart1;

// 函数声明
void ESP01S_Init(void);
uint8_t ESP01S_SendATCommand(char* cmd, char* expected_resp, uint32_t timeout);
uint8_t ESP01S_ConnectWiFi(char* ssid, char* password);
uint8_t ESP01S_ConnectTCP(char* ip, char* port);
void ESP01S_SendData(char* data);
uint8_t ESP01S_WaitForResponse(char* expected_resp, uint32_t timeout);
uint8_t ESP01S_SendAT(uint32_t timeout);
const char* ESP01S_GetLastResponse(void);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ESP01S_H */