// Lab 7 : Concurrency
// Brandon Frateschi
// University Central Florida
//
// For MSP430FR6989 LaunchPad.
// Use button and timer interrupt to control the status of an LED

// Header file from Code composer studio
#include <msp430fr6989.h>

#define redLED BIT0     // Red LED at P1.0
#define BUT1 BIT1       // Button 1 is at  Port 1.1

/*
Set the mode of operation
    MODE = 0; Buttons turns red LED on for exactly 3 seconds
    MODE = 1; Buttons turns red LED on and renews timer each time its pressed
    MODE = 2; De-bouncing of button 1
*/
#define MODE 2

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
    // Stop watchdog timer, and enable the GPIO pins
    WDTCTL = WDTPW | WDTHOLD;
    PM5CTL0 &= ~LOCKLPM5;

    // Set the output for both Red and Green LEDS, and set them to off
    P1DIR |= redLED;
    P1OUT &= ~redLED;

    // Configure ACLK to the 32 KHz crystal
    config_ACLK_to_32KHz_crystal();

    // Configure buttons
    P1DIR &= ~(BUT1);      // 0: input
    P1REN |= (BUT1);       // 1: enable built-in resistors
    P1OUT |= (BUT1);       // 1: built-in resistor is pulled up to Vcc
    P1IE |= (BUT1);        // 1: enable interrupts
    P1IES &= ~(BUT1);       // 1: interrupt on falling edge
    P1IFG &= ~(BUT1);      // 0: clear the interrupt flags

    _low_power_mode_3();
}

//**********************************
// Interrupt Service Routine for Timer0_A0
#pragma vector = TIMER0_A0_VECTOR
__interrupt void T0A0_ISR()
{
    if(MODE == 0 || MODE == 1)
    {
        // Turn off the LED
        P1OUT &= ~redLED;
    }
    // Check if the button is still pressed
    else if(MODE == 2 /*&& ( (P1IN & BUT1) == BUT1 )*/)
    {
        // Toggle red LED
        P1OUT ^= redLED;
    }

    // Configure Interrupts
    TA0CCTL0 &= ~CCIE;
    P1IE |= BUT1;

    // Clear and disable Timer
    //TA0CTL = TASSEL_1 | ID_1 | MC_0;

    // Clear interrupts
    TA0CCTL0 &= ~CCIFG;
    P1IFG &= ~BUT1;
}

//**********************************
// Interrupt Service Routine for Buttons
#pragma vector = PORT1_VECTOR
__interrupt void Port1_ISR()
{

    // Detect button 1 (BUT1 in P1IFG is 1)
    if ( MODE == 0 && (P1IFG & BUT1) == BUT1)
    {
        // Toggle the red LED and clear interrupt flag
        P1OUT |= redLED;

        // Use ACLK, divide by 2 to get the clock to 16 KHz, TAR cleared
        // Set upper limit for timer to 3 second (49,152 at 16 KHz)
        TA0CTL = TASSEL_1 | ID_1 | MC_1 | TACLR;
        TA0CCR0 = 49152-1;

        // Configure interrupts
        TA0CCTL0 |= CCIE;
        P1IE &= ~BUT1;

        // Clear interrupt flags
        TA0CCTL0 &= ~CCIFG;
        P1IFG &= ~BUT1;
    }
    else if(MODE == 1 && (P1IFG & BUT1) == BUT1)
    {
        // Toggle the red LED and clear interrupt flag
        P1OUT |= redLED;

        // Use ACLK, divide by 2 to get the clock to 16 KHz, TAR cleared
        // add 3 seconds to the current timer (49,152 at 16 KHz)
        TA0CTL = TASSEL_1 | ID_1 | MC_1 | TACLR;
        TA0CCR0 += 49152-1;

        // Configure interrupts
        TA0CCTL0 |= CCIE;

        // Clear interrupt flags
        TA0CCTL0 &= ~CCIFG;
        P1IFG &= ~BUT1;
    }
    else if(MODE == 2 && (P1IFG & BUT1) == BUT1)
    {
        // Use ACLK, clock left at 32 KHz, TAR cleared
        // Set upper limit for timer to 10 millisecond ( at 32 KHz)
        TA0CTL = TASSEL_1 | ID_0 | MC_1 | TACLR;
        TA0CCR0 = 3276-1;

        // Configure interrupts
        TA0CCTL0 |= CCIE;

        // Clear interrupt flags
        TA0CCTL0 &= ~CCIFG;
        P1IFG &= ~BUT1;
    }
}

