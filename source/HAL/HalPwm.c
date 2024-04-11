/**
 *  \file    HalPwm.c
 *  \brief   
 *  \date    2017. 9. 14.
 *  \author  felix
 *  \version 
 */


#include "HalPwm.h"
#include "el_helper_H430_IAR.h"
#include "SysConfig.h"
//#include "HalVic.h"
#include "DrvLed.h"
//#include "HalAdc.h"
#include "DrvEep.h"


/***************************************************************************//**
 * PWM IRQ vector numbers
 ******************************************************************************/
typedef enum {
  pwm_IRQ_EVT_PERIOD0             = 0u,
  pwm_IRQ_EVT_START0              = 1u,
  pwm_IRQ_EVT_STOP0               = 2u,
  pwm_IRQ_EVT_PERIOD1             = 3u,
  pwm_IRQ_EVT_START1              = 4u,
  pwm_IRQ_EVT_STOP1               = 5u,
  pwm_IRQ_EVT_PERIOD2             = 6u,
  pwm_IRQ_EVT_START2              = 7u,
  pwm_IRQ_EVT_STOP2               = 8u,
  pwm_IRQ_EVT_PERIOD3             = 9u,
  pwm_IRQ_EVT_START3              = 10u,
  pwm_IRQ_EVT_STOP3               = 11u,

  pwm_INTERRUPT_VECTOR_CNT        = 12u  /**< Number of available interrupt vectors */
} pwm_eInterruptVectorNum_t;


/* By Felix for easy integration */
#ifdef __IAR_SYSTEMS_ICC__
__no_init struct  {
    volatile unsigned short PERIOD;
    volatile unsigned short START;
    volatile unsigned short STOP;
    volatile unsigned short COUNTER;
} PWM_CH[4] @0x350u;
#else
struct  {
    volatile unsigned short PERIOD;
    volatile unsigned short START;
    volatile unsigned short STOP;
    volatile unsigned short COUNTER;
} PWM_CH[4];
#endif

typedef void (*tPwmIrqCbk) (pwm_eInterruptVectorNum_t irqvecnum, uint16_t contextdata);

__interrupt static void HalPwmInterruptHandler(void);
static void HalPwmEventHandler(pwm_eInterruptVectorNum_t irqsrc, uint16_t contextdata);
#if 0
static void HalPwmEventHandler0(uint16_t irqsrc, uint16_t contextdata);
static void HalPwmEventHandler1(uint16_t irqsrc, uint16_t contextdata);
static void HalPwmEventHandler2(uint16_t irqsrc, uint16_t contextdata);
#endif
static void HalPwmInitInterrupt(void);


static struct {
	tPwmIrqCbk irqCallback[pwm_INTERRUPT_VECTOR_CNT];
	void *contextData;

	struct
	{
	    uint16_t start;
	    uint16_t stop;
	    uint16_t duration;
	} ctrl[4];

} pwm;

//static InterruptCallback_t pwmIrqCallback[pwm_INTERRUPT_VECTOR_CNT];


//static tPwmInfo pwmInfo[4];

void HalPwmInit(void)
{
	/* Enable PWM driver */
    HalPwmSetCurrent(pwmCurrentSetting);
    PWM_CONTROL =   E_PWM_CONTROL_DRIVER_0_ENABLE |
                    E_PWM_CONTROL_DRIVER_1_ENABLE |
                    E_PWM_CONTROL_DRIVER_2_ENABLE |
#if defined(DEBUG)
                    E_PWM_CONTROL_DRIVER_3_ENABLE |	/* use OUT3 as debug output */
#endif
				  0x700u;  /*internal pull-up active */

    PWM_PRESCALER[0] = (PWM_CLOCK_FREQUENCY/PWM_PERIOD_TICK/PWM_FREQUENCY)-1U;
    PWM_CH[0].PERIOD = PWM_PERIOD_TICK;
    PWM_CH[0].START = 0U;
    PWM_CH[0].STOP = 0U;

    PWM_PRESCALER[1] = (PWM_CLOCK_FREQUENCY/PWM_PERIOD_TICK/PWM_FREQUENCY)-1U;
    PWM_CH[1].PERIOD = PWM_PERIOD_TICK;
    PWM_CH[1].START = 0U;
    PWM_CH[1].STOP = 0U;

    PWM_PRESCALER[2] = (PWM_CLOCK_FREQUENCY/PWM_PERIOD_TICK/PWM_FREQUENCY)-1U;
    PWM_CH[2].PERIOD = PWM_PERIOD_TICK;
    PWM_CH[2].START = 0U;
    PWM_CH[2].STOP = 0U;

    /* generate update timer event */
    PWM_PRESCALER[3] = (PWM_CLOCK_FREQUENCY/PWM_PERIOD_TICK/PWM_FREQUENCY)-1U;
    PWM_CH[3].PERIOD = PWM_PERIOD_TICK;
    PWM_CH[3].START = 0U;
    PWM_CH[3].STOP = 0U;

    HalPwmInitInterrupt();

    PWM_CONTROL |=  E_PWM_CONTROL_DRIVER_0_ENABLE | E_PWM_CONTROL_PWM_0_ENABLE |
                    E_PWM_CONTROL_DRIVER_1_ENABLE | E_PWM_CONTROL_PWM_1_ENABLE |
                    E_PWM_CONTROL_DRIVER_2_ENABLE | E_PWM_CONTROL_PWM_2_ENABLE |
                    E_PWM_CONTROL_PWM_3_ENABLE |  /* Enable PWM 3 for ADC trigger */
                    0x700u;  /*internal pull-up active */


    pwm.ctrl[0].start = 0U;
    pwm.ctrl[0].stop = 0U;
    pwm.ctrl[0].duration = 0U;
    pwm.ctrl[1].start = 0U;
    pwm.ctrl[1].stop = 0U;
    pwm.ctrl[1].duration = 0U;
    pwm.ctrl[2].start = 0U;
    pwm.ctrl[2].stop = 0U;
    pwm.ctrl[2].duration = 0U;
}


/**
 * \brief	Set PWM start/stop point
 *
 * \param[in] index	Index of PWM
 * \param[in] start	Start point of PWM
 * \param[in] stop	Stop point of PWM
 */
void HalPwmSetPoint(uint8_t index, uint16_t start, uint16_t stop)
{
    PWM_CH[index].START = start;
    PWM_CH[index].STOP = stop;
}

#if 0   /* Not used */
void HalPwmSetDuty(uint16_t ch0, uint16_t ch1, uint16_t ch2)
{

#if 1
    uint16_t dutySum = ch0 + ch1 + ch2;     /* dutySum never overflows */
    uint16_t overlap;
    uint16_t shift;

    pwm.ctrl[0].start = 0U;
    pwm.ctrl[0].duration = ch0;
    pwm.ctrl[0].stop = ch0;

    pwm.ctrl[1].start = pwm.ctrl[0].stop;
    pwm.ctrl[1].duration = ch1;
    pwm.ctrl[1].stop = pwm.ctrl[1].start + pwm.ctrl[1].duration;
    if(pwm.ctrl[1].stop > PWM_PERIOD_TICK)
    {
        pwm.ctrl[1].start = PWM_PERIOD_TICK - pwm.ctrl[1].duration;
        pwm.ctrl[1].stop = PWM_PERIOD_TICK;
    }

    pwm.ctrl[2].start = pwm.ctrl[1].stop;
    pwm.ctrl[2].duration = ch2;
    pwm.ctrl[2].stop = pwm.ctrl[2].start + pwm.ctrl[2].duration;
    if(pwm.ctrl[2].stop > PWM_PERIOD_TICK)
    {
        shift = (pwm.ctrl[2].stop - PWM_PERIOD_TICK)/4U;
        if(pwm.ctrl[1].start > shift)
        {
            pwm.ctrl[1].start -= shift;
        }
        else
        {
            pwm.ctrl[1].start = 0U;
        }
        pwm.ctrl[1].stop = pwm.ctrl[1].start + pwm.ctrl[1].duration;

        pwm.ctrl[2].start = PWM_PERIOD_TICK - pwm.ctrl[2].duration;
        pwm.ctrl[2].stop = PWM_PERIOD_TICK;
    }

    PWM_CH[0].START = 0;
    PWM_CH[0].STOP = pwm.ctrl[0].stop;
    PWM_CH[1].START = pwm.ctrl[1].start;
    PWM_CH[1].STOP = pwm.ctrl[1].stop;
    PWM_CH[2].START = pwm.ctrl[2].start;
    PWM_CH[2].STOP = pwm.ctrl[2].stop;
#else
    uint16_t dutySum = ch0 + ch1 + ch2;     /* dutySum never overflows */
    uint16_t overlap;

    pwmInfo[0].duration = ch0;
    pwmInfo[1].duration = ch1;
    pwmInfo[2].duration = ch2;

    if(dutySum >= PWM_PERIOD_TICK)
    {
        overlap = dutySum - PWM_PERIOD_TICK;
        /* todo */
    }
    else        /* no overlap */
    {

        pwmInfo[0].start = PWM_PERIOD_TICK - dutySum;
        pwmInfo[0].stop = pwmInfo[0].start + ch0;

        pwmInfo[1].start = pwmInfo[0].stop;
        pwmInfo[1].stop = pwmInfo[0].stop + ch1;

        pwmInfo[1].start = pwmInfo[0].stop;
        pwmInfo[1].stop = pwmInfo[0].stop + ch1;

    }
    PWM_CH[0].START = pwmInfo[0].start;
    PWM_CH[0].STOP = pwmInfo[0].stop;
    PWM_CH[1].START = pwmInfo[1].start;
    PWM_CH[1].STOP = pwmInfo[1].stop;
    PWM_CH[2].START = pwmInfo[2].start;
    PWM_CH[2].STOP = pwmInfo[2].stop;

#endif

    HalAdcSetTriggerDelay(0, pwm.ctrl[0].start, pwm.ctrl[0].duration);
    HalAdcSetTriggerDelay(1, pwm.ctrl[1].start, pwm.ctrl[1].duration);
    HalAdcSetTriggerDelay(2, pwm.ctrl[2].start, pwm.ctrl[2].duration);
}
#endif


void HalPwmSetCurrent(uint16_t currentSetting)
{
    PWM_DRIVER = currentSetting; /* enable pull-up */
}


static void HalPwmInitInterrupt(void)
{
    uint8_t i;

    for(i=0U; i<(uint8_t)pwm_INTERRUPT_VECTOR_CNT; i++)
    {
        pwm.irqCallback[i] = NULL;
    }

    vic_IfFuns->RegisterModule(vic_IRQ_PWM, &HalPwmInterruptHandler, 0U);
    vic_IfFuns->EnableModule(vic_IRQ_PWM);

    pwm.irqCallback[pwm_IRQ_EVT_PERIOD0] = &HalPwmEventHandler;
    PWM_IRQ_VENABLE = pwm_IRQ_EVT_PERIOD0;
#if 0
    pwmIrqCallback[pwm_IRQ_EVT_STOP0] = &HalPwmEventHandler0;
    PWM_IRQ_VENABLE = pwm_IRQ_EVT_STOP0;
    pwmIrqCallback[pwm_IRQ_EVT_STOP1] = &HalPwmEventHandler1;
    PWM_IRQ_VENABLE = pwm_IRQ_EVT_STOP1;
    pwmIrqCallback[pwm_IRQ_EVT_STOP2] = &HalPwmEventHandler2;
    PWM_IRQ_VENABLE = pwm_IRQ_EVT_STOP2;
#endif
}


static void HalPwmEventHandler(pwm_eInterruptVectorNum_t irqsrc, uint16_t contextdata)
{
#if 0
    static volatile uint16_t start = 0;
    static volatile uint16_t duty = PWM_PERIOD_TICK/10;

    uint16_t stop;

    stop = start+duty;

    PWM_CH[0].START = start;
    if(stop > PWM_PERIOD_TICK)
    {
        stop -= PWM_PERIOD_TICK;
    }
    PWM_CH[0].STOP = stop;
    start ++;

    if(start >= PWM_PERIOD_TICK)
    {
        start = 0;
    }
#else
    DrvLedPwmPeriodUpdate();
#endif
}

#if 0
static void HalPwmEventHandler0(uint16_t irqsrc, uint16_t contextdata)
{
    PWM_CH[0].START = pwmInfo[0].start;
    PWM_CH[0].STOP = pwmInfo[0].stop;
}

static void HalPwmEventHandler1(uint16_t irqsrc, uint16_t contextdata)
{
    PWM_CH[1].START = pwmInfo[1].start;
    PWM_CH[1].STOP = pwmInfo[1].stop;
}

static void HalPwmEventHandler2(uint16_t irqsrc, uint16_t contextdata)
{
    PWM_CH[2].START = pwmInfo[2].start;
    PWM_CH[2].STOP = pwmInfo[2].stop;
}
#endif

/***************************************************************************//**
 * @implementation
 * Every hardware PWM module has it's own interrupt handler, also because each
 * module has it's own interrupt vector. So the module base address is fixed.
 * The interrupt vector table is provided by the user-application from RAM. The
 * module specific vector tables lie behind the VIC interrupt vector table, which
 * address is saved in the "TABLE_BASE". The module interrupt handler
 * gets the address from the function "vic_GetPointerToEviornmentData".
 * The interrupt vector number is saved in a local variable, because the value
 * of IRQ_VECTOR could possible change during processing of
 * pwm_PWM0_InterruptHandler. Next the registered interrupt callback function
 * (pwm_RegisterInterruptCallback) is copied into a local function pointer,
 * checked if it's not NULL and at least called. At the end the interrupt
 * request flag is cleared, by writing back the interrupt vector number to
 * IRQ_VNO register.
 *
 ******************************************************************************/
__interrupt static void HalPwmInterruptHandler(void)
{
	pwm_eInterruptVectorNum_t irqvecnum = (pwm_eInterruptVectorNum_t) PWM_IRQ_VNO;

    if (irqvecnum < pwm_INTERRUPT_VECTOR_CNT) /* ensure the IRQ trigger is not already gone away until processing starts */
    {
//        pwm_sInterruptEnvironmentData_t* evironmentData = (pwm_sInterruptEnvironmentData_t*) vic_GetPointerToEnvironmentData(vic_IRQ_PWM);

    	tPwmIrqCbk fptr;

        uint16_t vicVmaxBackup = 0;
        uint16_t pwmVmaxBackup = 0;

        /* IRQ nesting on VIC level */
        vicVmaxBackup = VIC_IRQ_VMAX;
        VIC_IRQ_VMAX = vic_IRQ_PWM;

        /* IRQ nesting on module level */
        pwmVmaxBackup = PWM_IRQ_VMAX;
        PWM_IRQ_VMAX = 0;

//        fptr = evironmentData->InterrupVectorTable[irqvecnum];
        fptr = pwm.irqCallback[irqvecnum];

        if(fptr != NULL)
        {
            __enable_interrupt();

            /* Call interrupt callback function */
            fptr(irqvecnum,  0U/*(void*) evironmentData->ContextData*/);

            __disable_interrupt();
        }
        else
        {
            /* if there is no handler function for a particular IRQ, disable this IRQ  */
            PWM_IRQ_VDISABLE = (uint16_t) irqvecnum;
        }

        /* Clear interrupt request flag */
        PWM_IRQ_VNO = (uint16_t) irqvecnum;

        /* IRQ nesting on mocule level */
        PWM_IRQ_VMAX = pwmVmaxBackup;

        /* IRQ nesting on VIC level */
        VIC_IRQ_VMAX = vicVmaxBackup;
    }
    else {}

}

