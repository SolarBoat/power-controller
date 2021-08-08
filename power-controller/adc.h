/*
 * adc.h
 */

#ifndef ADC_H_
#define ADC_H_


unsigned int cellVoltage1, cellVoltage2, cellVoltage3, cellVoltage4;
unsigned int solarVoltageN, solarVoltageP;
unsigned int motorCurrent1, motorCurrent2;
unsigned int buckCurrent;
unsigned int systemCurrent;

unsigned char adcNewDataFlag;

void adc_init();


#endif /* ADC_H_ */
