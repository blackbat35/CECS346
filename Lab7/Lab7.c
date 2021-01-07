#include <stdint.h> // C99 data types
#include "tm4c123gh6pm.h"
#define MOTOR  (*((volatile unsigned long *)0x4000503C)) // Port B
#define SWITCH (*((volatile unsigned long *)0x40025044)) // Port F

// Function Prototypes (from startup.s)
void DisableInterrupts(); // Disable interrupts
void EnableInterrupts();  // Enable interrupts
void WaitForInterrupt();  // Go to low power mode while waiting for the next interrupt

void EdgeCounter_Init();  // Initialize edge trigger interrupt for PE1-0 both edge
void Port_Init();					// Initialize Port B LEDs

void GPIOPortF_Handler(); // Handle GPIO Port F interrupts
void Delay();
void Stepper_CW();
void Stepper_CCW();
unsigned long In;
unsigned long Out;
unsigned int CW_byte[4] ={0x03, 0x06, 0x0C, 0x09};
//unsigned int CW_byte[8] = { 0x09, 0x01, 0x03, 0x02, 0x06, 0x04, 0x0C, 0x08 };
unsigned int CCW_byte[8] = { 0x08, 0x0C, 0x04, 0x06, 0x02, 0x03, 0x01, 0x09};

void Port_Init(){
SYSCTL_RCGC2_R   |= 0x00000022; // 1) enable F clock
// Port F
GPIO_PORTF_LOCK_R   = 0x4C4F434B; // 2) unlock PortF (for PF0)
GPIO_PORTF_CR_R 	 |= 0x01; 			// allow changes to PF0 (PF4-1 unlocked by default)
GPIO_PORTF_AMSEL_R &= ~0x11; 			// 3) disable analog function for PF4-0
GPIO_PORTF_PCTL_R  &= ~0x000F000F;// 4) GPIO clear PCTL bits for PF4-0
GPIO_PORTF_DIR_R 	 &= ~0x11; 			// 5) PF4,PF0 input

GPIO_PORTF_AFSEL_R &= ~0x11; 			// 6) no alternate function for PF4-0
GPIO_PORTF_PUR_R 	 |= 0x11; 			// 7) enable internal pull-up resistors on PF4,PF0
GPIO_PORTF_DEN_R 	 |= 0x11; 			// 8) enable digital pins PF4-PF0

// Port B
GPIO_PORTB_AMSEL_R 		&= ~0x0F; 			// 3) disable analog function on PB3-0
GPIO_PORTB_PCTL_R 		&= ~0x0000FFFF; // 4) enable regular GPIO
GPIO_PORTB_DIR_R		  |= 0x0F; 				// 5) outputs on PB3-0
GPIO_PORTB_AFSEL_R 		&= ~0x0F;			  // 6) regular function on PB3-0
GPIO_PORTB_DEN_R 			|= 0x0F; 				// 7) enable digital on PB3-0
}
/*
void EdgeCounter_Init(void) {   

// Port F
GPIO_PORTF_IS_R  &= ~0x11;    //edge-sensitive 
GPIO_PORTF_IBE_R &= ~0x11;    //not on both side
//GPIO_PORTF_IEV_R &= 0x11;		 	//rising edge
GPIO_PORTF_ICR_R  = 0x11;			//clear flag4
GPIO_PORTF_IM_R  |= 0x11;			//arm interrupt on PF0
NVIC_PRI7_R = (NVIC_PRI7_R&0xFF00FFFF)|0x00A00000;
NVIC_EN0_R = 0x40000000;      // (h) enable interrupt 30 in NVIC
EnableInterrupts();	
}
*/
// Clockwise Stepper Functions
void Stepper_CW(){
int q = 0;
for(q = 0; q < 4; q++){
	MOTOR = CW_byte[q];
	Delay(); 
	}
}
// Counter Clockwise Stepper functions
void Stepper_CCW(){
int q = 0;
for(q = 0; q < 8; q++){
	MOTOR = CCW_byte[q];
	Delay();
	}
}
/*
void GPIOPortF_Handler() {
	GPIO_PORTF_ICR_R = 0x11;      // acknowledge flag4	
	if ((GPIO_PORTF_DATA_R & 0x10) == 0x10){
		Stepper_CW();
	}
	else if ((GPIO_PORTF_DATA_R & 0x01) == 0x01){
		Stepper_CCW();
	}
	Delay();
}
*/
void Delay(void) {
	volatile uint32_t time;
  time = 727240*200/(91*20);  // 1sec
  while(time) {
    time--;
  }
}
int main(void) {
	//EdgeCounter_Init(); 
	Port_Init();	
	while(1) {
		if ((SWITCH&0x10) == 0x10){
			Stepper_CCW();
		}
		else if ((SWITCH&0x01) == 0x01){
			Stepper_CW();
		}
		//WaitForInterrupt();
	}
}
