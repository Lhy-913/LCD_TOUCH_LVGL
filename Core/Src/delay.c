#include "delay.h"
#include "stdint.h"
#include "stm32f4xx.h"

// 获取 HCLK 时钟频率对应的 MHz 数值
// 例如 HCLK = 168MHz，则 System_Clock = 168
#define System_Clock (HAL_RCC_GetHCLKFreq()/1000000)

void delay_us(uint32_t time)
{
    uint32_t tick;           // 目标延时对应的时钟周期数
    uint32_t told, tnow;     // 上一次和当前读取的 SysTick 计数值
    uint32_t tcnt = 0;       // 累计经过的时钟周期数
    uint32_t load = SysTick->LOAD; // SysTick 重装载值

    // 将延时时间转换为时钟周期数
    // 例如 HCLK = 168MHz，延时 10us，则 tick = 1680
    tick = (uint64_t)time * System_Clock / 1000000;

    // 记录初始 SysTick 当前计数值
    told = SysTick->VAL;

    while(1)
    {
        // 读取当前 SysTick 计数值
        // SysTick 通常向下计数，计数到 0 后重新装载
        tnow = SysTick->VAL;

        // 只有计数值发生变化时，才计算经过的时钟周期
        if(tnow != told)
        {
            if(told > tnow)
            {
                // 正常向下计数，未发生重装载
                // 经过的周期数 = 上一次计数值 - 当前计数值
                tcnt += told - tnow;
            }
            else
            {
                // 发生重装载，计数值从 0 重新回到 LOAD
                // 经过的周期数 = 上一次计数值 + LOAD - 当前计数值
                tcnt += load - tnow + told;
            }

            // 更新上一次的计数值，为下一次计算做准备
            told = tnow;

            // 累计周期数达到目标值，结束延时
            if(tcnt >= tick)
            {
                break;
            }
        }
    }
}