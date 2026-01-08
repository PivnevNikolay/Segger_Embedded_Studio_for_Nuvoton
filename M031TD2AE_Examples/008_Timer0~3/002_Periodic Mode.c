/**-----------------------------------------------------------------------
 *
 | ** Bерсия драйверов V7.92 [2023-08-11]                              ** |
 | ** Код написан в IDE SEGGER Embedded Studio for ARM V7.32a (64-bit) ** |
 *    
 * 
 *     M032SE3AE
 *   ------------ 
 *  |            |
 *  |            |
 *  |        PA.0| --->  LED   _|¯|_  Period= 1.00s +Width= 500ms -Width= 500ms
 *  |            |
 *  |            |
 *  |      +3.3V | --->   +3.3V
 *  |        GND | --->   GND 
 *
 *  TIMER PERIODIC MODE --> Пример с использованием прерывания.
 *
 *
 * \сode debugging ScuratovaAnna, PivnevNikolay
 *
 *************************************************************************/

#include "NuMicro.h"
#include <stdio.h>

//********************************************************************
void SYS_Init(void) {
  SYS_UnlockReg();
  CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);
  CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);
  CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));
  CLK->PCLKDIV = (CLK_PCLKDIV_APB0DIV_DIV1 | CLK_PCLKDIV_APB1DIV_DIV1);
  CLK_EnableModuleClock(TMR0_MODULE);
  CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_HIRC, MODULE_NoMsk); // MODULE_NoMsk --> 0
  // CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_HIRC, 0);
  SystemCoreClockUpdate();
//*************************** GPIO PA.0 ******************************
  SYS->GPA_MFPL = (SYS->GPA_MFPL & ~(SYS_GPA_MFPL_PA0MFP_Msk)) | (SYS_GPA_MFPL_PA0MFP_GPIO);
  SYS_LockReg();
}
//********************************************************************
void TIMER_Init(void) {
  uint32_t u32_ticks = (SystemCoreClock / 1000000);
  TIMER_SET_OPMODE(TIMER0, TIMER_PERIODIC_MODE);
  TIMER_SET_PRESCALE_VALUE(TIMER0, u32_ticks - 1);
  //TIMER_SET_CMP_VALUE(TIMER0, 5);// LED  _|¯|_  Period= 10.00us +Width= 5.0us -Width= 5.0us
  TIMER_SET_CMP_VALUE(TIMER0, 500000);
  TIMER_EnableInt(TIMER0);
  NVIC_EnableIRQ(TMR0_IRQn);
  TIMER_Start(TIMER0);
}
//********************************************************************
void LED_GPIO_init(void) {
  GPIO_SetMode(PA, BIT0, GPIO_MODE_OUTPUT);
  PA0 = 1;
}
//********************************************************************
void TMR0_IRQHandler(void) {
  if (TIMER_GetIntFlag(TIMER0)) {
    PA0 ^= 1;
    TIMER_ClearIntFlag(TIMER0);
  }
}
//********************************************************************
int main(void) {
  SYS_Init();
  LED_GPIO_init();
  TIMER_Init();
  while (1) {

  }
}