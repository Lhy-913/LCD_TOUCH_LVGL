#ifndef LVGL_ARRAY_UPDATE
#define LVGL_ARRAY_UPDATE

#include "stm32f4xx.h"

#define Lvgl_Chart_buff_Max_Size 30//LVGL图表最大显示数量

typedef struct 
{
    uint16_t current_size;                     // 数组内有效数量
    uint16_t buff[Lvgl_Chart_buff_Max_Size];   // 链接数组
}Lvgl_Chart_buff;

extern Lvgl_Chart_buff User_Change_message_date;

void buff_upadte(Lvgl_Chart_buff *Chart_buff,uint16_t new_date);

#endif
