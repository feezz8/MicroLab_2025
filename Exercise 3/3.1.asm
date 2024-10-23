;
; Micro_2025_Ex3.1.asm
;
; Created: 10/19/2024 2:44:13 PM
; Author : Konstantinos
;
.include"m328PBdef.inc"
.def DC_VALUE = r18
//.equ X1 = 300

.org 0x00
    rjmp reset
.org 0x0100
table:
  .db 0, 8, 28, 48, 68, 88, 108, 128, 148, 168, 188, 208, 228, 248

reset:
; initialize stack pointer
    ldi r24, LOW(RAMEND)
    out SPL, r24
    ldi r24, HIGH(RAMEND)
    out SPH, r24

;Set registers for delay
    ldi r24, low(X1)           
    ldi r25, high(X1)         

    ldi r16, 0b00111111
    out DDRB, r16			    ; PB5-0 as output	
    clr r16
    out DDRD, r16			    ; PORTD as input
    ;ser r16
    ;out DDRC, r16			  
    
    ldi r16, (1 << WGM10) | (1 << COM1A1)
    sts TCCR1A, r16                    
    ldi r16, (1 << WGM12) | (1<<CS10)
    sts TCCR1B, r16                         ; Fast PWM 8-bit, prescaler = 1, non-inverse, connected to PB1
    ldi DC_VALUE, 7			    ; 50% duty cycle (for 8-bit, max 255) / 7th position on the table
    ldi ZL, low(table*2)
    ldi ZH, high(table*2)
    add ZL, DC_VALUE			    ; Add the index (in r16) to the Z register	
    lpm	r17, Z				    ; Load the value from program memory
    sts OCR1AL, r17			    ; Write duty cycle to OCR1A


MAIN:
    ;out PORTC, DC_VALUE 
    in r16, PIND
    
wait_to_unpres:
    in r19, PIND
    cpi r19, 0xFF                ;keep looping while button is still pressed
    brne wait_to_unpress
    
    sbrs r16, 03			    ; If PD3 is pressed then add 8%
    rjmp ADD8
    sbrs r16, 04			    ; If PD4 is pressed then reduce 8%
    rjmp SUB8
    rjmp MAIN

ADD8:
    cpi DC_VALUE, 13			    ; Check if we reached max value
    breq MAIN				    
    inc DC_VALUE			       ; Next table location
    ldi ZL, low(table*2)		    ; Load low byte of the table address into Z register
    ldi ZH, high(table*2)		    ; Load high byte of the table address into Z register
    add ZL, DC_VALUE			    ; Add the index (in r16) to the Z register	
    lpm r17, Z				    ; Load the value from program memory (Z points to the value)
    sts OCR1AL, r17			    ; Write duty cycle to OCR1A
    rjmp MAIN

SUB8:
    cpi DC_VALUE, 1			    ; Check if we reached min value
    breq MAIN
    dec DC_VALUE			    ; Previous Table Location
    ldi ZL, low(table*2)		    ; Load low byte of the table address into Z register
    ldi ZH, high(table*2)		    ; Load high byte of the table address into Z register
    add ZL, DC_VALUE			    ; Add the index (in r16) to the Z register	
    lpm r17, Z				    ; Load the value from program memory (Z points to the value)
    sts OCR1AL, r17			    ; Write duty cycle to OCR1A
    rjmp MAIN
    
        
/*
; Delay Routine for X1 msec
wait_x_msec:
    ldi r16, 16
extra_outer_delay:
    rcall delay_outer
    subi r16,1
    brne extra_outer_delay
    ret
     

;this routine is used to produce a delay 993 cycles
delay_inner:
    ldi r23, 247          ; 1 cycle
loop3:
    dec r23               ; 1 cycle
    nop                   ; 1 cycle
    brne loop3            ; 1 or 2 cycles
    nop                   ; 1 cycle
    ret                   ; 4 cycles
;this routine is used to produce a delay of (1000*X1) cycles
delay_outer:
    push r24              ; (2 cycles)
    push r25              ; (2 cycles) Save r24:r25
loop4:
    rcall delay_inner     ; (3+993)=996 cycles
    sbiw r24,1            ; 2 cycles
    brne loop4            ; 1 or 2 cycles
    pop r25               ; (2 cycles)
    pop r24               ; (2 cycles) Restore r24:r25
    ret                   ; 4 cycles
*/
