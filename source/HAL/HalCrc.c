/*
 * HalCrc.c
 *
 *  Created on: 2022. 2. 7.
 *      Author: FelixMin
 */

#include "HalCrc.h"
#include "el_helper_H430_IAR.h"


uint16_t HalCrcStart(uint8_t *data, uint16_t numWords)
{
    CRC16_CONFIG = 0x01u;   /* clear CRC sum, space = 1 */

    CRC16_LENGTH = numWords*2;

    /* start CRC calculation */
    CRC16_START = (uint16_t)data;

    while(CRC16_STATUS == 0u)
    {
        /* wait */
    }

    return CRC16_CRC_SUM;
}
