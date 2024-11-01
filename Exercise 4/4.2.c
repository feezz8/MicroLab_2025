#define F_CPU 16000000UL
#include "avr/io.h"
#include <util/delay.h>

uint8_t adc_low_val= 0;
void adc_init(void) {
	// Initialize ADC
	// Set reference voltage to AVcc (with external capacitor at AREF pin)
    // Set ADC1
	ADMUX = (1 << REFS0) | (1<<MUX0);
   
	// Set ADC pre-scaler to 128 (for 16 MHz clock, ADC clock = 16MHz /128 = 125 kHz - within the range)
    // Set ADC enable
	ADCSRA = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0) | (1<<ADEN);
}

uint16_t adc_read(void) {
	// Set ADSC flag of ADSCRA
	ADCSRA |= (1 << ADSC);
	
	// Wait for ADSC flag to become '0'
	while (ADCSRA & (1 << ADSC));
    
    //Return 10-bit result
    
    return ADC;
}

void write_2_nibbles (uint8_t input) {
    unsigned char pind = PIND, temp = input & 0xF0;
    pind = pind & 0x0F;
    PORTD = pind + temp;
    
    PORTD |= (1<<PD3);
    asm("nop");
    asm("nop");
    PORTD &= ~(1 << PD3); 
    
    temp = input & 0x0F;
    temp = temp << 4;
    PORTD = pind + temp;
    
    PORTD |= (1<<PD3);
    asm("nop");
    asm("nop");
    PORTD &= ~(1 << PD3);
    
    return;
}

void lcd_data (uint8_t input) {
    PORTD |= (1<<PD2);
    write_2_nibbles(input);
    _delay_ms(1);
    return;
}

void lcd_command (uint8_t input) {
    PORTD &= ~(1 << PD2);
    write_2_nibbles(input);
    _delay_ms(1);
    return;
}

void lcd_clear_display () {
    lcd_command(0x01);
    _delay_ms(5);
    return;
}

void lcd_init () {
    _delay_ms(200);
    PORTD = 0x30;
    for (int i=0; i<3; i++) {
        PORTD |= (1<<PD3);
        asm("nop");
        asm("nop");
        PORTD &= ~(1 << PD3);
        _delay_ms(1);
    }
    
    PORTD = 0x20;
    PORTD |= (1<<PD3);
    asm("nop");
    asm("nop");
    PORTD &= ~(1 << PD3);
    _delay_ms(1);
    lcd_command(0x28);
    lcd_command(0x0c);
    lcd_clear_display();
    lcd_command(0x06);
    return;
}

int main() {
    // PORTD as output (to send data and commands to LCD)
    DDRD = 0xFF;
    //PORTC as input (for the correct operation of ADC)
    DDRC = 0x00;
    
    adc_init();
    lcd_init();
    _delay_ms(100);
    

    
    while(1) {
        _delay_ms(1000);
        uint16_t adc_value =0;
        uint8_t first_digit=0, second_digit=0, third_digit =0;
        //int dummy =0;
        lcd_clear_display();
        // Take ADC measurement
        adc_value = (adc_read()*5);
        
        // Find first digit
        first_digit = (adc_value / 1024);
        
        // Find fist digit after the decimal point (aka second digit)
        adc_value = adc_value % 1024;
        adc_value = adc_value*10;
        second_digit =  adc_value / 1024;
        
        // Find second digit after the decimal point (aka third digit)
        adc_value = adc_value % 1024;
        adc_value = adc_value*10;
        third_digit = adc_value / 1024;
        
        // Print in LCD screen
        lcd_data(first_digit + '0');
        lcd_data('.');
        lcd_data(second_digit+ '0');
        lcd_data(third_digit + '0');    
    }
    
    return 0;
}
