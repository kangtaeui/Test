/**
 *  \file    HalAdc.c
 *  \brief   
 *  \date    2017. 9. 18.
 *  \author  felix
 *  \version 
 */


#include "HalAdc.h"
#include "el_helper_H430_IAR.h"
#include "SysConfig.h"

#define ADC_CH_VDIFF        0               /**< DIFF_AA */
#define ADC_CH_VS           1               /**< VS */
#define ADC_CH_LED0         2               /**< VLED0 = VS-VOUT0 */
#define ADC_CH_LED1         3               /**< VLED1 = VS-VOUT1 */
#define ADC_CH_LED2         4               /**< VLED2 = VS-VOUT2 */
#define ADC_CH_LED3         5               /**< VLED3 = VS-VOUT3 */
#define ADC_CH_OUT0         6               /**< VOUT0 */
#define ADC_CH_OUT1         7               /**< VOUT1 */
#define ADC_CH_OUT2         8               /**< VOUT2 */
#define ADC_CH_OUT3         9               /**< VOUT3 */
#define ADC_CH_JTAG_IO     10               /**< VJTAG_IO */
#define ADC_CH_VTEMP       11               /**< VTEMP */
#define ADC_CH_MAX          12

#define TRIGGER_POSITION_INVALID        0xFFFFU

#define ADC_FILTER_SIZE     16

/***************************************************************************//**
 * ADC_CTRL IRQ vector numbers
 ******************************************************************************/
typedef enum {
  adc_ctrl_IRQ_EVT_UPDATE_0           =           0u,
  adc_ctrl_IRQ_EVT_UPDATE_1           =           1u,
  adc_ctrl_IRQ_EVT_UPDATE_2           =           2u,
  adc_ctrl_IRQ_EVT_UPDATE_3           =           3u,
  adc_ctrl_IRQ_EVT_UPDATE_4           =           4u,
  adc_ctrl_IRQ_EVT_UPDATE_5           =           5u,
  adc_ctrl_IRQ_EVT_UPDATE_6           =           6u,
  adc_ctrl_IRQ_EVT_UPDATE_7           =           7u,
  adc_ctrl_IRQ_EVT_UPDATE_8           =           8u,
  adc_ctrl_IRQ_EVT_UPDATE_9           =           9u,
  adc_ctrl_IRQ_EVT_UPDATE_10          =           10u,
  adc_ctrl_IRQ_EVT_UPDATE_11          =           11u,
  adc_ctrl_IRQ_READY                  =           12u,

  adc_ctrl_INTERRUPT_VECTOR_CNT       =           13u  /**< Number of available interrupt vectors */
} adc_ctrl_eInterruptVectorNum_t;

typedef void (*tAdcIrqCbk) (adc_ctrl_eInterruptVectorNum_t irqvecnum, uint16_t contextdata);


typedef struct tagAdcFilter
{
    uint16_t filterBuffer[ADC_FILTER_SIZE];
    uint16_t filterOut;
    uint16_t filterSum;
    uint8_t filterIndex;
} tAdcFilter;

static void HalAdcInitInterrupt(void);
static void AdcFilterInit(tAdcFilter *adFilter, uint16_t defaultValue);
static void AdcFilterAdd(tAdcFilter *adFilter, uint16_t data);
__interrupt static void HalAdcInterruptHandler(void);
static void HalAdcLedEventHandler(adc_ctrl_eInterruptVectorNum_t irqsrc, uint16_t contextdata);
static void HalAdcSystemEventHandler(adc_ctrl_eInterruptVectorNum_t irqsrc, uint16_t contextdata);

static struct {
	tAdcIrqCbk irqCallback[adc_ctrl_INTERRUPT_VECTOR_CNT];
	uint16_t dma[ADC_CH_MAX];
	tLedAdcResult ledResult[3];
	struct
	{
	    uint16_t triggerOnTime;
	    uint16_t triggerOffTime;
	    uint16_t adLedOnTime;
	    uint16_t adLedOffTime;
	    uint16_t adVoutOnTime;
	    uint16_t adVoutOffTime;
	    enum
	    {
	        ONTIME_CONVERSION,
	        OFFTIME_CONVERSION
	    } convTiming;
	} ledDiagInfo[3];
} adc;

static tAdcFilter adcFilterVs, adcFilterVtemp;


void HalAdcInit(void)
{
    uint8_t i;

    ADC_CTRL_CONTROL = 0U;      /* no cyclic conversion */
    ADC_CTRL_ADCF12_CONFIG_bit.power_always_on = 0U;    /* ADC automatically turns OFF */
    ADC_CTRL_ADCF12_CONFIG_bit.cdiv = 1U;       /* ADC clock = 24MHz/2 = 12MHz */
    ADC_CTRL_ADCF12_CONFIG_bit.tsamp = 0u;    	/* ADC sampling time = 32cycle */

    ADC_CTRL_SAMPLE_EXT = 235u;  				/* ADC sample time extension - do not change */

    ADC_CTRL_DMA_BASE_ADDR = (uint16_t)(&adc.dma);

    /* Start initial conversion for battery and temperature */
    ADC_CTRL_IRQ_STATUS = 0x1FFFu;   /* clear flag */
    ADC_CTRL_REG_TRIGGER = (1U<<ADC_CH_VS) | (1U<<ADC_CH_VTEMP);
    
    while( (ADC_CTRL_IRQ_STATUS & (1U<<ADC_CH_VS) ) == 0u)  /* VS will be converted first */
    {
        /* wait */
    }
    AdcFilterInit(&adcFilterVs, adc.dma[ADC_CH_VS]); /* initialize with 12V */
    while( (ADC_CTRL_IRQ_STATUS & (1U<<ADC_CH_VTEMP) ) == 0u)  /* VS will be converted first */
    {
        /* wait */
    }
    AdcFilterInit(&adcFilterVtemp, adc.dma[ADC_CH_VTEMP]);  /* initialize with 25degC */

    for(i=0U; i<3U; i++)
    {
        adc.ledDiagInfo[i].convTiming = ONTIME_CONVERSION;
        adc.ledDiagInfo[i].triggerOnTime = TRIGGER_POSITION_INVALID;
        adc.ledDiagInfo[i].triggerOffTime = TRIGGER_POSITION_INVALID;
        adc.ledDiagInfo[i].adLedOffTime = ADC_RESULT_INVALID;
        adc.ledDiagInfo[i].adLedOnTime = ADC_RESULT_INVALID;
        adc.ledDiagInfo[i].adVoutOffTime = ADC_RESULT_INVALID;
        adc.ledDiagInfo[i].adVoutOnTime = ADC_RESULT_INVALID;

        ADC_CTRL_TRIGGER_DELAY[i] = TRIGGER_POSITION_INVALID;
    }

    ADC_CTRL_PWM0_CONFIG = (1U<<ADC_CH_LED0) | (1U<<ADC_CH_OUT0);
    ADC_CTRL_PWM1_CONFIG = (1U<<ADC_CH_LED1) | (1U<<ADC_CH_OUT1);
    ADC_CTRL_PWM2_CONFIG = (1U<<ADC_CH_LED2) | (1U<<ADC_CH_OUT2);
    ADC_CTRL_PWM3_CONFIG = (1U<<ADC_CH_VS) | (1U<<ADC_CH_VTEMP);

    ADC_CTRL_TRIGGER_DELAY[3] = 0U;

    HalAdcInitInterrupt();
}


void HalAdcSetTriggerDelay(uint16_t ch, uint16_t trigger)
{
    ADC_CTRL_TRIGGER_DELAY[ch] = trigger;
}



tLedAdcResult* HalAdcGetLedResult(uint16_t ch)
{
    return &adc.ledResult[ch];
}



uint16_t HalAdcGetVbat(void)
{
    return adcFilterVs.filterOut;
}


uint16_t HalAdcGetVtemp(void)
{
    return adcFilterVtemp.filterOut;
}

static void HalAdcInitInterrupt(void)
{
    uint8_t i;

    for(i=0U; i<(uint8_t)adc_ctrl_INTERRUPT_VECTOR_CNT; i++)
    {
        adc.irqCallback[i] = (tAdcIrqCbk)0U;
    }

    vic_IfFuns->RegisterModule(vic_IRQ_ADC_CTRL, &HalAdcInterruptHandler, 0U);
    vic_IfFuns->EnableModule(vic_IRQ_ADC_CTRL);

    /* Enable interrupt for VLED0 */
    adc.irqCallback[ADC_CH_OUT0] = &HalAdcLedEventHandler;
    ADC_CTRL_IRQ_VENABLE = ADC_CH_OUT0;
    /* Enable interrupt for VLED1 */
    adc.irqCallback[ADC_CH_OUT1] = &HalAdcLedEventHandler;
    ADC_CTRL_IRQ_VENABLE = ADC_CH_OUT1;
    /* Enable interrupt for VLED2 */
    adc.irqCallback[ADC_CH_OUT2] = &HalAdcLedEventHandler;
    ADC_CTRL_IRQ_VENABLE = ADC_CH_OUT2;

    /* Enable interrupt for VS, VTEMP */
    adc.irqCallback[ADC_CH_VTEMP] = &HalAdcSystemEventHandler;
    ADC_CTRL_IRQ_VENABLE = ADC_CH_VTEMP;

}


static void HalAdcLedEventHandler(adc_ctrl_eInterruptVectorNum_t irqsrc, uint16_t contextdata)
{
    uint16_t plugOffset = (uint16_t)irqsrc-ADC_CH_OUT0;

    tLedAdcResult *pLedResult = &adc.ledResult[plugOffset];

    pLedResult->vLed = adc.dma[ADC_CH_LED0 + plugOffset];
    pLedResult->vOut = adc.dma[ADC_CH_OUT0 + plugOffset];

    pLedResult->ready = 1u;
}


static void HalAdcSystemEventHandler(adc_ctrl_eInterruptVectorNum_t irqsrc, uint16_t contextdata)
{
    AdcFilterAdd(&adcFilterVs, adc.dma[ADC_CH_VS]);
    AdcFilterAdd(&adcFilterVtemp, adc.dma[ADC_CH_VTEMP]);
}


static void AdcFilterInit(tAdcFilter *adFilter, uint16_t defaultValue)
{
    uint8_t i;

    for(i=0U; i<ADC_FILTER_SIZE; i++)
    {
        adFilter->filterBuffer[i] = defaultValue;
    }
    adFilter->filterIndex = 0U;
    adFilter->filterOut = defaultValue;
    adFilter->filterSum = defaultValue*ADC_FILTER_SIZE;
}

static void AdcFilterAdd(tAdcFilter *adFilter, uint16_t data)
{
    adFilter->filterSum -= adFilter->filterBuffer[adFilter->filterIndex];
    adFilter->filterSum += data;
    adFilter->filterBuffer[adFilter->filterIndex] = data;
    adFilter->filterIndex++;
#if ADC_FILTER_SIZE == 16
    adFilter->filterOut = adFilter->filterSum/ADC_FILTER_SIZE;
    adFilter->filterIndex &= (ADC_FILTER_SIZE-1u);
#else
#error "ADC_FILTER_SIZE is not aligned"
#endif
}

/***************************************************************************//**
 * @implementation
 * Every hardware ADC_CTRL module has it's own interrupt handler, also because each
 * module has it's own interrupt vector. So the module base address is fixed.
 * The interrupt vector table is provided by the user-application from RAM. The
 * module specific vector tables lie behind the VIC interrupt vector table, which
 * address is saved in the "TABLE_BASE". The module interrupt handler
 * gets the address from the function "vic_GetPointerToEviornmentData".
 * The interrupt vector number is saved in a local variable, because the value
 * of IRQ_VECTOR could possible change during processing of
 * adc_ctrl_ADC_CTRL0_InterruptHandler. Next the registered interrupt callback function
 * (adc_ctrl_RegisterInterruptCallback) is copied into a local function pointer,
 * checked if it's not NULL and at least called. At the end the interrupt
 * request flag is cleared, by writing back the interrupt vector number to
 * IRQ_VNO register.
 *
 ******************************************************************************/
 __interrupt static void HalAdcInterruptHandler(void)
{
  adc_ctrl_eInterruptVectorNum_t irqvecnum = (adc_ctrl_eInterruptVectorNum_t) ADC_CTRL_IRQ_VNO;

  if (irqvecnum < adc_ctrl_INTERRUPT_VECTOR_CNT) /* ensure the IRQ trigger is not already gone away until processing starts */
  {
//    adc_ctrl_sInterruptEnvironmentData_t* evironmentData = (adc_ctrl_sInterruptEnvironmentData_t*) vic_GetPointerToEnvironmentData(vic_IRQ_ADC_CTRL);

	  tAdcIrqCbk fptr;

    uint16_t vicVmaxBackup = 0;
    uint16_t adc_ctrlVmaxBackup = 0;

    /* IRQ nesting on VIC level: enabled by default */
    vicVmaxBackup = VIC_IRQ_VMAX;
    VIC_IRQ_VMAX = vic_IRQ_ADC_CTRL;

    /* IRQ nesting on module level: by default disabled, handler may override this later on */
    adc_ctrlVmaxBackup = ADC_CTRL_IRQ_VMAX;
    ADC_CTRL_IRQ_VMAX = 0;

//    fptr = evironmentData->InterrupVectorTable[irqvecnum];
    fptr = adc.irqCallback[irqvecnum];

    if(fptr != NULL)
    {
      __enable_interrupt();

      /* Call interrupt callback function */
      fptr(irqvecnum,  0/*(void*) evironmentData->ContextData*/);

      __disable_interrupt();
    }
    else
    {
      /* if there is no handler function for a particular IRQ, disable this IRQ  */
      ADC_CTRL_IRQ_VDISABLE = (uint16_t) irqvecnum;
    }

    /* Clear interrupt request flag */
    ADC_CTRL_IRQ_VNO = (uint16_t) irqvecnum;

    /* IRQ nesting on module level */
    ADC_CTRL_IRQ_VMAX = adc_ctrlVmaxBackup;

    /* IRQ nesting on VIC level */
    VIC_IRQ_VMAX = vicVmaxBackup;
  }
  else {}

  /* RETI will reenable IRQs here */
}



