#include "Settings.h"
#include "ui.h"
#include "lcd.h"

// 设置界面颜色定义
#define SETTINGS_PANEL_COLOR  0x151B22    // 设置面板背景色
#define SETTINGS_TEXT_COLOR   0xE6EBF0    // 主要文字颜色
#define SETTINGS_MUTED_COLOR  0x9AA6B2    // 次级文字颜色
#define SETTINGS_ACCENT_COLOR 0x4AA8F0    // 强调色

void brightness_event_handler(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target(e);
    bring_now = lv_slider_get_value(slider);// 获取当前滑块值

}

void sleeptime_event_handler(lv_event_t * e)
{
    lv_obj_t * sleeptime_btn = lv_event_get_target(e);


}


// 创建无背景、无边框、无圆角、无阴影的基础容器
static lv_obj_t * settings_create_plain(lv_obj_t * parent)
{
    lv_obj_t * obj = lv_obj_create(parent);

    lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(obj, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(obj, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(obj, 0, LV_PART_MAIN);

    // 关闭滚动条及滚动功能
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

    return obj;
}


// 创建文字标签，统一设置文字内容、颜色和字体
static lv_obj_t * settings_create_label(lv_obj_t * parent, const char * text,
                                        uint32_t color, const lv_font_t * font)
{
    lv_obj_t * label = lv_label_create(parent);

    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_hex(color), LV_PART_MAIN);
    lv_obj_set_style_text_font(label, font, LV_PART_MAIN);

    return label;
}


// 设置下拉列表的统一样式
static void settings_style_dropdown(lv_obj_t * dropdown)
{
    lv_obj_set_width(dropdown, 164);
    lv_obj_set_height(dropdown, 27);

    // 背景颜色及不透明度
    lv_obj_set_style_bg_color(dropdown, lv_color_hex(0x222B34), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(dropdown, LV_OPA_COVER, LV_PART_MAIN);

    // 边框及圆角
    lv_obj_set_style_border_color(dropdown, lv_color_hex(0x34414D), LV_PART_MAIN);
    lv_obj_set_style_border_width(dropdown, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(dropdown, 4, LV_PART_MAIN);

    // 内部左右留白
    lv_obj_set_style_pad_left(dropdown, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_right(dropdown, 8, LV_PART_MAIN);

    // 文字颜色及字体
    lv_obj_set_style_text_color(dropdown, lv_color_hex(SETTINGS_TEXT_COLOR), LV_PART_MAIN);
    lv_obj_set_style_text_font(dropdown, &lv_font_montserrat_14, LV_PART_MAIN);

        // 获取下拉列表对象
    lv_obj_t * list = lv_dropdown_get_list(dropdown);

    // 下拉面板背景色
    lv_obj_set_style_bg_color(list, lv_color_hex(BACKGROUND_COLOR), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(list, LV_OPA_COVER, LV_PART_MAIN);

    // 普通选项背景色
    lv_obj_set_style_bg_color(list, lv_color_hex(0x18212B), LV_PART_SELECTED);
    lv_obj_set_style_bg_opa(list, LV_OPA_COVER, LV_PART_SELECTED);

    // 当前选中选项背景色
    lv_obj_set_style_bg_color(list, lv_color_hex(0x233342),
                              LV_PART_SELECTED | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(list, LV_OPA_COVER,
                            LV_PART_SELECTED | LV_STATE_CHECKED);

    // 点击选项时的背景色
    lv_obj_set_style_bg_color(list, lv_color_hex(0x30485D),
                              LV_PART_SELECTED | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(list, LV_OPA_COVER,
                            LV_PART_SELECTED | LV_STATE_PRESSED);

                                // 统一下拉选项文字颜色
    lv_obj_set_style_text_color(list, lv_color_hex(SETTINGS_TEXT_COLOR), LV_PART_MAIN);

    // 统一字体透明度，保持文字清晰
    lv_obj_set_style_text_opa(list, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_text_opa(list, LV_OPA_COVER, LV_PART_SELECTED);
    lv_obj_set_style_text_opa(list, LV_OPA_COVER,
                              LV_PART_SELECTED | LV_STATE_CHECKED);
    lv_obj_set_style_text_opa(list, LV_OPA_COVER,
                              LV_PART_SELECTED | LV_STATE_PRESSED);
}


// 创建单行设置项目，左侧为名称，右侧为设置控件
static lv_obj_t * settings_create_row(lv_obj_t * parent, const char * title)
{
    lv_obj_t * row = settings_create_plain(parent);

    // 设置行尺寸
    lv_obj_set_size(row, LV_PCT(100), 30);

    // 横向 Flex 布局
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);

    // 左右两端对齐，垂直居中
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_set_style_pad_column(row, 8, LV_PART_MAIN);

    // 创建设置项目名称
    lv_obj_t * label = settings_create_label(row, title, SETTINGS_TEXT_COLOR,
                                              &lv_font_montserrat_14);

    // 名称区域自动占据剩余宽度
    lv_obj_set_width(label, 0);
    lv_obj_set_flex_grow(label, 1);

    return row;
}


// 创建开关控件，并设置初始状态
static void settings_add_switch(lv_obj_t * row, bool checked)
{
    lv_obj_t * sw = lv_switch_create(row);

    // 开关尺寸
    lv_obj_set_size(sw, 42, 23);

    // 关闭状态轨道颜色
    lv_obj_set_style_bg_color(sw, lv_color_hex(0x46515D), LV_PART_INDICATOR);

    // 开启状态轨道颜色
    lv_obj_set_style_bg_color(sw, lv_color_hex(SETTINGS_ACCENT_COLOR),
                              LV_PART_INDICATOR | LV_STATE_CHECKED);

    lv_obj_set_style_bg_opa(sw, LV_OPA_COVER, LV_PART_INDICATOR);

    // 滑块颜色
    lv_obj_set_style_bg_color(sw, lv_color_hex(0xE6EBF0), LV_PART_KNOB);
    lv_obj_set_style_bg_opa(sw, LV_OPA_COVER, LV_PART_KNOB);

    // 设置开关初始状态
    if(checked) lv_obj_add_state(sw, LV_STATE_CHECKED);
}


// 创建下拉列表并设置选项及默认选中项
static void settings_add_dropdown(lv_obj_t * row, const char * options,
                                  uint16_t selected)
{
    lv_obj_t * dropdown = lv_dropdown_create(row);

    // 设置下拉选项
    lv_dropdown_set_options(dropdown, options);

    // 设置默认选中项，索引从 0 开始
    lv_dropdown_set_selected(dropdown, selected);

    // 应用统一样式
    settings_style_dropdown(dropdown);
}


// 设置界面初始化
void settings_init(void)
{
    /* 第一部分：创建设置页面及顶部标题栏。 */

    // 创建设置页面
    lv_obj_t * page = lv_obj_create(User_Screen);
    lv_obj_set_size(page, LV_PCT(100), LV_PCT(100));

    // 应用通用 UI 样式
    General_UI_Properties(page);

    // 设置页面背景及内部留白
    lv_obj_set_style_bg_color(page, lv_color_hex(BACKGROUND_COLOR), LV_PART_MAIN);
    lv_obj_set_style_pad_all(page, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_row(page, 8, LV_PART_MAIN);

    // 页面采用纵向 Flex 布局
    lv_obj_set_flex_flow(page, LV_FLEX_FLOW_COLUMN);

    // 禁止页面滚动
    lv_obj_remove_flag(page, LV_OBJ_FLAG_SCROLLABLE);


    // 顶部标题栏
    lv_obj_t * header = settings_create_plain(page);
    lv_obj_set_size(header, LV_PCT(100), 30);

    // 标题栏采用横向布局
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);

    // 标题和保存按钮分别靠左右两端
    lv_obj_set_flex_align(header, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // 页面标题
    settings_create_label(header, "Settings", LINE_COLOR,
                          &lv_font_montserrat_20);

    /* 第二部分：创建主体设置区域。 */

    // 创建主体容器
    lv_obj_t * body = settings_create_plain(page);

    // 主体宽度铺满页面，高度自动占据剩余空间
    lv_obj_set_width(body, LV_PCT(100));
    lv_obj_set_flex_grow(body, 1);

    // 主体采用纵向布局
    lv_obj_set_flex_flow(body, LV_FLEX_FLOW_COLUMN);


    /* 第三部分：创建设置内容面板。 */

    // 创建设置面板
    lv_obj_t * panel = lv_obj_create(body);

    // 面板铺满主体区域
    lv_obj_set_size(panel, LV_PCT(100), LV_PCT(100));

    // 设置面板背景颜色
    lv_obj_set_style_bg_color(panel, lv_color_hex(SETTINGS_PANEL_COLOR), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, LV_PART_MAIN);

    // 设置面板边框及圆角
    lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(panel, 6, LV_PART_MAIN);

    // 设置面板内部留白及项目间距
    lv_obj_set_style_pad_all(panel, 9, LV_PART_MAIN);
    lv_obj_set_style_pad_row(panel, 2, LV_PART_MAIN);

    // 设置项目纵向排列
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);

    // 关闭滚动条及滚动功能
    lv_obj_set_scrollbar_mode(panel, LV_SCROLLBAR_MODE_OFF);
    lv_obj_remove_flag(panel, LV_OBJ_FLAG_SCROLLABLE);


    /* 第四部分：亮度自适应设置。 */

    // 创建亮度自适应设置行
    lv_obj_t * row = settings_create_row(panel, "Brightness Adaptation");
    // 创建亮度自适应开关，默认关闭
    settings_add_switch(row, false);


    /* 第五部分：屏幕亮度设置。 */

    // 创建亮度设置行
    row = settings_create_row(panel, "Brightness");

    // 创建亮度滑块
    lv_obj_t * brightness = lv_slider_create(row);
    lv_obj_add_event_cb(brightness, brightness_event_handler, LV_EVENT_PRESSING, NULL);

    // 滑块尺寸
    lv_obj_set_width(brightness, 155);
    lv_obj_set_height(brightness, 40);

    // 亮度范围为 0~100，默认最大亮度
    lv_slider_set_range(brightness, 0, 100);
    lv_slider_set_value(brightness, 100, LV_ANIM_OFF);

    // 滑块轨道高度
    lv_obj_set_style_height(brightness, 16, LV_PART_MAIN);

    // 滑块轨道颜色
    lv_obj_set_style_bg_color(brightness, lv_color_hex(0x34414D), LV_PART_MAIN);

    // 已选中部分颜色
    lv_obj_set_style_bg_color(brightness, lv_color_hex(SETTINGS_ACCENT_COLOR), LV_PART_INDICATOR);

    // 滑块控制点颜色
    lv_obj_set_style_bg_color(brightness, lv_color_hex(0xE6EBF0), LV_PART_KNOB);

    // 当前亮度数值
    lv_obj_t * brightness_value = settings_create_label(row, "100", SETTINGS_MUTED_COLOR,
                                                        &lv_font_montserrat_12);

    // 数值标签宽度及文字对齐方式
    lv_obj_set_width(brightness_value, 26);
    lv_obj_set_style_text_align(brightness_value, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);


    /* 第六部分：屏幕待机时间设置。 */

    // 创建待机时间设置行
    row = settings_create_row(panel, "Sleep Time");
    lv_obj_add_event_cb(row, sleeptime_event_handler, LV_EVENT_PRESSED, NULL);

    // 待机时间选项，默认选择 1 minute
    settings_add_dropdown(row, "30 seconds\n1 minute\n5 minutes\n10 minutes", 1);


    /* 第七部分：屏幕保护时间设置。 */

    // 创建屏幕保护时间设置行
    row = settings_create_row(panel, "Screen Saver Time");

    // 屏幕保护时间选项，默认选择 30 minutes
    settings_add_dropdown(row, "1 minute\n5 minutes\n10 minutes\n30 minutes", 3);


    /* 第八部分：屏幕保护功能设置。 */

    // 创建屏幕保护设置行
    row = settings_create_row(panel, "Screen Saver");

    // 屏幕保护开关，默认开启
    settings_add_switch(row, true);


    /* 第九部分：定时功能设置。 */

    // 创建定时功能设置行
    row = settings_create_row(panel, "Time Schedule");

    // 定时功能开关，默认关闭
    settings_add_switch(row, false);


    /* 第十部分：屏幕保护类型设置。 */

    // 创建屏幕保护类型设置行
    row = settings_create_row(panel, "Screen Saver Type");

    // 屏幕保护类型选项，默认选择 Local Pictures
    settings_add_dropdown(row, "Local Pictures\nClock\nNone", 0);


    /* 第十一部分：屏幕锁定功能设置。 */

    // 创建屏幕锁定设置行
    row = settings_create_row(panel, "Screen Lock");

    // 屏幕锁定开关，默认关闭
    settings_add_switch(row, false);


    /* 第十三部分：字体大小设置。 */

    // 创建字体大小设置行
    row = settings_create_row(panel, "Font Size");

    // 字体大小选项，默认选择 Normal
    settings_add_dropdown(row, "Small\nNormal\nLarge", 1);
}
