/*
 * main.c
 *
 * Created: 12/2/2024 1:00:14 PM
 *  Author: kosta
 */ 

#define F_CPU 16000000UL
#include "header81.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include <string.h>

int main(void){
	
	/* Set up lcd screen and usart */
	
    twi_init();
    PCA9555_0_write(REG_CONFIGURATION_0, 0x00);
    lcd_init();
    _delay_ms(100);             //Maybe for the LCD
    lcd_clear_display();
	usart_init(103);
	
	uint8_t url[] = "\"http://192.168.1.250:5000/data\"";
	char success_1_msg[] = "1.Success";
	char fail_1_msg[] = "1.Fail";
	char success_2_msg[] = "2.Success";
	char fail_2_msg[] = "2.Fail";
	
	while(1){
		_delay_ms(1500);
		lcd_clear_display();
		uint8_t ret = esp_send_command(CMD_CONNECT, 0, 0);
	
		/* Check response and print return message */
		if(ret == 0){
			for(int i = 0; i < strlen(success_1_msg); i++){
				lcd_data(success_1_msg[i]);			
			}
		}
		else if(ret == 1){
			for(int i = 0; i < strlen(fail_1_msg); i++){
				lcd_data(fail_1_msg[i]);			
			}
		}
		else{
			lcd_data('!');
		}	
		
		/* Send command to connect to url */
		_delay_ms(1500);
		lcd_clear_display();
		
		ret = esp_send_command(CMD_URL, url, sizeof(url) - 1);
		/* Check response and print return message */
		if(ret == 0){
			for(int i = 0; i < strlen(success_2_msg); i++){
				lcd_data(success_2_msg[i]);
			}
		}
		else if(ret == 1){
			//lcd_clear_display();
			for(int i = 0; i < strlen(fail_2_msg); i++){
				lcd_data(fail_2_msg[i]);
			}
		}
		else{
			//lcd_clear_display();
			lcd_data('!');
		}
	}
}



/* Starting template for 8.2 */

/*Get a measurement from thermometer as in exercise 7*/

/*Adjust measurement in order to simulate human temperature*/

/*Take a reading from the Potensiometer we have chosen and translate it to simulate to 0-20cm scale*/

/* Send above measurements to the payload we will send with usar_command */

/* Start checking keyboard values, keyboard controls the status in the payload */

/* Check the following cases:	
	'8' was pressed status->CALL NURSE
	'#' was pressed status->OK
	blood pressure below 4 or above 12	status->CHECK PRESSURE
	temp below 34 or above 37 status->CHECK TEMP
	none of the above status->OK
*/

/*	Show temperature and blood pressure measurement in LCD SCREEN row 1 and status in LCD SCREEN row 2 */
