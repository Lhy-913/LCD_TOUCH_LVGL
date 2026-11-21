#include "printf.h"
#include <stdarg.h>

/* ARM Compiler 5 默认库禁用半主机，允许脱离调试器运行。 */
#if defined(__CC_ARM) && !defined(__MICROLIB)
#pragma import(__use_no_semihosting)

struct __FILE
{
    int handle;
};
FILE __stdout;

int ferror(FILE *f)
{
    (void)f;
    return 0;
}

void _ttywrch(int ch)
{
    uint8_t data = (uint8_t)ch;
    (void)HAL_UART_Transmit(&UART_PRINTF_HANDLE, &data, 1, HAL_MAX_DELAY);
}

void _sys_exit(int status)
{
    (void)status;
    while (1)
    {
    }
}
#endif

int fputc(int ch, FILE *f)
{
    uint8_t data = (uint8_t)ch;
    (void)f;
    if (HAL_UART_Transmit(&UART_PRINTF_HANDLE, &data, 1, HAL_MAX_DELAY) != HAL_OK)
    {
        return EOF;
    }
    return ch;
}

int uart_printf(const char *format, ...)
{
    char buffer[UART_PRINTF_BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    int length = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (length < 0)
    {
        return -1;
    }
    if (length >= (int)sizeof(buffer))
    {
        length = (int)sizeof(buffer) - 1;
    }
    if (length > 0 && HAL_UART_Transmit(&UART_PRINTF_HANDLE, (uint8_t *)buffer,
                                        (uint16_t)length, HAL_MAX_DELAY) != HAL_OK)
    {
        return -1;
    }
    return length;
}
