#define F_CPU 16000000UL
//#include "twi_pca.h"
#include "therm.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>

//#include "lcd_expander.h"
//#include "twi_pca.h"


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


uint16_t our_func() {
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

// Return the integer part of our reading
uint8_t integer_part(uint16_t input) {
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
