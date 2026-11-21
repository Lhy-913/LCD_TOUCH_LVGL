#include "Charging_Status.h"
#include "ui.h"

#include "Lvgl_Array_Update.h"
#include "CAN_To_BMS.h"

uint16_t Charging_POWER_Kw = 0; // 充电功率
uint16_t Charging_MAX_Kw = 0; // 图表显示的最大功率
uint16_t Charging_Time_Minutes = 0; // 充电剩余时间

bool Chart_Update_Start = false; // 有新的 BMS 数据待显示
bool is_charging = false; // 充电开关初始为关闭

bool Per_Minute_Data = 0; // 图表时间视图：0 为 1 分钟，1 为 3 分钟

static lv_obj_t *label_power;
static lv_obj_t *label_time;
static lv_obj_t *chart_label_top;
static lv_obj_t *chart_label_mid;

static lv_obj_t *Charging_Chart = NULL; // 充电状态图表
static lv_chart_series_t *ser = NULL; // 充电状态图表 - 数据序列1

static void Charging_Chart_Update(void)
{
    uint32_t count = lv_chart_get_point_count(Charging_Chart);
    int32_t *points = lv_chart_get_series_y_array(Charging_Chart, ser);
    bool changed = false;

    for (uint32_t i = 0; i < count; i++)
    {
        int32_t point = (int32_t)User_Change_message_date.buff[i];
        if (points[i] != point)
        {
            points[i] = point;
            changed = true;
        }
    }
    if (changed)
        lv_chart_refresh(Charging_Chart);
}

void Charging_Status_Update(void) // 更新充电界面的各状态值
{
    uint16_t power = (uint16_t)PowerReal_x10;
    uint16_t max_power = User_Change_message_date.Max_Date;

    if (Charging_POWER_Kw != power)
    {
        Charging_POWER_Kw = power;
        lv_label_set_text_fmt(label_power, "%u kW", Charging_POWER_Kw);
    }
    if (Charging_Time_Minutes != Estimated_Charging_Time)
    {
        Charging_Time_Minutes = Estimated_Charging_Time;
        lv_label_set_text_fmt(label_time, "%u min", Charging_Time_Minutes);
    }
    if (Charging_MAX_Kw != max_power)
    {
        Charging_MAX_Kw = max_power;
        lv_label_set_text_fmt(chart_label_top, "%u Kw", Charging_MAX_Kw);
        lv_label_set_text_fmt(chart_label_mid, "%u Kw", Charging_MAX_Kw / 2);
        lv_chart_set_axis_range(Charging_Chart, LV_CHART_AXIS_PRIMARY_Y, 0, Charging_MAX_Kw);
    }
    Charging_Chart_Update();
}

static void Change_Btn_event_cb(lv_event_t *e) // 充电状态更改按钮
{
    is_charging = !is_charging;
    lv_obj_t *Change_Btn = lv_event_get_target(e);
    lv_obj_t *Change_Btn_label = lv_obj_get_child(Change_Btn, 0);
    lv_label_set_text(Change_Btn_label, (is_charging ? "Pause" : "Start"));
    lv_obj_set_style_opa(Change_Btn_label, (is_charging ? LV_OPA_COVER : LV_OPA_80), LV_PART_MAIN);
    lv_obj_align_to(Change_Btn_label, Change_Btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(Change_Btn, lv_color_hex(is_charging ? Change_Pause_Btn_COLOR : Change_Start_Btn_COLOR), LV_PART_MAIN);
}

static void Per_Minute_Choose_3min_event_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    lv_obj_t *parent = lv_obj_get_parent(obj);

    lv_obj_set_style_opa(lv_obj_get_child(parent, 2),
                         LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_opa(obj, LV_OPA_60, LV_PART_MAIN);

    Per_Minute_Data = 1;
    lv_obj_set_style_pad_column(Charging_Chart, 1, LV_PART_MAIN);
    lv_chart_set_point_count(Charging_Chart, 30);

    Charging_Chart_Update();
}

static void Per_Minute_Choose_1min_event_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    lv_obj_t *parent = lv_obj_get_parent(obj);
    lv_obj_set_style_opa(lv_obj_get_child(parent, 1),
                         LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_opa(obj, LV_OPA_60, LV_PART_MAIN);

    Per_Minute_Data = 0;
    lv_obj_set_style_pad_column(Charging_Chart, 18, LV_PART_MAIN);
    lv_chart_set_point_count(Charging_Chart, 15);

    Charging_Chart_Update();
}

static void charging_chart_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *chart = lv_event_get_target(e);
    lv_obj_t *label = lv_event_get_user_data(e);

    if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST)
    {
        lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    if (code != LV_EVENT_LONG_PRESSED)
        return;

    uint32_t point_id = lv_chart_get_pressed_point(chart);
    if (point_id == LV_CHART_POINT_NONE)
        return;

    lv_chart_series_t *series =
        lv_chart_get_series_next(chart, NULL);
    if (series == NULL)
        return;

    int32_t *y_array =
        lv_chart_get_series_y_array(chart, series);
    if (y_array == NULL)
        return;

    lv_point_t point;
    lv_chart_get_point_pos_by_id(chart, series, point_id, &point);

    lv_label_set_text_fmt(label, "%d", (int)y_array[point_id]);
    lv_obj_update_layout(label);

    lv_obj_set_pos(label,
                   point.x - lv_obj_get_width(label) / 2,
                   point.y - lv_obj_get_height(label) - 7);

    lv_obj_remove_flag(label, LV_OBJ_FLAG_HIDDEN);
}

// 用户使用界面-充电状态
void charging_status_create(void)
{
    lv_obj_t *Charging_status = lv_obj_create(User_Screen);
    lv_obj_set_size(Charging_status, LV_PCT(100), LV_PCT(100));
    General_UI_Properties(Charging_status); // 清除边框、圆角和内边距，隐藏滚动条
    lv_obj_set_flex_flow(Charging_status, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(Charging_status, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_all(Charging_status, 10, LV_PART_MAIN);
    lv_obj_set_style_bg_color(Charging_status, lv_color_hex(0x0B0F14), 0);

    // ================= 左侧容器 =================
    lv_obj_t *Charging_status_Left = lv_obj_create(Charging_status);
    lv_obj_set_flex_grow(Charging_status_Left, 35);
    lv_obj_set_height(Charging_status_Left, LV_PCT(100));

    // 卡片风格：无边框、深色背景、圆角、内边距
    lv_obj_set_style_border_width(Charging_status_Left, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(Charging_status_Left, 12, LV_PART_MAIN);
    lv_obj_set_style_bg_color(Charging_status_Left, lv_color_hex(Information_Panel), LV_PART_MAIN); // 深灰黑
    lv_obj_set_style_pad_all(Charging_status_Left, 20, LV_PART_MAIN);

    // 内部用列向 Flex 布局，元素自动从上往下排
    lv_obj_set_flex_flow(Charging_status_Left, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(Charging_status_Left, 10, LV_PART_MAIN);

    // 充电状态标题
    lv_obj_t *label_state = lv_label_create(Charging_status_Left);
    lv_obj_set_style_text_color(label_state, lv_color_hex(LINE_COLOR), LV_PART_MAIN); // 荧光绿
    lv_obj_set_style_text_font(label_state, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_label_set_text(label_state, "Charging");

    // 充电提示文字
    lv_obj_t *label_tips = lv_label_create(Charging_status_Left);
    lv_obj_set_style_text_color(label_tips, lv_color_hex(0x9E9E9E), LV_PART_MAIN);
    lv_obj_set_style_text_font(label_tips, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_label_set_text(label_tips, "Charge power limited by the vehicle\nGet more info in charging details");
    lv_label_set_long_mode(label_tips, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(label_tips, LV_PCT(100));

    // 透明占位区将功率和剩余时间推到卡片底部
    lv_obj_t *spacer = lv_obj_create(Charging_status_Left);
    lv_obj_set_flex_grow(spacer, 1);
    lv_obj_set_width(spacer, LV_PCT(100));
    lv_obj_set_style_bg_opa(spacer, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(spacer, 0, LV_PART_MAIN);

    // 功率标题和数值
    lv_obj_t *label_power_title = lv_label_create(Charging_status_Left);
    lv_obj_set_style_text_color(label_power_title, lv_color_hex(0x9E9E9E), LV_PART_MAIN);
    lv_obj_set_style_text_font(label_power_title, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_label_set_text(label_power_title, "Power");

    label_power = lv_label_create(Charging_status_Left);
    lv_obj_set_style_text_color(label_power, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(label_power, &lv_font_montserrat_28, LV_PART_MAIN); // 大字
    lv_label_set_text_fmt(label_power, "%d kW", Charging_POWER_Kw);

    // 剩余充电时间
    lv_obj_t *Estimated_Time_to_Full = lv_label_create(Charging_status_Left);
    lv_obj_set_style_text_color(Estimated_Time_to_Full, lv_color_hex(0x9E9E9E), LV_PART_MAIN);
    lv_obj_set_style_text_font(Estimated_Time_to_Full, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_label_set_text(Estimated_Time_to_Full, "Estimated Time to Full");

    label_time = lv_label_create(Charging_status_Left);
    lv_obj_set_style_text_color(label_time, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(label_time, &lv_font_montserrat_22, LV_PART_MAIN);
    lv_label_set_text_fmt(label_time, "%d min", Charging_Time_Minutes);

    // 右侧容器
    lv_obj_t *Charging_status_Right = lv_obj_create(Charging_status);
    lv_obj_set_flex_grow(Charging_status_Right, 65);
    // 设置内边距
    lv_obj_set_style_pad_all(Charging_status_Right, 10, LV_PART_MAIN);
    // 关闭滚动条
    lv_obj_set_scrollbar_mode(Charging_status_Right, LV_SCROLLBAR_MODE_OFF);
    // 去除边框
    lv_obj_set_style_border_width(Charging_status_Right, 0, LV_PART_MAIN);
    // 高度设置
    lv_obj_set_height(Charging_status_Right, LV_PCT(100));
    // 设置背景颜色
    lv_obj_set_style_bg_color(Charging_status_Right, lv_color_hex(Information_Panel), 0);
    // 禁止滚动
    lv_obj_remove_flag(Charging_status_Right, LV_OBJ_FLAG_SCROLLABLE);

    // 创作者微信名片二维码-图片
    LV_IMAGE_DECLARE(Creator_WeChat_L);
    lv_obj_t *img1 = lv_image_create(Charging_status_Right);
    lv_image_set_src(img1, &Creator_WeChat_L);
    lv_obj_set_pos(img1, 0, 0);
    // 创作者微信名片二维码-文字介绍
    lv_obj_t *lable = lv_label_create(Charging_status_Right);
    lv_label_set_text(lable, "Creator Contact Card\n\n"
                             "Questions or help?\n"
                             "Scan to contact me.");
    lv_obj_align_to(lable, img1, LV_ALIGN_OUT_RIGHT_MID, 5, 1);
    lv_obj_set_style_text_color(lable, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(lable, &lv_font_montserrat_12, LV_PART_MAIN);

    // 创建充电按钮
    lv_obj_t *Change_btn = lv_btn_create(Charging_status_Right);
    lv_obj_set_size(Change_btn, LV_PCT(22), LV_PCT(10));
    lv_obj_align(Change_btn, LV_ALIGN_TOP_RIGHT, 5, -2);

    lv_obj_set_style_bg_color(Change_btn,
                              lv_color_hex(Change_Start_Btn_COLOR),
                              LV_PART_MAIN);
    lv_obj_set_style_bg_opa(Change_btn, LV_OPA_80, LV_PART_MAIN);
    lv_obj_set_style_radius(Change_btn, 8, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(Change_btn, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(Change_btn, 0, LV_PART_MAIN);

    lv_obj_add_event_cb(Change_btn, Change_Btn_event_cb,
                        LV_EVENT_LONG_PRESSED, NULL);

    // 创建按钮标签
    lv_obj_t *Change_btn_label = lv_label_create(Change_btn);
    lv_label_set_text(Change_btn_label, "Charging");
    lv_obj_set_style_text_color(Change_btn_label,
                                lv_color_hex(0xE8FFF0),
                                LV_PART_MAIN);
    lv_obj_set_style_text_font(Change_btn_label,
                               &lv_font_montserrat_14,
                               LV_PART_MAIN);
    lv_obj_center(Change_btn_label);

    // 电池区显示
    lv_obj_t *Battery = lv_obj_create(Charging_status_Right);
    // 去除边框
    lv_obj_set_style_border_width(Battery, 0, LV_PART_MAIN);
    // 隐藏滚动条并禁止滚动
    lv_obj_remove_flag(Battery, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_clip_corner(Battery, true, LV_PART_MAIN);
    // 子组件显示范围不超过自身
    lv_obj_set_style_clip_corner(Battery, true, LV_PART_MAIN);
    // 去除内边距
    lv_obj_set_style_pad_all(Battery, 0, LV_PART_MAIN);

    lv_obj_set_size(Battery, LV_PCT(98), LV_PCT(46));
    lv_obj_align(Battery, LV_ALIGN_CENTER, 0, 101);
    lv_obj_set_style_pad_all(Battery, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(Battery, lv_color_hex(BACKGROUND_COLOR), 0);

    // 电池区标题
    lv_obj_t *Power_Trend = lv_label_create(Battery);
    lv_obj_set_style_text_color(Power_Trend, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(Power_Trend, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_label_set_text(Power_Trend, "Power Trend");
    lv_obj_align(Power_Trend, LV_ALIGN_TOP_MID, 0, 2);

    /* 时间视图切换 */
    // 3 分钟按钮
    lv_obj_t *Per_Minute_Choose_3min = lv_btn_create(Battery);
    lv_obj_set_size(Per_Minute_Choose_3min, LV_PCT(13), LV_PCT(10));
    lv_obj_align(Per_Minute_Choose_3min, LV_ALIGN_TOP_RIGHT, -5, 8);
    lv_obj_add_event_cb(Per_Minute_Choose_3min, Per_Minute_Choose_3min_event_cb, LV_EVENT_SHORT_CLICKED, NULL);

    // 隐藏按钮背景
    lv_obj_set_style_bg_opa(Per_Minute_Choose_3min, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(Per_Minute_Choose_3min, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(Per_Minute_Choose_3min, 0, LV_PART_MAIN);

    lv_obj_t *lable_3min = lv_label_create(Per_Minute_Choose_3min);
    lv_label_set_text(lable_3min, "3min");
    lv_obj_set_style_text_color(lable_3min, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(lable_3min, &lv_font_montserrat_12, 0);
    lv_obj_center(lable_3min);

    // 1 分钟按钮
    lv_obj_t *Per_Minute_Choose_1min = lv_btn_create(Battery);
    lv_obj_set_size(Per_Minute_Choose_1min, LV_PCT(13), LV_PCT(10));
    lv_obj_align_to(Per_Minute_Choose_1min, Per_Minute_Choose_3min,
                    LV_ALIGN_OUT_LEFT_MID, 0, 0);
    lv_obj_set_style_opa(Per_Minute_Choose_1min, LV_OPA_80, LV_PART_MAIN);
    lv_obj_add_event_cb(Per_Minute_Choose_1min, Per_Minute_Choose_1min_event_cb, LV_EVENT_SHORT_CLICKED, NULL);

    // 隐藏按钮背景
    lv_obj_set_style_bg_opa(Per_Minute_Choose_1min, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(Per_Minute_Choose_1min, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(Per_Minute_Choose_1min, 0, LV_PART_MAIN);

    lv_obj_t *lable_1min = lv_label_create(Per_Minute_Choose_1min);
    lv_label_set_text(lable_1min, "1min");
    lv_obj_set_style_text_color(lable_1min, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(lable_1min, &lv_font_montserrat_12, 0);
    lv_obj_center(lable_1min);

    // 中间竖线
    lv_obj_t *separator = lv_obj_create(Battery);
    lv_obj_set_size(separator, 1, 15);
    lv_obj_align_to(separator, Per_Minute_Choose_3min,
                    LV_ALIGN_OUT_LEFT_MID, 0, 0);

    lv_obj_set_style_bg_color(separator, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(separator, LV_OPA_50, 0);
    lv_obj_set_style_border_width(separator, 0, 0);
    lv_obj_set_style_pad_all(separator, 0, 0);

    // 电池区域：用柱状图显示充电功率

    // 创建图表
    Charging_Chart = lv_chart_create(Battery);
    // 设置图表位置
    lv_obj_align(Charging_Chart, LV_ALIGN_BOTTOM_LEFT, LV_PCT(10), 0);

    General_UI_Properties(Charging_Chart);
    // 设置图表绘制区域的内边距
    lv_obj_set_style_pad_all(Charging_Chart, 0, LV_PART_MAIN);
    // 左右内边距
    lv_obj_set_style_pad_hor(Charging_Chart, 5, LV_PART_MAIN);
    // 上内边距
    lv_obj_set_style_pad_top(Charging_Chart, 8, LV_PART_MAIN);
    // 设置图表尺寸
    lv_obj_set_size(Charging_Chart, LV_PCT(90), LV_PCT(80));
    // 设置图表背景颜色
    lv_obj_set_style_bg_color(Charging_Chart, lv_color_hex(BACKGROUND_COLOR), 0);

    // 设置图表类型：柱状图
    lv_chart_set_type(Charging_Chart, LV_CHART_TYPE_BAR);

    // 创建一个隐藏的标签，用于显示用户选中图表中的值
    lv_obj_t *chart_value_label = lv_label_create(Charging_Chart);
    lv_obj_set_style_text_color(chart_value_label,
                                lv_color_hex(0xFFFFFF),
                                LV_PART_MAIN);
    lv_obj_add_flag(chart_value_label, LV_OBJ_FLAG_HIDDEN);

    lv_obj_add_event_cb(Charging_Chart,
                        charging_chart_event_cb,
                        LV_EVENT_LONG_PRESSED,
                        chart_value_label);
    // 添加松手后的回调函数
    lv_obj_add_event_cb(Charging_Chart,
                        charging_chart_event_cb,
                        LV_EVENT_ALL,
                        chart_value_label);

    // 设置数据点数量
    lv_chart_set_point_count(Charging_Chart, 15);

    // 设置 Y 轴范围：0 ~ Charging_MAX_Kw
    lv_chart_set_axis_range(Charging_Chart, LV_CHART_AXIS_PRIMARY_Y, 0, Charging_MAX_Kw);
    // 设置水平、垂直分割线数量
    lv_chart_set_div_line_count(Charging_Chart, 0, 0);
    if (Per_Minute_Data)
    {
        // 每组柱子之间的间距，单位 px
        lv_obj_set_style_pad_column(Charging_Chart, 1, LV_PART_MAIN);
    }
    else
    {
        lv_obj_set_style_pad_column(Charging_Chart, 18, LV_PART_MAIN);
    }

    /* 游标线样式 */
    // 设置图表游标线宽度为 1px
    lv_obj_set_style_line_width(Charging_Chart, 1, LV_PART_CURSOR);

    // 设置游标虚线每段的长度为 4px
    lv_obj_set_style_line_dash_width(Charging_Chart, 4, LV_PART_CURSOR);

    // 设置游标虚线每段之间的间隔为 4px
    lv_obj_set_style_line_dash_gap(Charging_Chart, 4, LV_PART_CURSOR);

    // 设置游标线端点不使用圆角
    lv_obj_set_style_line_rounded(Charging_Chart, false, LV_PART_CURSOR);

    lv_chart_cursor_t *cursor1 =
        lv_chart_add_cursor(Charging_Chart, lv_color_hex(0xFFFFFF), LV_DIR_HOR);
    lv_chart_cursor_t *cursor2 =
        lv_chart_add_cursor(Charging_Chart, lv_color_hex(0xFFFFFF), LV_DIR_HOR);

    lv_chart_set_cursor_pos_y(Charging_Chart, cursor1, 8);
    lv_chart_set_cursor_pos_y(Charging_Chart, cursor2, 75);
    // 添加一条数据序列，颜色为绿色，使用左侧 Y 轴
    ser = lv_chart_add_series(
        Charging_Chart,
        lv_color_hex(Chart_COLOR),
        LV_CHART_AXIS_PRIMARY_Y);

    // Y 轴刻度文字
    chart_label_top = lv_label_create(Battery);
    lv_label_set_text_fmt(chart_label_top, "%d Kw", Charging_MAX_Kw);
    lv_obj_set_style_text_color(chart_label_top, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_pad_all(chart_label_top, 0, LV_PART_MAIN);
    lv_obj_align_to(chart_label_top, Charging_Chart,
                    LV_ALIGN_OUT_LEFT_TOP, 2, -5);
    lv_obj_set_style_text_font(chart_label_top, &lv_font_montserrat_12, LV_PART_MAIN);

    chart_label_mid = lv_label_create(Battery);
    lv_label_set_text_fmt(chart_label_mid, "%d Kw", (Charging_MAX_Kw / 2));
    lv_obj_set_style_text_color(chart_label_mid, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_pad_all(chart_label_mid, 0, LV_PART_MAIN);
    lv_obj_align_to(chart_label_mid, Charging_Chart,
                    LV_ALIGN_OUT_LEFT_MID, 2, -9);
    lv_obj_set_style_text_font(chart_label_mid, &lv_font_montserrat_12, LV_PART_MAIN);

    lv_obj_t *chart_label_bottom = lv_label_create(Battery);
    lv_label_set_text(chart_label_bottom, "0 Kw");
    lv_obj_set_style_text_color(chart_label_bottom, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_pad_all(chart_label_bottom, 0, LV_PART_MAIN);
    lv_obj_align_to(chart_label_bottom, Charging_Chart,
                    LV_ALIGN_OUT_LEFT_BOTTOM, 2, -25);
    lv_obj_set_style_text_font(chart_label_bottom, &lv_font_montserrat_12, LV_PART_MAIN);
}
