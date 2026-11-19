#ifndef __CHARGING_STATUS_H__
#define __CHARGING_STATUS_H__

#include "lvgl/lvgl.h"

extern bool Chart_Update_Start ;

void charging_status_create(void); // 充电界面创建
void Charging_Status_Update(void); // 充电界面状态值更新	

#endif
