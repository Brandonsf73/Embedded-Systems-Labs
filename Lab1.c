// Lab 1 : Flash LEDs
// Brandon Frateschi
// Unversity Central Florida
//
// For MSP430FR6989 LaunchPad.
// Toggles the LEDS of the board on and off depending on the MODE of operation

// Header file from Code composer studio
#include <msp430.h>
#include <stdint.h>

#define redLED BIT0
#define greenLED BIT7

/* Set the MODEs of operation
	MODE == 1, only flash the Red LED
	MODE == 2, Flash the Red and Green LED
	MODE == 3, Flash Red at twice the rate of the Green LED
*/
#define MODE = 0;

int main(void)
{
	
    volatile uint32_t i;
    volatile unsigned j;

    // Stop watchdog timer enable 
	WDTCTL = WDTPW | WDTHOLD; 
	PM5CTL0 &= ~LOCKLPM5;

	// Sets the Red LED to off and sets it output
	P1DIR |= redLED;
	P1OUT &= ~redLED;

	// Sets the Green LED to off and sets it output
	P9DIR |= greenLED;
	P9OUT &= ~greenLED;
	// Adding this will sync both of the LEDs up
	P9OUT ^= greenLED;

	// Prefig for MODE 3
	if(MODE == 2 || MODE ==3 )
		P9OUT |= greenLED;

	// Main loops that will run for the set proram
	while(MODE == 3)
	{
		// Delay loop
		for(i=0;i<120000;i++) {}

		// Flash the redLED
		P1OUT ^= redLED;

		// Flash the greenLED if MODE is 2 or 3
		if(MODE == 2 || MODE ==3) 
			P9OUT ^= greenLED;

		//Double the frequency of the red LED in MODE 3
		if(MODE == 3)
		{
			P1OUT ^= redLED;
			P9OUT ^= greenLED;
			for(i=0;i<20000;i++) {}
			P1OUT ^= redLED;
			for(i=0;i<20000;i++) {}
		}
	}
}
