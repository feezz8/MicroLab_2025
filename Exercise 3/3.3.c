/*
 * main.c
 *
 * Created: 10/20/2024 12:46:48 PM
 *  Author: Konstantinos
 */ 

#define F_CPU 1600000UL
#include "avr/io.h"
#include <util/delay.h>

void adc_init(void) {
	// Configure ADC
	// Set reference voltage to AVcc (with external capacitor on AREF)
	ADMUX |= (1 << REFS0);
	
	//Set ADC left adjusted
	ADMUX |= (1 << ADLAR);
	
	// Set input channel to ADC1 (adjust according to your pin configuration)
	ADMUX &= ~(1 << MUX0);  //0000 = ADC0
	
	// Set ADC prescaler to 128 (for 16 MHz clock, ADC clock = 16MHz / 128 = 125 kHz)
	// This ensures the ADC clock is within the recommended range (50 kHz - 200 kHz)
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	
	// Enable the ADC
	ADCSRA |= (1 << ADEN);
}

uint16_t adc_read(void) {
	// Start the conversion
	ADCSRA |= (1 << ADSC);
	
	// Wait for the conversion to complete (ADSC becomes '0' when the conversion is done)
	while (ADCSRA & (1 << ADSC));
	
	// Return the 10-bit ADC result
	return ADC;
}

int main(){	
	int DC_VALUES_1[14];		//Create table for dc_values

	for (int i = 0; i < 13; i++){		//set up the DC_VALUE table
		DC_VALUES_1[i] = (8 + i*20);
	}

	/*	set TMR1A in fast PWM 8 bit mode with non inverted output
	and prescale  = 256
	*/
	TCCR1A = (1 << WGM10) | (1 << COM1A1);
	TCCR1B = (1 << WGM12) | (1 << CS12);
	
	DDRB |= 0b00011111;
	DDRD |= 0b00011111;		//PD4-PD0 output, PD5-PD7 input				
	int index = 7;			//Duty Cycle =  50%

	while(1){		
		/*CHECK IF PD6 WAS PRESSED ENTER MOPDE 1*/
	
			/*Increase or decrease the Duty Cycle
			based on which button is pressed*/												
			if(PORTD == 0x01){
				index++;
				if(index == 13){
					continue;
				}
				OCR1AL = DC_VALUES_1[index];
			}
			if(PORTD == 0x02){
				index--;
				if(index == 0){
					continue;
				}			
				OCR1AL = DC_VALUES_1[index];
			}
		/*CHECK IF PD7 WAS PRESSED ENTER MODE 2*/
			adc_init();
			OCR1AL = adc_read();
			
	}
}
