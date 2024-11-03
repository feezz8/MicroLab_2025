#include "51headers.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

int main() {
    //Set PORTB[3:0] as input
    DDRB = 0x00;
    //Set PORTD as output in order to connect PD2 and PD3 with IO0_0 and IO0_1
    DDRD = 0xFF;
    unsigned char A,B,C,D, F0, F1, input, result;
    twi_init();
    PCA9555_0_write(REG_CONFIGURATION_0, 0x00); //Set EXT_PORT0 as output
    
    while(1) {
        input = PINB;
        A = input & 0x01;
        B = (input & 0x02) >> 1;
        C = (input & 0x04) >> 2;
        D = (input & 0x08) >> 3;
        F0 = ~( ((~A) & B & C) | (~(B) & D));
        F0 &= 0x01;
        F1 = (A | B | C) & (B & (~D));
        F1 = (F1 & 0x01) << 1;
        result = F0 | F1;
        //Maybe I need PCA Config output to take it to LEDs?
        PCA9555_0_write(REG_OUTPUT_0, result);
    }
}
