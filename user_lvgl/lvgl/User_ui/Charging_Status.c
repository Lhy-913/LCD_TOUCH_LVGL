#include "Charging_Status.h"
#include "ui.h"

    int Charging_now = 70;//假装当前电量
    uint16_t Charging_MAX_Kw = 130;//假装当前充电最大功率
    bool Per_Minute_Data = 0;//每x分钟数据 0：x = 1分钟 1：x = 3分钟
    lv_obj_t * Charging_Chart = NULL;

    static const int32_t data[] = {
    23, 67, 41, 89, 12, 76, 54, 32, 95, 48,
    71, 16, 83, 37, 62, 28, 91, 45, 74, 19,
    56, 98, 34, 81, 25, 63, 47,130, 87, 52
    };


static void Per_Minute_Choose_3min_event_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);
    lv_obj_t * parent = lv_obj_get_parent(obj);

    lv_obj_set_style_opa(lv_obj_get_child(parent, 2),
                        LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_opa(obj, LV_OPA_80, LV_PART_MAIN);

    Per_Minute_Data = 1;
    lv_obj_set_style_pad_column(Charging_Chart, 1, LV_PART_MAIN);
    lv_chart_set_point_count(Charging_Chart, 30);

    lv_chart_series_t * ser = lv_chart_get_series_next(Charging_Chart, NULL);

    for(uint32_t i = 0; i < 30; i++)
    {
        lv_chart_set_series_value_by_id(Charging_Chart, ser, i, data[i]);
    }
}

static void Per_Minute_Choose_1min_event_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);
    lv_obj_t * parent = lv_obj_get_parent(obj);
    lv_obj_set_style_opa(lv_obj_get_child(parent, 1),
                        LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_opa(obj, LV_OPA_80, LV_PART_MAIN);

    Per_Minute_Data = 0;
    lv_obj_set_style_pad_column(Charging_Chart, 18, LV_PART_MAIN);
    lv_chart_set_point_count(Charging_Chart, 15);

    lv_chart_series_t * ser = lv_chart_get_series_next(Charging_Chart, NULL);

    for(uint32_t i = 0; i < 15; i++)
    {
        lv_chart_set_series_value_by_id(Charging_Chart, ser, i, data[i]);
    }
}

static void charging_chart_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * chart = lv_event_get_target(e);
    lv_obj_t * label = lv_event_get_user_data(e);

    if(code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    if(code != LV_EVENT_LONG_PRESSED) return;

    uint32_t point_id = lv_chart_get_pressed_point(chart);
    if(point_id == LV_CHART_POINT_NONE) return;

    lv_chart_series_t * series =
        lv_chart_get_series_next(chart, NULL);
    if(series == NULL) return;

    int32_t * y_array =
        lv_chart_get_series_y_array(chart, series);
    if(y_array == NULL) return;

    lv_point_t point;
    lv_chart_get_point_pos_by_id(chart, series, point_id, &point);

    lv_label_set_text_fmt(label, "%d", (int)y_array[point_id]);
    lv_obj_update_layout(label);

    lv_obj_set_pos(label,
                   point.x - lv_obj_get_width(label) / 2,
                   point.y - lv_obj_get_height(label) - 7);

        lv_obj_remove_flag(label, LV_OBJ_FLAG_HIDDEN);
}


//用户使用界面-充电状态
void charging_status_create(void)
{
    lv_obj_t * Charging_status = lv_obj_create(User_Screen);
    lv_obj_set_size(Charging_status, LV_PCT(100), LV_PCT(100));
    General_UI_Properties(Charging_status);//设置常规UI属性（禁止滚动显示，去圆角、边框、内边距）
    lv_obj_set_flex_flow(Charging_status, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(Charging_status, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_all(Charging_status, 10, LV_PART_MAIN);
    lv_obj_set_style_bg_color(Charging_status, lv_color_hex(0x0B0F14), 0);

        // ================= 左侧容器 =================
    lv_obj_t * Charging_status_Left = lv_obj_create(Charging_status);
    lv_obj_set_flex_grow(Charging_status_Left, 35);
    lv_obj_set_height(Charging_status_Left, LV_PCT(100));

    // 卡片风格：无边框、深色背景、圆角、内边距
    lv_obj_set_style_border_width(Charging_status_Left, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(Charging_status_Left, 12, LV_PART_MAIN);
    lv_obj_set_style_bg_color(Charging_status_Left, lv_color_hex(Information_Panel), LV_PART_MAIN);  // 深灰黑
    lv_obj_set_style_pad_all(Charging_status_Left, 20, LV_PART_MAIN);

    // 内部用列向 Flex 布局，元素自动从上往下排
    lv_obj_set_flex_flow(Charging_status_Left, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(Charging_status_Left, 10, LV_PART_MAIN);

    // ---------- "Charging" 状态 ----------
    lv_obj_t * label_state = lv_label_create(Charging_status_Left);
    lv_obj_set_style_text_color(label_state, lv_color_hex(LINE_COLOR), LV_PART_MAIN);  // 荧光绿
    lv_obj_set_style_text_font(label_state, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_label_set_text(label_state,"Charging");

    // ---------- 提示文字（灰色小字） ----------
    lv_obj_t * label_tips = lv_label_create(Charging_status_Left);
    lv_obj_set_style_text_color(label_tips, lv_color_hex(0x9E9E9E), LV_PART_MAIN);
    lv_obj_set_style_text_font(label_tips, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_label_set_text(label_tips, "Charge power limited by the vehicle\nGet more info in charging details");
    lv_label_set_long_mode(label_tips, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(label_tips, LV_PCT(100));

    // ---------- 占位，把数据推到卡片底部（像图片里那样） ----------
    lv_obj_t * spacer = lv_obj_create(Charging_status_Left);
    lv_obj_set_flex_grow(spacer, 1);
    lv_obj_set_width(spacer, LV_PCT(100));
    lv_obj_set_style_bg_opa(spacer, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(spacer, 0, LV_PART_MAIN);

    // ---------- Power（白色大字 + 灰色小标签） ----------
    lv_obj_t * label_power_title = lv_label_create(Charging_status_Left);
    lv_obj_set_style_text_color(label_power_title, lv_color_hex(0x9E9E9E), LV_PART_MAIN);
    lv_obj_set_style_text_font(label_power_title, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_label_set_text(label_power_title, "Power");

    lv_obj_t * label_power = lv_label_create(Charging_status_Left);
    lv_obj_set_style_text_color(label_power, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(label_power, &lv_font_montserrat_28, LV_PART_MAIN);  // 大字
    lv_label_set_text(label_power, "323 kW");

    // ---------- 剩余时间 ----------
    lv_obj_t * Estimated_Time_to_Full = lv_label_create(Charging_status_Left);
    lv_obj_set_style_text_color(Estimated_Time_to_Full, lv_color_hex(0x9E9E9E), LV_PART_MAIN);
    lv_obj_set_style_text_font(Estimated_Time_to_Full, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_label_set_text(Estimated_Time_to_Full, "Estimated Time to Full");

    lv_obj_t * label_time = lv_label_create(Charging_status_Left);
    lv_obj_set_style_text_color(label_time, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(label_time, &lv_font_montserrat_22, LV_PART_MAIN);
    lv_label_set_text(label_time, "14 min");





    //右侧容器
    lv_obj_t * Charging_status_Right = lv_obj_create(Charging_status);
    lv_obj_set_flex_grow(Charging_status_Right, 65);
    //设置内边距
    lv_obj_set_style_pad_all(Charging_status_Right, 10, LV_PART_MAIN);
    //关闭滚动条
    lv_obj_set_scrollbar_mode(Charging_status_Right, LV_SCROLLBAR_MODE_OFF);
    //去除边框
    lv_obj_set_style_border_width(Charging_status_Right, 0, LV_PART_MAIN);
    //高度设置
    lv_obj_set_height(Charging_status_Right, LV_PCT(100));
    //设置背景颜色
    lv_obj_set_style_bg_color(Charging_status_Right, lv_color_hex(Information_Panel), 0);
    //禁止滚动
    lv_obj_remove_flag(Charging_status_Right, LV_OBJ_FLAG_SCROLLABLE);

    //创作者微信名片二维码-图片
    LV_IMAGE_DECLARE(Creator_WeChat_L);
    lv_obj_t * img1 = lv_image_create(Charging_status_Right);
    lv_image_set_src(img1, &Creator_WeChat_L);
    lv_obj_set_pos(img1, 0, 0);
    //创作者微信名片二维码-文字介绍
    lv_obj_t *lable = lv_label_create(Charging_status_Right);
    lv_label_set_text(lable,"Creator Contact Card\n\n"
                            "Questions or help?\n"
                            "Scan to contact me.");
    lv_obj_align_to(lable, img1, LV_ALIGN_OUT_RIGHT_MID, 5, 1);
    lv_obj_set_style_text_color(lable, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(lable, &lv_font_montserrat_12, LV_PART_MAIN);


    //电池区显示
    lv_obj_t *Battery = lv_obj_create(Charging_status_Right);
    //去除边框
    lv_obj_set_style_border_width(Battery, 0, LV_PART_MAIN);
    // 隐藏滚动条并禁止滚动
    lv_obj_remove_flag(Battery, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_clip_corner(Battery, true, LV_PART_MAIN);
    //子组件显示范围不超过自身
    lv_obj_set_style_clip_corner(Battery, true, LV_PART_MAIN);
    //去除内边距
    lv_obj_set_style_pad_all(Battery, 0, LV_PART_MAIN);

    lv_obj_set_size(Battery, LV_PCT(98), LV_PCT(46));
    lv_obj_align(Battery, LV_ALIGN_CENTER, 0, 101);
    lv_obj_set_style_pad_all(Battery, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(Battery, lv_color_hex(BACKGROUND_COLOR), 0);

    //电池区标题
    lv_obj_t * Power_Trend = lv_label_create(Battery);
    lv_obj_set_style_text_color(Power_Trend, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(Power_Trend, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_label_set_text(Power_Trend, "Power Trend");
    lv_obj_align(Power_Trend, LV_ALIGN_TOP_MID, 0, 2);

    /*     时间视图切换       */
    // 3min 按钮
    lv_obj_t * Per_Minute_Choose_3min = lv_btn_create(Battery);
    lv_obj_set_size(Per_Minute_Choose_3min, LV_PCT(13), LV_PCT(10));
    lv_obj_align(Per_Minute_Choose_3min, LV_ALIGN_TOP_RIGHT, -5, 8);
    lv_obj_add_event_cb(Per_Minute_Choose_3min, Per_Minute_Choose_3min_event_cb, LV_EVENT_SHORT_CLICKED, NULL);

    // 隐藏按钮背景
    lv_obj_set_style_bg_opa(Per_Minute_Choose_3min, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(Per_Minute_Choose_3min, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(Per_Minute_Choose_3min, 0, LV_PART_MAIN);

    lv_obj_t * lable_3min = lv_label_create(Per_Minute_Choose_3min);
    lv_label_set_text(lable_3min, "3min");
    lv_obj_set_style_text_color(lable_3min, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(lable_3min, &lv_font_montserrat_12, 0);
    lv_obj_center(lable_3min);


    // 1min 按钮
    lv_obj_t * Per_Minute_Choose_1min = lv_btn_create(Battery);
    lv_obj_set_size(Per_Minute_Choose_1min, LV_PCT(13), LV_PCT(10));
    lv_obj_align_to(Per_Minute_Choose_1min, Per_Minute_Choose_3min,
                    LV_ALIGN_OUT_LEFT_MID, 0, 0);
    lv_obj_set_style_opa(Per_Minute_Choose_1min, LV_OPA_80, LV_PART_MAIN);
    lv_obj_add_event_cb(Per_Minute_Choose_1min, Per_Minute_Choose_1min_event_cb, LV_EVENT_SHORT_CLICKED, NULL);

    // 隐藏按钮背景
    lv_obj_set_style_bg_opa(Per_Minute_Choose_1min, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(Per_Minute_Choose_1min, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(Per_Minute_Choose_1min, 0, LV_PART_MAIN);

    lv_obj_t * lable_1min = lv_label_create(Per_Minute_Choose_1min);
    lv_label_set_text(lable_1min, "1min");
    lv_obj_set_style_text_color(lable_1min, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(lable_1min, &lv_font_montserrat_12, 0);
    lv_obj_center(lable_1min);


    // 中间竖线
    lv_obj_t * separator = lv_obj_create(Battery);
    lv_obj_set_size(separator, 1, 15);
    lv_obj_align_to(separator, Per_Minute_Choose_3min,
                    LV_ALIGN_OUT_LEFT_MID, 0, 0);

    lv_obj_set_style_bg_color(separator, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(separator, LV_OPA_50, 0);
    lv_obj_set_style_border_width(separator, 0, 0);
    lv_obj_set_style_pad_all(separator, 0, 0);


    //电池区显示-折线图  描绘每分钟的充电状态

    // 创建图表
    Charging_Chart = lv_chart_create(Battery);
    // 设置图表位置
    lv_obj_align(Charging_Chart, LV_ALIGN_BOTTOM_LEFT, LV_PCT(10), 0);

    General_UI_Properties(Charging_Chart);
    // 设置图表绘制区域的内边距
    lv_obj_set_style_pad_all(Charging_Chart, 0, LV_PART_MAIN);
    // 左右内边距
    lv_obj_set_style_pad_hor(Charging_Chart, 5, LV_PART_MAIN);
    //上内边距
    lv_obj_set_style_pad_top(Charging_Chart, 8, LV_PART_MAIN);
    // 设置图表尺寸
    lv_obj_set_size(Charging_Chart, LV_PCT(90), LV_PCT(80));
    //设置图表背景颜色
    lv_obj_set_style_bg_color(Charging_Chart, lv_color_hex(BACKGROUND_COLOR), 0);

    // 设置图表类型：折线图
    lv_chart_set_type(Charging_Chart, LV_CHART_TYPE_BAR);

    //创建一个隐藏的标签，用于显示用户选中图表中的值
    lv_obj_t * chart_value_label = lv_label_create(Charging_Chart);
    lv_obj_set_style_text_color(chart_value_label,
                                lv_color_hex(0xFFFFFF),
                                LV_PART_MAIN);
    lv_obj_add_flag(chart_value_label, LV_OBJ_FLAG_HIDDEN);

    lv_obj_add_event_cb(Charging_Chart,
                        charging_chart_event_cb,
                        LV_EVENT_LONG_PRESSED,
                        chart_value_label);
    //添加松手后的回调函数
    lv_obj_add_event_cb(Charging_Chart,
                    charging_chart_event_cb,
                    LV_EVENT_ALL,
                    chart_value_label);

    // 设置数据点数量
    if(Per_Minute_Data == 0)
    {
        lv_chart_set_point_count(Charging_Chart, 15);
    }
    else
    {
        lv_chart_set_point_count(Charging_Chart, 30);
    }
    // 设置 Y 轴范围：0 ~ Charging_MAX_Kw
    lv_chart_set_axis_range(Charging_Chart, LV_CHART_AXIS_PRIMARY_Y, 0, Charging_MAX_Kw);
    // 设置水平、垂直分割线数量
    lv_chart_set_div_line_count(Charging_Chart, 0, 0);
    if(Per_Minute_Data)
    {
    // 每组柱子之间的间距，单位 px
        lv_obj_set_style_pad_column(Charging_Chart, 1, LV_PART_MAIN);
    }
    else
    {
        lv_obj_set_style_pad_column(Charging_Chart, 18, LV_PART_MAIN);
    }


    /*     游标线样式     */
    // 设置图表游标线宽度为 1px
    lv_obj_set_style_line_width(Charging_Chart, 1, LV_PART_CURSOR);

    // 设置游标虚线每段的长度为 4px
    lv_obj_set_style_line_dash_width(Charging_Chart, 4, LV_PART_CURSOR);

    // 设置游标虚线每段之间的间隔为 4px
    lv_obj_set_style_line_dash_gap(Charging_Chart, 4, LV_PART_CURSOR);

    // 设置游标线端点不使用圆角
    lv_obj_set_style_line_rounded(Charging_Chart, false, LV_PART_CURSOR);

    lv_chart_cursor_t * cursor1 =
        lv_chart_add_cursor(Charging_Chart, lv_color_hex(0xFFFFFF), LV_DIR_HOR);
    lv_chart_cursor_t * cursor2 =
        lv_chart_add_cursor(Charging_Chart, lv_color_hex(0xFFFFFF), LV_DIR_HOR);

    lv_chart_set_cursor_pos_y(Charging_Chart, cursor1, 8);
    lv_chart_set_cursor_pos_y(Charging_Chart, cursor2, 75);
    // 添加一条数据序列，颜色为绿色，使用左侧 Y 轴
    lv_chart_series_t * ser = lv_chart_add_series(
        Charging_Chart,
        lv_color_hex(0x22C55E),
        LV_CHART_AXIS_PRIMARY_Y
    );


    for(uint32_t i = 0; i < (Per_Minute_Data ? 30:15); i++)
    {
        lv_chart_set_series_value_by_id(Charging_Chart, ser, i, data[i]);
    }

    //y坐标参数解释文本
    lv_obj_t * chart_label_top = lv_label_create(Battery);
    lv_label_set_text_fmt(chart_label_top, "%d Kw", Charging_MAX_Kw);
    lv_obj_set_style_text_color(chart_label_top, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_pad_all(chart_label_top, 0, LV_PART_MAIN);
    lv_obj_align_to(chart_label_top, Charging_Chart,
                    LV_ALIGN_OUT_LEFT_TOP, 2, -5);
    lv_obj_set_style_text_font(chart_label_top, &lv_font_montserrat_12, LV_PART_MAIN);

    lv_obj_t * chart_label_mid = lv_label_create(Battery);
    lv_label_set_text_fmt(chart_label_mid, "%d Kw", (Charging_MAX_Kw/2));
    lv_obj_set_style_text_color(chart_label_mid, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_pad_all(chart_label_mid, 0, LV_PART_MAIN);
    lv_obj_align_to(chart_label_mid, Charging_Chart,
                    LV_ALIGN_OUT_LEFT_MID, 2, -9);
    lv_obj_set_style_text_font(chart_label_mid, &lv_font_montserrat_12, LV_PART_MAIN);

    lv_obj_t * chart_label_bottom = lv_label_create(Battery);
    lv_label_set_text(chart_label_bottom, "0 Kw");
    lv_obj_set_style_text_color(chart_label_bottom, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_pad_all(chart_label_bottom, 0, LV_PART_MAIN);
    lv_obj_align_to(chart_label_bottom, Charging_Chart,
                    LV_ALIGN_OUT_LEFT_BOTTOM, 2, -25);
    lv_obj_set_style_text_font(chart_label_bottom, &lv_font_montserrat_12, LV_PART_MAIN);
}
