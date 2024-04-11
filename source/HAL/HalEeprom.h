/**
 *  \file    HalEeprom.h
 *  \brief   
 *  \date    2017. 9. 19.
 *  \author  felix
 *  \version 
 */



#ifndef HAL_HALEEPROM_H_
#define HAL_HALEEPROM_H_

#include "el_types.h"
#define EEPROM_ADDR_BASE            0x400U

void HalEepromInit(void);
void HalEepromErase32(uint16_t offset);
void HalEepromProgram32(uint16_t offset, uint16_t data[2]);

#endif /* HAL_HALEEPROM_H_ */
