#ifndef LCD_HEADER
#define	LCD_HEADER

#include <stdint.h>

void write_2_nibbles (uint8_t input);
void lcd_data (uint8_t input);
void lcd_command (uint8_t input);
void lcd_clear_display ();
void lcd_change_line ();
void lcd_init ();

#endif	/* LCD_HEADER */