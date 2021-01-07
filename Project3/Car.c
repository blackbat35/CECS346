#include <stdint.h> 		// C99 data types
#include "tm4c123gh6pm.h"
#define MOTOR 		(*((volatile unsigned long *)0x400053FC)) // Port B
#define BUTTON 		(*((volatile unsigned long *)0x4002400C)) // Port E
#define IR_SENSOR (*((volatile unsigned long *)0x40004010)) // Port A

// Function Prototypes (from startup.s)
void DisableInterrupts(); // Disable interrupts
void EnableInterrupts();  // Enable interrupts
void WaitForInterrupt();  // Go to low power mode while waiting for the next interrupt

// Function Prototypes
void Port_Init();															// Initialize Port B LEDs
void SysTick_Init(unsigned long period);      // Initialize SysTick timer for 0.05s delay with interrupt enabled
void SysTick_Handler();   										// Handle SysTick generated interrupts

// Linked data structure
struct State {
unsigned long Motor;
unsigned long Time;
unsigned long next[8];
};
typedef const struct State Styp;
#define Phase0     			  0
#define Phase1		   			1
#define Phase2					  2
#define Phase3						3
#define Phase4						4

Styp FSM[5] = {
{0x33, 3E4,	{0, 0, 0, 0, 0, 3, 1, 0}},  // 1
{0x66, 3E4, {1, 1, 1, 1, 1, 0, 2, 1}},  // 2
{0xCC, 3E4,	{2, 2, 2, 2, 2, 1, 3, 2}},	// 3
{0x99, 3E4,	{3, 3, 3, 3, 3, 2, 0, 3}},	// 4
{0x00, 3E4, {4, 4, 4, 4, 4, 4, 4, 4}}   // 5
};	

unsigned long S, Input; // index to the current state
int step_remaining;

// Initialize Port A, B, E
void Port_Init(){//volatile unsigned long delay) {
SYSCTL_RCGC2_R |= 0x13; 							// 1) A, B, E clock
// Port B
GPIO_PORTB_AMSEL_R 		&= ~0xFF; 			// 3) disable analog function on PB3-0
GPIO_PORTB_PCTL_R 		&= ~0xFFFFFFFF; // 4) enable regular GPIO
GPIO_PORTB_DIR_R		  |= 0xFF; 				// 5) outputs on PB3-0
GPIO_PORTB_AFSEL_R 		&= ~0xFF;			  // 6) regular function on PB3-0
GPIO_PORTB_DEN_R 			|= 0xFF; 				// 7) enable digital on PB3-0	

// Port E
GPIO_PORTE_CR_R 			|= 0x03;
GPIO_PORTE_AMSEL_R 		&= ~0x03; 			// 3) disable analog function on PE1-0
GPIO_PORTE_PCTL_R 		&= ~0x000000FF; // 4) enable regular GPIO
GPIO_PORTE_DIR_R		  &= ~0x03; 			// 5) inputs on PE1-0
GPIO_PORTE_AFSEL_R 		&= ~0x03;			  // 6) regular function on PE1-0
GPIO_PORTE_DEN_R 			|= 0x03; 				// 7) enable digital on PE1-0	


// Port A
GPIO_PORTA_CR_R       |= 0x04;				
GPIO_PORTA_AMSEL_R 		&= ~0x04;				// 3) disable analog function on PA2
GPIO_PORTA_PCTL_R 		&= ~0x00000F00;	// 4) enable regular GPIO
GPIO_PORTA_DIR_R 			&= ~0x04; 			// 5) inputs in PA2
GPIO_PORTA_AFSEL_R 		&= ~0x04;				// 6) regular funtion on PA2
GPIO_PORTA_DEN_R 			|= 0x04;				// 7) enabler digital on PA2
}

// Initialize SysTick timer with interrupt enabled
void SysTick_Init(unsigned long period) {
NVIC_ST_CTRL_R 		= 0x00; 							// disable SysTick during init
NVIC_ST_RELOAD_R  = period - 1; 				// max reload value
NVIC_ST_CURRENT_R = 0x00; 							// any write to CURRENT clears
NVIC_SYS_PRI3_R 	= (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x60000000;  // priority 3
NVIC_ST_CTRL_R 		= 0x07; 							// enable SysTick with core clock
EnableInterrupts();
}

// Handle SysTick generated interrupts
void SysTick_Handler() {
	S = FSM[S].next[Input]; 				//changing state
	MOTOR = FSM[S].Motor & 0xFF;  	//output of state
	if ((Input & 0x06) == 0x06)
		step_remaining++;	
	else if ((Input & 0x05) == 0x05)
		step_remaining--;
	else if ((Input & 0x02) == 0x02)
	{
		S = Phase4; 
		MOTOR = FSM[S].Motor & 0xFF;
	}
	if((step_remaining == 11000) || (step_remaining == 1500)){
		//WaitForInterrupt();
		S = Phase4; 
		MOTOR = FSM[S].Motor & 0xFF;
	}
}

int main(void){							
	Port_Init();
	step_remaining = 2000;
	while(1){
				SysTick_Init(FSM[S].Time);
				Input = IR_SENSOR + BUTTON ; // read sensors	
				WaitForInterrupt();
	}
}
