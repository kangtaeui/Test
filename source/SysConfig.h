/*
 * SysConfig.h
 *
 *  Created on: 2022. 2. 3.
 *      Author: FelixMin
 */

#ifndef SOURCE_SYSCONFIG_H_
#define SOURCE_SYSCONFIG_H_

#include "vic_Interface.h"

extern vic_cpInterfaceFunctions_t vic_IfFuns; /* VIC Interface ptr. */


#define ENABLE_LED_DIAG             1


#define SOFTWARE_VERSION_MAJOR		0
#define SOFTWARE_VERSION_MINOR		1

#define LINDIAG_NAD                     0x10u
#define LINDIAG_SUPPLIER_ID             0x10u
#define LINDIAG_FUNCTION_ID             0x0008u
#define LINDIAG_VARIANT                 0x40u

#define SYSTEM_CLOCK_FREQUENCY      12000000UL      /**< system clock frequency. One of 24MHz/12MHz/8MHz/4MHz */
#define PWM_CLOCK_FREQUENCY         48000000UL      /**< PWM/ADC clock frequency. One of 48MHz/24MHz */

#define PWM_FREQUENCY               100UL           /**< LED PWM frequency */
#define PWM_PERIOD_TICK             20000           /**< Maximum PWM duty in tick - maximum value is 32767 */
#define PWM_PERIOD_MS				(1000u/PWM_FREQUENCY)	/**< PWM period in ms unit */

#define TASK_PERIOD_IN_MS			10u
#define TASK_TIME_IN_MS(MS)			((MS)/TASK_PERIOD_IN_MS)

#define ADC_MIN_SAMPLE_US           150.0            /* Conversion time(in us unit) for 10 conversion - worst case is 140us */
#define ADC_MIN_SAMPLE_PERIOD       ((uint16_t)(ADC_MIN_SAMPLE_US/PWM_PERIOD_MS*PWM_PERIOD_TICK/1000))
#define ADC_SAMPLE_POSITION         0


#define LED_DIAG_FILTER_SHORT_THRES 5U              /* Short threshold - resolution is PWM period */
#define LED_DIAG_FILTER_OPEN_THRES  16U             /* Open threshold - resolution is PWM period */

#define VBAT_RECOVERY_THRES         ADC_VS_PHY2ADC(0.5)
#define BATTERY_RECOVERY_TIME       TASK_TIME_IN_MS(100)    /* battery UV/OV recover threshold */

#define LED_FADE_TIME_IN_OVERTEMP   10U              /* 100ms unit */

#define ADC_VS_PHY2ADC(VS)          ((uint16_t)((VS)*112.0))
#define ADC_VOUT_PHY2ADC(VOUT)      ADC_VS_PHY2ADC(VOUT)
#define ADC_VLED_PHY2ADC(VLED)      ((uint16_t)((VLED)*561.0))
#define ADC_TEMP_PHY2ADC(PHY)       (1140 + ((125.0-(PHY))*4.3))


typedef enum
{
    BATTERY_NORMAL,
    BATTERY_UV,
    BATTERY_OV
} tBatteryStatus;


#if defined(DEBUG)
#include "el_helper_H430_IAR.h"

#define GPIO_DEBUG_MASK				0x08u			/* OUT3 is used for debugging */

#define DEBUG_PORT_TOGGLE()			(GPIO_DATA_OE ^= GPIO_DEBUG_MASK)
#define DEBUG_PORT_SET()			(GPIO_DATA_OE &= GPIO_DEBUG_MASK^0xFFu)
#define DEBUG_PORT_CLR()			(GPIO_DATA_OE |= GPIO_DEBUG_MASK)
#else
#define DEBUG_PORT_TOGGLE()
#define DEBUG_PORT_SET()
#define DEBUG_PORT_CLR()
#endif


//#define pwmMapIndex					5
//#define linStatusId					0x0A
//#define linMlmmId					0x01
//#define pwmCurrentSetting			0x1111u	/* 10ms for test */

#endif /* SOURCE_SYSCONFIG_H_ */
