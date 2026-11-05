#include "User_Operation_Screen.h"
#include "ui.h"
//用户使用界面

//用户使用界面容器
void user_operation_screen_container(void)
{
 // 创建用户界面
    User_Screen  = lv_obj_create(main_container);
    lv_obj_set_flex_grow(User_Screen, User_Operation_Screen_Container_Height);
    lv_obj_set_size(User_Screen, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(User_Screen, lv_color_hex(0x0B0F14), 0);
    General_UI_Properties(User_Screen);//常规UI属性设置（禁止滚动显示，去圆角、边框、内边距）
    lv_obj_set_scroll_snap_x(User_Screen, LV_SCROLL_SNAP_CENTER);//设置滚动吸附对齐方式为居中
    //设置布局为flex,主轴水平向左
    lv_obj_set_flex_flow(User_Screen, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(User_Screen, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    /* 完全透明 */
    lv_obj_set_style_bg_opa(User_Screen, LV_OPA_TRANSP, LV_PART_MAIN);
    //禁止滚动
    lv_obj_set_scroll_dir(User_Screen, LV_DIR_HOR);
}
