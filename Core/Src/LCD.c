#include "lcd.h"
#include "stdint.h"
#include "fmc.h"
#include "lcd_image.h"
#include <stdbool.h>
#include "tim.h"

#include "lvgl/lvgl.h"

//引用DMA句柄
extern DMA_HandleTypeDef hdma_memtomem_dma2_stream0;
extern DMA_HandleTypeDef hdma_memtomem_dma2_stream1;

extern UART_HandleTypeDef huart1;


static uint16_t *src = NULL;//对接lvgl的画面数据指针
volatile uint8_t bring_now = 100;//初始亮度值
//总数据量和dma剩余处理数量
uint32_t len = 0;
uint32_t DMAremain = 0;

/*       屏幕        */
void lcd_wr_regno(uint16_t cmd)//写命令
{
//    cmd = cmd;//02优化速度时候得加点延时让地址自增完成
    LCD->LCD_CMD = cmd;
}

void lcd_wr_data(uint16_t data)//写数据
{
//    data = data;
    LCD->LCD_DATA = data;
}

void lcd_write_reg(uint16_t cmd,uint16_t data)//写入一次命令和数据
{
    LCD->LCD_CMD  = cmd;
    LCD->LCD_DATA = data;
}

void LCD_init(void)//NT35510初始化
{   
    lcd_write_reg(0xF000, 0x55);
    lcd_write_reg(0xF001, 0xAA);
    lcd_write_reg(0xF002, 0x52);
    lcd_write_reg(0xF003, 0x08);
    lcd_write_reg(0xF004, 0x01);
    
    /* AVDD Set AVDD 5.2V */
    lcd_write_reg(0xB000, 0x0D);
    lcd_write_reg(0xB001, 0x0D);
    lcd_write_reg(0xB002, 0x0D);
    /* AVDD ratio */
    lcd_write_reg(0xB600, 0x34);
    lcd_write_reg(0xB601, 0x34);
    lcd_write_reg(0xB602, 0x34);
    /* AVEE -5.2V */
    lcd_write_reg(0xB100, 0x0D);
    lcd_write_reg(0xB101, 0x0D);
    lcd_write_reg(0xB102, 0x0D);
    /* AVEE ratio */
    lcd_write_reg(0xB700, 0x34);
    lcd_write_reg(0xB701, 0x34);
    lcd_write_reg(0xB702, 0x34);
    /* VCL -2.5V */
    lcd_write_reg(0xB200, 0x00);
    lcd_write_reg(0xB201, 0x00);
    lcd_write_reg(0xB202, 0x00);
    /* VCL ratio */
    lcd_write_reg(0xB800, 0x24);
    lcd_write_reg(0xB801, 0x24);
    lcd_write_reg(0xB802, 0x24);
    /* VGH 15V (Free pump) */
    lcd_write_reg(0xBF00, 0x01);
    lcd_write_reg(0xB300, 0x0F);
    lcd_write_reg(0xB301, 0x0F);
    lcd_write_reg(0xB302, 0x0F);
    /* VGH ratio */
    lcd_write_reg(0xB900, 0x34);
    lcd_write_reg(0xB901, 0x34);
    lcd_write_reg(0xB902, 0x34);
    /* VGL_REG -10V */
    lcd_write_reg(0xB500, 0x08);
    lcd_write_reg(0xB501, 0x08);
    lcd_write_reg(0xB502, 0x08);
    lcd_write_reg(0xC200, 0x03);
    /* VGLX ratio */
    lcd_write_reg(0xBA00, 0x24);
    lcd_write_reg(0xBA01, 0x24);
    lcd_write_reg(0xBA02, 0x24);
    /* VGMP/VGSP 4.5V/0V */
    lcd_write_reg(0xBC00, 0x00);
    lcd_write_reg(0xBC01, 0x78);
    lcd_write_reg(0xBC02, 0x00);
    /* VGMN/VGSN -4.5V/0V */
    lcd_write_reg(0xBD00, 0x00);
    lcd_write_reg(0xBD01, 0x78);
    lcd_write_reg(0xBD02, 0x00);
    /* VCOM */
    lcd_write_reg(0xBE00, 0x00);
    lcd_write_reg(0xBE01, 0x64);
    /* Gamma Setting */
    lcd_write_reg(0xD100, 0x00);
    lcd_write_reg(0xD101, 0x33);
    lcd_write_reg(0xD102, 0x00);
    lcd_write_reg(0xD103, 0x34);
    lcd_write_reg(0xD104, 0x00);
    lcd_write_reg(0xD105, 0x3A);
    lcd_write_reg(0xD106, 0x00);
    lcd_write_reg(0xD107, 0x4A);
    lcd_write_reg(0xD108, 0x00);
    lcd_write_reg(0xD109, 0x5C);
    lcd_write_reg(0xD10A, 0x00);
    lcd_write_reg(0xD10B, 0x81);
    lcd_write_reg(0xD10C, 0x00);
    lcd_write_reg(0xD10D, 0xA6);
    lcd_write_reg(0xD10E, 0x00);
    lcd_write_reg(0xD10F, 0xE5);
    lcd_write_reg(0xD110, 0x01);
    lcd_write_reg(0xD111, 0x13);
    lcd_write_reg(0xD112, 0x01);
    lcd_write_reg(0xD113, 0x54);
    lcd_write_reg(0xD114, 0x01);
    lcd_write_reg(0xD115, 0x82);
    lcd_write_reg(0xD116, 0x01);
    lcd_write_reg(0xD117, 0xCA);
    lcd_write_reg(0xD118, 0x02);
    lcd_write_reg(0xD119, 0x00);
    lcd_write_reg(0xD11A, 0x02);
    lcd_write_reg(0xD11B, 0x01);
    lcd_write_reg(0xD11C, 0x02);
    lcd_write_reg(0xD11D, 0x34);
    lcd_write_reg(0xD11E, 0x02);
    lcd_write_reg(0xD11F, 0x67);
    lcd_write_reg(0xD120, 0x02);
    lcd_write_reg(0xD121, 0x84);
    lcd_write_reg(0xD122, 0x02);
    lcd_write_reg(0xD123, 0xA4);
    lcd_write_reg(0xD124, 0x02);
    lcd_write_reg(0xD125, 0xB7);
    lcd_write_reg(0xD126, 0x02);
    lcd_write_reg(0xD127, 0xCF);
    lcd_write_reg(0xD128, 0x02);
    lcd_write_reg(0xD129, 0xDE);
    lcd_write_reg(0xD12A, 0x02);
    lcd_write_reg(0xD12B, 0xF2);
    lcd_write_reg(0xD12C, 0x02);
    lcd_write_reg(0xD12D, 0xFE);
    lcd_write_reg(0xD12E, 0x03);
    lcd_write_reg(0xD12F, 0x10);
    lcd_write_reg(0xD130, 0x03);
    lcd_write_reg(0xD131, 0x33);
    lcd_write_reg(0xD132, 0x03);
    lcd_write_reg(0xD133, 0x6D);
    lcd_write_reg(0xD200, 0x00);
    lcd_write_reg(0xD201, 0x33);
    lcd_write_reg(0xD202, 0x00);
    lcd_write_reg(0xD203, 0x34);
    lcd_write_reg(0xD204, 0x00);
    lcd_write_reg(0xD205, 0x3A);
    lcd_write_reg(0xD206, 0x00);
    lcd_write_reg(0xD207, 0x4A);
    lcd_write_reg(0xD208, 0x00);
    lcd_write_reg(0xD209, 0x5C);
    lcd_write_reg(0xD20A, 0x00);

    lcd_write_reg(0xD20B, 0x81);
    lcd_write_reg(0xD20C, 0x00);
    lcd_write_reg(0xD20D, 0xA6);
    lcd_write_reg(0xD20E, 0x00);
    lcd_write_reg(0xD20F, 0xE5);
    lcd_write_reg(0xD210, 0x01);
    lcd_write_reg(0xD211, 0x13);
    lcd_write_reg(0xD212, 0x01);
    lcd_write_reg(0xD213, 0x54);
    lcd_write_reg(0xD214, 0x01);
    lcd_write_reg(0xD215, 0x82);
    lcd_write_reg(0xD216, 0x01);
    lcd_write_reg(0xD217, 0xCA);
    lcd_write_reg(0xD218, 0x02);
    lcd_write_reg(0xD219, 0x00);
    lcd_write_reg(0xD21A, 0x02);
    lcd_write_reg(0xD21B, 0x01);
    lcd_write_reg(0xD21C, 0x02);
    lcd_write_reg(0xD21D, 0x34);
    lcd_write_reg(0xD21E, 0x02);
    lcd_write_reg(0xD21F, 0x67);
    lcd_write_reg(0xD220, 0x02);
    lcd_write_reg(0xD221, 0x84);
    lcd_write_reg(0xD222, 0x02);
    lcd_write_reg(0xD223, 0xA4);
    lcd_write_reg(0xD224, 0x02);
    lcd_write_reg(0xD225, 0xB7);
    lcd_write_reg(0xD226, 0x02);
    lcd_write_reg(0xD227, 0xCF);
    lcd_write_reg(0xD228, 0x02);
    lcd_write_reg(0xD229, 0xDE);
    lcd_write_reg(0xD22A, 0x02);
    lcd_write_reg(0xD22B, 0xF2);
    lcd_write_reg(0xD22C, 0x02);
    lcd_write_reg(0xD22D, 0xFE);
    lcd_write_reg(0xD22E, 0x03);
    lcd_write_reg(0xD22F, 0x10);
    lcd_write_reg(0xD230, 0x03);
    lcd_write_reg(0xD231, 0x33);
    lcd_write_reg(0xD232, 0x03);
    lcd_write_reg(0xD233, 0x6D);
    lcd_write_reg(0xD300, 0x00);
    lcd_write_reg(0xD301, 0x33);
    lcd_write_reg(0xD302, 0x00);
    lcd_write_reg(0xD303, 0x34);
    lcd_write_reg(0xD304, 0x00);
    lcd_write_reg(0xD305, 0x3A);
    lcd_write_reg(0xD306, 0x00);
    lcd_write_reg(0xD307, 0x4A);
    lcd_write_reg(0xD308, 0x00);
    lcd_write_reg(0xD309, 0x5C);
    lcd_write_reg(0xD30A, 0x00);

    lcd_write_reg(0xD30B, 0x81);
    lcd_write_reg(0xD30C, 0x00);
    lcd_write_reg(0xD30D, 0xA6);
    lcd_write_reg(0xD30E, 0x00);
    lcd_write_reg(0xD30F, 0xE5);
    lcd_write_reg(0xD310, 0x01);
    lcd_write_reg(0xD311, 0x13);
    lcd_write_reg(0xD312, 0x01);
    lcd_write_reg(0xD313, 0x54);
    lcd_write_reg(0xD314, 0x01);
    lcd_write_reg(0xD315, 0x82);
    lcd_write_reg(0xD316, 0x01);
    lcd_write_reg(0xD317, 0xCA);
    lcd_write_reg(0xD318, 0x02);
    lcd_write_reg(0xD319, 0x00);
    lcd_write_reg(0xD31A, 0x02);
    lcd_write_reg(0xD31B, 0x01);
    lcd_write_reg(0xD31C, 0x02);
    lcd_write_reg(0xD31D, 0x34);
    lcd_write_reg(0xD31E, 0x02);
    lcd_write_reg(0xD31F, 0x67);
    lcd_write_reg(0xD320, 0x02);
    lcd_write_reg(0xD321, 0x84);
    lcd_write_reg(0xD322, 0x02);
    lcd_write_reg(0xD323, 0xA4);
    lcd_write_reg(0xD324, 0x02);
    lcd_write_reg(0xD325, 0xB7);
    lcd_write_reg(0xD326, 0x02);
    lcd_write_reg(0xD327, 0xCF);
    lcd_write_reg(0xD328, 0x02);
    lcd_write_reg(0xD329, 0xDE);
    lcd_write_reg(0xD32A, 0x02);
    lcd_write_reg(0xD32B, 0xF2);
    lcd_write_reg(0xD32C, 0x02);
    lcd_write_reg(0xD32D, 0xFE);
    lcd_write_reg(0xD32E, 0x03);
    lcd_write_reg(0xD32F, 0x10);
    lcd_write_reg(0xD330, 0x03);
    lcd_write_reg(0xD331, 0x33);
    lcd_write_reg(0xD332, 0x03);
    lcd_write_reg(0xD333, 0x6D);
    lcd_write_reg(0xD400, 0x00);
    lcd_write_reg(0xD401, 0x33);
    lcd_write_reg(0xD402, 0x00);
    lcd_write_reg(0xD403, 0x34);
    lcd_write_reg(0xD404, 0x00);
    lcd_write_reg(0xD405, 0x3A);
    lcd_write_reg(0xD406, 0x00);
    lcd_write_reg(0xD407, 0x4A);
    lcd_write_reg(0xD408, 0x00);
    lcd_write_reg(0xD409, 0x5C);
    lcd_write_reg(0xD40A, 0x00);
    lcd_write_reg(0xD40B, 0x81);

    lcd_write_reg(0xD40C, 0x00);
    lcd_write_reg(0xD40D, 0xA6);
    lcd_write_reg(0xD40E, 0x00);
    lcd_write_reg(0xD40F, 0xE5);
    lcd_write_reg(0xD410, 0x01);
    lcd_write_reg(0xD411, 0x13);
    lcd_write_reg(0xD412, 0x01);
    lcd_write_reg(0xD413, 0x54);
    lcd_write_reg(0xD414, 0x01);
    lcd_write_reg(0xD415, 0x82);
    lcd_write_reg(0xD416, 0x01);
    lcd_write_reg(0xD417, 0xCA);
    lcd_write_reg(0xD418, 0x02);
    lcd_write_reg(0xD419, 0x00);
    lcd_write_reg(0xD41A, 0x02);
    lcd_write_reg(0xD41B, 0x01);
    lcd_write_reg(0xD41C, 0x02);
    lcd_write_reg(0xD41D, 0x34);
    lcd_write_reg(0xD41E, 0x02);
    lcd_write_reg(0xD41F, 0x67);
    lcd_write_reg(0xD420, 0x02);
    lcd_write_reg(0xD421, 0x84);
    lcd_write_reg(0xD422, 0x02);
    lcd_write_reg(0xD423, 0xA4);
    lcd_write_reg(0xD424, 0x02);
    lcd_write_reg(0xD425, 0xB7);
    lcd_write_reg(0xD426, 0x02);
    lcd_write_reg(0xD427, 0xCF);
    lcd_write_reg(0xD428, 0x02);
    lcd_write_reg(0xD429, 0xDE);
    lcd_write_reg(0xD42A, 0x02);
    lcd_write_reg(0xD42B, 0xF2);
    lcd_write_reg(0xD42C, 0x02);
    lcd_write_reg(0xD42D, 0xFE);
    lcd_write_reg(0xD42E, 0x03);
    lcd_write_reg(0xD42F, 0x10);
    lcd_write_reg(0xD430, 0x03);
    lcd_write_reg(0xD431, 0x33);
    lcd_write_reg(0xD432, 0x03);
    lcd_write_reg(0xD433, 0x6D);
    lcd_write_reg(0xD500, 0x00);
    lcd_write_reg(0xD501, 0x33);
    lcd_write_reg(0xD502, 0x00);
    lcd_write_reg(0xD503, 0x34);
    lcd_write_reg(0xD504, 0x00);
    lcd_write_reg(0xD505, 0x3A);
    lcd_write_reg(0xD506, 0x00);
    lcd_write_reg(0xD507, 0x4A);
    lcd_write_reg(0xD508, 0x00);
    lcd_write_reg(0xD509, 0x5C);
    lcd_write_reg(0xD50A, 0x00);
    lcd_write_reg(0xD50B, 0x81);

    lcd_write_reg(0xD50C, 0x00);
    lcd_write_reg(0xD50D, 0xA6);
    lcd_write_reg(0xD50E, 0x00);
    lcd_write_reg(0xD50F, 0xE5);
    lcd_write_reg(0xD510, 0x01);
    lcd_write_reg(0xD511, 0x13);
    lcd_write_reg(0xD512, 0x01);
    lcd_write_reg(0xD513, 0x54);
    lcd_write_reg(0xD514, 0x01);
    lcd_write_reg(0xD515, 0x82);
    lcd_write_reg(0xD516, 0x01);
    lcd_write_reg(0xD517, 0xCA);
    lcd_write_reg(0xD518, 0x02);
    lcd_write_reg(0xD519, 0x00);
    lcd_write_reg(0xD51A, 0x02);
    lcd_write_reg(0xD51B, 0x01);
    lcd_write_reg(0xD51C, 0x02);
    lcd_write_reg(0xD51D, 0x34);
    lcd_write_reg(0xD51E, 0x02);
    lcd_write_reg(0xD51F, 0x67);
    lcd_write_reg(0xD520, 0x02);
    lcd_write_reg(0xD521, 0x84);
    lcd_write_reg(0xD522, 0x02);
    lcd_write_reg(0xD523, 0xA4);
    lcd_write_reg(0xD524, 0x02);
    lcd_write_reg(0xD525, 0xB7);
    lcd_write_reg(0xD526, 0x02);
    lcd_write_reg(0xD527, 0xCF);
    lcd_write_reg(0xD528, 0x02);
    lcd_write_reg(0xD529, 0xDE);
    lcd_write_reg(0xD52A, 0x02);
    lcd_write_reg(0xD52B, 0xF2);
    lcd_write_reg(0xD52C, 0x02);
    lcd_write_reg(0xD52D, 0xFE);
    lcd_write_reg(0xD52E, 0x03);
    lcd_write_reg(0xD52F, 0x10);
    lcd_write_reg(0xD530, 0x03);
    lcd_write_reg(0xD531, 0x33);
    lcd_write_reg(0xD532, 0x03);
    lcd_write_reg(0xD533, 0x6D);
    lcd_write_reg(0xD600, 0x00);
    lcd_write_reg(0xD601, 0x33);
    lcd_write_reg(0xD602, 0x00);
    lcd_write_reg(0xD603, 0x34);
    lcd_write_reg(0xD604, 0x00);
    lcd_write_reg(0xD605, 0x3A);
    lcd_write_reg(0xD606, 0x00);
    lcd_write_reg(0xD607, 0x4A);
    lcd_write_reg(0xD608, 0x00);
    lcd_write_reg(0xD609, 0x5C);
    lcd_write_reg(0xD60A, 0x00);
    lcd_write_reg(0xD60B, 0x81);

    lcd_write_reg(0xD60C, 0x00);
    lcd_write_reg(0xD60D, 0xA6);
    lcd_write_reg(0xD60E, 0x00);
    lcd_write_reg(0xD60F, 0xE5);
    lcd_write_reg(0xD610, 0x01);
    lcd_write_reg(0xD611, 0x13);
    lcd_write_reg(0xD612, 0x01);
    lcd_write_reg(0xD613, 0x54);
    lcd_write_reg(0xD614, 0x01);
    lcd_write_reg(0xD615, 0x82);
    lcd_write_reg(0xD616, 0x01);
    lcd_write_reg(0xD617, 0xCA);
    lcd_write_reg(0xD618, 0x02);
    lcd_write_reg(0xD619, 0x00);
    lcd_write_reg(0xD61A, 0x02);
    lcd_write_reg(0xD61B, 0x01);
    lcd_write_reg(0xD61C, 0x02);
    lcd_write_reg(0xD61D, 0x34);
    lcd_write_reg(0xD61E, 0x02);
    lcd_write_reg(0xD61F, 0x67);
    lcd_write_reg(0xD620, 0x02);
    lcd_write_reg(0xD621, 0x84);
    lcd_write_reg(0xD622, 0x02);
    lcd_write_reg(0xD623, 0xA4);
    lcd_write_reg(0xD624, 0x02);
    lcd_write_reg(0xD625, 0xB7);
    lcd_write_reg(0xD626, 0x02);
    lcd_write_reg(0xD627, 0xCF);
    lcd_write_reg(0xD628, 0x02);
    lcd_write_reg(0xD629, 0xDE);
    lcd_write_reg(0xD62A, 0x02);
    lcd_write_reg(0xD62B, 0xF2);
    lcd_write_reg(0xD62C, 0x02);
    lcd_write_reg(0xD62D, 0xFE);
    lcd_write_reg(0xD62E, 0x03);
    lcd_write_reg(0xD62F, 0x10);
    lcd_write_reg(0xD630, 0x03);
    lcd_write_reg(0xD631, 0x33);
    lcd_write_reg(0xD632, 0x03);
    lcd_write_reg(0xD633, 0x6D);
    /* LV2 Page 0 enable */
    lcd_write_reg(0xF000, 0x55);
    lcd_write_reg(0xF001, 0xAA);
    lcd_write_reg(0xF002, 0x52);
    lcd_write_reg(0xF003, 0x08);
    lcd_write_reg(0xF004, 0x00);
    /* Display control */
    lcd_write_reg(0xB100, 0xCC);
    lcd_write_reg(0xB101, 0x00);
    /* Source hold time */
    lcd_write_reg(0xB600, 0x05);
    /* Gate EQ control */
    lcd_write_reg(0xB700, 0x70);
    lcd_write_reg(0xB701, 0x70);
    /* Source EQ control (Mode 2) */
    lcd_write_reg(0xB800, 0x01);
    lcd_write_reg(0xB801, 0x03);
    lcd_write_reg(0xB802, 0x03);
    lcd_write_reg(0xB803, 0x03);
    /* Inversion mode (2-dot) */
    lcd_write_reg(0xBC00, 0x02);
    lcd_write_reg(0xBC01, 0x00);
    lcd_write_reg(0xBC02, 0x00);
    /* Timing control 4H w/ 4-delay */
    lcd_write_reg(0xC900, 0xD0);
    lcd_write_reg(0xC901, 0x02);
    lcd_write_reg(0xC902, 0x50);
    lcd_write_reg(0xC903, 0x50);
    lcd_write_reg(0xC904, 0x50);
    lcd_write_reg(0x3500, 0x00);
    lcd_write_reg(0x3A00, 0x05); /* 16-bit/pixel */
    lcd_wr_regno(0x1100);
    
    HAL_Delay(120);
    lcd_wr_regno(0x2900);
    lcd_clear(BLACK);
}

bool Lcd_Scan_Line_Check(void)//扫描线检查
{    
    lcd_wr_regno(0x4500);
    uint16_t LCD_scan_line = LCD->LCD_DATA;
    lcd_wr_regno(0x4501);
    LCD_scan_line = LCD_scan_line << 8 | (LCD->LCD_DATA);
    
    if(LCD_scan_line > 630 && LCD_scan_line < 640)return true;
    
    return false;
}

void lcd_show_lvgl(uint16_t x,uint16_t y,uint16_t OVER_X,uint16_t OVER_Y,uint16_t *Buf_Color)//打点函数
{
    lcd_set_cursor(x,y,OVER_X,OVER_Y);  
    if(Tear_Reduction)while(!Lcd_Scan_Line_Check()){}    
    lcd_wr_regno(0x2C00);
    for(uint32_t i = 0; i < (OVER_X - x+1)*(OVER_Y - y+1); i++)//这里的+1在使用LVGL时候必须加，不然会出现全屏规则排列的横线
    {
        LCD->LCD_DATA = *Buf_Color++;
    }
}

void lcd_dmashow_lvgl(uint16_t x, uint16_t y, uint16_t OVER_X, uint16_t OVER_Y, uint16_t *Buf_Color)//DMA打点函数
{
    src = Buf_Color;
    lcd_set_cursor(x, y, OVER_X, OVER_Y);
    DMAremain = (uint32_t)(OVER_X - x + 1) * (OVER_Y - y + 1);
    
    if(Tear_Reduction)while(!Lcd_Scan_Line_Check()){}

    lcd_wr_regno(0x2C00);
    lcd_dma_send_chunk();
}

static void lcd_dma_send_chunk(void)//DMA传输
{    
        
    len = (DMAremain > 65535) ? 65535 : DMAremain;
    HAL_DMA_Start_IT(&hdma_memtomem_dma2_stream0,
                     (uint32_t)src,
                     (uint32_t)&LCD->LCD_DATA,
                     len);
    src += len;
}
extern lv_display_t * disp;
void XferCpltCallback(DMA_HandleTypeDef *hdma)//DMA回调函数
{
    DMAremain -= len;
    if (hdma == &hdma_memtomem_dma2_stream0) 
    {
        if(DMAremain) lcd_dma_send_chunk();     // DMA续杯
        else lv_display_flush_ready(disp);
    }
}

void lcd_clear(uint32_t color)//清屏
{
    lcd_set_start_cursor(0, 0);
    lcd_wr_regno(0x2C00);
    for(uint32_t i = 0; i < 480*800; i++)
    {
        LCD->LCD_DATA = color;
    }
}

void lcd_set_start_cursor(uint16_t x,uint16_t y)//设置起始光标
{
    lcd_wr_regno(0x2A00);   
        lcd_wr_data(x>>8);
    lcd_wr_regno(0x2A01);   
        lcd_wr_data(x&0xFF);
    lcd_wr_regno(0x2B00);   
        lcd_wr_data(y>>8);
    lcd_wr_regno(0x2B01);   
        lcd_wr_data(y&0xFF);
}

void lcd_set_cursor(uint16_t x,uint16_t y,uint16_t xOver,uint16_t yOver)//设置完整光标边界
{
//    xOver -= 1;//数值从0开始计数，需减一（使用LVGL时候需注释，会影响）
//    yOver -= 1;

    lcd_write_reg(0x2A00, x >> 8);
    lcd_write_reg(0x2A01, x & 0xFF);
    lcd_write_reg(0x2A02, xOver >> 8);
    lcd_write_reg(0x2A03, xOver & 0xFF);
    
    lcd_write_reg(0x2B00, y >> 8);
    lcd_write_reg(0x2B01, y & 0xFF);
    lcd_write_reg(0x2B02, yOver >> 8);
    lcd_write_reg(0x2B03, yOver & 0xFF);
}

void lcd_set_point(uint16_t x,uint16_t y,uint32_t color)//画点
{
    lcd_set_start_cursor(x,y);
    lcd_wr_regno(0x2C00);
    LCD->LCD_DATA = color;
}

void lcd_Draw_Line(bool Direction,uint16_t Start,uint16_t Over,uint16_t Start_Column,uint32_t color)//画线
{
    if(!Direction)//x轴画线
    {
        lcd_write_reg(0x2A00, Start >> 8);
        lcd_write_reg(0x2A01, Start & 0xFF);
        lcd_write_reg(0x2A02, Over >> 8);
        lcd_write_reg(0x2A03, Over & 0xFF);
        
        lcd_write_reg(0x2B00, Start_Column >> 8);
        lcd_write_reg(0x2B01, Start_Column & 0xFF);
    }
    else if(Direction)//y轴画线
    {
        lcd_write_reg(0x2B00, Start >> 8);
        lcd_write_reg(0x2B01, Start & 0xFF);
        lcd_write_reg(0x2B02, Over >> 8);
        lcd_write_reg(0x2B03, Over & 0xFF);
        
        lcd_write_reg(0x2A00,  Start_Column >> 8);
        lcd_write_reg(0x2A01,  Start_Column & 0xFF);
        lcd_write_reg(0x2A02, (Start_Column + 1) >> 8);
        lcd_write_reg(0x2A03, (Start_Column + 1) & 0xFF);
    }
     
    lcd_wr_regno(0x2C00);
    for(uint16_t i=0;i<Over-Start;i++)
    {
     LCD->LCD_DATA = color;
    }
}

void lcd_display_dir(bool Screen_Orientation, bool Vertical_Flip)
{
    lcd_write_reg(0x3600,
        Screen_Orientation ? (Vertical_Flip ? 0x22 : 0x21) : (Vertical_Flip ? 0x03 : 0x00));

    lcd_set_cursor(0, 0, 
    (Screen_Orientation ? 800 : 480), (Screen_Orientation ? 480 : 800));
}

void LCD_Brightness(uint8_t time)//LCD亮度设置
{
    if(time > 100)time = 100;
    else if(time < 20)time = 20;

    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, time);
}


void lcd_text(void)
{
//LCD_Brightness(10);
    //以下测试
    
    
//    lcd_Draw_Line(1,0,480,100,BLACK);
    
    for(;;) //刷屏
    {
lcd_clear(BROWN);
HAL_Delay(500);
lcd_clear(BRRED);
HAL_Delay(500);
lcd_clear(GRAY);
HAL_Delay(500);
lcd_clear(DARKBLUE);
HAL_Delay(500);
lcd_clear(LIGHTBLUE);
HAL_Delay(500);
lcd_clear(GRAYBLUE);
HAL_Delay(500);
lcd_clear(LIGHTGREEN);
HAL_Delay(500);
lcd_clear(LGRAY);
HAL_Delay(500);
lcd_clear(LGRAYBLUE);
HAL_Delay(500);
lcd_clear(LBBLUE);
HAL_Delay(500);
lcd_clear(YELLOW);
HAL_Delay(500);
    }

//测量扫描一行的时间以及每次扫描偏移的行数。  当前结果是：每37us获得一次扫描行，每次偏移平均数为180行
//    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
//    DWT->CYCCNT = 0;
//    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

//uint16_t LCD_scan_line;
//uint16_t last_line = 0;
//uint32_t last_tick = 0;
//uint32_t now_tick;
//uint32_t delta;
//uint32_t line_count = 0;
//uint64_t total_ticks = 0;

//char uart_buf[64];
//int len;
//while(1)
//{
//    lcd_wr_regno(0x4500);
//    LCD_scan_line = LCD->LCD_DATA;

//    lcd_wr_regno(0x4501);
//    LCD_scan_line = LCD_scan_line << 8 | (LCD->LCD_DATA);

//    now_tick = DWT->CYCCNT;

//    if(LCD_scan_line != last_line)
//    {
//        if(last_line != 0)
//        {
//            delta = now_tick - last_tick;

//            total_ticks += delta;
//            line_count++;

//            if(line_count >= 100)
//            {
//                uint32_t avg_ticks = total_ticks / line_count;
//                uint32_t avg_us = avg_ticks / (HAL_RCC_GetHCLKFreq() / 1000000U);

//                len = snprintf(uart_buf, sizeof(uart_buf),
//                               "Line=%04X Avg=%lu us\r\n",
//                               LCD_scan_line,
//                               (unsigned long)avg_us);

//                HAL_UART_Transmit(&huart1,
//                                  (uint8_t *)uart_buf,
//                                  len,
//                                  HAL_MAX_DELAY);

//                line_count = 0;
//                total_ticks = 0;
//            }
//        }

//        last_line = LCD_scan_line;
//        last_tick = now_tick;
//    }
//}
}
