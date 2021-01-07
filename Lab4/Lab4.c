// LaunchPad built-in hardware
// SW1 left switch is negative logic PF4 on the Launchpad
// SW2 right switch is negative logic PF0 on the Launchpad
// red LED connected to PF1 on the Launchpad
// blue LED connected to PF2 on the Launchpad
// green LED connected to PF3 on the Launchpad

// 1. Pre-processor Directives Section
#include <stdint.h> // C99 data types
// Constant declarations to access port registers using 
// symbolic names instead of addresses
#include "tm4c123gh6pm.h"



// 2. Declarations Section
//   Global Variables
unsigned long In;  // input from PF4
unsigned long Out; // outputs to PF3,PF2,PF1 (multicolor LED)

//   Function Prototypes
void PortF_Init(void);
void Delay(void);
void SysTick_Init(void);
void SysTick_Wait(uint32_t);
// 3. Subroutines Section
// MAIN: Mandatory for a C Program to be executable
int main(void) {
  PortF_Init();        // Call initialization of port PF4 PF2
SysTick_Init();	
  while(1) {
    In = GPIO_PORTF_DATA_R&0x10; // read PF4 into In
    if (In == 0x00) {              // zero means SW1 is pressed
      GPIO_PORTF_DATA_R = 0x08;  // LED is green
    } else {                      // 0x10 means SW1 is not pressed
      GPIO_PORTF_DATA_R = 0x02;  // LED is red
    }
    SysTick_Wait(1600000);                     // wait 0.1 sec
    GPIO_PORTF_DATA_R = 0x04;    // LED is blue
    SysTick_Wait(1600000);                     // wait 0.1 sec
  }
}

// Subroutine to initialize port F pins for input and output
// PF4 and PF0 are input SW1 and SW2 respectively
// PF3,PF2,PF1 are outputs to the LED
// Inputs: None
// Outputs: None
// Notes: These five pins are connected to hardware on the LaunchPad
void PortF_Init(void) {
	uint32_t volatile delay;
  
	SYSCTL_RCGC2_R |= 0x00000020;     // 1) F clock
  delay = SYSCTL_RCGC2_R;           // delay   
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock PortF PF0  
  GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0       
  GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog function
  GPIO_PORTF_PCTL_R = 0x00000000;   // 4) GPIO clear bit PCTL  
  GPIO_PORTF_DIR_R = 0x0E;          // 5) PF4,PF0 input, PF3,PF2,PF1 output   
  GPIO_PORTF_AFSEL_R = 0x00;        // 6) no alternate function
  GPIO_PORTF_PUR_R = 0x11;          // enable pullup resistors on PF4,PF0       
  GPIO_PORTF_DEN_R = 0x1F;          // 7) enable digital pins PF4-PF0        
}
// Color    LED(s) PortF
// dark     ---    0
// red      R--    0x02
// blue     --B    0x04
// green    -G-    0x08
// yellow   RG-    0x0A
// sky blue -GB    0x0C
// white    RGB    0x0E
// pink     R-B    0x06

// Subroutine to wait about 0.1 sec
// Inputs: None
// Outputs: None
// Notes: the Keil simulation runs a little slower than the real board
void SysTick_Init() {
NVIC_ST_CTRL_R = 0x00; // disable SysTick during init
NVIC_ST_RELOAD_R = 0x00FFFFFF; // max reload value
NVIC_ST_CURRENT_R = 0; // any write to CURRENT clears
NVIC_ST_CTRL_R = 0x05; // enable SysTick with core clock
}
// The delay parameter is in units of the clock
void SysTick_Wait(uint32_t delay) {
NVIC_ST_RELOAD_R = delay - 1; // number of counts to wait
NVIC_ST_CURRENT_R = 0; // any value written to CURRENT clears
while ((NVIC_ST_CTRL_R & 0x00010000) == 0) { } // wait for COUNT
}
