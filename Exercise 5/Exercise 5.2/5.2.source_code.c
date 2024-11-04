#include "52headers.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

int main() {
    twi_init();
    //Set Port Expander 0 as output
    PCA9555_0_write(REG_CONFIGURATION_0,0x00);
    //Set Port Expander 1 [3:0] as output ((and [7:4] as input))
    PCA9555_0_write(REG_CONFIGURATION_1,0xFE);
    uint8_t input;
    //DDRD = 0xFF;
    PCA9555_0_write(REG_OUTPUT_1,0x00); //maybe

    
    while(1)
    {
        //PCA9555_0_write(REG_OUTPUT_0, 0xFF);
        input = PCA9555_0_read(REG_INPUT_1);
        
        
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
