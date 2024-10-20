;
; Micro_2025_Ex3.1.asm
;
; Created: 10/19/2024 2:44:13 PM
; Author : Konstantinos
;

.include"m328PBdef.inc"
.def DC_VALUE = r18


.org 0x0100									; Place the table at address 0x0100 in program memory

table:
  .db 0, 8, 28, 48, 68, 88, 108, 128, 148, 168, 188, 208, 228, 248

	ldi r16, 0b00111111
	out DDRB, r16			
	ldi r16, (1 << WGM10) | (1 << COM1A1)   ; Fast PWM 8-bit, non-inverted
	sts TCCR1A, r16                         ; Write to TCCR1A
	ldi r16, (1 << WGM12)                   ; Fast PWM mode, no prescaler
	sts TCCR1B, r16                         ; Write to TCCR1B (no prescaler yet)
	ldi r16, (1 << CS12)					; Prescaler =256 /  freq = 62,5kHz
	sts TCCR1B, r16                         ; Start Timer1 with prescaler 256
	ldi DC_VALUE, 7							; 50% duty cycle (for 8-bit, max 255) / 7th position on the table
	ldi r30, low(table)						; Load low byte of the table address into Z register
	ldi r31, high(table)					; Load high byte of the table address into Z register
	add r30, DC_VALUE						; Add the index (in r16) to the Z register	
	lpm r17, Z								; Load the value from program memory (Z points to the value)

	sts OCR1AL, DC_VALUE                     ; Write duty cycle to OCR1A


	clr r16
	out DDRD, r16							;PORTD as input

LOOP1:
	in r16, PIND
	sbrs r16, 03							;if PD3 is pressed then add 8%
	rjmp ADD8
	sbrs r16, 04							;if PD4 is pressed then reduce 2%
	rjmp SUB8
	rjmp LOOP1

ADD8:
	cpi DC_VALUE, 13						;check if we reached max value
	breq LOOP1
	inc DC_VALUE
	ldi r30, low(table)						; Load low byte of the table address into Z register
	ldi r31, high(table)					; Load high byte of the table address into Z register
	add r30, DC_VALUE						; Add the index (in r16) to the Z register	
	lpm r17, Z								; Load the value from program memory (Z points to the value)
	sts OCR1AL, DC_VALUE                    ; Write duty cycle to OCR1A
	rjmp LOOP1

SUB8:
	cpi DC_VALUE, 1							;check if we reached min value
	breq LOOP1
	dec DC_VALUE
	ldi r30, low(table)						; Load low byte of the table address into Z register
	ldi r31, high(table)					; Load high byte of the table address into Z register
	add r30, DC_VALUE						; Add the index (in r16) to the Z register	
	lpm r17, Z								; Load the value from program memory (Z points to the value)
	sts OCR1AL, DC_VALUE                    ; Write duty cycle to OCR1A
	rjmp LOOP1
