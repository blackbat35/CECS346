// Input/Output:
//   PE2 - ???
//   PE1 - ???
//   PE0 - ???
//   PA3 - ???
//   PA2 - ???

#include <stdint.h>
#include "tm4c123gh6pm.h"
#define SENSOR_PORTA (*((volatile unsigned long *)0x40004030))
#define SENSOR_PORTF (*((volatile unsigned long *)0x40025040))
#define LIGHT_PORTB  (*((volatile unsigned long *)0x400050FC))
#define LIGHT_PORTF  (*((volatile unsigned long *)0x40025028))
// Linked data structure
//void SysTick_Init(void);                         // initialize SysTick Timer
//void SysTick_Wait(unsigned long delay1);
//void SysTick_Wait10ms(unsigned long delay1);     // waiting time
struct State {
unsigned long TrafficLights_Cars;
unsigned long TrafficLights_Right_Turn;
unsigned long Time;
unsigned long Next[8];};
typedef const struct State STyp;
#define Go_West        		0	
#define Wait_West      		1	 
#define Go_South   		2
#define Wait_South  	 	3	
#define Go_South_Rt    		4
#define Wait_South_Rt   	5
#define Go_Rt			6
#define Wait_Rt		7
#define Wait_South_Rt_2  	8
#define Go_South_Wait_Rt  	9
STyp FSM[10]={
{0x0C, 0x02, 6, {0, 0, 1, 1, 1, 1, 1, 1}}, // Stage 1
{0x14, 0x02, 2, {2, 2, 2, 2, 6, 6, 4, 4}}, // Stage 2
{0x21, 0x02, 6, {2, 3, 2, 3, 4, 3, 4, 3}}, // Stage 3
{0x22, 0x02, 2, {0, 0, 0, 0, 6, 0, 6, 0}}, // Stage 4
{0x21, 0x08, 6, {4, 8, 9, 8, 4, 8, 4, 8}}, // Stage 5
{0x21, 0x0A, 2, {2, 3, 2, 3, 2, 3, 2, 4}}, // Stage 6
{0x24, 0x08, 6, {6, 7, 4, 7, 6, 7, 7, 7}}, // Stage 7
{0x24, 0x0A, 2, {0, 0, 2, 0, 0, 0, 2, 0}}, // Stage 8
{0x22, 0x0A, 2, {0, 0, 0, 0, 0, 0, 0, 0}}, // Stage 9
{0x21, 0x0A, 2, {2, 3, 2, 2, 2, 3, 2, 3}}};// Stage 10
unsigned long S; // index to the current state
unsigned long Input1, Input2;
void Delay(int x);
int main(void){ volatile unsigned long delay;

SYSCTL_RCGC2_R |= 0x23; // 1) A B F (01 + 02 + 20)
delay = SYSCTL_RCGC2_R; // 2) no need to unlock

	// A Port: Input(South_West)
GPIO_PORTA_AMSEL_R &= ~0x0C; // 3) disable analog function on PA3-2v
GPIO_PORTA_PCTL_R &= ~0x0000FF00; // 4) enable regular GPIO
GPIO_PORTA_DIR_R &= ~0x0C; // 5) inputs on PA3-2
GPIO_PORTA_AFSEL_R &= ~0x0C; // 6) regular function on PA3-2
GPIO_PORTA_DEN_R |= 0x0C; // 7) enable digital on PA3-2

	// B Port: Output(West - South)
GPIO_PORTB_AMSEL_R &= ~0x3F; // 3) disable analog function on PB5-0
GPIO_PORTB_PCTL_R &= ~0x00FFFFFF; // 4) enable regular GPIO
GPIO_PORTB_DIR_R |= 0x3F; // 5) outputs on PB2-0
GPIO_PORTB_AFSEL_R &= ~0x3F; // 6) regular function on PB5-0
GPIO_PORTB_DEN_R |= 0x3F; // 7) enable digital on PB5-0

	// F Port: Output(Right_turn)
GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock PortF PF0  
GPIO_PORTF_AMSEL_R &= ~0x1A; // 3) disable analog function on PF4,3,1
GPIO_PORTF_PCTL_R &= ~0x000FF0F0; // 4) enable regular GPIO
GPIO_PORTF_DIR_R |= 0x0A; // 5) outputs on PF3, PF1
GPIO_PORTF_DIR_R &= ~0x10; // 5) input on PF4
GPIO_PORTF_AFSEL_R &= ~0x1A; // 6) regular function on PF4,3,1
GPIO_PORTF_PUR_R |= 0x10;         // enable pullup resistors on PF4  
GPIO_PORTF_DEN_R |= 0x1A; // 7) enable digital on PF4,3,1 
	
	
	S = Go_West;

	while(1){
					LIGHT_PORTB = FSM[S].TrafficLights_Cars; // set lights
					LIGHT_PORTF = FSM[S].TrafficLights_Right_Turn; // set lights
					Delay(FSM[S].Time);
					//SysTick_Wait10ms(FSM[S].Time);
					Input1 = (SENSOR_PORTA >> 2) + ((SENSOR_PORTF ^0x10) >> 2); // read sensors
					S = FSM[S].Next[Input1];
	}
}
/*
LIGHT_PORTB = FSM[CS].Out & 0x3F; //set lights PB
		LIGHT_PORTF = (FSM[CS].Out & 0xA00) >> 8; //set lights PF
		delay10ms(FSM[CS].Time);
		Input = (SENSOR_PORTA >> 2) + (SENSOR_PORTF >> 1); //read switches
		CS = FSM[CS].Next[Input];
*/
// Subroutine to wait about 0.1 sec
// Inputs: None
// Outputs: None
// Notes: the Keil simulation runs slower than the real board
void Delay(int x) {
	volatile uint32_t time;
  time = x*727240*200/(91*2);  // 1sec
  while(time) {
    time--;
  }
}

