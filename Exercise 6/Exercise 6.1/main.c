/*
 * main.c
 *
 * Created: 11/8/2024 2:58:34 PM
 *  Author: kosta
 */ 
#define F_CPU 16000000UL
#include "5.3.headers.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h> 
 

uint16_t pressed_keys = 0; //Holds value for pressed key

 uint8_t scan_row(uint8_t row){	 
		uint16_t result = 0;
		/*
		uint8_t temp = 0xFF;
		temp &= ~(1 << row);
		PCA9555_0_write(REG_OUTPUT_1,temp);
		*/
		PCA9555_0_write(REG_OUTPUT_1,0xFE);
	    uint8_t input = PCA9555_0_read(REG_INPUT_1);
		
        if((input & 0x10) == 0) //1st element from row is pressed
        {
			result |= 0b0001;
        }
        if((input & 0x20) == 0) // 2nd element from row is pressed
        {
			result |= 0b0010;
        }
        if((input & 0x40) == 0) //3rd element from row  is pressed
        {
			result |= 0b0100;
        }
        if((input & 0x80) == 0) //4th element from row is pressed
        {
			result |= 0b1000;
        }
		
	return result;
 }
 
 
 uint16_t scan_keypad(){
	 uint16_t result = 0;
	 result |= scan_row(0); 
	 result |= (scan_row(1) << 4);
	 result |= (scan_row(2) << 4);
	 result |= (scan_row(3) << 4);

	 
	return result;				//Return total buttons pressed
 }
 
/* void scan_keypad_rising_edge(){
	 uint16_t pressed_keys_temp = 0;
	 pressed_keys_temp = scan_keypad();
	 
	
		 }
	 
		 pressed_keys =	pressed_keys_temp;	 
	 }
		
	return;  
			 
 }
 
*/
 int main(){
    twi_init();
    //Set Port Expander 0 as output
    PCA9555_0_write(REG_CONFIGURATION_0,0x00);
    //Set Port Expander 1 [3:0] as output ((and [7:4] as input))
    PCA9555_0_write(REG_CONFIGURATION_1, 0xF0);
    uint8_t input;
    PCA9555_0_write(REG_OUTPUT_1, 0x0E); //maybe
	//PCA9555_0_write(REG_CONFIGURATION_1, 0x00);

	
	
	
	while(1){
        uint8_t input = PCA9555_0_read(REG_INPUT_1);
        
        
        if((input & 0x10) == 0x00) //*
            // WE ALWAYS GO HERE
        {
            PCA9555_0_write(REG_OUTPUT_0, 0x01);
        }
        else if((input & 0x20) == 0x00) // 0
        {
            PCA9555_0_write(REG_OUTPUT_0, 0x02);
        }
        else if((input & 0x40) == 0x00) // #
        {
            PCA9555_0_write(REG_OUTPUT_0, 0x04);
        }
        else if((input & 0x80) == 0x00) // D
        {
            PCA9555_0_write(REG_OUTPUT_0, 0x08);
        }
        else
        {
            PCA9555_0_write(REG_OUTPUT_0, 0x00);
        }		
	
		
		
	}
	 return 0;
 }
