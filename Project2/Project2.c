#include <stdint.h> // C99 data types
#include "tm4c123gh6pm.h"
#define LIGHT2 (*((volatile unsigned long *)0x400053FC)) // Port B
#define SENSOR (*((volatile unsigned long *)0x4002401C)) // Port E
#define RESET  (*((volatile unsigned long *)0x40004080)) // Port A

// Function Prototypes (from startup.s)
void DisableInterrupts(); 			// Disable interrupts
void EnableInterrupts();  			// Enable interrupts
void WaitForInterrupt(); 			 // Go to low power mode while waiting for the next interrupt

// Function Prototypes
void EdgeCounter_Init();  		              // Initialize edge trigger interrupt for PE1-0 both edge
void Port_Init();				// Initialize Port B LEDs
void SysTick_Init(unsigned long period);        // Initialize SysTick timer for 0.05s delay with interrupt enabled
void Delay();
void GPIOPortA_Handler(); 			// Handle GPIO Port A interrupts
void GPIOPortE_Handler();		  	 // Handle GPIO Port E interrupts
void SysTick_Handler();   			// Handle SysTick generated interrupts

// Linked data structure
struct State {
unsigned long Light2;
unsigned long Time;
unsigned long Next[4];
};
typedef const struct State Styp;
#define Initialize      			0
#define Wait_for_Staging     		1 
#define Count_down_Y1   		2
#define Count_down_Y2		3
#define Go				4
#define False_Start_Left	    	5
#define False_Start_Right		6
#define False_Start_Both		7
#define Win_Left			8
#define Win_Right			9
#define Win_Both			10

Styp FSM[11] = {
{0xFF, 15999999, 	{1,  1, 1, 1}},     // 0
{0x00, 7999999, 	{1,  1, 1, 2}}, 	// 1
{0x88, 7999999,  	{7,  6, 5, 3}}, 	// 2
{0x44, 7999999, 	{7,  6, 5, 4}}, 	// 3
{0x22, 799999,  	{10, 9, 8, 4}}, 	// 4
{0x10, 15999999,  	{1,  1, 1, 1}}, 	// 5
{0x01, 15999999,  	{1,  1, 1, 1}}, 	// 6
{0x11, 15999999,  	{1,  1, 1, 1}}, 	// 7
{0x20, 15999999,  	{1,  1, 1, 1}}, 	// 8
{0x02, 15999999,  	{1,  1, 1, 1}}, 	// 9
{0x22, 15999999, 	{1,  1, 1, 1}}};	// 10

unsigned long S; // index to the current state
unsigned long Input, tick;

int main(void){
	Port_Init();							
	EdgeCounter_Init();           // initialize GPIO Port A, E interrupt
	S = False_Start_Both;
	while(1){
				LIGHT2 = FSM[S].Light2;
				SysTick_Init(FSM[S].Time);
				Delay();
				WaitForInterrupt();
				Input = SENSOR;
				S = FSM[S].Next[Input];		
	}
}
// Initialize Port A, B, E
void Port_Init(){//volatile unsigned long delay) {
SYSCTL_RCGC2_R |= 0x13; 						// 1) A, B, E clock
// Port B
GPIO_PORTB_AMSEL_R 		&= ~0xFF; 			// 3) disable analog function on PB3-0
GPIO_PORTB_PCTL_R 		&= ~0xFFFFFFFF; 		// 4) enable regular GPIO
GPIO_PORTB_DIR_R		  	|= 0xFF; 		      	// 5) outputs on PB3-0
GPIO_PORTB_AFSEL_R 		&= ~0xFF;			  // 6) regular function on PB3-0
GPIO_PORTB_DEN_R 			|= 0xFF; 			  // 7) enable digital on PB3-0	

// Port E
GPIO_PORTE_CR_R 			|= 0x03;
GPIO_PORTE_AMSEL_R 		&= ~0x03; 			// 3) disable analog function on PE1-0
GPIO_PORTE_PCTL_R 			&= ~0x000000FF;		// 4) enable regular GPIO
GPIO_PORTE_DIR_R		 	 &= ~0x03; 			// 5) inputs on PE1-0
GPIO_PORTE_AFSEL_R 		&= ~0x03;			// 6) regular function on PE1-0
GPIO_PORTE_DEN_R 			|= 0x03; 			// 7) enable digital on PE1-0	


// Port A
GPIO_PORTA_CR_R       		|= 0x20;				
GPIO_PORTA_AMSEL_R 		&= ~0x20;			// 3) disable analog function on PA5
GPIO_PORTA_PCTL_R 		&= ~0x00F00000;		// 4) enable regular GPIO
GPIO_PORTA_DIR_R 			&= ~0x20; 			// 5) inputs in PA5
GPIO_PORTA_AFSEL_R 		&= ~0x20;			// 6) regular function on PA5
GPIO_PORTA_DEN_R 			|= 0x20;			// 7) enabler digital on PA5
}

// Initialize SysTick timer for 0.05s delay with interrupt enabled
void SysTick_Init(unsigned long period) {
NVIC_ST_CTRL_R = 0x00; 						   // disable SysTick during init
NVIC_ST_RELOAD_R = period; 					    // max reload value
NVIC_ST_CURRENT_R = 0x00; 					   // any write to CURRENT clears
NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x60000000;  // priority 3
NVIC_ST_CTRL_R = 0x07; 						   // enable SysTick with core clock
EnableInterrupts();
}
// Initialize edge trigger interrupt for PE1-0 both edge
void EdgeCounter_Init(void) {   

// Port E
GPIO_PORTE_IS_R  			&= ~0x03;    				//edge-sensitive 
GPIO_PORTE_IBE_R 			&= 0x03;    				//on both side
GPIO_PORTE_ICR_R  			 = 0x03;				//clear flag4
GPIO_PORTE_IM_R  			|= 0x03;				//arm interrupt on PE1-0
NVIC_PRI1_R 				  = (NVIC_PRI1_R&0xFFFFFF00)|0x00000040; // priority 2

// Port A
GPIO_PORTA_IS_R  			&= ~0x20;    				//edge-sensitive 
GPIO_PORTA_IBE_R 			&= ~0x20;    				//not on both side
GPIO_PORTA_ICR_R  			 = 0x20;				//clear flag4
GPIO_PORTA_IM_R  			|= 0x20;				//arm interrupt on PA5
NVIC_PRI0_R 		  		 = (NVIC_PRI0_R&0xFFFFFF00)|0x00000020; // priority 1

NVIC_EN0_R 			 	|= 0x00000011;      		// (h) enable interrupt in NVIC for port A, E
	
EnableInterrupts();	
}
// Handle GPIO Port E interrupts. When Port E interrupt triggers, do what's necessary then increment global variable both Edges
void GPIOPortE_Handler() {
	GPIO_PORTE_ICR_R = 0x03;      // acknowledge flag4
}

void Delay(void) {
  volatile uint32_t time;
  time = 727240*200/(91*4);  // 1sec
  while(time) {
    time--;
  }
}

// Handle GPIO Port A interrupts. When Port A interrupt triggers, do what's necessary then increment global variable both Edges
void GPIOPortA_Handler() {
	GPIO_PORTA_ICR_R = 0x20;      // acknowledge flag4
	S = Initialize;
	LIGHT2 = FSM[S].Light2;
	Delay();
}

// Handle SysTick generated interrupts. When timer interrupt triggers, do what's necessary then toggle red and blue LEDs at the same time
void SysTick_Handler() {
	tick++;
}
