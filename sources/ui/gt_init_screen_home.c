#include "gt_ui.h"


/** screen_home */
gt_obj_st * screen_home = NULL;
static gt_obj_st * txt1 = NULL;

gt_obj_st * gt_init_screen_home(void)
{
	screen_home = gt_obj_create(NULL);

	gt_screen_set_bgcolor(screen_home, gt_color_hex(0xFFFFFF));

	

	/** txt1 */
	/** txt1 */
	txt1 = gt_textarea_create(screen_home);
	gt_obj_set_pos(txt1, 143, 116);
	gt_obj_set_size(txt1, 206, 90);
	gt_textarea_set_font_color(txt1, gt_color_hex(0x000000));
	gt_textarea_set_font_size(txt1, 16);
	gt_textarea_set_font_family_cn(txt1, 80);
	gt_textarea_set_font_align(txt1, GT_ALIGN_CENTER_MID);
	gt_textarea_set_font_thick_en(txt1, 22);
	gt_textarea_set_font_thick_cn(txt1, 22);
	gt_textarea_set_font_gray(txt1, 4);
	gt_textarea_add_str(txt1, "≤‚ ‘Œƒ◊÷", 0, gt_color_hex(0x000000));
	gt_textarea_set_bg_color(txt1, gt_color_hex(0x00ffff));
	gt_textarea_set_bg_opa(txt1, GT_OPA_45);


	return screen_home;
}

