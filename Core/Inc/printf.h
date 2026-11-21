#ifndef UART_PRINTF_H
#define UART_PRINTF_H

#include <stdio.h>
#include "usart.h"

/* 修改此句柄选择串口，需先完成对应串口初始化。 */
#define UART_PRINTF_HANDLE huart1
#define UART_PRINTF_BUFFER_SIZE 256

/* 返回实际发送字节数，格式化或发送失败返回 -1。
 * 阻塞发送，单条最多 255 字节，超出部分截断；仅在主循环调用。 */
int uart_printf(const char *format, ...);

#endif
