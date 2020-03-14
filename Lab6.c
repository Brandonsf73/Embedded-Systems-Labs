// Lab 6 : Advanced Timer
// Brandon Frateschi
// University Central Florida
//
// For MSP430FR6989 LaunchPad.
// Using two timers to generate a PWM

// Header file from Code composer studio
#include <msp430fr6989.h>

#define redLED BIT0 // Red at P1.0
#define greenLED BIT7 // Green at P9.7
#define PWM_PIN BIT0

/*
MODE = 0
    Channel 0 toggles the red LED every 0.1 seconds
    Channel 1 toggles the green LED every 0.5 seconds
MODE = 1
    Channel 0 toggles the red LED every 0.1 seconds
    Channel 1 toggles the green LED every 0.5 seconds
    Channel 2 toggles Interrupts for Ch 0 and Ch 1 every 4 seconds
MODE = 2
    Generate PWM
*/
#define MODE 0

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
    // Enable GPIO pins and disable WDT
    WDTCTL = WDTPW | WDTHOLD;
    PM5CTL0 &= ~LOCKLPM5;

    // Configure LEDs
    P1DIR |= redLED;
    P9DIR |= greenLED;
    P1OUT &= ~redLED;
    P9OUT &= ~greenLED;

    // Activate the ACLK
    config_ACLK_to_32KHz_crystal();

    // Configure TimerA channels based on run mode
    if(MODE == 0)
    {
        // Configure interrupt times
        TA0CCR0 = 3277-1; // @ 32 KHz --> 0.1 seconds
        TA0CCR1 = 16385-1; // @ 32 KHz --> 0.5 seconds

        // Configure timer (ACLK) (divide by 1) (continuous mode)
        TA0CTL = TASSEL_1 | ID_0 | MC_2 | TACLR;
    }
    else if (MODE == 1)
    {
        // Configure interrupt times
        TA0CCR0 = 819-1; // @ 8 KHz --> 0.1 seconds
        TA0CCR1 = 4095-1; // @ 8 KHz --> 0.5 seconds
        TA0CCR2 = 32798-1; // @ 8KHz --> 4 seconds

        //Enable channel 2 interrupts
        TA0CCTL2 |= CCIE;
        TA0CCTL2 &= ~CCIFG;

        // Configure timer (ACLK) (divide by 4) (continuous mode)
        TA0CTL = TASSEL_1 | ID_2 | MC_2 | TACLR;
    }

    if(MODE == 0 || MODE == 1)
    {
        // Configure timer interrupts
        TA0CCTL0 |= CCIE;
        TA0CCTL0 &= ~CCIFG;
        TA0CCTL1 |= CCIE;
        TA0CCTL1 &= ~CCIFG;

        _enable_interrupts();
        while(MODE == 1) {}
    }

    if(MODE == 2 || MODE == 3)
    {
        // Divert pin to TA0.1 functionality
        P1DIR |= PWM_PIN; // P1DIR bit = 1
        P1SEL1 &= ~PWM_PIN; // P1SEL1 bit = 0
        P1SEL0 |= PWM_PIN; // P1SEL0 bit = 1

        // (ACLK @ 32 KHz) (Divide by 1) (Up mode)
        TA0CCR0 = (33-1); // @ 32 KHz --> 0.001 seconds (1000 Hz)
        TA0CTL = TASSEL_1 | ID_0 | MC_1 | TACLR;

        // Configuring Channel 1 for PWM
        // Output pattern: Reset/set
        TA0CCTL1 |= OUTMOD_7;

        // Modify this value between to change the bright ness value
        TA0CCR1 = 32;
    }

    if(MODE == 3)
    {
        // Configure Timer 1
        TA1CCR0 = 31768-1; // @ 32 KHz --> 1 second
        TA1CTL = TASSEL_1 | ID_0 | MC_1 | TACLR;
        TA1CCTL0 |= CCIE;
        TA1CCTL0 &= ~CCIFG;
        TA0CCR1 = 0;
    }
    _low_power_mode_3();
    return;
}

// ISR of Channel 0 (A0 vector)
#pragma vector = TIMER0_A0_VECTOR
__interrupt void T0A0_ISR()
{
    // Toggle the red LED
    P1OUT ^= redLED;

    // Schedule the next interrupt
    if(MODE == 0)
        TA0CCR0 += 3277;
    else if(MODE == 1)
        TA0CCR0 += 819;

    // Hardware clears Channel 0 flag
    TA0CCTL0 &= ~CCIFG;
}

// ISR of Channel 1 (A1 vector)
#pragma vector = TIMER0_A1_VECTOR
__interrupt void T0A1_ISR()
{
    if( (TA0CCTL1 & CCIFG) == CCIFG )
    {
        // Toggle the green LED
        P9OUT ^= greenLED;

        // Schedule the next interrupt
        if(MODE == 0)
            TA0CCR1 += 16385;
        else if(MODE == 1)
            TA0CCR1 += 4095;

        // Clear Channel 1 interrupt flag
        TA0CCTL1 &= ~CCIFG;
    }
    if( MODE == 1 && ((TA0CCTL2 & CCIFG) == CCIFG) )
    {
        // Turn off LEDs, if the
        P9OUT &= ~greenLED;
        P1OUT &= ~redLED;

        // Toggle the interrupts
        TA0CCTL1 ^= CCIE;
        TA0CCTL0 ^= CCIE;

        // Set the next interrupts
        TA0CCR0 = 819 + TA0CCR2;
        TA0CCR1 = 4095 + TA0CCR2;
        TA0CCR2 += 32798;

        // Clear Channel 2 interrupt flag
        TA0CCTL2 &= ~CCIFG;
    }
}

// ISR of Timer 1 Channel 0 (A0 vector)
#pragma vector = TIMER1_A0_VECTOR
__interrupt void T1A0_ISR()
{
    P9OUT ^= greenLED;
    TA0CCR1 += 5;
    // Increase the brightness level
    if(TA0CCR1 > 32)
        TA0CCR1 = 1;

    // Hardware clears Channel 0 flag
    TA1CCTL0 &= ~CCIFG;
}
