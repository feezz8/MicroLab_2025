#include "5.3.headers.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 16000000UL
#include <util/delay.h>


int main() {
	twi_init();
	PCA9555_0_write(REG_CONFIGURATION_1, 0x00); //Set EXT_PORT0 as output
	lcd_init();
	
	char fezz1[] = "Konstantinos", fezz2[] = "Fezos", oar1[] = "Odysseas", oar2[] = "Tsouknidas";
	while (1) {
		lcd_clear_display();
		for (int i=0; i<12; i++){
			lcd_data(fezz1[i]);
		}
		lcd_change_line();
		for (int i=0; i<5; i++){
			lcd_data(fezz2[i]);
		}
		
		
		_delay_ms(50000);
		// Now time for the next name
		lcd_clear_display();
		for (int i=0; i<8; i++) {
			lcd_data(oar1[i]);
		}
		lcd_change_line();
		for (int i=0; i<10; i++){
			lcd_data(oar2[i]);
		}
		_delay_ms(50000);
		
	}
	
	return 0;
}
