#ifndef UI_H
#define UI_H

#include "lvgl/lvgl.h"
#include <stdint.h>

extern lv_obj_t *scr;
extern lv_obj_t *main_container;
extern lv_obj_t *User_Screen;
extern lv_obj_t *User_Selection_Screen;
extern uint8_t user_chooser;

#define User_Operation_Screen_Container_Height 89
#define User_Selection_Screen_Container_Height 11


#define LINE_COLOR 0x8FE649 //荧光绿
#define BACKGROUND_COLOR 0x0B0F14 //科技黑
#define Information_Panel 0x202225 //科技灰

/* 函数声明 */
void ui_create(void);
void General_UI_Properties(lv_obj_t * obj);//常规UI属性设置（禁止滚动显示，去圆角、边框、内边距）

#endif
