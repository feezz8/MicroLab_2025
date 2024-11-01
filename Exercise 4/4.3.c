#define F_CPU 16000000UL
#include "avr/io.h"
#include <util/delay.h>
#include <stdbool.h>


void adc_init(void) {
	// Initialize ADC
	// Set reference voltage to AVcc (with external capacitor at AREF pin)
    // Set ADC2
	ADMUX = (1 << REFS0) | (1<<MUX1);
   
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
    //PORTB as output for the LEDs
    DDRB = 0xFF;
    
    adc_init();
    lcd_init();
    
    _delay_ms(100);
    
    uint16_t adc_value =0;
    char clr[] = "CLEAR";
    char gas[] = "GAS DETECTED";
    bool already_on= 0, flag=0;
    int counter =0;
    
    while(1){
        _delay_ms(100);
        adc_value = adc_read();
        // 205 is the value for Vgas that gives density 70 ppm
        if(adc_value > 205) {  // density > 70 ppm
            flag=1;
            int dummy = adc_value / 16;
            if (already_on--) { // Turn LEDs off
                PORTB = 0x00;
                
            }
            else {              // Turn LEDs on
                PORTB = dummy;
                already_on++;
            }
            lcd_clear_display();
            for (int i=0; i<12; i++) {
                //int distance = gas[i] - 'A';
                lcd_data(gas[i]);
            }
            
        }
        else {  // density <= 70 ppm
            int dummy = adc_value / 16;
            PORTB = dummy;
            if(flag) {
                flag =0;
                lcd_clear_display();
                for (int i=0; i<5; i++) {
                    //int distance = clr[i] - 'A';
                    lcd_data(clr[i]);
                }
                counter++;
            }
            if(counter <= 10) {
                counter++;
            }
            else{
                lcd_clear_display();
                counter =0;
            }
        }
    }
    return 0;
}
