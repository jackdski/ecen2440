/*
 * bluetooth.h
 *
 *  Created on: Oct 29, 2017
 *      Author: amabo
 */

#ifndef BLUETOOTH_H_
#define BLUETOOTH_H_
#include "msp.h"

void configure_BLUE_UART();
void BLUART_send_byte(uint8_t data);

void BLUEART_SEND_DATA(float totalDistance, float spd,uint8_t direction);

#endif /* BLUETOOTH_H_ */
