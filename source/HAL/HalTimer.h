/**
 *  \file    HalTimer.h
 *  \brief   
 *  \date    2017. 9. 13.
 *  \author  felix
 *  \version 
 */



#ifndef HAL_HALTIMER_H_
#define HAL_HALTIMER_H_


#include "el_types.h"



void HalTimerInit(void);

uint16_t HalTimerGetCounter1ms(void);
uint16_t HalTimerGetFlagTask(void);

#endif /* HAL_HALTIMER_H_ */
