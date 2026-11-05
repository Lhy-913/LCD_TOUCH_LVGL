#ifndef __LCD_H_
#define __LCD_H_

#include <stdint.h>
#include <stdbool.h>
#include "dma.h"

#define LCD_Height 800  // LCD高度
#define LCD_Width  480  // LCD宽度

#define disp_choose 1      // 横竖屏选择   0竖1横
#define Vertical_choose 1  // 垂直翻转

#define DMA_choose 1       // DMA传输   
#define DMA2D_choose 1     // DMA2D优化（实测效果提升不大，仅1-2帧的优化）

#define Tear_Reduction 0 /* 撕裂抑制   开启后帧率会下降，因为要判断扫描线能否在特定区域中才启动传输，以此减少读写线重合的概率
                           （停用，持续的检测降低大量性能，而且效果也差）  */

extern volatile uint8_t bring_now;

//正点原子的F103ZET6精英开发板搭载的LCD型号：NT35510

typedef struct LCD_TYPE//将地址映射为数据写入
{
   volatile uint16_t LCD_CMD;
   volatile uint16_t LCD_DATA;
}USER_LCD;


#define LCD_BASE ((uint32_t)(0x60000000 + (0x80000 - 2)))
#define LCD      ((USER_LCD *)LCD_BASE)

/*   屏幕   */
void lcd_wr_regno(uint16_t cmd);//写命令
void lcd_wr_data(uint16_t data);//写数据
void lcd_write_reg(uint16_t cmd,uint16_t data);//写一次数据和命令

void lcd_clear(uint32_t color);//清屏

//这俩函数一开始是处理图片内容的
//void lcd_show_image(uint16_t x,uint16_t y,uint16_t IMAGE_X,uint16_t IMAGE_Y);//显示一个区域的颜色
//void lcd_DMAshow_image(uint16_t x,uint16_t y,uint16_t IMAGE_X,uint16_t IMAGE_Y);//DMA方式显示一个区域的颜色

void lcd_show_lvgl(uint16_t x,uint16_t y,uint16_t OVER_X,uint16_t OVER_Y,uint16_t *Buf_Color);//显示一个区域的颜色
void lcd_dmashow_lvgl(uint16_t x,uint16_t y,uint16_t OVER_X,uint16_t OVER_Y,uint16_t *Buf_Color);//DMA方式显示一个区域的颜色
static void lcd_dma_send_chunk(void);//dma数据传输
bool Lcd_Scan_Line_Check(void);//扫描线检查



void lcd_set_start_cursor(uint16_t x,uint16_t y);//设置起始光标地址
void lcd_set_cursor(uint16_t x,uint16_t y,uint16_t xOver,uint16_t yOver);//设置光标边界
void lcd_set_point(uint16_t x,uint16_t y,uint32_t color);//画点
void lcd_Draw_Line(bool Direction,uint16_t xStart,uint16_t xOver,uint16_t Start_Column,uint32_t color);//画线
void lcd_text(void);//测试代码

void LCD_init(void);//LCD屏幕初始化
void lcd_display_dir(bool Screen_Orientation,bool Vertical_Flip);//横竖屏显示选择 0竖1横 0默认方向，1垂直翻转
void LCD_Brightness(uint8_t time);//LCD亮度设置





/* 常用画笔颜色 */
#define WHITE           0xFFFF      /* 白色 */
#define BLACK           0x0000      /* 黑色 */
#define RED             0xF800      /* 红色 */
#define GREEN           0x07E0      /* 绿色 */
#define BLUE            0x001F      /* 蓝色 */ 
#define MAGENTA         0XF81F      /* 品红色/紫红色 = BLUE + RED */
#define YELLOW          0XFFE0      /* 黄色 = GREEN + RED */
#define CYAN            0X07FF      /* 青色 = GREEN + BLUE */  

/* 非常用颜色 */
#define BROWN           0XBC40      /* 棕色 */
#define BRRED           0XFC07      /* 棕红色 */
#define GRAY            0X8430      /* 灰色 */ 
#define DARKBLUE        0X01CF      /* 深蓝色 */
#define LIGHTBLUE       0X7D7C      /* 浅蓝色 */ 
#define GRAYBLUE        0X5458      /* 灰蓝色 */ 
#define LIGHTGREEN      0X841F      /* 浅绿色 */  
#define LGRAY           0XC618      /* 浅灰色(PANNEL),窗体背景色 */ 
#define LGRAYBLUE       0XA651      /* 浅灰蓝色(中间层颜色) */ 
#define LBBLUE          0X2B12      /* 浅棕蓝色(选择条目的反色) */ 


#endif
