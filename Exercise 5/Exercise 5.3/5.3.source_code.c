#define F_CPU 16000000UL
#include "5.3.headers.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


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
/*
void write_2_nibbles_command (uint8_t input) {

	uint8_t temp = input & 0xF0;
	//temp |= 0x04;

	temp |= 0x08;
	PCA9555_0_write(REG_OUTPUT_0, temp);
	asm("nop");
	asm("nop");
	temp &= ~(0x08);
	PCA9555_0_write(REG_OUTPUT_0, temp);
	
	
	temp = input & 0x0F;
	temp = temp << 4;
	//temp |= 0x04;
	
	temp |= 0x08;
	PCA9555_0_write(REG_OUTPUT_0, temp);
	asm("nop");
	asm("nop");
	temp &= ~(0x08);
	PCA9555_0_write(REG_OUTPUT_0, temp);
	
	return;
}

*/

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
