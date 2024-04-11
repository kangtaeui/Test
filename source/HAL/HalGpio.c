/**
 *  \file    HalGpio.c
 *  \brief   
 *  \date    2017. 9. 13.
 *  \author  felix
 *  \version 
 */

#include "HalGpio.h"
#include "el_helper_H430_IAR.h"
#include "SysConfig.h"

/*******************************************************************************
 * Init GPO
*******************************************************************************/
#define GPIO_OUT0_LOW() (GPIO_DATA_OE |= 0x01)//LOW, PULL-DOWN
#define GPIO_OUT0_HIGH() (GPIO_DATA_OE &= ~0x01)//HIGH, INPUT, WE need PULL-UP
#define GPIO_OUT1_LOW() (GPIO_DATA_OE |= 0x02)
#define GPIO_OUT1_HIGH() (GPIO_DATA_OE &= ~0x02)
#define GPIO_OUT2_LOW() (GPIO_DATA_OE |= 0x04)
#define GPIO_OUT2_HIGH() (GPIO_DATA_OE &= ~0x04)
#define GPIO_OUT3_LOW() (GPIO_DATA_OE |= 0x08)
#define GPIO_OUT3_HIGH() (GPIO_DATA_OE &= ~0x08)
   
void InitGpioOut(void)
{
/*******************************************************************************
  GPIO_SELECT = 0000 1100
                xxx0 xxxx ; Set JTAG IO to JTAG
                xxxx 1xxx ; Set OUT3 to GPIO
                xxxx x1xx ; Set OUT2 to GPIO
                xxxx xx0x ; Set OUT1 to PWM
                xxxx xxx0 ; Set OUT0 to PWM

*******************************************************************************/  
  GPIO_SELECT |= 0x0c;

/*******************************************************************************
  PWM_CONTROL =  0000 1100 1100 0000
                 000x xxxx xxxx xxxx ; Not Used
                 xxx0 xxxx xxxx xxxx ; Set JTAG IO to NO PULL UP
                 xxxx 1xxx xxxx xxxx ; Set OUT3 to PULL UP
                 xxxx x1xx xxxx xxxx ; Set OUT2 to PULL UP
                 xxxx xx0x xxxx xxxx ; Set OUT1 to NO PULL UP
                 xxxx xxx0 xxxx xxxx ; Set OUT0 to NO PULL UP
                 xxxx xxxx 1xxx xxxx ; Enable OUT3 MODULE
                 xxxx xxxx x1xx xxxx ; Enable OUT2 MODULE
                 xxxx xxxx xx0x xxxx ; Disable OUT1 MODULE
                 xxxx xxxx xxx0 xxxx ; Disable OUT0 MODULE
                 xxxx xxxx xxxx 0xxx ; Disable OUT3 PWM
                 xxxx xxxx xxxx x0xx ; Disable OUT2 PWM
                 xxxx xxxx xxxx xx0x ; Disable OUT1 PWM
                 xxxx xxxx xxxx xxx0 ; Disable OUT0 PWM
*******************************************************************************/      
  PWM_CONTROL = 0x0cc0;
  
/*******************************************************************************
  GPIO_DATA_IE =  0000 0000 0000 0000
                  0000 0000 0000 xxxx ; Not Used
                  xxxx xxxx xxx0 xxxx ; Disable JTAG IO INPUT PATH
                  xxxx xxxx xxxx 0xxx ; Disable OUT3 INPUT PATH
                  xxxx xxxx xxxx x0xx ; Disable OUT2 INPUT PATH
                  xxxx xxxx xxxx xx0x ; Disable OUT1 INPUT PATH
                  xxxx xxxx xxxx xxx0 ; Disable OUT0 INPUT PATH
*******************************************************************************/    
  GPIO_DATA_IE = 0x0000;

/*******************************************************************************
  GPIO_DATA_OUT =  0000 0000
                   xxx0 xxxx ; Set JTAG IO to INPUT
                   000x 0000; Not Used

*******************************************************************************/ 
  GPIO_DATA_OUT = 0x00;
    
/*******************************************************************************
  GPIO_DATA_OE =  0000 0000
                  xxx0 xxxx ; Set JTAG IO to INPUT
                  xxxx 0xxx ; Set OUT3 to INPUT
                  xxxx x0xx ; Set OUT2 to INPUT
                  xxxx xx0x ; Set OUT1 to INPUT
                  xxxx xxx0 ; Set OUT0 to INPUT

*******************************************************************************/  
  GPIO_DATA_OE = 0x00;

}


/*******************************************************************************
 * Init GPI
*******************************************************************************/
#define GET_GPIO_OUT0(data) ((data)=(GPIO_DATA_IN&0x01);)
#define GET_GPIO_OUT1(data) ((data)=(GPIO_DATA_IN&0x02);)
#define GET_GPIO_OUT2(data) ((data)=(GPIO_DATA_IN&0x04);)
#define GET_GPIO_OUT3(data) ((data)=(GPIO_DATA_IN&0x08);)

void InitGpioInput(void)
{
/*******************************************************************************
  GPIO_SELECT = 0000 1000
                xxx0 xxxx ; Set JTAG IO to JTAG
                xxxx 1xxx ; Set OUT3 to GPIO
                xxxx x1xx ; Set OUT2 to GPIO
                xxxx xx0x ; Set OUT1 to PWM
                xxxx xxx0 ; Set OUT0 to PWM

*******************************************************************************/  
  GPIO_SELECT = 0x0c;

/*******************************************************************************
  PWM_CONTROL =  0000 0000 1111 0000
                 000x xxxx xxxx xxxx ; Not Used
                 xxx0 xxxx xxxx xxxx ; Set JTAG IO to NO PULL UP
                 xxxx 1xxx xxxx xxxx ; Set OUT3 to PULL UP
                 xxxx x0xx xxxx xxxx ; Set OUT2 to NO PULL UP
                 xxxx xx0x xxxx xxxx ; Set OUT1 to NO PULL UP
                 xxxx xxx0 xxxx xxxx ; Set OUT0 to NO PULL UP
                 xxxx xxxx 1xxx xxxx ; Enable OUT3 MODULE
                 xxxx xxxx x1xx xxxx ; Enable OUT2 MODULE
                 xxxx xxxx xx1x xxxx ; Enable OUT1 MODULE
                 xxxx xxxx xxx1 xxxx ; Enable OUT0 MODULE
                 xxxx xxxx xxxx 0xxx ; Disable OUT3 PWM
                 xxxx xxxx xxxx x0xx ; Disable OUT2 PWM
                 xxxx xxxx xxxx xx0x ; Disable OUT1 PWM
                 xxxx xxxx xxxx xxx0 ; Disable OUT0 PWM
*******************************************************************************/      
  PWM_CONTROL = 0x08f0;
  
/*******************************************************************************
  GPIO_DATA_IE =  0000 0000 0000 1000
                  0000 0000 0000 xxxx ; Not Used
                  xxxx xxxx xxx0 xxxx ; Disable JTAG IO INPUT PATH
                  xxxx xxxx xxxx 1xxx ; Disable OUT3 INPUT PATH
                  xxxx xxxx xxxx x0xx ; Disable OUT2 INPUT PATH
                  xxxx xxxx xxxx xx0x ; Disable OUT1 INPUT PATH
                  xxxx xxxx xxxx xxx0 ; Disable OUT0 INPUT PATH
*******************************************************************************/    
  GPIO_DATA_IE = 0x0008;

/*******************************************************************************
  GPIO_DATA_OE =  0000 0000
                  xxx0 xxxx ; Set JTAG IO to INPUT
                  xxxx 0xxx ; Set OUT3 to INPUT
                  xxxx x0xx ; Set OUT2 to INPUT
                  xxxx xx0x ; Set OUT1 to INPUT
                  xxxx xxx0 ; Set OUT0 to INPUT

*******************************************************************************/  
  GPIO_DATA_OE = 0x00;
}

void GpioOutTest(void)
{
  uint16_t i,j;

  
  InitGpioOut();
  
  for(;;)
  {
      WDOG_CONTROL = 0xA502u; /* restart watchdog */
      
      GPIO_OUT3_LOW();  
      
      for(i=0;i<250;i++)
      {
        WDOG_CONTROL = 0xA502u; /* restart watchdog */
        for(j=0;j<295;j++)
          asm(" nop");
      }//50ms delay
      
      GPIO_OUT3_HIGH();  
     
      for(i=0;i<250;i++)
      {
        WDOG_CONTROL = 0xA502u; /* restart watchdog */
        for(j=0;j<295;j++)
          asm(" nop");
      }//50ms delay
  }
}

void GpioInputTest(void)
{
  static uint16_t index=0;
  uint16_t key=0;
  
  InitGpioInput();

  GPIO_OUT2_HIGH();// Init OUTPUT2 to turn off LED
            
  for(;;)
  {
      WDOG_CONTROL = 0xA502u; /* restart watchdog */
      
      key=GPIO_DATA_IN&0x08;//Read OUT3
      if(index == 1)//if key was on,
      {
        if(key!=0)//if key is off,
        {
            GPIO_OUT2_HIGH();  
            index = 0;
        }
      }
      else
      {
        if(key==0)//if key is on,
        {
            GPIO_OUT2_LOW();  
            index = 1;
        }        
      }//if(index == 1)
  }//for(;;)  
}
