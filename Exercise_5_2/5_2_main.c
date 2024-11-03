#include "52headers.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

int main() {
    twi_init();
    //Set Port Expander 0 as output
    PCA9555_0_write(REG_CONFIGURATION_0,0x00);
    //Set Port Expander 1 [3:0] as output and [7:4] as input
    PCA9555_0_write(REG_CONFIGURATION_1,0xF0);
    uint8_t input;
    
    
    while(1)
    {
        input = PCA9555_0_read(REG_INPUT_1);
        if(input & 0x10) //*
        {
            PCA9555_0_write(REG_OUTPUT_0, 0x10);
        }
        else if(input & 0x20) // 0
        {
            PCA9555_0_write(REG_OUTPUT_0, 0x20);
        }
        else if(input & 0x40) // #
        {
            PCA9555_0_write(REG_OUTPUT_0, 0x40);
        }
        else if(input & 0x80) // D
        {
            PCA9555_0_write(REG_OUTPUT_0, 0x80);
        }
        else
        {
            PCA9555_0_write(REG_OUTPUT_0, 0x00);
        }
    }
    return 0;
}
