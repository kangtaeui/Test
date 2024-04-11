/**
 *  \file    HalEeprom.c
 *  \brief   
 *  \date    2017. 9. 19.
 *  \author  felix
 *  \version 
 */

#include "HalEeprom.h"
#include "el_helper_H430_IAR.h"


#include "rom_API.h"
#include "eeprom_access_interface.h"

static eepromIf_cpInterfaceFunctions_t eepIfFuns;


void HalEepromInit(void)
{
    EEPROM_CTRL_IP_ENABLE = 0xA501U;        /* enable EEPROM */
    romIf_MainInterface->Interface_Get(ROMIF_MTP_ACCESS, (romIf_cpGenericInterface_t *) &eepIfFuns, 0u);
}

void HalEepromErase32(uint16_t offset)
{
    eepIfFuns->MemoryErase( EE_PASSWORD,
                            (eeprom_pAddress_t)(offset+EE_START),
                            4u,
                            0u /*((1U<<(offset>>4U)) ^ 0xFFU)*/,
                            0u );
}

void HalEepromProgram32(uint16_t offset, uint16_t data[2])
{
    eepIfFuns->MemoryWrite( EE_PASSWORD,
                            (eeprom_cpData_t)data,
                            (eeprom_pAddress_t)(offset+EE_START),
                            4u,
                            0u /*((1U<<(offset>>4U)) ^ 0xFFU)*/,
                            0u );
}
