/*
 * HalSys.h
 *
 *  Created on: 2022. 3. 2.
 *      Author: FelixMin
 */

#ifndef SOURCE_HAL_HALSYS_H_
#define SOURCE_HAL_HALSYS_H_


void HalSysInit(void);
void HalSysSoftwareReset(void);

void HalSysGotoSleep(void);


void HalWdogService(void);


#endif /* SOURCE_HAL_HALSYS_H_ */
