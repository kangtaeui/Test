
#include <el_helper.h>
#include "CommonInterruptHandler.h"
#include "rom_API.h"

#include "HalSys.h"
#include "HalTimer.h"
#include "HalGpio.h"

#define eint() asm("eint \n\
                    nop")                    
#define dint() asm("dint \n\
                    nop")

vic_cpInterfaceFunctions_t vic_IfFuns; /* VIC Interface pointer */

void main(void)
{ 
    dint();
/*******************************************************************************
    Init Clocks
*******************************************************************************/
    SYS_STATE_CONTROL_bit.pwm_clk_sel = 1u;
    SYS_STATE_CONTROL_bit.sys_clk_sel = 1u;
    
/*******************************************************************************
    Init Vector Interrupt Controller
*******************************************************************************/ 
    if(romIf_MainInterface->Interface_Get(ROMIF_API_VIC, (romIf_cpGenericInterface_t *) &vic_IfFuns, NULL) != TRUE)
    {
        while(1);
    }
    InterruptHandlerInitialize(vic_IfFuns);
    
    HalTimerInit();

    eint();

/*******************************************************************************
    JTAG PORT --> GPIO
*******************************************************************************/       
#if 0
    SYS_STATE_TEST_MODE = 0xA500; /* Disable DEBUG */
    GPIO_SELECT |= 0x10u;  /* JTAG_IO as GPIO */
    GPIO_DATA_OE = 0x10u; /* JTAG_IO as GPO */

    //PWM_CONTROL |= 0x1000u; /* pull up enable on JTAG_IO */
    GPIO_DATA_OUT = 0x00u; /* Reset JTAG IO */
    //GPIO_DATA_OUT = 0x10u; /* Set JTAG IO */   
#endif
    
/*******************************************************************************
    GPIO TEST
*******************************************************************************/      
    //GpioOutTest();    
    GpioInputTest(); 
    
    for(;;)
    {
/*******************************************************************************
    Control Duty Ratio
*******************************************************************************/
        if(HalTimerGetFlagTask() != 0u)
        {   
            asm(" NOP");
        }
        
    }//for(;;)
}//main()


