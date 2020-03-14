// Lab 2 : Using the push buttons
// Brandon Frateschi
// Unversity Central Florida
//
// For MSP430FR6989 LaunchPad.
// Make use of the buttons to control the LEDs

// Header file from Code composer studio
#include <msp430.h>

#define redLED BIT0     // redLED at P1.0
#define greenLED BIT7   // greenLED at P9.7
#define BUT1 BIT1       // Button S1 at P1.1
#define BUT2 BIT2       // Button S2 at P1.2

/* Sets the MODE of operations
	MODE = 1; Indepdent events, one button press does not affect the other
	MODE = 2; Mutually Exclusive, only one can be active at a time.
	MODE = 3; Mutually Inclusive, both have to be pressed at once.
*/
#define MODE 0;

int main(void)
{
	// Stop watchdog timer, and enable the GPIO pins
	WDTCTL = WDTPW | WDTHOLD; 
	PM5CTL0 &= ~LOCKLPM5;

	// Set pins for LEDs and set them to off
    P1DIR |= redLED;
    P9DIR |= greenLED;
    P1OUT &= ~redLED;
    P9OUT &= ~greenLED;

	// Configure Buttons
	// The buttons are set to be 0 when pressed in
	P1DIR &= ~(BUT1|BUT2);
	P1REN |= (BUT1|BUT2);
	P1OUT &= ~(BUT1|BUT2);

	// Independent, Red and Green are controlled seperately
	// Both can be active at once
	while(MODE == 1)
	{
		// Check for Button 1 to toggle the Red LED
		if( (P1IN&BUT1) == 0)
			P1OUT |= redLED;    
		else
			P1OUT &= ~redLED;

		// Check but button 2 to toggle The Green LED
		if(( P1IN&BUT2) == 0)
			P9OUT |= greenLED; 
		else
			P9OUT &= ~greenLED; 
	}

	// Mutually Exclusive, Only one can be on at a time
	while(MODE == 2)
	{
		// Check if button 1 is pressed
		// while it is, leave the Red LED on.
		while((P1IN&BUT1) == 0)
			P1OUT |= redLED;
		P1OUT &= ~redLED;

		// Check if button 2 is pressed
		// while it is, leave the Green LED on.
		while((P1IN&BUT2) == 0)
			P9OUT |= greenLED;
		P9OUT &= ~greenLED;
	}

	// Inclusive events, both buttons have to pressed to enable the LEDs
	while (MODE == 3)
	{
		if((P1IN&(BUT1|BUT2)) == 0)
		{
			P1OUT |= redLED;
			P9OUT |= greenLED;
		}
		else
		{
			P1OUT &= ~redLED;
			P9OUT &= ~greenLED;
		}
	}

	return 0;
}
