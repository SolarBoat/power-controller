/**
 * main.c
 */

#include <msp430.h>
#include "adc.h"
#include "uart.h"
#include "pcComInterface.h"


#define PIN_DEBUG BIT3


void init_system_clock(void);
void init_adc_sampling(void);


PCDataFrame dataFrame = {.frameID = PC_DATA_FRAME_ID};

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    P4DIR |= PIN_DEBUG;

    PM5CTL0 &= ~LOCKLPM5;

    __enable_interrupt();
    init_system_clock();
    adc_init();
    uart_init();

    while(1) {
        while (!adcNewDataFlag);
        adcNewDataFlag = 0;
        P4OUT ^= PIN_DEBUG;

        dataFrame.buckPWM = 0;
        dataFrame.cellVoltage1 = cellVoltage1;
        dataFrame.cellVoltage2 = cellVoltage2;
        dataFrame.cellVoltage3 = cellVoltage3;
        dataFrame.cellVoltage4 = cellVoltage4;
        dataFrame.solarVoltageN = solarVoltageN;
        dataFrame.solarVoltageP = solarVoltageP;
        dataFrame.buckCurrent = buckCurrent;
        dataFrame.systemCurrent = systemCurrent;
        dataFrame.motorCurrent1 = motorCurrent1;
        dataFrame.motorCurrent2 = motorCurrent1;

        uart_write(dataFrame.data, PC_DATA_FRAME_LENGTH);
    }
}

void init_system_clock(void) {
    __bis_SR_register(SCG0);                // disable FLL
    CSCTL3 |= SELREF__REFOCLK;              // Set REFO as FLL reference source
    CSCTL1 = DCOFTRIMEN_1 | DCOFTRIM_3 | DCORSEL_7;// DCOFTRIM=3, DCO Range = 24MHz
    CSCTL2 = FLLD_0 + 732;                  // DCODIV = 24MHz
    __delay_cycles(3);
    __bic_SR_register(SCG0);                // enable FLL
}
