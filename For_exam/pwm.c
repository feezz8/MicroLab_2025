#define F_CPU 16000000UL
#include "pwm.h"
#include "avr/io.h"
#include <util/delay.h>
#include <stdbool.h>

void pwm_init(void){
    //WGM12 & WGM10 for Fast PWM 8-bit
    //[COM1A1 - COM1A0]: 10 => non inverted, connected to PB1
    //[CS12-CS10] for pre-scaler (denominator of CLK) for the timer (here ==1)
	TCCR1A = (1 << WGM10) | (1 << COM1A1);
	TCCR1B = (1 << WGM12) | (1 << CS10);
}