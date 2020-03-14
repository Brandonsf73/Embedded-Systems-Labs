// Lab 4 : Interrupts
// Brandon Frateschi
// University Central Florida
//
// For MSP430FR6989 LaunchPad.
// Set a timer to control the flashing of the LED using Interrupts

// Header file from Code composer studio
#include <msp430fr6989.h>

#define redLED BIT0     // Red LED at P1.0
#define greenLED BIT7   // Green LED at P9.7
#define BUT1 BIT1       // Button 1 is at  Port 1.1
#define BUT2 BIT2       // Button 2 is at  Port 1.2

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
    // Set the mode of operation
    // mode = 0; Use continuous mode, and the interrupt flag TAIFG
    // mode = 1; Use Up mode, and the interrupt flag CCIFG
    // mode = 2; Use the buttons to trigger interrupts
    // mode = 3; Low Power mode, Continuous Mode
    // mode = 4; Low Power mode, Up Mode
    // mode = 5; Low Power mode, Button
    const int mode = 3;

    // Stop watchdog timer, and enable the GPIO pins
    WDTCTL = WDTPW | WDTHOLD;
    PM5CTL0 &= ~LOCKLPM5;

    // Set the output for both Red and Green LEDS, and set them to off
    P1DIR |= redLED;
    P9DIR |= greenLED;
    P1OUT &= ~redLED;

    if(mode == 1 || mode == 4)
        P9OUT |= greenLED;
    else
        P9OUT &= ~greenLED;

    // Configuration for each running mode
    if(mode == 0 || mode == 1 || mode == 3 || mode == 4)
    {
        // Configure ACLK to the 32 KHz crystal
        config_ACLK_to_32KHz_crystal();

        if(mode == 0 || mode == 3)
        {
            // Use ACLK, divide by 1, continuous mode, TAR cleared, enable interrupt
            TA0CTL = TASSEL_1 | ID_0 | MC_2 | TACLR | TAIE;
            TA0CTL &= ~TAIFG;
        }
        else if(mode == 1 || mode == 4)
        {
            // Use ACLK, divide by 1, up mode, TAR cleared
            TA0CTL = TASSEL_1 | ID_0 | MC_1 | TACLR;

            // Set upper limit for timer to 1 second and configure interrupts
            TA0CCR0 = 32768 - 1;
            TA0CCTL0 |= CCIE;
            TA0CCTL0 &= ~CCIFG;
        }
    }
    else if(mode == 2 || mode == 5)
    {
        // Configure buttons
        P1DIR &= ~(BUT1|BUT2);      // 0: input
        P1REN |= (BUT1|BUT2);       // 1: enable built-in resistors
        P1OUT |= (BUT1|BUT2);       // 1: built-in resistor is pulled up to Vcc
        P1IE |= (BUT1|BUT2);        // 1: enable interrupts
        P1IES |= (BUT1|BUT2);       // 1: interrupt on falling edge
        P1IFG &= ~(BUT1|BUT2);      // 0: clear the interrupt flags
    }

    if(mode <= 2)
    {
        // Enable the global interrupts
        _enable_interrupts();

        // Infinite run loop for the main program
        while(1) {}
    }
    else if (mode >= 3)
    {
        _low_power_mode_3();
    }
}

//**********************************
// Interrupt Service Routine for Timer0_A1
#pragma vector = TIMER0_A1_VECTOR 
__interrupt void T0A1_ISR()
{
    // Toggle both LEDs and Clear the TAIFG flag
    P1OUT ^= redLED;
    P9OUT ^= greenLED;
    TA0CTL &= ~TAIFG;
}

//**********************************
// Interrupt Service Routine for Timer0_A0
#pragma vector = TIMER0_A0_VECTOR
__interrupt void T0A0_ISR()
{
    // Toggle the LEDs
    P1OUT ^= redLED;
    P9OUT ^= greenLED;
}

//**********************************
// Interrupt Service Routine for Buttons
#pragma vector = PORT1_VECTOR 
__interrupt void Port1_ISR()
{
    // Detect button 1 (BUT1 in P1IFG is 1)
    if ( (P1IFG&BUT1) == BUT1 )
    {
        // Toggle the red LED and clear interrupt flag
        P1OUT ^= redLED;
        P1IFG &= ~BUT1;
    }
    // Detect button 2 (BUT2 in P1IFG is 1)
    if ( (P1IFG&BUT2) == BUT2 )
    {
        // Toggle the green LED and clear interrupt flag
        P9OUT ^= greenLED;
        P1IFG &= ~BUT2;
    }
}

