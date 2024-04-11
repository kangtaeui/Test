/**
 *  \file    HalMulDiv.c
 *  \brief   
 *  \date    2017. 9. 14.
 *  \author  felix
 *  \version 
 */


#include "HalMulDiv.h"
#include "el_helper_H430_IAR.h"


uint32_t HalMulUnsigned(uint16_t op1, uint16_t op2)
{
    H430_MUL_MPY = op1;
    H430_MUL_OP2 = op2;

    return (int32_t)H430_MUL_RES;
}

int32_t HalMulSigned(int16_t op1, int16_t op2)
{
    H430_MUL_MPYS = op1;
    H430_MUL_OP2 = op2;

    return (int32_t)H430_MUL_RES;
}


int32_t HalMacSigned(int16_t op1, int16_t op2)
{
    H430_MUL_MACS = op1;
    H430_MUL_OP2 = op2;

    return (int32_t)H430_MUL_RES;
}


uint32_t HalDivUnsigned(uint32_t op1, uint16_t op2)
{
    DIVIDER_OP1 = op1;
    DIVIDER_OP2 = op2;

    return DIVIDER_RESULT;
}

int32_t HalDivSigned(int32_t op1, int16_t op2)
{
    DIVIDER_OP1 = op1;
    DIVIDER_OP2S = op2;

    return DIVIDER_RESULT;
}
