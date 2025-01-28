#define F_CPU 16000000UL
#include "adc.h"
#include "avr/io.h"
#include <util/delay.h>
#include <stdbool.h>
#include <stdint.h>
#include <avr/interrupt.h>



//WE NEED PORTC DEFINED AS INPUT AND ALSO TURNED OFF FROM DIP SWITCHES
void adc_init(int mux_value, bool left_adj, bool interrupts) {
	// Initialize ADC
	// Set reference voltage to AVcc (with external capacitor at AREF pin)
	ADMUX = (1 << REFS0);
    
    if(left_adj){
        // Set ADC as left-adjusted
        ADMUX |= (1<<ADLAR);
    }
    
    switch (mux_value) {
        case 0:
            break;
        case 1:
            ADMUX |= (1 << MUX0);  
            break;
        case 2:
            ADMUX |= (1 << MUX1);
            break;
        case 3:
            ADMUX |= (1 << MUX0) | (1 << MUX1);
            break;
        case 4:
            ADMUX |= (1 << MUX2);
            break;
        default:
            break;
    }
	
	// Set ADC pre-scaler to 128 (for 16 MHz clock, ADC clock = 16MHz /128 = 125 kHz - within the range of 50-200 kHz)
	ADCSRA = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	
	// Enable the ADC
	ADCSRA |= (1 << ADEN);
    
    if (interrupts) {
        //Set ADC Interrupt Enable
        ADCSRA |= (1 << ADIE);
        //Enable Global Interrupts
        sei();
    }
}

uint16_t adc_right_read(void) {
	// Set ADSC flag of ADSCRA
	ADCSRA |= (1 << ADSC);
	
	// Wait for ADSC flag to become '0'
    while (ADCSRA & (1 << ADSC));
    
    //Return 10-bit result
    return ADC;
}

uint8_t adc_left_read(void) {
	// Set ADSC flag of ADSCRA
	ADCSRA |= (1 << ADSC);
	
	// Wait for ADSC flag to become '0'
    while (ADCSRA & (1 << ADSC));
    
    //Return 8-bit result
    return ADCH;
}
