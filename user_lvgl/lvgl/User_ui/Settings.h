#ifndef __SETTINGS_H__
#define __SETTINGS_H__
#include "lvgl/lvgl.h"

extern uint32_t Sleep_Time_Selected_Index;
extern bool Settings_Brightness_Adaptation;
extern uint8_t Settings_Brightness;
extern uint32_t Settings_Screen_Saver_Time_Index;
extern bool Settings_Screen_Saver;
extern bool Settings_Time_Schedule;
extern uint32_t Settings_Screen_Saver_Type_Index;
extern bool Settings_Screen_Lock;
extern uint32_t Settings_Font_Size_Index;

void settings_init(void);

#endif
