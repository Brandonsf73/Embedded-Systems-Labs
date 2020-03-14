// Lab 3 : Using the Timer
// Brandon Frateschi
// Unversity Central Florida
//
// For MSP430FR6989 LaunchPad.
// Set a timer to control the falshing of the LEDs

// Header file from Code composer studio
#include <msp430fr6989.h>

#define redLED BIT0     //redLED at P1.0
#define greenLED BIT7   //greenLED at P9.7

//**********************************
// Configures ACLK to 32 KHz crystal
void config_ACLK_to_32KHz_crystal() 
{
	// By default, ACLK runs on LFMODCLK at 5MHz/128 = 39 KHz
	// Reroute pins to LFXIN/LFXOUT functionality
	PJSEL1 &= ~BIT4;
	PJSEL0 |= BIT4;

	// Wait until the oscillator fault flags remain cleared
	CSCTL0 = CSKEY; // Unlock CS registers

	do {
		CSCTL5 &= ~LFXTOFFG; // Local fault flag
		SFRIFG1 &= ~OFIFG; // Global fault flag
	} while((CSCTL5 & LFXTOFFG) != 0);

	CSCTL0_H = 0; // Lock CS registers

	return;
}

void main(void) 
{
	// Set the mode of operations
	// mode == 1 Continous Mode
	// mode == 2 Up Mode
	// mode == 3 self design Continous Mode, Binary Counter
	// mode == 4 self design Up Mode, Binary Counter
	volatile uint8_t mode;

	// Stop the Watchdog timer and enabl the GIPO pins
	WDTCTL = WDTPW | WDTHOLD; 
	PM5CTL0 &= ~LOCKLPM5; 

	// Configure the output pins and set LEDs to off
	P1DIR |= redLED; 
	P9DIR |= greenLED; 
	P1OUT &= ~redLED; 
	P9OUT &= ~greenLED; 

	// Configure ACLK to the 32 KHz crystal (function call)
	config_ACLK_to_32KHz_crystal() 

	// Configure Timer_A
	if(mode == 1 || mode == 3)
	{
		// Use ACLK, divide by 1, continuous mode, clear TAR
		TA0CTL = TASSEL_1 | ID_0 | MC_2 | TACLR;
	}
	else if(mode == 2 || mode == 4)
	{
		// Set timer period to one second
		TA0CCR0 = 32768;
		// Timer_A: ACLK, div by 1, up mode, clear TAR
		TA0CTL = TASSEL_1 | ID_0 | MC_1 | TACLR;

	}
	// Ensure flag is cleared at the start
	TA0CTL &= ~TAIFG;

	// Infinite loop for modes 1 and 2
	while(mode == 1 || mode == 3) 
	{
		// Empty while loop; waits here until TAIFG is raised
		while( (TA0CTL&TAIFG) == 0 ) {}

		P9OUT ^= greenLED;
		
		TA0CTL &= ~TAIFG // Clear the flag
	}

	// Infinite loop for modes 1 and 2
	while(mode == 2 || mode == 4) 
	{
		// Empty while loop; waits here until TAIFG is raised
		while( (TA0CTL&TAIFG) == 0 ) {}

		if(P1OUT&greenLED) == greenLED)
		{
			P1OUT ^= redLED;
		}
		P9OUT ^= greenLED;
		
		TA0CTL &= ~TAIFG // Clear the flag
	}
}
