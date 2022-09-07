// EdgeInterrupt.c
// Runs on LM4F120 or TM4C123
// Request an interrupt on the falling edge of PE0 and PE1 (when the user
// button is pressed) and increment a counter in the interrupt.  Note
// that button bouncing is not addressed.
// Marzooq Shah
// May 3, 2015

/* This example accompanies the book
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers"
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
   Volume 1, Program 9.4
   
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2014
   Volume 2, Program 5.6, Section 5.5

 Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/ */
 

// user button connected to PF4 (increment counter on falling edge)



#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"



void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

// global variable visible in Watch window of debugger
// increments at least once per button press
volatile uint32_t FallingEdges = 0;
void EdgeTrigger_Init(void){                          
	
	
	

	SYSCTL_RCGCGPIO_R |= 0x00000010; // (a) activate clock for port E
	
	FallingEdges = SYSCTL_RCGCGPIO_R;
	FallingEdges = SYSCTL_RCGCGPIO_R;
	FallingEdges = SYSCTL_RCGCGPIO_R;
	
  FallingEdges = 0;             // (b) initialize counter
  GPIO_PORTE_DIR_R &= ~0x03;    // (c) make PE0 and PE1 input
  GPIO_PORTE_AFSEL_R &= ~0x03;  //     disable alt funct on PE0 and PE1
  GPIO_PORTE_DEN_R |= 0x03;     //     enable digital I/O on PE0 and PE1   
  //GPIO_PORTE_PCTL_R &= ~0x000F0000; // configure PF4 as GPIO
  GPIO_PORTE_AMSEL_R = 0;       //     disable analog functionality on PE
  //GPIO_PORTE_PUR_R |= 0x03;     //     enable weak pull-up on PE0 and PE1
  GPIO_PORTE_IS_R &= ~0x03;     // (d) PE0 and PE1 are edge-sensitive
  GPIO_PORTE_IBE_R &= ~0x03;    //     PE0 and PE1 are not both edge sensitive
  GPIO_PORTE_IEV_R |= 0x03;    //     Rising edge Trigger **********
  GPIO_PORTE_ICR_R = 0x03;      // (e) clear flag4
  GPIO_PORTE_IM_R |= 0x03;      // (f) arm interrupt on PE *** No IME bit as mentioned in Book ***
  NVIC_PRI1_R = (NVIC_PRI1_R&0xFFFFFF0F)|0x00000060; // (g) priority 3
  NVIC_EN0_R = 0x00000010;      // (h) enable interrupt 30 in NVIC
  //EnableInterrupts();           // (i) Clears the I bit

}

