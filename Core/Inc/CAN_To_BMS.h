#ifndef __CAN_TO_BMS_H
#define __CAN_TO_BMS_H

#include "stm32f4xx.h"

#define Adjust_Charging_Status 0x112      // 修改BMS充电状态
#define Get_Change_info 0x111             // 获取BMS充电信息(内容是SOC和实时功率)

#define Battery_Capacity_Ah 2.2
#define Nominal_Voltage 18.5

void Can_User_Filter_Init(void);
void Can_send_message(uint32_t id, uint8_t *data, uint8_t length);
void Can_read_FIFO0_message(uint32_t *id, uint8_t *data, uint8_t *length);
void Can_Get_BMS_Message(void);

extern uint16_t Estimated_Charging_Time;//剩余充电时间(单位分钟)
extern float PowerReal;//实时功率
extern float PowerReal_x10;//实时功率*10

typedef enum
{
	BMS_MODE_NULL,
	BMS_MODE_CHARGE,	// 充电模式
	BMS_MODE_DISCHARGE,	// 放电模式
	BMS_MODE_STANDBY,	// 待机模式
	BMS_MODE_SLEEP,		// 睡眠模式
}BMS_SysModeTypedef;

#endif 
