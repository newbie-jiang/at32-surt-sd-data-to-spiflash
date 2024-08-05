#include "gt_ui.h"


/** screen_home */
gt_obj_st * screen_home = NULL;
static gt_obj_st * img1 = NULL;
static gt_obj_st * txt1 = NULL;

gt_obj_st * gt_init_screen_home(void)
{
	screen_home = gt_obj_create(NULL);

	gt_screen_set_bgcolor(screen_home, gt_color_hex(0xFFFFFF));

	

	/** img1 */
	img1 = gt_img_create(screen_home);
	gt_obj_set_pos(img1, 30, 153);
	gt_obj_set_size(img1, 128, 128);
	gt_img_set_src(img1, "f:img_128x128_GTpic6.png");

	

	/** txt1 */
	/** txt2 */
	txt1 = gt_textarea_create(screen_home);
	gt_obj_set_pos(txt1, 30, 37);
	gt_obj_set_size(txt1, 200, 90);
	gt_textarea_set_font_color(txt1, gt_color_hex(0x000000));
	gt_textarea_set_font_size(txt1, 16);
	gt_textarea_set_font_family_cn(txt1, 80);
	gt_textarea_set_font_align(txt1, GT_ALIGN_CENTER_MID);
	gt_textarea_set_font_thick_en(txt1, 22);
	gt_textarea_set_font_thick_cn(txt1, 22);
	gt_textarea_set_font_gray(txt1, 4);
	gt_textarea_add_str(txt1, "测试", 0, gt_color_hex(0x000000));
	gt_textarea_set_bg_color(txt1, gt_color_hex(0xffffff));


	return screen_home;
}

