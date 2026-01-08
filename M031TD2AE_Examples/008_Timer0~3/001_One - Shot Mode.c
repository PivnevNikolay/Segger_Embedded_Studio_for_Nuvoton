/**-----------------------------------------------------------------------
 \date  09.01.2025
 *
 | ** Bерсия драйверов V7.92 [2023-08-11]                              ** |
 | ** Код написан в IDE SEGGER Embedded Studio for ARM V7.32a (64-bit) ** |
 *    
 * 
 *     M032SE3AE
 *   ------------ 
 *  |            |
 *  |            |
 *  |        PA.0| --->  _|¯|_ 
 *  |            |
 *  |            |
 *  |      +3.3V | --->   +5V
 *  |        GND | --->   GND 
 *
 *  TIMER ONESHOT MODE --> Пример без прерываний (опрос флага).
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
  // CLK_EnableModuleClock(ISP_MODULE);
  CLK_EnableModuleClock(TMR0_MODULE);
  CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_HIRC, MODULE_NoMsk); // MODULE_NoMsk --> 0
  // CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_HIRC, 0);
  SystemCoreClockUpdate();
  //*************************** GPIO PA.0 ****************************
  SYS->GPA_MFPL = (SYS->GPA_MFPL & ~(SYS_GPA_MFPL_PA0MFP_Msk)) | (SYS_GPA_MFPL_PA0MFP_GPIO); // PA.0
  SYS_LockReg();
}

void delay_us(uint32_t us) {
  uint32_t u32_ticks = (SystemCoreClock / 1000000);
  TIMER_SET_OPMODE(TIMER0, TIMER_ONESHOT_MODE);
  TIMER_SET_PRESCALE_VALUE(TIMER0, u32_ticks - 1);
  TIMER_SET_CMP_VALUE(TIMER0, us);
  TIMER_Start(TIMER0);
  while (!TIMER_GetIntFlag(TIMER0)); // Ожидание завершения
  TIMER_ClearIntFlag(TIMER0);        // Сброс флага
}

void LED_GPIO_init(void) {
  GPIO_SetMode(PA, BIT0, GPIO_MODE_OUTPUT);
  PA0 = 1;
}

int main(void) {
  SYS_Init();
  LED_GPIO_init();
  while (1) {
    PA0 ^= 1;
    delay_us(300);
  }
}
//************************** End of file ****************************