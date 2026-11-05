#include "User_Interface_Selection.h"
#include "ui.h"

//用户选择界面

void user_selection_btn_callback(lv_event_t *e)
{
    // 获取事件目标对象和当前选择的索引
    lv_obj_t *target = lv_event_get_target(e);
    user_chooser = lv_obj_get_index(target);

    // 获取当前选择的页面对象,并滚动到该页面
    lv_obj_t *page = lv_obj_get_child(User_Screen, user_chooser);
    lv_obj_scroll_to_x(User_Screen,lv_obj_get_x(page),LV_ANIM_ON);
}
void ui_create_user_selection(lv_obj_t * User_Selection_Screen,lv_obj_t * icon1)
{
    lv_obj_t * selection_container = lv_btn_create(User_Selection_Screen);
    lv_obj_set_flex_grow(selection_container, 1);
    lv_obj_set_height(selection_container, LV_PCT(100));
    lv_obj_set_style_bg_color(selection_container, lv_color_hex(Information_Panel), 0);
    //禁止滚动和显示滚动条
    lv_obj_remove_flag(selection_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(selection_container, LV_SCROLLBAR_MODE_OFF);
    // 删除圆角
    lv_obj_set_style_radius(selection_container, 0, LV_PART_MAIN);
    /* 删除边框 */
    lv_obj_set_style_border_width(selection_container, 0, LV_PART_MAIN);
    /* 删除按钮阴影 */
    lv_obj_set_style_shadow_width(selection_container, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(selection_container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_shadow_offset_y(selection_container, 0, LV_PART_MAIN);
    //删除内边距
    lv_obj_set_style_pad_all(selection_container, 0, LV_PART_MAIN);

    /* 将 icon1 移动到 selection_container 中 */
    lv_obj_set_parent(icon1, selection_container);

    /* 设置图标文本的位置*/
    lv_obj_center(icon1);
    //设置按钮点击事件回调函数
    lv_obj_add_event_cb(selection_container, user_selection_btn_callback, LV_EVENT_SHORT_CLICKED, NULL);
}

void ui_create_user_selection_container(void)//用户选择界面容器
{
        //创建用户选择界面
    User_Selection_Screen = lv_obj_create(main_container);
    lv_obj_set_flex_grow(User_Selection_Screen, User_Selection_Screen_Container_Height);
    lv_obj_set_size(User_Selection_Screen, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(User_Selection_Screen, lv_color_hex(Information_Panel), 0);
    // 删除圆角
    lv_obj_set_style_radius(User_Selection_Screen, 0, LV_PART_MAIN);
    /* 删除边框 */
    lv_obj_set_style_border_width(User_Selection_Screen, 0, LV_PART_MAIN);

    //设置布局为flex,主轴水平向左
    lv_obj_set_flex_flow(User_Selection_Screen, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(User_Selection_Screen, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    /* 清除父容器四周的内边距 */
    lv_obj_set_style_pad_all(User_Selection_Screen, 1, LV_PART_MAIN);

    /* 清除子元素之间的间距 */
    lv_obj_set_style_pad_row(User_Selection_Screen, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_column(User_Selection_Screen, 0, LV_PART_MAIN);
}
