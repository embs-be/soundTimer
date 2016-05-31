/*
 * File:   main.c
 * Author: slava
 *
 * Created on May 8, 2016, 7:17 PM
 */
#define _XTAL_FREQ 125000

// CONFIG
#pragma config FOSC = INTOSCIO  // Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA4/OSC2/CLKOUT pin, I/O function on RA5/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // MCLR Pin Function Select bit (MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Detect (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)

#include <xc.h>

typedef void (*Handler)(void);

void waitForKeyPress(void);
void waitForKeyRelease(void);

Handler handler = waitForKeyPress;
unsigned char keyPressed;

void beep() {
    unsigned char i;

    for (i = 0; i < 40; i++) {
        GP4 = 1;
        __delay_us(100);
        GP4 = 0;
    }
}

unsigned char msec_count = 0;
unsigned char sec_count = 0;
unsigned char min_count = 0;
unsigned char blip_count = 0;
unsigned char beep_count = 0;

unsigned char bp = 0;

void timerOff(void) {

    if (keyPressed) {
        waitForKeyRelease();
    }

    handler = waitForKeyPress;
    INTCONbits.INTF = 0; // clear the interrupt flag
    INTCONbits.INTE = 1; // enable the external interrupt
}

void mainTimer(void) {

    if (keyPressed) {
        timerOff();
    }

    if (bp) {
        bp = 0;
        msec_count++;
        if (msec_count > 4) {
            msec_count = 0;
            sec_count++;
            if (sec_count > 59) {
                sec_count = 0;
                min_count++;

                if (min_count > 2) {
                    beep_count = 9;
                } else {
                    beep_count = min_count;
                }
            }
            blip_count = min_count + 1;
        }

        if (blip_count > 0) {
            blip_count--;
            GP5 = 0;
            __delay_ms(10);
            GP5 = 1;
        }

        if (beep_count > 0) {
            beep_count--;
            beep();
            if ((min_count > 2) && (beep_count == 0)) {
                timerOff();
            }
        }
    }
}

void keyReleased(void) {
    GP5 = 1;

    handler = mainTimer;

    msec_count = 0;
    sec_count = 0;
    min_count = 0;
    blip_count = 0;
    beep_count = 0;

    INTCONbits.INTF = 0; // clear the interrupt flag
    INTCONbits.INTE = 1; // enable the external interrupt

    keyPressed = 0;
}

void waitForKeyRelease(void) {
    unsigned char keyBuf;

    keyBuf = 0xff;

    while (keyBuf) {
        keyBuf <<= 1;
        if (GP2 == 0) keyBuf++;
        __delay_us(10);
    }

}

void waitForKeyPress(void) {
    SLEEP();
    GP5 = 0; //LED on
    waitForKeyRelease();
    keyReleased();
}

void interrupt tc_int(void) { // interrupt function
    if (INTCONbits.INTF) { // if timer flag is set & interrupt enabled
        INTCONbits.INTE = 0; // disable the external interrupt
        INTCONbits.INTF = 0; // clear the interrupt flag
        keyPressed = 1;
    }
    // Timer1 Interrupt
    if (PIR1bits.TMR1IF == 1) { // timer 1 interrupt flag
        PIR1bits.TMR1IF = 0; // interrupt must be cleared by software
        TMR1H = 231; // preset for timer1 MSB register 200ms
        TMR1L = 144; // preset for timer1 LSB register
        bp = 1;
    }
}

void main(void) {
    CMCON0 = 7;
    ANSELbits.ANS = 0;
    ADCON0bits.ADON = 0;

    OSCCONbits.IRCF0 = 1; //set OSCCON IRCF bits to select OSC frequency=125kHz
    OSCCONbits.IRCF1 = 0;
    OSCCONbits.IRCF2 = 0;

    GPIO = 0x20;
    TRISIO = 0x0C; //all pins as Output PIN except 2 and 3

    // input with pull up
    OPTION_REGbits.nGPPU = 0;
    WPU2 = 1;
    OPTION_REGbits.INTEDG = 0; // falling edge trigger the interrupt

    //Timer1 Registers Prescaler= 1 - TMR1 Preset = 34285 - Freq = 1.00 Hz - Period = 1.000032 seconds
    T1CONbits.T1CKPS1 = 0; // bits 5-4  Prescaler Rate Select bits
    T1CONbits.T1CKPS0 = 0; // bit 4
    T1CONbits.TMR1CS = 0; // bit 1 Timer1 Clock Source Select bit...0 = Internal clock (FOSC/4)
    T1CONbits.TMR1ON = 1; // bit 0 enables timer

    TMR1H = 231; // preset for timer1 MSB register 200ms
    TMR1L = 144; // preset for timer1 LSB register

    // Interrupt Registers
    PIR1bits.TMR1IF = 0; // clear timer1 interrupt flag TMR1IF
    PIE1bits.TMR1IE = 1; // enable Timer1 interrupts
    INTCONbits.PEIE = 1; // bit6 Peripheral Interrupt Enable bit...1 = Enables all unmasked peripheral interrupts

    INTCONbits.INTF = 0; // clear the interrupt flag
    INTCONbits.INTE = 1; // enable the external interrupt
    INTCONbits.GIE = 1; // Global interrupt enable

    while (1) {
        handler();
    }

    return;
}
