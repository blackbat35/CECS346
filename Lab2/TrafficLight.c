// Documentation
// CECS346 Lab 2 - Traffic Light Controller (Simple)
// Description: ???
// Student: ???

// Input/Output:
//   PE2 - ???
//   PE1 - ???
//   PE0 - ???
//   PA3 - ???
//   PA2 - ???

// Preprocessor Directives
#include <stdint.h>
#include "tm4c123gh6pm.h"

// Global Variables
uint8_t In;
uint8_t Out;

// Function Prototypes - Each subroutine defined
void Delay(void);

int main(void) { 
	// Initialize GPIO on Ports A, E

	// Initial state: Red LED lit
  Out = ???;

	while(1) {
		Delay();
		
		In = ???; // Read value of west and south
		
		// Check the following conditions and set Out appropriately:
		//   If south is enabled and red LED is on, then red LED turns off and green LED turns on.
		//   If west is enabled and green LED is on, then green LED turns off and yellow LED turns on.
		//   If yellow LED is on, then yellow LED turns off and red LED turns on.
		Out = ???;
		
		??? = Out; // Update LEDs based on new value of Out
	}
}

// Subroutine to wait about 0.1 sec
// Inputs: None
// Outputs: None
// Notes: the Keil simulation runs slower than the real board
void Delay(void) {
	volatile uint32_t time;
  time = 727240*200/91*2;  // 0.1sec
  while(time) {
    time--;
  }
}
