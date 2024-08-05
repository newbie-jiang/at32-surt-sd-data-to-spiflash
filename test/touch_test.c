#include "test.h"

uint16_t x_dot[5], y_dot[5];


void lcd_touch_init(void)
{	
  lcd_init(LCD_DISPLAY_VERTICAL);
	/* initialize touch */
  touch_init(LCD_DISPLAY_VERTICAL);
}


void touch_test_while(void)
{
	
	lcd_string_show(10, 60, 200, 24, 24, (uint8_t *)"X:");

    lcd_string_show(10, 90, 200, 24, 24, (uint8_t *)"Y:");

    /* read coordinate */
    if(touch_read_xy(x_dot, y_dot) == SUCCESS)
    {
      /* display x coordinate */
      lcd_num_show(40, 60, 200, 24, 24, x_dot[0], 3);

      /* display y coordinate */
      lcd_num_show(40, 90, 200, 24, 24, y_dot[0], 3);
    }	


}