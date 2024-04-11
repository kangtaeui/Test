/**
 *  \file    HalPwm.h
 *  \brief   
 *  \date    2017. 9. 14.
 *  \author  felix
 *  \version 
 */



#ifndef HAL_HALPWM_H_
#define HAL_HALPWM_H_


#include "el_types.h"


void HalPwmInit(void);
void HalPwmSetPoint(uint8_t index, uint16_t start, uint16_t stop);
void HalPwmSetDuty(uint16_t chR, uint16_t chG, uint16_t chB);
void HalPwmSetCurrent(uint16_t currentSetting);


#endif /* HAL_HALPWM_H_ */
