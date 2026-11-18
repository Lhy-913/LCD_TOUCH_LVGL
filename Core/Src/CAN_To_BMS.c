#include "CAN_To_BMS.h"
#include "stm32f4xx.h"
#include "usart.h"
#include "printf.h"
#include <string.h>
#include "Lvgl_Array_Update.h"

#include "can.h"

uint16_t Estimated_Charging_Time = 0;//剩余充电时间(单位分钟)
float Soc,PowerReal;//剩余电量，实时功率

// BMS_CAN 发送/接收头
CAN_TxHeaderTypeDef BMS_CAN_TxHeader;
CAN_RxHeaderTypeDef BMS_CAN_RxHeader;

BMS_SysModeTypedef IGT6_Send_Chonge_Status_Commond;

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
        memcpy(&Soc,buff,sizeof(float));
        memcpy(&PowerReal,buff+4,sizeof(float));
        Estimated_Charging_Time = (Battery_Capacity_Ah * Nominal_Voltage)*(1.000f - Soc)/PowerReal * 60.0f;
        buff_upadte(&User_Change_message_date,PowerReal);
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

