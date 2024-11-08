#define F_CPU 16000000UL
#include "5.3.headers.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>

int main() {
	twi_init();
	PCA9555_0_write(REG_CONFIGURATION_1, 0x00); //Set EXT_PORT0 as output
	lcd_init();
	
	char fezz1[] = "Konstantinos", fezz2[] = "Fezos", oar1[] = "Odysseas", oar2[] = "Tsouknidas";
	while (1) {
		lcd_clear_display();
		for (int i=0; i<strlen(fezz1); i++){
			lcd_data(fezz1[i]);
		}
		lcd_change_line();
		for (int i=0; i<strlen(fezz2); i++){
			lcd_data(fezz2[i]);
		}
		
		
		_delay_ms(3000);
		// Now time for the next name
		lcd_clear_display();
		for (int i=0; i<strlen(oar1); i++) {
			lcd_data(oar1[i]);
		}
		lcd_change_line();
		for (int i=0; i<strlen(oar2); i++){
			lcd_data(oar2[i]);
		}
		_delay_ms(3000);
		
	}
	
	return 0;
}
