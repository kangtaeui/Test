/**
 *  \file    HalMulDiv.h
 *  \brief   
 *  \date    2017. 9. 14.
 *  \author  felix
 *  \version 
 */



#ifndef HAL_HALMULDIV_H_
#define HAL_HALMULDIV_H_


#include "el_types.h"


uint32_t HalMulUnsigned(uint16_t op1, uint16_t op2);
int32_t HalMulSigned(int16_t op1, int16_t op2);
int32_t HalMacSigned(int16_t op1, int16_t op2);

uint32_t HalDivUnsigned(uint32_t op1, uint16_t op2);
int32_t HalDivSigned(int32_t op1, int16_t op2);

#endif /* HAL_HALMULDIV_H_ */
