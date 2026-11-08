#include "esp01s.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>

/* 接收缓冲区：AT+CWLAP 的多 AP 响应可达数百字节 */
#define ESP_RX_BUFFER_SIZE 1536
/* 帧间隔：总线连续静默超过该时间，即认为一帧接收结束 */
#define ESP_FRAME_IDLE_MS 30 /* 115200 下一个字节约 87us */

static uint8_t esp_rx_byte; /* 单字节接收落点，固定不变 */
uint8_t esp_rx_buffer[ESP_RX_BUFFER_SIZE];
volatile uint16_t esp_rx_index = 0;
volatile uint32_t esp_rx_tick = 0; /* 最后一次收到字节的时刻 */
volatile uint8_t esp_rx_overflow = 0;

// 硬件复位 ESP-01S
void ESP01S_HardwareReset(void)
{
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_RESET);
  HAL_Delay(100);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_SET);
  HAL_Delay(500);
}

// 初始化：上电、复位、启动串口中断接收
void ESP01S_Init(void)
{
  /* CH_PD 拉高；若硬件已直接接到 3.3V，这句只是空操作，无害 */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_SET);
  HAL_Delay(100);

  ESP01S_HardwareReset();

  /* 复位期间可能收到杂散字节，重新挂接收前先清干净 */
  esp_rx_index = 0;
  esp_rx_overflow = 0;
  esp_rx_buffer[0] = '\0';

  /* 关键：落点必须是 esp_rx_byte，与回调保持一致 */
  HAL_UART_Receive_IT(&huart1, &esp_rx_byte, 1);
}

// 串口接收完成回调
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    if (esp_rx_index < ESP_RX_BUFFER_SIZE - 1)
    {
      esp_rx_buffer[esp_rx_index++] = esp_rx_byte;
      esp_rx_buffer[esp_rx_index] = '\0'; /* 始终保证可 strstr */
    }
    else
    {
      esp_rx_overflow = 1; /* 满了就丢，不再越界 */
    }

    esp_rx_tick = HAL_GetTick(); /* 记录“刚收到字节” */

    HAL_UART_Receive_IT(&huart1, &esp_rx_byte, 1);
  }
}

// 串口错误回调：溢出/帧错误后必须重新挂接收，否则 RX 会永久停止
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    __HAL_UART_CLEAR_OREFLAG(huart);
    __HAL_UART_CLEAR_NEFLAG(huart);
    __HAL_UART_CLEAR_FEFLAG(huart);
    HAL_UART_Receive_IT(&huart1, &esp_rx_byte, 1); /* 重新挂上 */
  }
}

// 取最近一帧原始响应，供上层打印诊断
const char *ESP01S_GetLastResponse(void)
{
  return (const char *)esp_rx_buffer;
}

// 发送AT指令并等待期望的响应
uint8_t ESP01S_SendATCommand(char *cmd, char *expected_resp, uint32_t timeout)
{
  esp_rx_index = 0;
  esp_rx_overflow = 0;
  esp_rx_buffer[0] = '\0';

  HAL_UART_Transmit(&huart1, (uint8_t *)cmd, strlen(cmd), 1000);
  HAL_UART_Transmit(&huart1, (uint8_t *)"\r\n", 2, 1000);

  return ESP01S_WaitForResponse(expected_resp, timeout);
}

// 等待响应（以总线空闲作为帧结束判据）
uint8_t ESP01S_WaitForResponse(char *expected_resp, uint32_t timeout)
{
  uint32_t start = HAL_GetTick();

  while ((HAL_GetTick() - start) < timeout)
  {
    /* 有数据 且 距最后一个字节已超过帧间隔 → 认为一帧收完 */
    if (esp_rx_index > 0 && (HAL_GetTick() - esp_rx_tick) >= ESP_FRAME_IDLE_MS)
    {
      if (strstr((char *)esp_rx_buffer, expected_resp) != NULL)
      {
        return 1;
      }
      /* 没命中不清缓冲：CWJAP 是多帧响应，后面可能还有 OK */
    }
  }
  return 0;
}

// 发送AT指令并打印响应（用于调试）
uint8_t ESP01S_SendAT(uint32_t timeout)
{
  if (!ESP01S_SendATCommand("AT", "OK", timeout))
  {
    return 1;
  }
  return 0;
}

// 连接WiFi
uint8_t ESP01S_ConnectWiFi(char *ssid, char *password)
{
  char cmd[128] = {0};
  if (!ESP01S_SendATCommand("AT+CWMODE=1", "OK", 2000))
    return 0;
  snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"", ssid, password);
  /* 连接热点 + DHCP，手册上限 15s，这里留足余量 */
  return ESP01S_SendATCommand(cmd, "OK", 20000);
}

// 建立TCP连接
uint8_t ESP01S_ConnectTCP(char *ip, char *port)
{
  char cmd[128];
  if (!ESP01S_SendATCommand("AT+CIPMUX=0", "OK", 2000))
    return 0; // 单连接模式
  snprintf(cmd, sizeof(cmd), "AT+CIPSTART=\"TCP\",\"%s\",%s", ip, port);
  return ESP01S_SendATCommand(cmd, "OK", 5000);
}

// 发送数据
void ESP01S_SendData(char *data)
{
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "AT+CIPSEND=%u", (unsigned)strlen(data));
  if (ESP01S_SendATCommand(cmd, ">", 2000))
  {
    HAL_UART_Transmit(&huart1, (uint8_t *)data, strlen(data), 1000);
    ESP01S_WaitForResponse("SEND OK", 5000);
  }
}
