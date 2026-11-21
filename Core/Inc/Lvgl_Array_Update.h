#ifndef LVGL_ARRAY_UPDATE
#define LVGL_ARRAY_UPDATE

#include "stm32f4xx.h"

#include "stdbool.h"

#define Lvgl_Chart_buff_Max_Size 30//LVGL图表最大显示数量

typedef struct 
{
    uint16_t current_size;                     // 数组内有效数量
    uint16_t Max_Date;                         // 数组内最大值元素
    float buff[Lvgl_Chart_buff_Max_Size];   // 链接数组
}Lvgl_Chart_buff;

extern Lvgl_Chart_buff User_Change_message_date;

void buff_upadte(Lvgl_Chart_buff *Chart_buff, uint16_t new_date,bool Per_Minute_Data);//数组循环更新，返回数组中最大值

#endif
