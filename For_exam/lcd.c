#define F_CPU 16000000UL
#include "lcd.h"
#include "avr/io.h"
#include <util/delay.h>
#include <stdbool.h>
#include <stdint.h>
#include <avr/interrupt.h>

//WE MUST SET PORTD AS OUTPUT

void write_2_nibbles (uint8_t input) {
    //In high nibble of temp the high nibble of our input
    //In low nibble of pind the low nibble of previous PIND
    unsigned char pind = PIND, temp = input & 0xF0;
    pind = pind & 0x0F;
    PORTD = pind + temp;
    
    PORTD |= (1<<PD3);      //Here we raise E to 1
    asm("nop");
    asm("nop");
    PORTD &= ~(1 << PD3);   //Here we drop E to 0 to close the pulse 
    
    //In high nibble of temp the low nibble of our input
    //In low nibble of pind the low nibble of previous PIND
    temp = input & 0x0F;
    temp = temp << 4;
    PORTD = pind + temp;
    
    PORTD |= (1<<PD3);      //Here we raise E to 1
    asm("nop");
    asm("nop");
    PORTD &= ~(1 << PD3);   //Here we drop E to 0 to close the pulse 
    
    return;
}

void lcd_data (uint8_t input) {
    PORTD |= (1<<PD2);      //RS = 1
    write_2_nibbles(input);
    _delay_ms(1);
    return;
}

void lcd_command (uint8_t input) {
    PORTD &= ~(1 << PD2);   //RS = 0
    write_2_nibbles(input);
    _delay_ms(1);
    return;
}

void lcd_clear_display () {
    lcd_command(0x01);
    _delay_ms(5);
    return;
}

void lcd_change_line () {
	//I send command 0xC0 so I write in DDRAM Address the 100 0000
	lcd_command(0xC0);
	_delay_ms(5);
	return;
}

void lcd_init () {
    _delay_ms(200);
    PORTD = 0x30;
    for (int i=0; i<3; i++) {
        PORTD |= (1<<PD3);      //Here we raise E to 1
        asm("nop");
        asm("nop");
        PORTD &= ~(1 << PD3);   //Here we drop E to 0 to close the pulse
        _delay_ms(1);
    }
    
    PORTD = 0x20;
    PORTD |= (1<<PD3);          //Here we raise E to 1
    asm("nop");
    asm("nop");
    PORTD &= ~(1 << PD3);       //Here we drop E to 0 to close the pulse
    _delay_ms(1);
    lcd_command(0x28);
    lcd_command(0x0c);
    lcd_clear_display();
    lcd_command(0x06);
    return;
}
