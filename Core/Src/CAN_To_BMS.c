#include "CAN_To_BMS.h"
#include "stm32f4xx.h"
#include "usart.h"
#include <string.h>

#include "Lvgl_Array_Update.h"
#include "Charging_Status.h"

#include "can.h"

extern bool Per_Minute_Data;
uint16_t Estimated_Charging_Time = 0;//剩余充电时间(单位分钟)
float Soc,PowerReal,PowerReal_x10;//剩余电量，实时功率

// BMS_CAN 发送/接收头
CAN_TxHeaderTypeDef BMS_CAN_TxHeader;
CAN_RxHeaderTypeDef BMS_CAN_RxHeader;

BMS_SysModeTypedef IGT6_Send_Chonge_Status_Commond;

//soc用
static bool eta_rate_ready = false;
static float eta_rate_base_soc = 0.0f;
static uint32_t eta_rate_base_tick = 0U;
static float eta_soc_rate_per_min = 0.0f;


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
        static const char error[] = "CAN TX error\r\n";
        HAL_UART_Transmit(&huart1, (uint8_t *)error, sizeof(error) - 1U, 5U);
    }
}
void Can_read_FIFO0_message(uint32_t *id, uint8_t *data, uint8_t *length)
{
    uint8_t HAL_Status_Number;
    if (HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0) == 0)
    {
        //没有待接收的报文
        return;
    }
    HAL_Status_Number = HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &BMS_CAN_RxHeader, data);
    if (HAL_Status_Number != HAL_OK)
    {
        // 接收失败处理  
        static const char error[] = "CAN RX error\r\n";
        HAL_UART_Transmit(&huart1, (uint8_t *)error, sizeof(error) - 1U, 5U);
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
    if (id == Get_Change_info && length == sizeof(Soc) + sizeof(PowerReal))
    {        
        //将数据完整复制到buff
        memcpy(&Soc,buff,sizeof(float));
        memcpy(&PowerReal,buff+4,sizeof(float));
        PowerReal_x10 = PowerReal*10;
uint32_t now = HAL_GetTick();

if (PowerReal <= 0.5f || Soc >= 100.0f)
{
    Estimated_Charging_Time = 0;
    eta_rate_ready = false;
    eta_soc_rate_per_min = 0.0f;
}
else
{
    if (!eta_rate_ready)
    {
        eta_rate_ready = true;
        eta_rate_base_soc = Soc;
        eta_rate_base_tick = now;
    }

    float soc_gain = Soc - eta_rate_base_soc;
    uint32_t elapsed_ms = now - eta_rate_base_tick;

    // SOC 累计上升 0.5 个百分点后，用实际充电速度更新估算
    if (soc_gain >= 0.5f && elapsed_ms > 0U)
    {
        eta_soc_rate_per_min =
            soc_gain * 60000.0f / (float)elapsed_ms;

        eta_rate_base_soc = Soc;
        eta_rate_base_tick = now;
    }

    float minutes;

    if (eta_soc_rate_per_min > 0.0f)
    {
        // 根据近期实际 SOC 增长速度估算，能反映功率下降
        minutes = (100.0f - Soc) / eta_soc_rate_per_min;
    }
    else
    {
        // 首次收到数据时立即显示，不等待 SOC 累计变化
        float remaining_wh = Battery_Capacity_Ah * Nominal_Voltage
                           * (1.0f - Soc / 100.0f);
        minutes = remaining_wh / PowerReal * 60.0f;
    }

    Estimated_Charging_Time =
        (minutes < 1.0f) ? 1U : (uint16_t)(minutes + 0.5f);
}

        buff_upadte(&User_Change_message_date,PowerReal_x10,Per_Minute_Data);
        Chart_Update_Start = true;
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

