/*
 * pcComInterface.h
 *
 *  Created on: Aug 22, 2021
 *      Author: Tobias
 */

#ifndef PCCOMINTERFACE_H_
#define PCCOMINTERFACE_H_

#define PC_DATA_FRAME_ID 0x01
#define PC_DATA_FRAME_LENGTH 2*11+1

typedef union PCDataFrame {
    struct {
        unsigned char frameID;
        unsigned int buckPWM;
        unsigned int cellVoltage1, cellVoltage2, cellVoltage3, cellVoltage4;
        unsigned int solarVoltageN, solarVoltageP;
        unsigned int buckCurrent;
        unsigned int systemCurrent;
        unsigned int motorCurrent1, motorCurrent2;
    };
    unsigned char data[PC_DATA_FRAME_LENGTH];
} PCDataFrame;

#endif /* PCCOMINTERFACE_H_ */
