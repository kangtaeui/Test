/**
 *  \file    HalAdc.h
 *  \brief   
 *  \date    2017. 9. 18.
 *  \author  felix
 *  \version 
 */



#ifndef HAL_HALADC_H_
#define HAL_HALADC_H_


#include "el_types.h"

#define ADC_RESULT_INVALID      0xFFFFU


typedef struct {
    uint16_t ready;
    uint16_t vLed;
    uint16_t vOut;
} tLedAdcResult;


void HalAdcInit(void);
void HalAdcSetTriggerDelay(uint16_t ch, uint16_t trigger);

tLedAdcResult* HalAdcGetLedResult(uint16_t ch);
uint16_t HalAdcGetVbat(void);
uint16_t HalAdcGetVtemp(void);

#endif /* HAL_HALADC_H_ */
