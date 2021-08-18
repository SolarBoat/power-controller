/*
 * uart.c
 */

#include <msp430.h>
#include "uart.h"

#define UART_PIN_TX BIT3

void uart_init() {
    UCA1CTLW0 |= UCSSEL_2; // SMCLK
    UCA1BRW = 156; // 9600 baud
    UCA1MCTLW |= UCOS16 | UCBRF_4; // 9600 baud

    P4SEL0 |= UART_PIN_TX;
    UCA1CTLW0 &= ~UCSWRST; // start
}

void uart_write(unsigned char *data, unsigned char length) {
    while(UCA1STATW & UCBUSY);
    unsigned char i = 0;
    for (; i < length; i++) {
        while(!(UCA1IFG & UCTXIFG));
        UCA1TXBUF = data[i];
    }
}


