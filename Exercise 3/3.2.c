#define F_CPU 16000000UL
#include "avr/io.h"
#include <util/delay.h>

void adc_init(void) {
	// Initialize ADC
	// Set reference voltage to AVcc (with external capacitor at AREF pin)
	ADMUX = (1 << REFS0);
    
	// Set input channel to ADC1 (=0001)
	ADMUX |= (1 << MUX0);
	
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
    
    //
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
	
	adc_init();			//Initialize ADC
	
	int DC_VALUES[13];		//Create table for dc_values
	
	for (int i = 0; i < 13; i++){		//set up the DC_VALUE table
		DC_VALUES[i] = (8 + i*20);
	}

	//	set fast non-inverted PWM 8 bit and pre-scale  = 1
	TCCR1A = (1 << WGM10) | (1 << COM1A1);
	TCCR1B = (1 << WGM12) | (1 << CS10);
	
    DDRB = 0xFF;       //PB5-0 as output
    DDRC= 0x00;              //PORTC as input
    DDRD = 0b00011111;		//PD4-PD0 output the rest is input			
    int index = 7;			//Duty Cycle =  50%
    OCR1AL = DC_VALUES[index];
	
	uint16_t ADC_RESULT = 0;
	int reset = 0;
	
	while(1){	
		if(reset == 16){
		    reset = 0;
		    ADC_RESULT = (ADC_RESULT >> 4); //Divide by 2^4 = 16
	            if (ADC_RESULT >= 0 & ADC_RESULT <= 200) {
	                PORTD = 0x01;
	            }
	            else if(ADC_RESULT > 200 & ADC_RESULT <= 400) {
	                PORTD = 0x02;
	            }
	            else if(ADC_RESULT > 400 & ADC_RESULT <= 600) {
	                PORTD = 0x04;
	            }
	            else if(ADC_RESULT > 600 & ADC_RESULT <= 800) {
	                PORTD = 0x08;
	            }
	            else {
	                PORTD = 0x10;
	            }
            ADC_RESULT =0;
		}
		_delay_ms(100);					//Call delay
		ADC_RESULT += adc_read();			//Read ADC_value
		reset++;						//Keep track of how many values we have
		
        /*Increase or decrease the Duty Cycle
		based on which button is pressed*/
        unsigned char d = PIND, check = PIND;
        
        //wait for the button to be un-pressed
        while (!(check & (1 << PD5)) | !(check & (1 << PD6))) {
            check = PIND;
        }
        
        unsigned char pd5 = d & 0x20;
        unsigned char pd6 = d & 0x40;

		if((pd5 ==0) && index < 12){ // with PD5 we increase
			index++;
			OCR1AL = DC_VALUES[index];
		}
		if((pd6 ==0) && index > 0){ // with PD6 we decrease
			index--;
			OCR1AL = DC_VALUES[index];
        }
														
		
	}
}
