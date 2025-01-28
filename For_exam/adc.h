#ifndef ADC_HEADER
#define	ADC_HEADER

#include <stdbool.h>
#include <stdint.h>

void adc_init(int mux_value, bool left_adj, bool interrupts);
uint16_t adc_right_read(void);
uint8_t adc_left_read(void);

#endif	/* ADC_HEADER */

