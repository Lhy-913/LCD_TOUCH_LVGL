#ifndef __TOUCH_H_
#define __TOUCH_H_
#include "stdint.h"

//触摸屏RST引脚设定
#define GT1151_RST_PORT  GPIOI
#define GT1151_RST_PIN_  GPIO_PIN_8

//触摸屏INT引脚设定
#define GT1151_INT_PORT  GPIOH
#define GT1151_INT_PIN_  GPIO_PIN_7

//时钟引脚设定
#define GT1151_IIC_SCL_PORT  GPIOH
#define GT1151_IIC_SCL_PIN   GPIO_PIN_6

//数据引脚设定
#define GT1151_IIC_SDA_PORT  GPIOI
#define GT1151_IIC_SDA_PIN   GPIO_PIN_3

#define GT1151_RST_PIN(x)     do{x ? \
                              HAL_GPIO_WritePin(GT1151_RST_PORT,GT1151_RST_PIN_,GPIO_PIN_SET):\
                              HAL_GPIO_WritePin(GT1151_RST_PORT,GT1151_RST_PIN_,GPIO_PIN_RESET);\
                          }while(0);

#define GT1151_IIC_SCL(x)     do{x ? \
                              HAL_GPIO_WritePin(GT1151_IIC_SCL_PORT,GT1151_IIC_SCL_PIN,GPIO_PIN_SET):\
                              HAL_GPIO_WritePin(GT1151_IIC_SCL_PORT,GT1151_IIC_SCL_PIN,GPIO_PIN_RESET);\
                          }while(0)

#define GT1151_IIC_SDA(x)     do{ x ? \
                              HAL_GPIO_WritePin(GT1151_IIC_SDA_PORT, GT1151_IIC_SDA_PIN, GPIO_PIN_SET): \
                              HAL_GPIO_WritePin(GT1151_IIC_SDA_PORT, GT1151_IIC_SDA_PIN, GPIO_PIN_RESET); \
                          }while(0)  

#define CT_READ_SDA       HAL_GPIO_ReadPin(GT1151_IIC_SDA_PORT, GT1151_IIC_SDA_PIN) /* 读取SDA */    
#define GT1151_READ       HAL_GPIO_ReadPin(GT1151_INT_PORT, GT1151_INT_PIN_) /* 读取GT1151的INT电平 */

                          
//GT1151常用命令
#define GT1151_READ_REG  0x814E//触摸状态
#define GT1151_CLEAR     0x00//清除触摸状态命令

#define GT1151_TOUCH1    0x8150//读取第一个触摸点的x低位，连续读取会获取后续其他触摸点的数据xy的高低位
                          
void GT1151_wr_data(uint16_t reg,uint8_t *buf,uint16_t len);//发送数据
void GT1151_Rd_data(uint16_t reg,uint8_t *buf,uint16_t len);//读取数据
void GT1151_init(void);//触摸屏初始化
void GT1151_run(void); //读取一次触摸屏坐标数据

//下列函数从 正点原子”触摸屏实验源码“文件内 移植
                          
/* IIC所有操作函数 */
void ct_iic_start(void);                    /* 发送IIC开始信号 */
void ct_iic_stop(void);                     /* 发送IIC停止信号 */

void ct_iic_ack(void);                      /* IIC发送ACK信号 */
void ct_iic_nack(void);                     /* IIC不发送ACK信号 */
uint8_t ct_iic_wait_ack(void);              /* IIC等待ACK信号 */

void ct_iic_send_byte(uint8_t txd);         /* IIC发送一个字节 */
uint8_t ct_iic_read_byte(unsigned char ack);/* IIC读取一个字节 */

extern uint8_t GT1151_FLAG;
extern uint8_t GT1151_data[4]; //触摸数据

struct TOUCH_XY//触摸屏当前x、y坐标
{
    uint16_t x;
    uint16_t y;
};

extern struct TOUCH_XY Touch_XYdata;
#endif
