// dac.c
// This software configures DAC output
// Lab 6 requires a minimum of 4 bits for the DAC, but you could have 5 or 6 bits
// Runs on LM4F120 or TM4C123
// Program written by: Marzooq Shah
// Date Created: 3/6/17 
// Last Modified: Apr 13, 2022
// Hardware connections
// TO STUDENTS "REMOVE THIS LINE AND SPECIFY YOUR HARDWARE********

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
// Code files contain the actual implemenation for public functions
// this file also contains an private functions and private data

// **************DAC_Init*********************
// Initialize 6-bit DAC, called once 
// Input: none
// Output: none
void DAC_Init(void){
		volatile int32_t wait;
		SYSCTL_RCGCGPIO_R |= 0x02;
		wait = SYSCTL_RCGCGPIO_R;						// NOP
		wait = SYSCTL_RCGCGPIO_R;		
		wait = SYSCTL_RCGCGPIO_R;			
		
		// port B
		GPIO_PORTB_DIR_R |= 0x3F;
		GPIO_PORTB_DEN_R |= 0x3F;
		GPIO_PORTB_DR4R_R |= 0x3F;

}


// **************DAC_Out*********************
// output to DAC
// Input: 6-bit data, 0 to 63 
// Input=n is converted to n*3.3V/63
// Output: none
void DAC_Out(uint32_t data){
	
	GPIO_PORTB_DATA_R = data;
	
}
