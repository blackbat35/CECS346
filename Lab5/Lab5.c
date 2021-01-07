// EdgeInterrupt.c
// Based on TExaSware\C12_EdgeInterrupt

/* This example accompanies the book
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers"
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2013
   Volume 1, Program 9.4
   
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2013
   Volume 2, Program 5.6, Section 5.5

 Copyright 2013 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

#include <stdint.h> // C99 data types
#include "tm4c123gh6pm.h"

// Function Prototypes (from startup.s)
void DisableInterrupts(); // Disable interrupts
void EnableInterrupts();  // Enable interrupts
void WaitForInterrupt();  // Go to low power mode while waiting for the next interrupt

// Function Prototypes
void EdgeCounter_Init(void);  // Initialize edge trigger interrupt for PF0 (SW2) rising edge
void PortF_LEDInit(void);     // Initialize Port F LEDs
void SysTick_Init(unsigned long period);      // Initialize SysTick timer for 0.1s delay with interrupt enabled

void GPIOPortF_Handler(void); // Handle GPIO Port F interrupts
void SysTick_Handler(void);   // Handle SysTick generated interrupts

// global variable visible in Watch and Memory window of debugger
// increments at least once per button release
volatile uint32_t RisingEdges = 0;

int main(void){
  PortF_LEDInit();
	EdgeCounter_Init();           // initialize GPIO Port F interrupt
	SysTick_Init(1600000);
	
	// initialize LaunchPad LEDs to red
	GPIO_PORTF_DATA_R = (GPIO_PORTF_DATA_R & ~0x0E) + 0x02;
	//EnableInterrupts();
  while(1){
		WaitForInterrupt();
		
  }
}

// Color    LED(s) PortF
// red      R--    0x02
// blue     --B    0x04
// green    -G-    0x08

// Initialize Port F LEDs
void PortF_LEDInit(void) {
uint32_t volatile delay;
SYSCTL_RCGC2_R |= 0x20; 
delay = SYSCTL_RCGC2_R;      	
//GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock PortF PF0  
GPIO_PORTF_AMSEL_R &= ~0x0A; 			// 3) disable analog function on PF3,1
GPIO_PORTF_PCTL_R &= ~0x0000F0F0; // 4) enable regular GPIO
GPIO_PORTF_DIR_R |= 0x07; 				// 5) outputs on PF2, PF1
//GPIO_PORTF_DIR_R &= ~0x01; 				// 5) input on PF0
GPIO_PORTF_AFSEL_R &= ~0x07; 			// 6) regular function on PF2,1
//GPIO_PORTF_PUR_R |= 0x01;         // 7)enable pullup resistors on PF0  
GPIO_PORTF_DEN_R |= 0x07; 				// 8) enable digital on PF2,1
}

// Initialize SysTick timer for 0.1s delay with interrupt enabled
void SysTick_Init(unsigned long period) {
NVIC_ST_CTRL_R = 0x00; // disable SysTick during init
NVIC_ST_RELOAD_R = period - 1; // max reload value
NVIC_ST_CURRENT_R = 0; // any write to CURRENT clears
NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x40000000;
NVIC_ST_CTRL_R = 0x07; // enable SysTick with core clock
EnableInterrupts();
}

// Initialize edge trigger interrupt for PF0 (SW2) rising edge
void EdgeCounter_Init(void) {                          
SYSCTL_RCGC2_R |= 0x20; 
RisingEdges = 0;
GPIO_PORTF_LOCK_R = 0x4C4F434B;
GPIO_PORTF_DIR_R &= ~0x01; 				// 2) input on PF0
GPIO_PORTF_AFSEL_R &= ~0x01;
GPIO_PORTF_DEN_R |= 0x01;
GPIO_PORTF_PCTL_R &= ~0x0000000F;
GPIO_PORTF_AMSEL_R = 0;
GPIO_PORTF_PUR_R |= 0x10;
	
GPIO_PORTF_IS_R  &= ~0x01;    //edge-sensitive 
GPIO_PORTF_IBE_R &= ~0x01;    //not on both side
GPIO_PORTF_IEV_R &= 0x01;		 	//rising edge
GPIO_PORTF_ICR_R  = 0x01;			//clear flag4
GPIO_PORTF_IM_R  |= 0x01;			//arm interrupt on PF0
NVIC_PRI7_R = (NVIC_PRI7_R&0xFF00FFFF)|0x00A00000;
NVIC_EN0_R = 0x40000000;      // (h) enable interrupt 30 in NVIC
EnableInterrupts();	
}

// Handle GPIO Port F interrupts. When Port F interrupt triggers, do what's necessary then increment global variable RisingEdges
void GPIOPortF_Handler(void) {
GPIO_PORTF_ICR_R = 0x01;
RisingEdges = RisingEdges + 1;
}

// Handle SysTick generated interrupts. When timer interrupt triggers, do what's necessary then toggle red and blue LEDs at the same time
void SysTick_Handler(void) {
	GPIO_PORTF_DATA_R ^= 0x04;
	GPIO_PORTF_DATA_R ^= 0x02;
}
