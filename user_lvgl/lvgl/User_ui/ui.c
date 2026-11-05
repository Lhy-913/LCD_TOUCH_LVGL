/*
 * ui.c —— 充电桩主页界面（LVGL v9）
 * 对应设计图「界面 1：主页 / 充电选择」，文本全部英文
 *
 * 布局（Flex Column，百分比自适应任意分辨率）：
 *   ① 顶栏        ~10%   标题 + 时间日期
 *   ② 欢迎区      ~35%   Welcome / Smart Charging Station / 标语
 *   ③ 功能区      ~55%   3 个菜单按钮横排（开始充电 / 充电记录 / 设置）
 */
#include "ui.h"
#include "lvgl/lvgl.h"

#include "User_Interface_Selection.h"
#include "User_Operation_Screen.h"
#include "Charging_Status.h"
#include "Charging_Records.h"
#include "Settings.h"

lv_obj_t * scr;//屏幕
lv_obj_t * main_container          = NULL; // 最底层容器

lv_obj_t * User_Screen             = NULL; // 用户使用界面
lv_obj_t * User_Selection_Screen   = NULL; // 用户选择界面
uint8_t    user_chooser            = 0;    // 用户当前选择

void General_UI_Properties(lv_obj_t * obj)//常规UI属性设置（禁止滚动显示，去圆角、边框、内边距）
{
    // 删除圆角
    lv_obj_set_style_radius(obj, 0, LV_PART_MAIN);
    /* 删除边框 */
    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN);
    // 设置内边距
    lv_obj_set_style_pad_all(obj, 0, LV_PART_MAIN);
    //禁止显示滚动条
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
}


void ui_create(void)
{
    scr = lv_screen_active();
    main_container = lv_obj_create(scr);
    lv_obj_set_size(main_container,LV_PCT(100),LV_PCT(100));
    lv_obj_set_style_bg_color(main_container, lv_color_hex(0x0B0F14), 0);

    /* 删除阴影 */
    lv_obj_set_style_shadow_width(main_container, 0, LV_PART_MAIN);
    /* 删除轮廓 */
    lv_obj_set_style_outline_width(main_container, 0, LV_PART_MAIN);
    /* 删除轮廓间距 */
    lv_obj_set_style_outline_pad(main_container, 0, LV_PART_MAIN);
    // 删除圆角
    lv_obj_set_style_radius(main_container, 0, LV_PART_MAIN);
    /* 删除边框 */
    lv_obj_set_style_border_width(main_container, 0, LV_PART_MAIN);
    // 设置内边距
    lv_obj_set_style_pad_all(main_container, 0, LV_PART_MAIN);

    //设置布局为flex,主轴垂直向下，横轴居左，两个主要模块紧贴
    lv_obj_set_flex_flow(main_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(main_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(main_container, 0, LV_PART_MAIN);

    // 用户使用界面
    user_operation_screen_container();
    charging_status_create();      // 充电状态界面
    charging_information_create(); // 充电信息界面
    settings_init();               // 设置界面


    //用户选择界面容器
    ui_create_user_selection_container();

    lv_obj_t * icon1 = lv_label_create(User_Selection_Screen);
    /* 设置字体 */
    lv_obj_set_style_text_font(icon1, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_label_set_text(icon1, LV_SYMBOL_CHARGE " Charging_status");
    ui_create_user_selection(User_Selection_Screen,icon1);

    lv_obj_t * icon2 = lv_label_create(User_Selection_Screen);
    /* 设置字体 */
    lv_obj_set_style_text_font(icon2, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_label_set_text(icon2, LV_SYMBOL_LIST " Charging_Information");
    ui_create_user_selection(User_Selection_Screen,icon2);

    lv_obj_t * Charging_Btn = lv_label_create(User_Selection_Screen);
    /* 设置字体 */
    lv_obj_set_style_text_font(Charging_Btn, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_label_set_text(Charging_Btn, LV_SYMBOL_SETTINGS " Settings");
    ui_create_user_selection(User_Selection_Screen,Charging_Btn);
}
