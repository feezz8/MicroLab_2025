/*
 * _8.c
 *
 * Created: 12/2/2024 1:01:21 PM
 *  Author: kosta
 */ 

#define F_CPU 16000000UL
#include "header81.h"
#include "avr/io.h"
#include <util/delay.h>
#include <stdbool.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <string.h>


uint16_t pressed_keys = 0;
uint8_t ascii[16] = {'*', '0', '#', 'D', '7', '8', '9', 'C', '4', '5', '6', 'B', '1', '2', '3', 'A'};

/* Routine: usart_init
Description:
This routine initializes the
usart as shown below.
------- INITIALIZATIONS -------
Baud rate: 9600 (Fck= 16MH)
Asynchronous mode
Transmitter on
Reciever on
Communication parameters: 8 Data ,1 Stop, no Parity
--------------------------------
parameters: ubrr to control the BAUD.
return value: None.*/


void usart_init(unsigned int ubrr){
	UCSR0A=0;
	UCSR0B=(1<<RXEN0)|(1<<TXEN0);
	UBRR0H=(unsigned char)(ubrr>>8);
	UBRR0L=(unsigned char)ubrr;
	UCSR0C=(3 << UCSZ00);
	return;	
}

/* Routine: usart_transmit
Description:
This routine sends a byte of data
5
using usart.
parameters:
data: the byte to be transmitted
return value: None. */

void usart_transmit(uint8_t data){
	while(!(UCSR0A&(1<<UDRE0)));
	UDR0=data;
}

/* Routine: usart_receive
Description:
This routine receives a byte of data
from usart.
parameters: None.
return value: the received byte */

uint8_t usart_receive(){
	while(!(UCSR0A&(1<<RXC0)));
	return UDR0;
}
/*Function that receives a response and returns its length, the package is saved in buf*/

int usart_receive_buffer(uint8_t *buf, int length){
	int package = 0;
	while(package < length && ((*buf++ = usart_receive()) != BUF_END)) ++package;
	return package;	
	
}

void usart_transmit_buffer(uint8_t *buf, int length) {
	while (length-- > 0) usart_transmit(*buf++);
}

/*function to send commands to the ESP*/
uint8_t esp_send_command(uint8_t command_type, uint8_t *argc, int argv){
	/*Works like syscalls returns 0 on success 1 on fail* -1 on undefined error/
	/*define type of commands*/
	static uint8_t connect[] = "ESP:connect";
	static uint8_t payload[] = "ESP:payload:";
	static uint8_t transmit[] = "ESP:transmit";
	static uint8_t url[]  = "ESP:url:";
	
	/*define type of returns*/
	
	static uint8_t success[] = "\"Success\"\n";
	static uint8_t fail[] = "\"Failure\"\n";
	
	uint8_t buf[50];
	int buf_length;
	
	
	/*Transmit command and its arguments if needed*/
	switch(command_type){
		case CMD_CONNECT:
			usart_transmit_buffer(connect, sizeof(connect)-1);
			break;
		case CMD_URL:
			usart_transmit_buffer(url, sizeof(url) - 1);
			usart_transmit_buffer(argc, argv);
			break;
		case CMD_PAYLOAD:
			usart_transmit_buffer(payload, sizeof(payload)-1);
			usart_transmit_buffer(argc, argv);
			break;
		case CMD_TRANSMIT:
			usart_transmit_buffer(transmit, sizeof(transmit) - 1);
			break;	
		default:
			return -1;
			break;
	}
	
	/*Don't forget new line character as suggested */
	usart_transmit('\n');
	buf_length = usart_receive_buffer(buf, sizeof(buf));
	//usart_transmit('\n');
	
	/*Now lets see the response from the Gateway*/
	switch(command_type){
		case CMD_CONNECT:
		case CMD_URL:
		case CMD_PAYLOAD:
			if(!strncmp(buf, success, buf_length)) return 0;
			else if(!strncmp(buf, fail, buf_length)) return 1;
			break;
		case CMD_TRANSMIT:
			{
				int ret;
				for(ret = 0; ret < buf_length && ret < argv; ++ret) argc[ret] = buf[ret];
				return ret;
				break;
			}
		default:
			return -1;
			break;
	}
	return -1;
}

uint8_t fake_d = 0x00; //global to simulate PORTD

//initialize TWI clock
void twi_init(void)
{
	TWSR0 = 0; // PRESCALER_VALUE=1
	TWBR0 = TWBR0_VALUE; // SCL_CLOCK 100KHz
}

// Read one byte from the TWI device (request more data from device)
//TWEA =1 -> ACK
unsigned char twi_readAck(void)
{
	TWCR0 = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
	while(!(TWCR0 & (1<<TWINT)));
	return TWDR0;
}

//Read one byte from the TWI device, read is followed by a stop condition
//TWEA =0 -> No ACK
unsigned char twi_readNak(void)
{
	TWCR0 = (1<<TWINT) | (1<<TWEN);
	while(!(TWCR0 & (1<<TWINT)));
	return TWDR0;
}

// Issues a start condition and sends address and transfer direction.
// return 0 = device accessible, 1= failed to access device
unsigned char twi_start(unsigned char address)
{
	uint8_t twi_status;
	// send START condition
	TWCR0 = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
	
	// wait until transmission completed
	while(!(TWCR0 & (1<<TWINT)));
	
	// check value of TWI Status Register.
	twi_status = TW_STATUS & 0xF8;          // Isn't the & unnecessary
	if ( (twi_status != TW_START) && (twi_status != TW_REP_START)) {
		return 1;
	}
	
	// send device address
	TWDR0 = address;
	TWCR0 = (1<<TWINT) | (1<<TWEN);
	
	// wail until transmission completed and ACK/NACK has been received
	while(!(TWCR0 & (1<<TWINT)));
	
	// check value of TWI Status Register.
	twi_status = TW_STATUS & 0xF8;
	
	if ( (twi_status != TW_MT_SLA_ACK) && (twi_status != TW_MR_SLA_ACK) ) {
		return 1;
	}
	return 0;
}

// Send start condition, address, transfer direction.
// Use ACK polling to wait until device is ready
void twi_start_wait(unsigned char address)
{
	uint8_t twi_status;
	
	while(1) {
		// send START condition
		TWCR0 = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
		
		// wait until transmission completed
		while(!(TWCR0 & (1<<TWINT)));
		
		// check value of TWI Status Register.
		twi_status = TW_STATUS & 0xF8;
		if ( (twi_status != TW_START) && (twi_status != TW_REP_START)) {
			continue;
		}
		
		// send device address
		TWDR0 = address;
		TWCR0 = (1<<TWINT) | (1<<TWEN);
		
		// wail until transmission completed
		while(!(TWCR0 & (1<<TWINT)));
		
		// check value of TWI Status Register.
		twi_status = TW_STATUS & 0xF8;
		if ( (twi_status == TW_MT_SLA_NACK )||(twi_status ==TW_MR_DATA_NACK) )
		{
			/* device busy, send stop condition to terminate write operation */
			TWCR0 = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
			
			// wait until stop condition is executed and bus released
			while(TWCR0 & (1<<TWSTO));
			continue;
		}
		break;
	}
}

// Send one byte to TWI device, Return 0 if write successful or 1 if write failed
unsigned char twi_write( unsigned char data )
{
	// send data to the previously addressed device
	TWDR0 = data;
	TWCR0 = (1<<TWINT) | (1<<TWEN);
	
	// wait until transmission completed
	while(!(TWCR0 & (1<<TWINT)));
	if( (TW_STATUS & 0xF8) != TW_MT_DATA_ACK) {
		return 1;
	}
	return 0;
}

// Send repeated start condition, address, transfer direction
//Return: 0 device accessible
// 1 failed to access device
unsigned char twi_rep_start(unsigned char address)
{
	return twi_start( address );
}

// Terminates the data transfer and releases the twi bus
void twi_stop(void)
{
	// send stop condition
	TWCR0 = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
	
	// wait until stop condition is executed and bus released
	while(TWCR0 & (1<<TWSTO));
}

void PCA9555_0_write(PCA9555_REGISTERS reg, uint8_t value)
{
	twi_start_wait(PCA9555_0_ADDRESS + TWI_WRITE);
	twi_write(reg);
	twi_write(value);
	twi_stop();
}

uint8_t PCA9555_0_read(PCA9555_REGISTERS reg)
{
	uint8_t ret_val;
	twi_start_wait(PCA9555_0_ADDRESS + TWI_WRITE);
	twi_write(reg);
	twi_rep_start(PCA9555_0_ADDRESS + TWI_READ);
	ret_val = twi_readNak();
	twi_stop();
	return ret_val;
}

void write_2_nibbles(uint8_t input) {
	unsigned char temp = input & 0xF0;
	unsigned char pind  = fake_d & 0x0F;
	fake_d = pind + temp;
	PCA9555_0_write(REG_OUTPUT_0, fake_d);
	
	fake_d |= 0x08;
	PCA9555_0_write(REG_OUTPUT_0, fake_d);
	
	asm("nop");
	asm("nop");
	
	fake_d &= ~(0x08);
	PCA9555_0_write(REG_OUTPUT_0, fake_d);
	
	
	temp = input & 0x0F;
	temp = temp << 4;
	
	fake_d = pind + temp;
	PCA9555_0_write(REG_OUTPUT_0, fake_d);
	
	fake_d |= 0x08;
	PCA9555_0_write(REG_OUTPUT_0, fake_d);
	
	asm("nop");
	asm("nop");
	
	fake_d &= ~(0x08);
	PCA9555_0_write(REG_OUTPUT_0, fake_d);
	
	return;
}

void lcd_data (uint8_t input) {
	
	fake_d |= 0x04;
	PCA9555_0_write(REG_OUTPUT_0, fake_d);
	
	write_2_nibbles(input);
	_delay_ms(1);
	return;
}

void lcd_command (uint8_t input) {
	
	fake_d &= ~(0x04);
	PCA9555_0_write(REG_OUTPUT_0, fake_d);
	
	write_2_nibbles(input);
	_delay_ms(1);
	return;
}

void lcd_clear_display () {
	lcd_command(0x01);
	_delay_ms(5);
	return;
}

void lcd_change_line () {
	//I send command 0xC0 so I write in DDRAM Address the 100 0000
	lcd_command(0xC0);
	_delay_ms(5);
	return;
}

void lcd_init () {
	_delay_ms(200);
	
	fake_d = 0x30;
	PCA9555_0_write(REG_OUTPUT_0, fake_d);
	
	for (int i=0; i<3; i++) {
		
		fake_d |= 0x08;
		PCA9555_0_write(REG_OUTPUT_0, fake_d);
		
		asm("nop");
		asm("nop");
		
		fake_d &= ~(0x08);
		PCA9555_0_write(REG_OUTPUT_0, fake_d);
		
		_delay_ms(1);
	}
	
	fake_d = 0x20;
	PCA9555_0_write(REG_OUTPUT_0, fake_d);
	
	fake_d |= 0x08;
	PCA9555_0_write(REG_OUTPUT_0, fake_d);
	
	asm("nop");
	asm("nop");
	
	fake_d &= ~(0x08);
	PCA9555_0_write(REG_OUTPUT_0, fake_d);
	
	_delay_ms(1);
	
	lcd_command(0x28);
	lcd_command(0x0c);
	lcd_clear_display();
	lcd_command(0x06);
	return;

}

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
    
    //Return 10-bit result
    return ADC;
}

bool one_wire_reset() {
	uint8_t temp;
	
	//Set PD4 as output and equal to 0
	DDRD |= 0x10;
	PORTD &= ~(0x10);
	
	_delay_us(480);   //480 uSec Reset Pulse
	
	//Set PD4 as input and disable pull-up
	DDRD &= ~(0x10);
	PORTD &= ~(0x10);
	
	//Wait 100 usec for connected devices to transmit the presence pulse
	_delay_us(100);
	
	//temp = PD4
	temp = PIND & 0x10;
	_delay_us(380);
	
	//If a connected device is detected(PD4==0) return 1 else return 0
	if(temp == 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

uint8_t one_wire_receive_bit() {
	uint8_t result = 0;
	
	//Set PD4 as output and equal to 0
	DDRD |= 0x10;
	PORTD &= ~(0x10);
	_delay_us(2);
	
	//Set PD4 as input and disable pull-up
	DDRD &= ~(0x10);
	PORTD &= ~(0x10);
	_delay_us(10);
	
	//Store PD4 in LSB of result
	result = (PIND & 0x10) >> 4;
	
	//Delay 49 uSec to meet the standards
	_delay_us(49);	
	return result;
}

void one_wire_transmit_bit(uint8_t input) {
	bool one = input & 0x01;
	
	//Set PD4 as output and equal to 0
	DDRD |= 0x10;
	PORTD &= ~(0x10);
	_delay_us(2);
	
	//PD4 = LSB of input
	if(one)
	{
		PORTD |= 0x10;
	}
	else
	{
		PORTD &= ~(0x10);
	}
	_delay_us(58);
	
	//Set PD4 as input and disable pull-up
	DDRD &= ~(0x10);
	PORTD &= ~(0x10);
	_delay_us(1);
	return;
}


uint8_t one_wire_receive_byte() {
	uint8_t result =0;
	//Load in result from LSB to MSB
	for(int i=0; i<8; i++)
	{
		result |= (one_wire_receive_bit() << i);
	}
	return result;
}
void one_wire_transmit_byte(uint8_t input) {
	uint8_t temp;
	//We transmit from LSB to MSB
	for(int i=0; i<8; i++)
	{
		temp = input & (1<<i);
		temp = temp >> i;
		one_wire_transmit_bit(temp);
	}
	return;
}
uint16_t get_temperature_reading() {
	bool flag = 0; // If flag = 1 then we haven't found a device
	if(!one_wire_reset())
	{
		flag = 1;
	}
	one_wire_transmit_byte(0xCC);       // Send command to bypass the selection of device, since we only have 1 device
	one_wire_transmit_byte(0x44);       // Send command to start the measurement of temperature
	while(one_wire_receive_bit() != 1); // Loop up until the device sends bit = 1
	if(!one_wire_reset())
	{
		flag = 1;
	}
	one_wire_transmit_byte(0xCC);       // Send command to bypass the selection of device, since we only have 1 device
	one_wire_transmit_byte(0xBE);       // Send command to read 16-bit temperature
	
	uint16_t reg24, reg25;
	reg24 = one_wire_receive_byte();
	reg25 = (one_wire_receive_byte()) << 8;
	reg25 |= reg24;
	if(flag)
	{
		return 0x8000;
	}
	else
	{
		return reg25;
	}
}
int integer_part(uint16_t input) {
	uint8_t low, high, integer;
	low = input & 0x00FF;
	high = (input & 0xFF00) >> 8;
	integer = low >> 4;
	integer |= ((high << 4) & 0x70);
	return integer;
}

// Return the decimal part of our reading
int decimal_part(uint16_t input) {
	uint8_t low = input & 0x000F;
	int result = 0;
	if(low & 0x08)
	{
		result += 5000;
	}
	if(low & 0x04)
	{
		result += 2500;
	}
	if(low & 0x02)
	{
		result += 1250;
	}
	if(low & 0x01)
	{
		result += 625;
	}
	return result;
}

void send_three_to_screen(int input) {
	uint8_t to_send;
	to_send = input % 1000;
	to_send = input / 100 ;
	lcd_data('0'+to_send);
	to_send = input % 100;
	to_send = to_send / 10;
	lcd_data('0'+to_send);
	to_send = input % 10;
	lcd_data ('0'+to_send);
}

uint16_t scan_row(uint8_t row){
	uint16_t result = 0;
	
	uint8_t dummy = 0xFF;
	dummy &= ~(1 << (row-1));
	PCA9555_0_write(REG_OUTPUT_1,dummy);
	
	uint8_t input = PCA9555_0_read(REG_INPUT_1);
	if((input & 0x10) == 0x00) //1st element from row is pressed
	{
		result |= 0x01;
	}
	if((input & 0x20) == 0x00) // 2nd element from row is pressed
	{
		result |= 0x02;
	}
	if((input & 0x40) == 0x00) //3rd element from row  is pressed
	{
		result |= 0x04;
	}
	if((input & 0x80) == 0x00) //4th element from row is pressed
	{
		result |= 0x08;
	}
	return result;
}


uint16_t scan_keypad(){
	uint16_t result =0;
	result |= scan_row(1);		//Scan 1st row
	result |= (scan_row(2) << 4);		//Scan 2nd row
	result |= (scan_row(3) << 8);		//Scan 3rd row
	result |= (scan_row(4) << 12);		//Scan 4th row
	return result;				//Return total buttons pressed
}

uint16_t scan_keypad_rising_edge(){
	uint16_t pressed_keys_temp = 0, dummy = 0, result =0;;
	pressed_keys_temp = scan_keypad();
	_delay_ms(15);
	dummy = scan_keypad();
	
	//Here we take 2 measurements and we only keep the buttons which are pressed in both (Correct??)
	pressed_keys_temp &= dummy;
	
	//pressed_keys = pressed_keys_temp;
	
	//Here we only update pressed_keys to only keep keys that are now pressed and weren't before
	//result = (~(pressed_keys)) & pressed_keys_temp;
	result = pressed_keys_temp;
	return result;
}

uint8_t keypad_to_ascii() {
	uint16_t from_keys = scan_keypad_rising_edge();
	int counter = 0;
	while((from_keys & 0x0001) == 0 && counter != 16) {
		counter++;
		from_keys = from_keys >> 1;
	}
	if(counter == 16){
		return 0;
	}
	return ascii[counter];
}
