
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

 uint16_t scan_row(uint8_t row){	 
		uint16_t result = 0;
		PCA9555_0_write(REG_CONFIGURATION_1,row);
	    uint8_t input = PCA9555_0_read(REG_INPUT_1);
        if(input & 0x10) //1st element from row is pressed
        {
			result += 0x10;
        }
        else if(input & 0x20) // 2nd element from row is pressed
        {
			result += 0x20;
        }
        else if(input & 0x40) //3rd element from row  is pressed
        {
			result += 0x40;
        }
        else if(input & 0x80) //4th element from row is pressed
        {
			result += 0x80;
        }
		
	return result;
 }
 
 
 uint16_t scan_keypad(){
	 uint16_t result;
	 if(scan_row(0x00) != 0x00){
		 result += scan_row(0x00);		//Scan 1st row
	 }
	 if(scan_row(0x02) != 0x00){
		 result += scan_row(0x01);		//Scan 2nd row
	 }
	 if(scan_row(0x04) != 0x00){
		 result += scan_row(0x02);		//Scan 3rd row
	 }
	 if(scan_row(0x08) != 0x00){
		 result += scan_row(0x04);		//Scan 4th row
	 }
	 
	return result;				//Return total buttons pressed
 }
 
 void scan_keypad_rising_edge(){
	 uint16_t pressed_keys_temp = 0;
	 uint16_t pressed_keys_new = 0;
	 while((pressed_keys_temp = scan_keypad()) != 0x00){
		 _delay_ms(20);
		 if((pressed_keys_new = scan_keypad()) != pressed_keys_temp){
			pressed_keys_temp =- pressed_keys_new;
		 }
	 
		 pressed_keys =	pressed_keys_temp;	 
	 }
		
	return;  
			 
 }
 

 int main(){
	 
	twi_init();
	PCA9555_0_write(REG_CONFIGURATION_1, 0x00); //Set EXT_PORT0 as output
	lcd_init();
	
	while(1){
		
		
		scan_keypad_rising_edge();
		_delay_ms(1000);
		lcd_data(pressed_keys + '0');
		_delay_ms(1000);
		lcd_clear_display();
		
		
	}
	 return 0;
 }
