#include "Charging_Records.h"
#include "ui.h"

// 记录页面卡片的背景色（深灰）。
#define RECORD_CARD_COLOR   0x181D23
// 次要文字颜色，用于说明文字、单位和图例标题。
#define RECORD_MUTED_COLOR  0x9AA3AD
// 进度条底轨以及环形图未填充部分的颜色。
#define RECORD_TRACK_COLOR  0x2A3138
// 会话来源环形图中 Automation 对应的蓝色。
#define RECORD_BLUE_COLOR   0x6CB8FF
// 会话来源环形图中 Offline 对应的橙色。
#define RECORD_ORANGE_COLOR 0xF3A33C

// 创建统一样式的内容卡片，返回卡片对象指针。
// parent：卡片的父对象，决定卡片在界面中的归属关系。
static lv_obj_t * create_record_card(lv_obj_t * parent)
{
    // 创建基础容器，后续所有样式和子控件都挂载在该卡片上。
    lv_obj_t * card = lv_obj_create(parent);
    // 设置卡片主体背景及不透明度，确保底色完整显示。
    lv_obj_set_style_bg_color(card, lv_color_hex(RECORD_CARD_COLOR), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, LV_PART_MAIN);
    // 去除边框和阴影，圆角与内边距负责卡片的视觉层次及内容留白。
    lv_obj_set_style_border_width(card, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(card, 11, LV_PART_MAIN);
    lv_obj_set_style_pad_all(card, 10, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(card, 0, LV_PART_MAIN);
    // 卡片不需要滚动，关闭滚动条并禁止滚动操作。
    lv_obj_set_scrollbar_mode(card, LV_SCROLLBAR_MODE_OFF);
    lv_obj_remove_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    return card;
}

// 创建透明的布局容器：只负责组织子控件，不绘制卡片背景。
static lv_obj_t * create_plain_container(lv_obj_t * parent)
{
    // 创建容器，并将背景设为透明。
    lv_obj_t * container = lv_obj_create(parent);
    lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, LV_PART_MAIN);
    // 清除边框、圆角、内边距和阴影，避免影响内部控件的布局尺寸。
    lv_obj_set_style_border_width(container, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(container, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(container, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(container, 0, LV_PART_MAIN);
    // 该容器仅用于排布控件，不启用滚动。
    lv_obj_set_scrollbar_mode(container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_remove_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    return container;
}

// 创建小型折线趋势图。
// points：折线数据数组；count：数据点数量；max_value：Y 轴最大值。
static void create_sparkline(lv_obj_t * parent, const int32_t * points,
                             uint32_t count, int32_t max_value)
{
    // 在父容器中创建图表，宽度占父容器的 42%，高度占满父容器。
    lv_obj_t * chart = lv_chart_create(parent);
    lv_obj_set_width(chart, LV_PCT(42));
    lv_obj_set_height(chart, LV_PCT(100));
    // 图表背景透明、无边框；数据线宽度为 2 px，线段端点使用圆角。
    lv_obj_set_style_bg_opa(chart, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(chart, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(chart, 2, LV_PART_MAIN);
    lv_obj_set_style_line_width(chart, 2, LV_PART_ITEMS);
    lv_obj_set_style_line_rounded(chart, true, LV_PART_ITEMS);
    lv_obj_set_style_width(chart, 2, LV_PART_INDICATOR);
    lv_obj_set_style_height(chart, 2, LV_PART_INDICATOR);
    lv_obj_set_style_border_width(chart, 0, LV_PART_INDICATOR);

    // 设置为折线图，并隐藏横向、纵向网格分割线。
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
    lv_chart_set_div_line_count(chart, 0, 0);
    // 设置数据点数量及 Y 轴显示范围：从 0 到 max_value。
    lv_chart_set_point_count(chart, count);
    lv_chart_set_axis_range(chart, LV_CHART_AXIS_PRIMARY_Y, 0, max_value);

    // 添加一组使用 LINE_COLOR 颜色、关联主 Y 轴的数据序列。
    lv_chart_series_t * series =
        lv_chart_add_series(chart, lv_color_hex(LINE_COLOR), LV_CHART_AXIS_PRIMARY_Y);

    // 按索引逐个写入折线数据，最后刷新图表以更新显示。
    for(uint32_t i = 0; i < count; i++) {
        lv_chart_set_series_value_by_id(chart, series, i, points[i]);
    }
    lv_chart_refresh(chart);
}

// 用户使用界面：充电记录 / 能耗统计页面。
// 本函数负责创建页面中的所有控件及其布局。
void charging_information_create(void)
{
    // 会话数量趋势图的模拟数据，每个元素代表一个时间采样点。
    static const int32_t session_points[] = {
        18, 24, 21, 48, 27, 35, 17, 39, 48, 23, 24, 17
    };
    // 能耗趋势图的模拟数据，与上方会话趋势图使用相同的采样点数量。
    static const int32_t energy_points[] = {
        22, 29, 25, 57, 33, 45, 41, 21, 27, 49, 55, 46
    };
    // 站点名称、对应能耗值以及进度条显示比例，数组下标一一对应。
    static const char * station_names[] = {
        "PortFi Station 3", "PortFi Station 4", "PortFi Station 5"
    };
    static const uint16_t station_energy[] = {738, 54, 18}; // 各站点能耗数值（kWh）。
    static const uint8_t station_percent[] = {92, 7, 2}; // 各站点进度条填充宽度（百分比）。
    // 会话来源的颜色、名称和占比；三个数组通过相同下标关联。
    static const uint32_t source_colors[] = {
        RECORD_BLUE_COLOR, LINE_COLOR, RECORD_ORANGE_COLOR
    };
    static const char * source_names[] = {
        "Automation", "Id Tag User", "Offline"
    };
    static const char * source_percent[] = {"84.4%", "9.1%", "6.5%"}; // 各来源的会话占比文本。
    // 环形图各分段的起止角度，按来源顺序与 source_colors[] 对应。
    static const lv_value_precise_t arc_angles[][2] = {
        {0, 303.8f}, {303.8f, 336.6f}, {336.6f, 360}
    };

    // 创建页面根容器，尺寸铺满 User_Screen。
    lv_obj_t * records = lv_obj_create(User_Screen);
    lv_obj_set_size(records, LV_PCT(100), LV_PCT(100));
    // 应用项目统一的基础 UI 样式，再单独设置本页面的背景和布局参数。
    General_UI_Properties(records);
    lv_obj_set_style_bg_color(records, lv_color_hex(BACKGROUND_COLOR), LV_PART_MAIN);
    // 根容器使用纵向 Flex 布局：子区域从上到下排列，区域间距为 10 px。
    lv_obj_set_style_pad_all(records, 12, LV_PART_MAIN);
    lv_obj_set_style_pad_row(records, 10, LV_PART_MAIN);
    lv_obj_set_flex_flow(records, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(records, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_START);
    lv_obj_remove_flag(records, LV_OBJ_FLAG_SCROLLABLE);

    /* 顶部区域：左侧页面标题，右侧时间范围与导出按钮。 */
    // 顶部标题栏横向排布，SPACE_BETWEEN 将标题和右侧工具推向两端。
    lv_obj_t * header = create_plain_container(records);
    lv_obj_set_size(header, LV_PCT(100), LV_PCT(10));
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // 页面标题文字。
    lv_obj_t * title = lv_label_create(header);
    lv_label_set_text(title, "Energy History");
    lv_obj_set_style_text_color(title, lv_color_hex(LINE_COLOR), LV_PART_MAIN);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, LV_PART_MAIN);

    /* 第一部分：顶部统计指标卡，左右并排显示会话数和总能耗。 */
    // 统计卡片的横向父容器，两张卡片各自通过 flex_grow 平分可用宽度。
    lv_obj_t * stats = create_plain_container(records);
    lv_obj_set_size(stats, LV_PCT(100), LV_PCT(25));
    lv_obj_set_flex_flow(stats, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(stats, 10, LV_PART_MAIN);

    // 左侧统计卡：显示会话总数、对应趋势折线和活跃站点说明。
    lv_obj_t * sessions_card = create_record_card(stats);
    lv_obj_set_width(sessions_card, 0);
    lv_obj_set_flex_grow(sessions_card, 1);
    lv_obj_set_height(sessions_card, LV_PCT(100));
    lv_obj_set_flex_flow(sessions_card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(sessions_card, 4, LV_PART_MAIN);

    // 卡片上半行横向排布：左侧数值区，右侧迷你趋势图。
    lv_obj_t * sessions_row = create_plain_container(sessions_card);
    lv_obj_set_size(sessions_row, LV_PCT(100), 52);
    lv_obj_set_flex_flow(sessions_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(sessions_row, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // 会话数值区占据行内剩余空间，内部以纵向方式放置名称和数值。
    lv_obj_t * sessions_values = create_plain_container(sessions_row);
    lv_obj_set_width(sessions_values, 0);
    lv_obj_set_flex_grow(sessions_values, 1);
    lv_obj_set_height(sessions_values, LV_PCT(100));
    lv_obj_set_flex_flow(sessions_values, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(sessions_values, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    // 指标名称和统计值分别使用较小、较大的字体显示。
    lv_obj_t * sessions_label = lv_label_create(sessions_values);
    lv_label_set_text(sessions_label, "Sessions");
    lv_obj_set_style_text_color(sessions_label, lv_color_hex(RECORD_MUTED_COLOR),
                                LV_PART_MAIN);
    lv_obj_set_style_text_font(sessions_label, &lv_font_montserrat_12, LV_PART_MAIN);

    lv_obj_t * sessions_value = lv_label_create(sessions_values);
    lv_label_set_text(sessions_value, "77");
    lv_obj_set_style_text_color(sessions_value, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(sessions_value, &lv_font_montserrat_28, LV_PART_MAIN);

    // 将会话趋势数据绘制成迷你折线图，Y 轴最大值设为 60。
    create_sparkline(sessions_row, session_points,
                     sizeof(session_points) / sizeof(session_points[0]), 60);

    // 会话卡片底部的辅助说明。
    lv_obj_t * sessions_footer = lv_label_create(sessions_card);
    lv_label_set_text(sessions_footer, "At 3 active stations  " LV_SYMBOL_RIGHT);
    lv_obj_set_style_text_color(sessions_footer, lv_color_hex(RECORD_MUTED_COLOR),
                                LV_PART_MAIN);
    lv_obj_set_style_text_font(sessions_footer, &lv_font_montserrat_12,
                               LV_PART_MAIN);

    // 右侧统计卡：显示总能耗、能耗趋势以及单次会话能耗说明。
    lv_obj_t * energy_card = create_record_card(stats);
    lv_obj_set_width(energy_card, 0);
    lv_obj_set_flex_grow(energy_card, 1);
    lv_obj_set_height(energy_card, LV_PCT(100));
    lv_obj_set_flex_flow(energy_card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(energy_card, 4, LV_PART_MAIN);

    // 能耗卡片上半行，同样采用“左侧数值 + 右侧折线图”的布局。
    lv_obj_t * energy_row = create_plain_container(energy_card);
    lv_obj_set_size(energy_row, LV_PCT(100), 52);
    lv_obj_set_flex_flow(energy_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(energy_row, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // 能耗文字区域占据行内剩余宽度，名称与数值纵向排列。
    lv_obj_t * energy_values = create_plain_container(energy_row);
    lv_obj_set_width(energy_values, 0);
    lv_obj_set_flex_grow(energy_values, 1);
    lv_obj_set_height(energy_values, LV_PCT(100));
    lv_obj_set_flex_flow(energy_values, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(energy_values, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    // 总能耗指标名称及数值。
    lv_obj_t * energy_label = lv_label_create(energy_values);
    lv_label_set_text(energy_label, "Total Energy (kWh)");
    lv_obj_set_style_text_color(energy_label, lv_color_hex(RECORD_MUTED_COLOR),
                                LV_PART_MAIN);
    lv_obj_set_style_text_font(energy_label, &lv_font_montserrat_12, LV_PART_MAIN);

    lv_obj_t * energy_value = lv_label_create(energy_values);
    lv_label_set_text(energy_value, "641");
    lv_obj_set_style_text_color(energy_value, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(energy_value, &lv_font_montserrat_28, LV_PART_MAIN);

    // 绘制总能耗趋势折线图，Y 轴最大值设为 80。
    create_sparkline(energy_row, energy_points,
                     sizeof(energy_points) / sizeof(energy_points[0]), 80);

    // 显示单次会话能耗的最小值、平均值和最大值。
    lv_obj_t * energy_footer = lv_label_create(energy_card);
    lv_label_set_text(energy_footer,
                      "Per session: min 0  avg 8  max 42 kWh");
    lv_obj_set_style_text_color(energy_footer, lv_color_hex(RECORD_MUTED_COLOR),
                                LV_PART_MAIN);
    lv_obj_set_style_text_font(energy_footer, &lv_font_montserrat_12,
                               LV_PART_MAIN);

    /* 第二部分：底部左右区域，左侧为站点利用率，右侧为会话来源统计。 */
    // 底部区域横向排布；子卡片宽度按 flex_grow 的 60:40 比例分配。
    lv_obj_t * bottom = create_plain_container(records);
    lv_obj_set_width(bottom, LV_PCT(100));
    lv_obj_set_flex_grow(bottom, 1);
    lv_obj_set_flex_flow(bottom, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(bottom, 10, LV_PART_MAIN);

    // 左侧站点利用率卡片，占底部区域约 60% 的可分配宽度。
    lv_obj_t * stations_card = create_record_card(bottom);
    lv_obj_set_width(stations_card, 0);
    lv_obj_set_flex_grow(stations_card, 30);
    lv_obj_set_height(stations_card, LV_PCT(100));
    lv_obj_set_flex_flow(stations_card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(stations_card, 8, LV_PART_MAIN);

    // 站点卡片标题栏：左侧标题，右侧显示能耗单位 kWh。
    lv_obj_t * stations_header = create_plain_container(stations_card);
    lv_obj_set_size(stations_header, LV_PCT(100), 24);
    lv_obj_set_flex_flow(stations_header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(stations_header, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // 站点利用率区域的标题和单位。
    lv_obj_t * stations_title = lv_label_create(stations_header);
    lv_label_set_text(stations_title, "Most Utilized Stations");
    lv_obj_set_style_text_color(stations_title, lv_color_hex(0xFFFFFF),
                                LV_PART_MAIN);
    lv_obj_set_style_text_font(stations_title, &lv_font_montserrat_14,
                               LV_PART_MAIN);

    lv_obj_t * stations_unit = lv_label_create(stations_header);
    lv_label_set_text(stations_unit, "kWh");
    lv_obj_set_style_text_color(stations_unit, lv_color_hex(RECORD_MUTED_COLOR),
                                LV_PART_MAIN);
    lv_obj_set_style_text_font(stations_unit, &lv_font_montserrat_12,
                               LV_PART_MAIN);

    // 站点列表容器，纵向排列下面循环创建的三行站点数据。
    lv_obj_t * station_rows = create_plain_container(stations_card);
    lv_obj_set_width(station_rows, LV_PCT(100));
    lv_obj_set_flex_grow(station_rows, 1);
    lv_obj_set_flex_flow(station_rows, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(station_rows, 12, LV_PART_MAIN);

    // 依次创建三个站点行；同一索引对应名称、进度比例和能耗数值。
    for(uint32_t i = 0; i < 3; i++) {
        lv_obj_t * row = create_plain_container(station_rows);
        lv_obj_set_size(row, LV_PCT(100), 28);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START,
                              LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(row, 8, LV_PART_MAIN);

        // 站点名称固定分配 118 px，过长时以省略号显示。
        lv_obj_t * name = lv_label_create(row);
        lv_obj_set_width(name, 118);
        lv_label_set_long_mode(name, LV_LABEL_LONG_DOT);
        lv_label_set_text(name, station_names[i]);
        lv_obj_set_style_text_color(name, lv_color_hex(0xD8DEE5), LV_PART_MAIN);
        lv_obj_set_style_text_font(name, &lv_font_montserrat_12, LV_PART_MAIN);

        // 进度条底轨：占据名称和数值之间的剩余宽度。
        lv_obj_t * track = lv_obj_create(row);
        lv_obj_set_height(track, 12);
        lv_obj_set_width(track, 0);
        lv_obj_set_flex_grow(track, 1);
        lv_obj_set_style_bg_color(track, lv_color_hex(RECORD_TRACK_COLOR),
                                  LV_PART_MAIN);
        lv_obj_set_style_border_width(track, 0, LV_PART_MAIN);
        lv_obj_set_style_radius(track, 6, LV_PART_MAIN);
        lv_obj_set_style_pad_all(track, 0, LV_PART_MAIN);
        lv_obj_remove_flag(track, LV_OBJ_FLAG_SCROLLABLE);

        // 进度填充块：宽度按 station_percent[i] 设置，高度铺满底轨。
        lv_obj_t * bar = lv_obj_create(track);
        lv_obj_set_size(bar, LV_PCT(station_percent[i]), LV_PCT(100));
        lv_obj_set_pos(bar, 0, 0);
        lv_obj_set_style_bg_color(bar, lv_color_hex(LINE_COLOR), LV_PART_MAIN);
        lv_obj_set_style_border_width(bar, 0, LV_PART_MAIN);
        lv_obj_set_style_radius(bar, 6, LV_PART_MAIN);
        lv_obj_set_style_pad_all(bar, 0, LV_PART_MAIN);
        lv_obj_remove_flag(bar, LV_OBJ_FLAG_SCROLLABLE);

        // 进度条右侧显示该站点的实际能耗数值，并按右侧对齐。
        lv_obj_t * value = lv_label_create(row);
        lv_obj_set_width(value, 38);
        lv_obj_set_style_text_align(value, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
        lv_label_set_text_fmt(value, "%u", (unsigned)station_energy[i]);
        lv_obj_set_style_text_color(value, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_text_font(value, &lv_font_montserrat_12, LV_PART_MAIN);
    }

    // 右侧会话来源卡片，占底部区域约 40% 的可分配宽度。
    lv_obj_t * sources_card = create_record_card(bottom);
    lv_obj_set_width(sources_card, 0);
    lv_obj_set_flex_grow(sources_card, 40);
    lv_obj_set_height(sources_card, LV_PCT(100));
    lv_obj_set_flex_flow(sources_card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(sources_card, 6, LV_PART_MAIN);

    // 会话来源卡片标题栏，右侧放置圆形信息提示图标。
    lv_obj_t * sources_header = create_plain_container(sources_card);
    lv_obj_set_size(sources_header, LV_PCT(100), 24);
    lv_obj_set_flex_flow(sources_header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(sources_header, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // 标题说明环形图统计的是会话由什么来源发起。
    lv_obj_t * sources_title = lv_label_create(sources_header);
    lv_label_set_text(sources_title, "Sessions started by:");
    lv_obj_set_style_text_color(sources_title, lv_color_hex(0xFFFFFF),
                                LV_PART_MAIN);
    lv_obj_set_style_text_font(sources_title, &lv_font_montserrat_14,
                               LV_PART_MAIN);

    // 创建圆形信息图标背景。
    lv_obj_t * info = lv_obj_create(sources_header);
    lv_obj_set_size(info, 16, 16);
    lv_obj_set_style_bg_color(info, lv_color_hex(RECORD_TRACK_COLOR), LV_PART_MAIN);
    lv_obj_set_style_border_width(info, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(info, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_all(info, 0, LV_PART_MAIN);
    lv_obj_remove_flag(info, LV_OBJ_FLAG_SCROLLABLE);

    // 在信息图标中心显示字母 i。
    lv_obj_t * info_text = lv_label_create(info);
    lv_label_set_text(info_text, "i");
    lv_obj_set_style_text_color(info_text, lv_color_hex(RECORD_MUTED_COLOR),
                                LV_PART_MAIN);
    lv_obj_set_style_text_font(info_text, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_center(info_text);

    // 内容主体横向布局：左侧环形统计图，右侧颜色图例及占比。
    lv_obj_t * sources_body = create_plain_container(sources_card);
    lv_obj_set_width(sources_body, LV_PCT(100));
    lv_obj_set_flex_grow(sources_body, 1);
    lv_obj_set_flex_flow(sources_body, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(sources_body, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // 环形图区域固定为 132 × 132 px，为 120 × 120 px 的圆弧留出边距。
    lv_obj_t * donut = create_plain_container(sources_body);
    lv_obj_set_size(donut, 132, 132);

    // 先绘制完整的环形底轨，作为所有彩色数据分段的背景。
    lv_obj_t * donut_track = lv_arc_create(donut);
    lv_obj_set_size(donut_track, 120, 120);
    lv_obj_align(donut_track, LV_ALIGN_CENTER, 0, 0);
    // 将圆弧起点旋转到顶部（270°），背景弧覆盖完整 360°。
    lv_arc_set_rotation(donut_track, 270);
    lv_arc_set_bg_angles(donut_track, 0, 360);
    // 设置底轨粗细和颜色；隐藏 indicator 部分，只保留主体背景圆弧。
    lv_obj_set_style_arc_width(donut_track, 14, LV_PART_MAIN);
    lv_obj_set_style_arc_color(donut_track, lv_color_hex(RECORD_TRACK_COLOR),
                               LV_PART_MAIN);
    lv_obj_set_style_arc_opa(donut_track, LV_OPA_TRANSP, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(donut_track, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_set_style_border_width(donut_track, 0, LV_PART_KNOB);
    lv_obj_set_style_border_opa(donut_track, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_set_style_outline_width(donut_track, 0, LV_PART_KNOB);
    lv_obj_set_style_outline_opa(donut_track, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_set_style_shadow_width(donut_track, 0, LV_PART_KNOB);
    lv_obj_set_style_shadow_opa(donut_track, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_remove_flag(donut_track, LV_OBJ_FLAG_CLICKABLE);

    // 按来源数量创建三段彩色圆弧，每段的起止角度来自 arc_angles[]。
    for(uint32_t i = 0; i < 3; i++) {
        lv_obj_t * arc = lv_arc_create(donut);
        lv_obj_set_size(arc, 120, 120);
        lv_obj_align(arc, LV_ALIGN_CENTER, 0, 0);
        lv_arc_set_rotation(arc, 270);
        lv_arc_set_bg_angles(arc, 0, 360);
        // 设置当前分段的起始角和结束角，形成对应占比的圆弧。
        lv_arc_set_angles(arc, arc_angles[i][0], arc_angles[i][1]);
        // 隐藏圆弧主体，只显示 indicator 数据弧；关闭旋钮背景、边框和阴影。
        lv_obj_set_style_arc_opa(arc, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_arc_width(arc, 14, LV_PART_INDICATOR);
        lv_obj_set_style_arc_color(arc, lv_color_hex(source_colors[i]),
                                   LV_PART_INDICATOR);
        // 取消彩色圆弧两端的圆形端头。
        lv_obj_set_style_arc_rounded(arc, false, LV_PART_INDICATOR);
        // 隐藏圆弧末端的拖动旋钮。
        lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
        lv_obj_set_style_bg_opa(arc, LV_OPA_TRANSP, LV_PART_KNOB);
        lv_obj_set_style_border_width(arc, 0, LV_PART_KNOB);
        lv_obj_set_style_border_opa(arc, LV_OPA_TRANSP, LV_PART_KNOB);
        lv_obj_set_style_outline_width(arc, 0, LV_PART_KNOB);
        lv_obj_set_style_outline_opa(arc, LV_OPA_TRANSP, LV_PART_KNOB);
        lv_obj_set_style_shadow_width(arc, 0, LV_PART_KNOB);
        lv_obj_set_style_shadow_opa(arc, LV_OPA_TRANSP, LV_PART_KNOB);
        lv_obj_remove_flag(arc, LV_OBJ_FLAG_CLICKABLE);
    }

    // 环形图中心覆盖一个透明容器，用于居中显示总会话数和单位。
    lv_obj_t * donut_center = create_plain_container(donut);
    lv_obj_set_size(donut_center, 76, 52);
    lv_obj_align(donut_center, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_flex_flow(donut_center, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(donut_center, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // 环形图中心的总会话数。
    lv_obj_t * donut_value = lv_label_create(donut_center);
    lv_label_set_text(donut_value, "77");
    lv_obj_set_style_text_color(donut_value, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(donut_value, &lv_font_montserrat_24, LV_PART_MAIN);

    // 总数下方的单位说明。
    lv_obj_t * donut_caption = lv_label_create(donut_center);
    lv_label_set_text(donut_caption, "sessions");
    lv_obj_set_style_text_color(donut_caption, lv_color_hex(RECORD_MUTED_COLOR),
                                LV_PART_MAIN);
    lv_obj_set_style_text_font(donut_caption, &lv_font_montserrat_12,
                               LV_PART_MAIN);

    // 右侧图例容器，纵向排列来源名称及其占比。
    lv_obj_t * legend = create_plain_container(sources_body);
    lv_obj_set_width(legend, 0);
    lv_obj_set_flex_grow(legend, 1);
    lv_obj_set_height(legend, LV_PCT(100));
    lv_obj_set_flex_flow(legend, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(legend, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(legend, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_left(legend, 8, LV_PART_MAIN);

    // 创建三条图例行，颜色点、来源名称和占比均使用相同数组索引。
    for(uint32_t i = 0; i < 3; i++) {
        lv_obj_t * legend_row = create_plain_container(legend);
        lv_obj_set_size(legend_row, LV_PCT(100), 18);
        lv_obj_set_flex_flow(legend_row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(legend_row, LV_FLEX_ALIGN_START,
                              LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(legend_row, 6, LV_PART_MAIN);

        // 小色点用于标识该来源，与环形图中对应分段的颜色一致。
        lv_obj_t * dot = lv_obj_create(legend_row);
        lv_obj_set_size(dot, 8, 8);
        lv_obj_set_style_bg_color(dot, lv_color_hex(source_colors[i]),
                                  LV_PART_MAIN);
        lv_obj_set_style_border_width(dot, 0, LV_PART_MAIN);
        lv_obj_set_style_radius(dot, 4, LV_PART_MAIN);
        lv_obj_set_style_pad_all(dot, 0, LV_PART_MAIN);
        lv_obj_remove_flag(dot, LV_OBJ_FLAG_SCROLLABLE);

        // 来源名称占据图例行的剩余宽度。
        lv_obj_t * source_name = lv_label_create(legend_row);
        lv_obj_set_width(source_name, 0);
        lv_obj_set_flex_grow(source_name, 1);
        lv_label_set_text(source_name, source_names[i]);
        lv_obj_set_style_text_color(source_name, lv_color_hex(0xD8DEE5),
                                    LV_PART_MAIN);
        lv_obj_set_style_text_font(source_name, &lv_font_montserrat_12,
                                   LV_PART_MAIN);

        // 图例最右侧显示对应来源的百分比。
        lv_obj_t * source_value = lv_label_create(legend_row);
        lv_label_set_text(source_value, source_percent[i]);
        lv_obj_set_style_text_color(source_value, lv_color_hex(0xFFFFFF),
                                    LV_PART_MAIN);
        lv_obj_set_style_text_font(source_value, &lv_font_montserrat_12,
                                   LV_PART_MAIN);
    }
}
