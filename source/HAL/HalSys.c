/*
 * HalSys.c
 *
 *  Created on: 2022. 3. 2.
 *      Author: FelixMin
 */

#include "HalSys.h"
#include "SysConfig.h"
#include "el_helper_H430_IAR.h"


static void WdogInit(void);


void HalSysInit(void)
{
#if PWM_CLOCK_FREQUENCY == 48000000
    SYS_STATE_CONTROL_bit.pwm_clk_sel = 1u;
#elif PWM_CLOCK_FREQUENCY == 24000000
    SYS_STATE_CONTROL_bit.pwm_clk_sel = 0u;
#else
#error "Define PWM_CLOCK_FREQUENCY"
#endif

#if SYSTEM_CLOCK_FREQUENCY == 24000000
    SYS_STATE_CONTROL_bit.sys_clk_sel = 0u;
#elif SYSTEM_CLOCK_FREQUENCY == 12000000
    SYS_STATE_CONTROL_bit.sys_clk_sel = 1u;
#else
#error "Define SYSTEM_CLOCK_FREQUENCY"
#endif

    SYS_STATE_RESET_ENABLE = E_SYS_STATE_RESET_ENABLE_WATCHDOG |
    E_SYS_STATE_RESET_ENABLE_WATCHDOG_ZERO;
    SYS_STATE_POWER;
    WdogInit();
}


void HalSysSoftwareReset(void)
{
    SYS_STATE_RESET_ENABLE |= E_SYS_STATE_RESET_ENABLE_SOFTWARE;    /* Enable software reset */
    SYS_STATE_SW_RESET = 0x10u;     /* assert a system reset */
}


void HalSysGotoSleep(void)
{
    uint16_t i;

    SYS_STATE_SLEEP_ENABLE |= E_SYS_STATE_SLEEP_ENABLE_SOFTWARE;    /* Enable software sleep */

    SYS_STATE_POWER = 1u;           /* activate sleep mode */

    for(i=0u; i<0xFFFFu; i++)
    {

    }

    asm("dint"); /* disable global interrupt */
    HalSysSoftwareReset();
}


/**
 * \brief Initialize watchdog
 *
 */
static void WdogInit(void)
{
    WDOG_WINDOW = 0u;   /* disable window mode */
    WDOG_PRESCALER = (SYSTEM_CLOCK_FREQUENCY/100000u) - 1u;    /* 0.01ms tick */
    WDOG_RELOAD = 50000u;    /* 0.5sec timeout */

    WDOG_CONTROL = 0xA501u; /* enable watchdog */
    HalWdogService();
}


void HalWdogService(void)
{
    WDOG_CONTROL = 0xA502u; /* restart watchdog */
}
