// Lab 5 : LCD
// Brandon Frateschi
// University Central Florida
//
// For MSP430FR6989 LaunchPad.
// Displaying LCD numbers

// Header file from Code composer studio
#include <msp430fr6989.h>

#define redLED BIT0     // Red LED at P1.0
#define greenLED BIT7   // Green LED at P9.7
#define BUT1 BIT1
#define BUT2 BIT2

/* Set the MODE of operation
    MODE = 0; Display 430
    MODE = 1; Display n
    MODE = 2; Stopwatch
    MODE = 3; Stopwatch with Halt and Reset
*/
#define MODE 0

// The LCD screen lights up the digits based on a hexademical number
// This tells the LCD how to display each of the numbers on a digital clock
const unsigned char LCD_Num[10] = {0xFC, 0x60, 0xDB, 0xF1, 0x67, 0xB7, 0xBF, 0xE0, 0xFF, 0xF7};
unsigned int n = 0;

//**********************************************************
// Initializes the LCD_C module
// *** Source: Function obtained from MSP430FR6989’s Sample Code ***
void Initialize_LCD() 
{
    PJSEL0 = BIT4 | BIT5; // For LFXT

    // Initialize LCD segments 0 - 21; 26 - 43
    LCDCPCTL0 = 0xFFFF;
    LCDCPCTL1 = 0xFC3F;
    LCDCPCTL2 = 0x0FFF;

    // Configure LFXT 32kHz crystal
    CSCTL0_H = CSKEY >> 8; // Unlock CS registers
    CSCTL4 &= ~LFXTOFF; // Enable LFXT

    do {
        CSCTL5 &= ~LFXTOFFG; // Clear LFXT fault flag
        SFRIFG1 &= ~OFIFG;
    }while (SFRIFG1 & OFIFG); // Test oscillator fault flag

    CSCTL0_H = 0; // Lock CS registers

    // Initialize LCD_C
    // ACLK, Divider = 1, Pre-divider = 16; 4-pin MUX
    LCDCCTL0 = LCDDIV__1 | LCDPRE__16 | LCD4MUX | LCDLP;

    // VLCD generated internally,
    // V2-V4 generated internally, v5 to ground
    // Set VLCD voltage to 2.60v
    // Enable charge pump and select internal reference for it
    LCDCVCTL = VLCD_1 | VLCDREF_0 | LCDCPEN;
    LCDCCPCTL = LCDCPCLKSYNC; // Clock synchronization enabled
    LCDCMEMCTL = LCDCLRM; // Clear LCD memory

    //Turn LCD on
    LCDCCTL0 |= LCDON;

    return;
}

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

//**********************************
// Display the n value on the LCD
void display_num_lcd(unsigned int n)
{
    clear_num_lcd();
    if(n >= 0)
        LCDM8 = LCD_Num[n%10];

    if(n >= 10)
        LCDM15 = LCD_Num[(n/10)%10];
    else
        return;
    if(n >= 100)
        LCDM19 = LCD_Num[(n/100)%10];
    else
        return;
    if(n >= 1000)
        LCDM4 = LCD_Num[(n/1000)%10];
    else
        return;
    if(n >= 10000)
        LCDM6 = LCD_Num[(n/10000)%10];
    else
        return;
    if(n >= 100000)
        LCDM10 = LCD_Num[(n/100000)%10];
    else
        return;
}

//************************
// Clears the LCD display
void clear_num_lcd()
{
    LCDCMEMCTL = LCDCLRM;
}

int main(void)
{
    volatile int i = 0;
    WDTCTL = WDTPW | WDTHOLD; // Stop WDT
    PM5CTL0 &= ~LOCKLPM5; // Enable GPIO pins

    P1DIR |= redLED; // Pins as output
    P9DIR |= greenLED;
    P1OUT |= redLED; // Red on
    P9OUT &= ~greenLED; // Green off

    // Initializes the LCD_C module
    Initialize_LCD();

    // The line below can be used to clear all the segments
    LCDCMEMCTL = LCDCLRM;

    // Display 430 on the rightmost three digits
    if(MODE == 0)
        display_num_lcd(430);

    //Display the max 16 bit integer value
    else if(MODE == 1)
        display_num_lcd(65535);
    else if(MODE == 2 || MODE == 3)
    {
        // Configure the LCD screen
        config_ACLK_to_32KHz_crystal();
        TA0CTL = TASSEL_1 | ID_0 | MC_1 | TACLR;
        TA0CCR0 = 32768 - 1;
        TA0CCTL0 |= CCIE;
        TA0CCTL0 &= ~CCIFG;
        display_num_lcd(0);

        // Enable the buttons to be used to control the Timer
        if(MODE == 3)
        {
            P1DIR &= ~(BUT1|BUT2);      // 0: input
            P1REN |= (BUT1|BUT2);       // 1: enable built-in resistors
            P1OUT |= (BUT1|BUT2);       // 1: built-in resistor is pulled up to Vcc
            P1IE |= (BUT1|BUT2);        // 1: enable interrupts
            P1IES |= (BUT1|BUT2);       // 1: interrupt on falling edge
            P1IFG &= ~(BUT1|BUT2);      // 0: clear the interrupt flags
        }
    }

    if(MODE >= 2)
    {
        _low_power_mode_3();
    }
    else
    {
        for(;;)
        {
            for(i=0; i<=50000; i++) {} // Delay loop
            P1OUT ^= redLED;
            P9OUT ^= greenLED;
        }
    }

    return 0;
}

//**********************************
// Interrupt Service Routine for Timer0_A1
#pragma vector = TIMER0_A0_VECTOR // Link the ISR to the vector
__interrupt void T0A0_ISR()
{
    display_num_lcd(n++);

    P1OUT ^= redLED;
    P9OUT ^= greenLED;

    // Clear the interrupt flag
    TA0CTL &= ~TAIFG;
}

//**********************************
// Interrupt Service Routine for Buttons
#pragma vector = PORT1_VECTOR 
__interrupt void Port1_ISR()
{
    static int enable = 1;
    volatile int i = 0;

    // Detect button 1
    // Stop the timer
    if ( (P1IFG&BUT1) == BUT1 )
    {
        if(enable == 1)
        {
            TA0CCTL0 &= ~CCIE;
            enable = 0;
        }
        else if (enable == 0)
        {
            TA0CCTL0 |= CCIE;
            TA0CTL |= TACLR;
            enable = 1;
        }
        for(i = 0; i <= 15000; i++) {}
        P1IFG &= ~BUT1;
    }
    
    // Detect button 2
    // Set the LCD to display 0 and leave the flag to be cleared by the tier routine
    if ( (P1IFG&BUT2) == BUT2 )
    {
        n = 0;
        clear_num_lcd();
        display_num_lcd(n++);
        P1IFG &= ~BUT2;
    }
}
