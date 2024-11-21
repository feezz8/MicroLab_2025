#define F_CPU 16000000UL
#include "63header.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
 

uint16_t pressed_keys = 0;
uint8_t ascii[16] = {'*', '0', '#', 'D', '7', '8', '9', 'C', '4', '5', '6', 'B', '1', '2', '3', 'A'};

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

int main() {
    twi_init();
	PCA9555_0_write(REG_CONFIGURATION_1, 0xF0); //PORT1[7:3] = input, [3:0] = output
    DDRB = 0xFF;                                //Set PORTB as output;
    PORTB = 0x00;
    uint8_t first, second;
    
    while(1) {
        
        //Maybe we need some delays between the whiles to avoid de-bouncing??
        
        //Wait for the first key to be pressed and when it is, store it in variable first
        while((first = keypad_to_ascii()) == 0);
        
        //Loop up until the user un-presses the first key
        while((keypad_to_ascii()) != 0);
        
        //Wait for the second key to be pressed and when it is, store it in variable second
        while((second = keypad_to_ascii()) == 0);
        
        //Now we have the values of the pressed keys and we can continue
        if(first == '5' && second == '8')
        {
            PORTB = 0xFF;
            _delay_ms(3000);
            PORTB = 0x00;
            _delay_ms(2000);
        }
        else
        {
            for(int i=0; i<5; i++)
            {
                PORTB = 0xFF;
                _delay_ms(500);
                PORTB = 0x00;
                _delay_ms(500);
            }
        }
    }
}
    
    
