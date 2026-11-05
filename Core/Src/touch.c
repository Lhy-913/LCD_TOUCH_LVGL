#include "touch.h"
#include "stdint.h"
#include "gpio.h"
#include "delay.h"
#include "LCD.h"

#include <stdio.h>
#include <string.h>

struct TOUCH_XY Touch_XYdata;//触摸屏的xy坐标结构体

uint8_t GT1151_FLAG;    //触摸标志位
uint8_t GT1151_data[4]; //触摸数据
uint8_t TOUCH_ID[4];    //触摸屏ID

extern UART_HandleTypeDef huart1;

//触摸屏型号：GT1151
#define Touch_ADDAR 0X14

void ct_iic_start(void)            /* 发送IIC开始信号 */
{
    GT1151_IIC_SDA(1);
    GT1151_IIC_SCL(1);
    delay_us(2);
    GT1151_IIC_SDA(0);      /* START信号: 当SCL为高时, SDA从高变成低, 表示起始信号 */
    delay_us(2);
    GT1151_IIC_SCL(0);      /* 钳住I2C总线，准备发送或接收数据 */
    delay_us(2);
}

void ct_iic_stop(void)
{
    GT1151_IIC_SDA(0);      /* STOP信号: 当SCL为高时, SDA从低变成高, 表示停止信号 */
    delay_us(2);
    GT1151_IIC_SCL(1);
    delay_us(2);
    GT1151_IIC_SDA(1);      /* 发送I2C总线结束信号 */
    delay_us(2);
}


void ct_iic_ack(void)/* IIC发送ACK信号 */
{
    GT1151_IIC_SDA(0);  /* SCL 0 -> 1  时SDA = 0,表示应答 */
    delay_us(2);
    GT1151_IIC_SCL(1);
    delay_us(2);
    GT1151_IIC_SCL(0);
    delay_us(2);
    GT1151_IIC_SDA(1);  /* 主机释放SDA线 */
    delay_us(2);
}    
void ct_iic_nack(void)             /* IIC不发送ACK信号 */
{
    GT1151_IIC_SDA(1);  /* SCL 0 -> 1  时 SDA = 1,表示不应答 */
    delay_us(2);
    GT1151_IIC_SCL(1);
    delay_us(2);
    GT1151_IIC_SCL(0);
    delay_us(2);
}
uint8_t ct_iic_wait_ack(void)      /* IIC等待ACK信号 */
{
    uint8_t waittime = 0;
    uint8_t rack = 0;
    
    GT1151_IIC_SDA(1);      /* 主机释放SDA线(此时外部器件可以拉低SDA线) */
    delay_us(2);
    GT1151_IIC_SCL(1);      /* SCL=1, 此时从机可以返回ACK */
    delay_us(2);

        while (CT_READ_SDA) /* 等待应答 */
        {
            waittime++;

            if (waittime > 250)
            {
                ct_iic_stop();
                rack = 1;
                break;
            }

               delay_us(2);
        }

    GT1151_IIC_SCL(0);      /* SCL=0, 结束ACK检查 */
    delay_us(2);
    return rack;
}

void ct_iic_send_byte(uint8_t data)         /* IIC发送一个字节 */
{
    uint8_t t;
        for (t = 0; t < 8; t++)
        {
            GT1151_IIC_SDA((data & 0x80) >> 7); /* 高位先发送 */
            delay_us(2);
            GT1151_IIC_SCL(1);
            delay_us(2);
            GT1151_IIC_SCL(0);
            data <<= 1;     /* 左移1位,用于下一次发送 */
        }

    GT1151_IIC_SDA(1);      /* 发送完成, 主机释放SDA线 */
}

uint8_t ct_iic_read_byte(unsigned char ack)/* IIC读取一个字节 */
{
    uint8_t i, receive = 0;

    for (i = 0; i < 8; i++ )    /* 接收1个字节数据 */
    {
        receive <<= 1;  /* 高位先输出,所以先收到的数据位要左移 */
        GT1151_IIC_SCL(1);
        delay_us(2);

        if (CT_READ_SDA)
        {
            receive++;
        }
        
        GT1151_IIC_SCL(0);
        delay_us(2);

    }

    if (!ack)
    {
        ct_iic_nack();  /* 发送nACK */
    }
    else
    {
        ct_iic_ack();   /* 发送ACK */
    }

    return receive;
}

void GT1151_wr_data(uint16_t reg,uint8_t *buf,uint16_t len)//写寄存器命令  参数：目标寄存器，待发送数据，数据长度
{
    ct_iic_start(); 
    ct_iic_send_byte(Touch_ADDAR<<1 | 0);//写命令
    ct_iic_wait_ack();
    ct_iic_send_byte(reg>>8);    //发送高位
    ct_iic_wait_ack();
    ct_iic_send_byte(reg & 0xFF);//发送低位
    ct_iic_wait_ack();
    
    for(uint16_t i=0;i<len;i++)
    {
        ct_iic_send_byte(buf[i]);
        if(ct_iic_wait_ack())break;
    }
    ct_iic_stop();  /* 产生一个停止条件 */
}

void GT1151_Rd_data(uint16_t reg,uint8_t *buf,uint16_t len)//读寄存器命令  参数：目标寄存器，数据缓存数组，接收长度
{
    ct_iic_start(); 
    ct_iic_send_byte(Touch_ADDAR<<1 | 0); //写命令
    ct_iic_wait_ack();
    ct_iic_send_byte(reg>>8);    //发送高位
    ct_iic_wait_ack();
    ct_iic_send_byte(reg & 0xFF);//发送低位
    ct_iic_wait_ack();
    
    ct_iic_start();              //寄存器地址索引结束，再次发出起始信号
    ct_iic_send_byte(Touch_ADDAR<<1 | 1);//读命令
    ct_iic_wait_ack();

    for(uint16_t i=0;i<len;i++)
    {
        buf[i] = ct_iic_read_byte(i==(len-1)  ?  0 : 1);
    }
     ct_iic_stop();  /* 产生一个停止条件 */
}

void GT1151_init(void)
{
    
GPIO_InitTypeDef GPIO_InitStruct = {0};
GPIO_InitStruct.Pin = GT1151_INT_PIN_;
GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
GPIO_InitStruct.Pull = GPIO_PULLUP;      // 无上下拉

HAL_GPIO_Init(GT1151_INT_PORT, &GPIO_InitStruct);
    
    //上电时序处理
    GT1151_RST_PIN(0);
    HAL_Delay(10);
    GT1151_RST_PIN(1);
    HAL_Delay(10);
    
GPIO_InitStruct.Pin = GT1151_INT_PIN_;
GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;//中断模式（下降沿触发）
GPIO_InitStruct.Pull = GPIO_NOPULL;
HAL_GPIO_Init(GT1151_INT_PORT, &GPIO_InitStruct);
    HAL_Delay(100);


GT1151_Rd_data(0x8140,TOUCH_ID,4);
HAL_UART_Transmit(&huart1,TOUCH_ID,sizeof(TOUCH_ID),HAL_MAX_DELAY);

}

void GT1151_run(void)
{
    GT1151_Rd_data(GT1151_TOUCH1,GT1151_data,4);//读取数据
    Touch_XYdata.x = ((uint16_t)GT1151_data[1] << 8) | GT1151_data[0];
    Touch_XYdata.y = ((uint16_t)GT1151_data[3] << 8) | GT1151_data[2];
    
//    //用于触摸测试，不用就注释
//    uint8_t clear = GT1151_CLEAR;
//    GT1151_wr_data(GT1151_READ_REG, &clear, 1);//读取后清除寄存器旧值
//        
//    char buf[32];
//    int len = sprintf(buf, "X=%u,Y=%u\r\n",
//                      (unsigned int)Touch_XYdata.x,
//                      (unsigned int)Touch_XYdata.y);
//    HAL_UART_Transmit(&huart1, (uint8_t *)buf, len, HAL_MAX_DELAY);//发送xy的读取数据             
//    lcd_set_point(Touch_XYdata.x,Touch_XYdata.y,BLACK);//接收到信号直接画点   
//    //测试用
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == TOUCH_INT_Pin)
    {
        if(TOUCH_ID[0]=='1'&&TOUCH_ID[1]=='1'&&TOUCH_ID[2]=='5'&&TOUCH_ID[3]=='8')//1158型号的触摸芯片（但这确实是1151类型的芯片...）
        {
            GT1151_FLAG = 1;
        }
        else GT1151_FLAG = 0;
    }
}
