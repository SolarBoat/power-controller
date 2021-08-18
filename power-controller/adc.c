/*
 * adc.c
 */

#include <msp430.h>
#include "adc.h"

#define PIN_CELL_1 BIT0
#define PIN_CELL_2 BIT1
#define PIN_CELL_3 BIT2
#define PIN_CELL_4 BIT3
#define PIN_SOLAR_N BIT4
#define PIN_SOLAR_P BIT5
#define PIN_MOTOR_1_I BIT6
#define PIN_MOTOR_2_I BIT7
#define PIN_BUCK_I BIT0
#define PIN_SYSTEM_I BIT1
#define PIN_SYS_I_OA_IN BIT7
#define PIN_SYS_I_OA_OUT BIT5

#define ADC_SAMPLE_AVG 64

#define ADC_REF_VOLTAGE 2.5
#define CONVERSION_FACTOR_CELL_1 (82.0 / 15.0 + 1.0) * ADC_REF_VOLTAGE * 256 / 4096 / ADC_SAMPLE_AVG
#define CONVERSION_FACTOR_CELL_2 (82.0 / 15.0 + 1.0) * ADC_REF_VOLTAGE * 256 / 4096 / ADC_SAMPLE_AVG
#define CONVERSION_FACTOR_CELL_3 (82.0 / 15.0 + 1.0) * ADC_REF_VOLTAGE * 256 / 4096 / ADC_SAMPLE_AVG
#define CONVERSION_FACTOR_CELL_4 (82.0 / 15.0 + 1.0) * ADC_REF_VOLTAGE * 256 / 4096 / ADC_SAMPLE_AVG
#define CONVERSION_FACTOR_SOLAR_N (82.0 / 8.2 + 1.0) * ADC_REF_VOLTAGE * 256 / 4096 / ADC_SAMPLE_AVG
#define CONVERSION_FACTOR_SOLAR_P (82.0 / 8.2 + 1.0) * ADC_REF_VOLTAGE * 256 / 4096 / ADC_SAMPLE_AVG
#define CONVERSION_FACTOR_MOTOR_1_I 5.0 * ADC_REF_VOLTAGE * 256 / 4096 / ADC_SAMPLE_AVG * 0.968
#define CONVERSION_FACTOR_MOTOR_2_I 5.0 * ADC_REF_VOLTAGE * 256 / 4096 / ADC_SAMPLE_AVG * 0.956
#define CONVERSION_FACTOR_BUCK_I 2.5 * ADC_REF_VOLTAGE * 256 / 4096 / ADC_SAMPLE_AVG
#define CONVERSION_FACTOR_SYSTEM_I 2.5 * ADC_REF_VOLTAGE * 1024 / 4096 / ADC_SAMPLE_AVG / 5.0


void adc_init_opamp();


unsigned long adcSampleBuffer[10] = {0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0};
unsigned char adcSampleCount = 0;
unsigned char adcChannel = 0;


void adc_init() {
    adcNewDataFlag = 0;
    // Pin IO
    P1SEL0 |= (PIN_CELL_1 | PIN_CELL_2 | PIN_CELL_3 | PIN_CELL_4 |
               PIN_SOLAR_N | PIN_SOLAR_P | PIN_MOTOR_1_I | PIN_MOTOR_2_I);
    P1SEL1 |= (PIN_CELL_1 | PIN_CELL_2 | PIN_CELL_3 | PIN_CELL_4 |
               PIN_SOLAR_N | PIN_SOLAR_P | PIN_MOTOR_1_I | PIN_MOTOR_2_I);
    P5SEL0 |= (PIN_BUCK_I | PIN_SYSTEM_I);
    P5SEL1 |= (PIN_BUCK_I | PIN_SYSTEM_I);

    adc_init_opamp();

    // Timer init
    TB0CTL |= (TBSSEL_2 | ID_2); // SMCLK, DIV4,
    TB0CCR0 = 9375; // 64 Hz * 10
    TB0CCTL0 |= CCIE; // Interrupt enable

    // internal reference
    PMMCTL2 |= (REFVSEL_2 | INTREFEN); // 2.5V Reference, Internal Reference enable

    // ADC Register init
    ADCCTL0 |= (ADCSHT_2 | ADCON); // 16 Cycles
    ADCCTL1 |= (ADCSHP | ADCSSEL_3 | ADCDIV_7); // pulse, SMLCLK, clock_div 8
    ADCCTL2 = (ADCPDIV_2 | ADCRES_2); // Prediv 64, 12 Bit Resolution
    ADCMCTL0 |= ADCSREF_1; // VREF and Ground

    ADCCTL0 |= ADCENC; // enable adc
    TB0CTL |= MC_1; // start timer

    ADCCTL0 |= ADCSC; // start first conversion
}

void adc_init_opamp() {
    P3SEL0 |= PIN_SYS_I_OA_IN | PIN_SYS_I_OA_OUT;
    P3SEL1 |= PIN_SYS_I_OA_IN | PIN_SYS_I_OA_OUT;

    SAC3OA |= NMUXEN | PMUXEN | NSEL_1; // enable, low power, PGA source
    SAC3OA |= OAPM;
    SAC3PGA = GAIN0 | GAIN1 | MSEL_2; // noninverting, gain 5
    SAC3OA |= SACEN | OAEN;
}

#pragma vector=TIMER0_B0_VECTOR
__interrupt void TIMER0_B0_ISR (void) {
    adcSampleBuffer[adcChannel] += ADCMEM0;
    adcChannel++;
    if (adcChannel >= 10) {
        adcChannel = 0;
        adcSampleCount++;
    }
    ADCCTL0 &= ~ADCENC; // disable adc
    ADCMCTL0 &= 0xFFF0; // clear selected channel
    ADCMCTL0 |= adcChannel; // set channel
    ADCCTL0 |= ADCENC; // enable adc
    ADCCTL0 |= ADCSC; // start conversion

    if (adcSampleCount >= ADC_SAMPLE_AVG) {
        cellVoltage1 = (unsigned int) (adcSampleBuffer[0] * CONVERSION_FACTOR_CELL_1);
        cellVoltage2 = (unsigned int) (adcSampleBuffer[1] * CONVERSION_FACTOR_CELL_2);
        cellVoltage3 = (unsigned int) (adcSampleBuffer[2] * CONVERSION_FACTOR_CELL_3);
        cellVoltage4 = (unsigned int) (adcSampleBuffer[3] * CONVERSION_FACTOR_CELL_4);
        solarVoltageN = (unsigned int) (adcSampleBuffer[4] * CONVERSION_FACTOR_SOLAR_N);
        solarVoltageP = (unsigned int) (adcSampleBuffer[5] * CONVERSION_FACTOR_SOLAR_P);
        motorCurrent1 = (unsigned int) (adcSampleBuffer[6] * CONVERSION_FACTOR_MOTOR_1_I);
        motorCurrent2 = (unsigned int) (adcSampleBuffer[7] * CONVERSION_FACTOR_MOTOR_2_I);
        buckCurrent = (unsigned int) (adcSampleBuffer[8] * CONVERSION_FACTOR_BUCK_I);
        systemCurrent = (unsigned int) (adcSampleBuffer[9] * CONVERSION_FACTOR_SYSTEM_I);
        unsigned char i = 0;
        for (; i < 10; i++) {
            adcSampleBuffer[i] = 0;
        }
        adcSampleCount = 0;
        adcNewDataFlag = 1;
    }
}
