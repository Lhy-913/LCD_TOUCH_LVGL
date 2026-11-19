#include "CAN_To_BMS.h"
#include "stm32f4xx.h"
#include "usart.h"
#include "printf.h"
#include <string.h>

#include "Lvgl_Array_Update.h"
#include "Charging_Status.h"

#include "can.h"

extern bool Per_Minute_Data;
extern lv_obj_t * Charging_Chart;//充电状态图表
extern lv_chart_series_t * ser;  //充电状态图表 - 数据序列1

extern const float *data;
    ;
uint16_t Estimated_Charging_Time = 0;//剩余充电时间(单位分钟)
float Soc,PowerReal,PowerReal_x10;//剩余电量，实时功率

// BMS_CAN 发送/接收头
CAN_TxHeaderTypeDef BMS_CAN_TxHeader;
CAN_RxHeaderTypeDef BMS_CAN_RxHeader;

BMS_SysModeTypedef IGT6_Send_Chonge_Status_Commond;


//soc计算

static bool eta_initialized = false;
static bool eta_rate_valid = false;
static float eta_base_soc = 0.0f;
static float eta_rate_percent_per_min = 0.0f;
static uint32_t eta_base_tick = 0;


void Can_send_message(uint32_t id, uint8_t *data, uint8_t length)
{
    BMS_CAN_TxHeader.StdId = id;                    // 标识号
    BMS_CAN_TxHeader.ExtId = 0x00;                  // 扩展标识号
    BMS_CAN_TxHeader.IDE = CAN_ID_STD;              // 标识符类型
    BMS_CAN_TxHeader.RTR = CAN_RTR_DATA;            // 远程帧类型
    BMS_CAN_TxHeader.DLC = length;                  // 数据长度
    BMS_CAN_TxHeader.TransmitGlobalTime = DISABLE;  // 全局时间戳
    uint32_t TxMailbox;

    if (HAL_CAN_AddTxMessage(&hcan1, &BMS_CAN_TxHeader, data, &TxMailbox) != HAL_OK)
    {
        // 发送失败处理
        HAL_UART_Transmit(&huart1,(uint8_t*)("send message error!"),sizeof(("send message error!")),HAL_MAX_DELAY);
        return;
    }
    
    if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0)
    {
        HAL_UART_Transmit(&huart1,(uint8_t*)("TxMailboxes is full!"),sizeof(("TxMailboxes is full!")),HAL_MAX_DELAY);
        return;
    }
    uart_printf("AddTxMessage OK!");

}
void Can_read_FIFO0_message(uint32_t *id, uint8_t *data, uint8_t *length)
{
    uint8_t HAL_Status_Number;
    if (HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0) == 0)
    {
        //没有待接收的报文
        uart_printf("NO Message!");
        return;
    }
    HAL_Status_Number = HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &BMS_CAN_RxHeader, data);
    if (HAL_Status_Number != HAL_OK)
    {
        // 接收失败处理  
        uart_printf("Get Message Error! Error_Number is %d",HAL_Status_Number);
        return;
    }
    *id = BMS_CAN_RxHeader.StdId;       // 读取标识号
    *length = BMS_CAN_RxHeader.DLC;     // 读取接收的数据长度
}

void Can_Get_BMS_Message(void)
{
    uint32_t id = 0;
    uint8_t buff[8];
    uint8_t length = 0;
    Can_read_FIFO0_message(&id,buff,&length);
    if(id == Get_Change_info)
    {        
        //将数据完整复制到buff
        memcpy(&Soc,buff,sizeof(float));
        memcpy(&PowerReal,buff+4,sizeof(float));
        PowerReal_x10 = PowerReal*10;
        //打印充电状态数据
        uart_printf("Rx_id: %lu  SOC: %.2f  PowerReal: %.2f W\r\n",
                    (unsigned long)id,
                    Soc,
                    PowerReal);
                    
        uint32_t now = HAL_GetTick();

if (!eta_initialized)
{
    eta_base_soc = Soc;          // Soc 的单位是百分比，范围 0～100
    eta_base_tick = now;
    eta_initialized = true;
}
else if ((uint32_t)(now - eta_base_tick) >= 300000U) // 5 分钟
{
    uint32_t elapsed_ms = now - eta_base_tick;
    eta_rate_percent_per_min =
        (Soc - eta_base_soc) * 60000.0f / elapsed_ms;

    eta_rate_valid = (eta_rate_percent_per_min > 0.01f);
    eta_base_soc = Soc;
    eta_base_tick = now;
}

if (Soc >= 100.0f)
{
    Estimated_Charging_Time = 0;
}
else if (eta_rate_valid)
{
    float minutes_left = (100.0f - Soc) / eta_rate_percent_per_min;
    Estimated_Charging_Time = (uint16_t)(minutes_left + 0.5f);
}
else if (PowerReal > 0.5f) // 最近 5 分钟尚未测出有效 SOC 增长率时
{
    Estimated_Charging_Time =
        (uint16_t)(((Battery_Capacity_Ah * Nominal_Voltage) *
        (1.0f - Soc / 100.0f) / PowerReal * 60.0f) + 0.5f);
}
else
{
    Estimated_Charging_Time = 0; // 暂无有效充电速度，无法估算
}
            
        buff_upadte(&User_Change_message_date,PowerReal_x10,Per_Minute_Data);
        for(uint32_t i = 0; i < (Per_Minute_Data ? 30:15); i++)
        {
            lv_chart_set_series_value_by_id(Charging_Chart, ser, i, data[i]);
        }
    }
}


void Can_User_Filter_Init(void)
{
    CAN_FilterTypeDef user_filter = {0};
    user_filter.FilterBank = 0;
    user_filter.FilterActivation = ENABLE;
    user_filter.FilterMode = CAN_FILTERMODE_IDLIST;
    user_filter.FilterFIFOAssignment = CAN_FilterFIFO0;
    user_filter.FilterIdHigh = (0x111 << 5);

    user_filter.FilterScale  = CAN_FILTERSCALE_16BIT;
    user_filter.SlaveStartFilterBank = 14;
    HAL_CAN_ConfigFilter(&hcan1, &user_filter);
    
    HAL_CAN_Start(&hcan1);
}

