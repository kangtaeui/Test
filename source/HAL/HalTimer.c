/**
 *  \file    HalTimer.c
 *  \brief   
 *  \date    2017. 9. 13.
 *  \author  felix
 *  \version 
 */

#include "HalTimer.h"
#include "el_helper_H430_IAR.h"
#include "SysConfig.h"



/***************************************************************************//**
 * SWTIMER IRQ vector numbers
 ******************************************************************************/
typedef enum {
  swtimer_IRQ_EVT_CNT_ZERO_0               = 0u,
  swtimer_IRQ_EVT_CNT_ZERO_1               = 1u,
  swtimer_IRQ_EVT_DIV_ZERO_0               = 2u,
  swtimer_IRQ_EVT_DIV_ZERO_1               = 3u,

  swtimer_INTERRUPT_VECTOR_CNT             = 4u  /**< Number of available interrupt vectors */

} swtimer_eInterruptVectorNum_t;



/***************************************************************************//**
 * Callback function pointer type
 ******************************************************************************/
typedef void (*tTimerIrqCbk) (swtimer_eInterruptVectorNum_t irqsrc, void* contextdata);


/***************************************************************************//**
 * Pointer to SCI environment data
 ******************************************************************************/
static struct {
	tTimerIrqCbk irqCallback[swtimer_INTERRUPT_VECTOR_CNT];
	void *ContextData;
	uint16_t counter1ms;
	uint16_t counterTask;
	uint16_t flagTask;
} timer;


static void HalTimerInitInterrupt(void);
static void HalTimerCnt0IrqHandler(swtimer_eInterruptVectorNum_t irqsrc, void* contextdata);
__interrupt static void HalTimerInterruptHandler(void);


void HalTimerInit(void)
{
    HalTimerInitInterrupt();

    SWTIMER_CONFIG_bit.cnt_auto_reloads = 3U;       /* enable auto reload */

    SWTIMER_DIV0_RELOAD = (SYSTEM_CLOCK_FREQUENCY/1000000UL)-1U;    /* 1MHz clock */
    SWTIMER_CNT0_RELOAD = 1000U - 1U;                               /* 1ms period */

    SWTIMER_COMMAND = 0x05U;                        /* restart pre-divider, counter */
    SWTIMER_CONFIG_bit.enables |= 1U;               /* enable counter 0 */

    timer.counter1ms = 0U;
    timer.counterTask = 0U;
    timer.flagTask = 0U;
}


uint16_t HalTimerGetCounter1ms(void)
{
    return timer.counter1ms;
}


uint16_t HalTimerGetFlagTask(void)
{
    uint16_t flag = timer.counterTask;

    if(flag != 0u)
    {
        timer.counterTask = 0u;
    }

    return flag;
}


static void HalTimerInitInterrupt(void)
{
    uint8_t i;

    /* initialize interrupt callbacks */
    for(i=0U; i<(uint8_t)swtimer_INTERRUPT_VECTOR_CNT; i++)
    {
        timer.irqCallback[i] = 0U;
    }

    vic_IfFuns->RegisterModule(vic_IRQ_SWTIMER, &HalTimerInterruptHandler, 0U);
    vic_IfFuns->EnableModule(vic_IRQ_SWTIMER);

    /* enable swTimer count0 interrupt */
    timer.irqCallback[swtimer_IRQ_EVT_CNT_ZERO_0] = &HalTimerCnt0IrqHandler;
    SWTIMER_IRQ_VENABLE = swtimer_IRQ_EVT_CNT_ZERO_0;

}


static void HalTimerCnt0IrqHandler(swtimer_eInterruptVectorNum_t irqsrc, void* contextdata)
{
    SWTIMER_IRQ_STATUS_bit.evt_cnt_zero &= (0x01U^0xFFU);   /* clear flag */

    timer.counter1ms++;
    timer.counterTask++;

    if(timer.counterTask >= TASK_PERIOD_IN_MS)
    {
    	timer.counterTask = 0u;
    	timer.flagTask = 1u;
    }
}



/***************************************************************************//**
 * @implementation
 * Every hardware SWTIMER module has it's own interrupt handler, also because each
 * module has it's own interrupt vector. So the module base address is fixed.
 * The interrupt vector table is provided by the user-application from RAM. The
 * module specific vector tables lie behind the VIC interrupt vector table, which
 * address is saved in the "TABLE_BASE". The module interrupt handler
 * gets the address from the function "vic_GetPointerToEviornmentData".
 * The interrupt vector number is saved in a local variable, because the value
 * of IRQ_VNO could possible change during processing of
 * loc_InterruptHandler. Next the registered interrupt callback function
 * (swtimer_RegisterInterruptCallback) is copied into a local function pointer,
 * checked if it's not NULL and at least called. At the end the interrupt
 * request flag is cleared, by writing back the interrupt vector number to
 * IRQ_VNO register.
 *
 ******************************************************************************/
__interrupt static void HalTimerInterruptHandler(void)
{
    swtimer_eInterruptVectorNum_t irqvecnum = (swtimer_eInterruptVectorNum_t) SWTIMER_IRQ_VNO;

    if (irqvecnum < swtimer_INTERRUPT_VECTOR_CNT) /* ensure the IRQ trigger is not already gone away until processing starts */
    {

        tTimerIrqCbk fptr;

        uint16_t vicVmaxBackup = 0;
        uint16_t swtVmaxBackup = 0;

        /* IRQ nesting on VIC level */
        vicVmaxBackup = VIC_IRQ_VMAX;
        VIC_IRQ_VMAX = vic_IRQ_SWTIMER;

        /* IRQ nesting on module level */
        swtVmaxBackup = SWTIMER_IRQ_VMAX;
        SWTIMER_IRQ_VMAX = 0;

        fptr = timer.irqCallback[irqvecnum];

        if(fptr != NULL)
        {
            __enable_interrupt();

            fptr(irqvecnum,  0);

            __disable_interrupt();
        }
        else
        {
            /* if there is no handler function for a particular IRQ, disable this IRQ  */
            SWTIMER_IRQ_VDISABLE = (uint16_t) irqvecnum;
        }

        /* Clear interrupt request flag */
        SWTIMER_IRQ_VNO = (uint16_t) irqvecnum;

        /* IRQ nesting on mocule level */
        SWTIMER_IRQ_VMAX = swtVmaxBackup;

        /* IRQ nesting on VIC level */
        VIC_IRQ_VMAX = vicVmaxBackup;
    }
    else {}

  /* RETI will reenable IRQs here */
}

