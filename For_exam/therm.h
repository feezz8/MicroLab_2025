#ifndef THERM_HEADER
#define	THERM_HEADER

//#include "twi_pca.h"
#include <stdint.h>
#include <stdbool.h>


bool one_wire_reset();
uint8_t one_wire_receive_bit();
void one_wire_transmit_bit(uint8_t input);
uint8_t one_wire_receive_byte();
void one_wire_transmit_byte(uint8_t input);
uint16_t our_func();

// Return the integer part of our reading
uint8_t integer_part(uint16_t input);

// Return the decimal part of our reading
int decimal_part(uint16_t input);

void send_three_to_screen(int input);
#endif	/* KEYPAD_HEADER */