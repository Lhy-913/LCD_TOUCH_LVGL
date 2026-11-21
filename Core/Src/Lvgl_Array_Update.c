#include "Lvgl_Array_Update.h"
#include "printf.h"
#include "stdbool.h"

Lvgl_Chart_buff User_Change_message_date = {0};

void buff_upadte(Lvgl_Chart_buff *Chart_buff, uint16_t new_date, bool Per_Minute_Data)
{
    // 防止空内容传入
    if (Chart_buff == NULL)
    {
        uart_printf("An empty value was passed in");
        return;
    }

    // 判断当前数组是否已满
    if (Chart_buff->current_size < Lvgl_Chart_buff_Max_Size)
    {
        // 未满，直接添加数据
        Chart_buff->buff[Chart_buff->current_size] = new_date;
        Chart_buff->current_size++;
    }
    else
    {
        // 已满，所有数据向前移动一位
        for (uint16_t i = 0; i < Lvgl_Chart_buff_Max_Size - 1; i++)
        {
            Chart_buff->buff[i] = Chart_buff->buff[i + 1];
        }

        // 最后一个位置填充新数据
        Chart_buff->buff[Lvgl_Chart_buff_Max_Size - 1] = new_date;
    }

    for (uint8_t i = 0; i < (Per_Minute_Data ? 30 : 15); i++)
    {
        if (Chart_buff->Max_Date < Chart_buff->buff[i])
            Chart_buff->Max_Date = Chart_buff->buff[i];
    }
}
