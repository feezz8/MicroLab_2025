/*
 * main.c
 *
 * Created: 12/2/2024 7:11:06 PM
 *  Author: kosta
 */ 


#define F_CPU 16000000UL
#include "header81.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include <string.h>

#define CHK_TEMP 1
#define CHK_PRESSURE 2
#define CALL_NRS 3
#define STATUS_OK 0

int main(void){	
    twi_init();
    PCA9555_0_write(REG_CONFIGURATION_0, 0x00);
	PCA9555_0_write(REG_CONFIGURATION_1, 0xF0); //PORT1[7:3] = input, [3:0] = output
    lcd_init();
    _delay_ms(100);             //Maybe for the LCD
    lcd_clear_display();
    usart_init(103);
	adc_init();
	/*Get a measurement from the potensiometer and adjust accordingly*/

	char check_temp[] = "CHECK TEMP";
	char check_pressure[] = "CHECK PRESSURE";
	char ok[] = "OK";
	char call_nurse[] = "NURSE CALL";

	char payload[1024]; // Adjust size as needed for larger payloads
	char temp[256];     // Temporary buffer for intermediate formatting

	uint8_t url[] = "\"http://192.168.1.250:5000/data\"";
	char success_1_msg[] = "1.Success";
	char fail_1_msg[] = "1.Fail";
	char success_2_msg[] = "2.Success";
	char fail_2_msg[] = "2.Fail";
	
	start:
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
		goto start;
	}
	else{
		lcd_data('!');
		goto start;
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
		goto start;
	}
	else{
		//lcd_clear_display();
		lcd_data('!');
		goto start;
	}
				
	_delay_ms(3000);
	lcd_clear_display();
		
	reset:
	while(keypad_to_ascii() != '8'){		
		/*Get a measurement from thermometer as in exercise 7*/
		uint16_t temperature = get_temperature_reading();
		uint8_t patient_temperature_int = integer_part(temperature) + 12;
		uint8_t patient_temperature_dec = decimal_part(temperature);
		
		int status = 0;
	
		/*Get ADC reading*/
		uint16_t patient_pressure_adc = adc_read();
		int patient_pressure = patient_pressure_adc / 51;
		 
		if(patient_temperature_int > 36 || patient_temperature_int < 34){
			status = CHK_TEMP;
		} 	
		
		if(patient_pressure > 12 || patient_pressure < 4){
			status = CHK_PRESSURE;
		}
		
		// Start the JSON payload
		snprintf(payload, sizeof(payload), "");

		// Add each JSON object to the payload, these are similar for all status
		snprintf(temp, sizeof(temp), "{\"name\": \"temperature\",\"value\": \"%d.%d\"},",patient_temperature_int, patient_temperature_dec);
		strncat(payload, temp, sizeof(payload) - strlen(payload) - 1);

		snprintf(temp, sizeof(temp), "{\"name\": \"pressure\",\"value\": \"%d\"},", patient_pressure);
		strncat(payload, temp, sizeof(payload) - strlen(payload) - 1);

		snprintf(temp, sizeof(temp), "{\"name\": \"team\",\"value\": \"58\"},");
		strncat(payload, temp, sizeof(payload) - strlen(payload) - 1);

		
		/* Send appropriate payload */
		switch(status){
			case STATUS_OK:
				snprintf(temp, sizeof(temp), "{\"name\": \"status\",\"value\": \"OK\"}");
				strncat(payload, temp, sizeof(payload) - strlen(payload) - 1);
				// Close the JSON array
				strncat(payload, "]", sizeof(payload) - strlen(payload) - 1);
				send_three_to_screen(patient_temperature_int);
				lcd_data('.');
				send_three_to_screen(patient_temperature_dec);
				lcd_data(' ');
				lcd_data('\xdf');
				lcd_data('C');
				lcd_data(' ');
				send_three_to_screen(patient_pressure);	
				lcd_change_line();
				for(int i = 0; i < strlen(ok); i++){
					lcd_data(ok[i]);					
				}
				break;
			case CHK_TEMP:
				snprintf(temp, sizeof(temp), "{\"name\": \"status\",\"value\": \"CHECK TEMP\"}");
				strncat(payload, temp, sizeof(payload) - strlen(payload) - 1);
				
				send_three_to_screen(patient_temperature_int);
				lcd_data('.');
				send_three_to_screen(patient_temperature_dec);
				lcd_data(' ');
				lcd_data('\xdf');
				lcd_data('C');
				lcd_data(' ');
				send_three_to_screen(patient_pressure);
				lcd_change_line();
				for(int i = 0; i < strlen(check_temp); i++){
					lcd_data(check_temp[i]);
				}
				break;
			case CHK_PRESSURE:
				snprintf(temp, sizeof(temp), "{\"name\": \"status\",\"value\": \"CHECK PRESSURE\"}");
				strncat(payload, temp, sizeof(payload) - strlen(payload) - 1);
				
				send_three_to_screen(patient_temperature_int);
				lcd_data('.');
				send_three_to_screen(patient_temperature_dec);
				lcd_data(' ');
				lcd_data('\xdf');
				lcd_data('C');
				lcd_data(' ');
				send_three_to_screen(patient_pressure);
				lcd_change_line();
				for(int i = 0; i < strlen(check_pressure); i++){
					lcd_data(check_pressure[i]);
				}
				break;
			default:
				lcd_data('!');
				break;
		}
		
		esp_send_command(CMD_PAYLOAD, payload, strlen(payload));
		_delay_ms(3000);
		lcd_clear_display();
		continue;
		
	nurse:
		while(keypad_to_ascii() != '#'){
						
			uint16_t temperature = get_temperature_reading();
			uint8_t patient_temperature_int = integer_part(temperature) + 12;
			uint8_t patient_temperature_dec = decimal_part(temperature);
			
			/*Get ADC reading*/
			uint16_t patient_pressure_adc = adc_read();
			int patient_pressure = patient_pressure_adc / 51;
			snprintf(payload, sizeof(payload), "");

			// Add each JSON object to the payload, these are similar for all status
			snprintf(temp, sizeof(temp), "{\"name\": \"temperature\",\"value\": \"%d.%d\"},",patient_temperature_int, patient_temperature_dec);
			strncat(payload, temp, sizeof(payload) - strlen(payload) - 1);

			snprintf(temp, sizeof(temp), "{\"name\": \"pressure\",\"value\": \"%d\"},", patient_pressure);
			strncat(payload, temp, sizeof(payload) - strlen(payload) - 1);

			snprintf(temp, sizeof(temp), "{\"name\": \"team\",\"value\": \"58\"},");
			strncat(payload, temp, sizeof(payload) - strlen(payload) - 1);
			
			snprintf(temp, sizeof(temp), "{\"name\": \"status\",\"value\": \"NURSE CALL\"}");
			strncat(payload, temp, sizeof(payload) - strlen(payload) - 1);
			status = CALL_NRS;
			
			send_three_to_screen(patient_temperature_int);
			lcd_data('.');
			send_three_to_screen(patient_temperature_dec);
			lcd_data(' ');
			lcd_data('\xdf');
			lcd_data('C');
			lcd_data(' ');
			send_three_to_screen(patient_pressure);
			lcd_change_line();
			for(int i = 0; i < strlen(call_nurse); i++){
				lcd_data(call_nurse[i]);
			}
			esp_send_command(CMD_PAYLOAD, payload, strlen(payload));
			_delay_ms(3000);
			lcd_clear_display();		
		}
		status = STATUS_OK;
		goto reset;
	}
	goto nurse;
}
