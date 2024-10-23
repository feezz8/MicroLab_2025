#define F_CPU 16000000UL
#include "avr/io.h"
#include <util/delay.h>
#include <stdbool.h>

void adc_init(void) {
	// Initialize ADC
	// Set reference voltage to AVcc (with external capacitor at AREF pin)
	ADMUX = (1 << REFS0);
   
	// Set ADC pre-scaler to 128 (for 16 MHz clock, ADC clock = 16MHz /128 = 125 kHz - within the range)
	ADCSRA = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	
	// Enable the ADC
	ADCSRA |= (1 << ADEN);
}

uint16_t adc_read(void) {
	// Set ADSC flag of ADSCRA
	ADCSRA |= (1 << ADSC);
	
	// Wait for ADSC flag to become '0'
	while (ADCSRA & (1 << ADSC));
    
    
	/* if we want to be 1000% certain use the following code
    uint16_t result = 0x0000, dummy = 0x0000;
    result = result | ADCL;
    dummy = dummy | ADCH;
    dummy = dummy << 8;
    result = result | dummy;
	return result;
     */ 
    
    //Return 10-bit result
    return ADC;
}

int main(){	
	int DC_VALUES[14];		//Create table for dc_values

	for (int i = 0; i < 13; i++){		//set up the DC_VALUE table
		DC_VALUES[i] = (8 + i*20);
	}
    adc_init();			//Initialize ADC


	// set fast non-inverted PWM 8 bit and pre-scale  = 1
	TCCR1A = (1 << WGM10) | (1 << COM1A1);
	TCCR1B = (1 << WGM12) | (1<<CS10);
	
	DDRB = 0xFF;        //PORTB as output
    DDRC = 0x00;        //PORTC is left as input
	DDRD = 0x00;		//PORTD as input				
	int index = 7;			//Duty Cycle =  50%
    OCR1AL = DC_VALUES[index];
    unsigned char mode1 = 1;
    unsigned char mode2 = 1;
    bool flag = true;

	while(1){		
        // Set the desired mode
        mode1 = PIND & 0x40;
        mode2 = PIND & 0x80;
        if (mode1 ==0) {
            flag = true;
            OCR1AL = DC_VALUES[index];
        }
        if (mode2 ==0) {
            flag = false;            
        }
        if(flag) {
            _delay_ms(100);
            unsigned char pd1 = PIND & 0x02, pd2 = PIND & 0x04, check = PIND;
        
            //wait for the button to be un-pressed
            while (!(check & (1 << PD1)) | !(check & (1<<PD2))) {
                check = PIND;
            }

            
            if((pd1 ==0) && index < 12){ // with PD1 we increase
                index++;
                OCR1AL = DC_VALUES[index];
            }
            if((pd2 ==0) && index > 0){ // with PD2 we decrease
                index--;
                OCR1AL = DC_VALUES[index];
            }
        }
        else {
            OCR1AL = adc_read() >> 2; 
        }
    }
}
