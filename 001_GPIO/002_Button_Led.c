/**-----------------------------------------------------------------------
 *\date  12.08.2023
 *\brief
 *
 * ** За основу взят примеры Level1_Training.          **
 * ** Второй пример кнопки и светодиоды.               **
 * ** Код написан в IDE SEGGER Embedded Studio for ARM **
 *
 *     M031EC1AE          ARDUINO MULTI-FUNCTION SHIELD
 *   ------------         ------------
 *  |            |       |
 *  |            |       |
 *  |            |       |
 *  |        PA.0| ----> | LED D1 Pin 13
 *  |        PA.1| ----> | LED D2 Pin 12
 *  |        PA.2| ----> | LED D3 Pin 11
 *  |        PA.3| ----> | LED D4 Pin 10
 *  |            |       |
 *  |        +5V | <---> | +5V
 *  |        GND | <---> | GND
 *  |            |       |
 *  |        PB.0| <---  | Button A1
 *  |        PB.1| <---  | Button A2
 *  |        PB.2| <---  | Button A3
 *  |            |       |          
 *\authors ScuratovaAnna, PivnevNikolay
 *\ сode debugging PivnevNikolay
 *
 * ARDUINO MULTI-FUNCTION SHIELD
 * Идея и примеры взяты отсюда:
 * http://publicatorbar.ru/2017/12/21/arduino-multi-function-shield/
 * Все примеры реализованы с применением ARDUINO MULTI-FUNCTION SHIELD
**************************************************************************
 *\brief
 */
#include "NuMicro.h"
#include "stdbool.h"
#include <stdio.h>

void SYS_Init(void) {
  SYS_UnlockReg();
  //********* Enable HIRC clock (Internal RC 48MHz)  **************//
  // Enable HIRC clock (Internal RC 48MHz)
  CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);
  CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);
  CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));
  //********* UART0_MODULE  ***************************************//
  CLK_EnableModuleClock(UART0_MODULE);
  CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));
  //********* TMR0_MODULE  ****************************************//
  CLK_EnableModuleClock(TMR0_MODULE);
  CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_HIRC, 0);

  SystemCoreClockUpdate();
  //********* GPIO PA.0 PA.1 PA.2 PA.3 ****************************//
  SYS->GPA_MFPL = (SYS->GPA_MFPL & ~(SYS_GPA_MFPL_PA0MFP_Msk)) | (SYS_GPA_MFPL_PA0MFP_GPIO); // PA.0
  SYS->GPA_MFPL = (SYS->GPA_MFPL & ~(SYS_GPA_MFPL_PA1MFP_Msk)) | (SYS_GPA_MFPL_PA1MFP_GPIO); // PA.1
  SYS->GPA_MFPL = (SYS->GPA_MFPL & ~(SYS_GPA_MFPL_PA2MFP_Msk)) | (SYS_GPA_MFPL_PA2MFP_GPIO); // PA.2
  SYS->GPA_MFPL = (SYS->GPA_MFPL & ~(SYS_GPA_MFPL_PA3MFP_Msk)) | (SYS_GPA_MFPL_PA3MFP_GPIO); // PA.3

  //********* UART0 RXD=PA.15 and TXD=PA.14 ***********************//
  SYS->GPA_MFPH = (SYS->GPA_MFPH & ~(SYS_GPA_MFPH_PA14MFP_Msk | SYS_GPA_MFPH_PA15MFP_Msk)) |
                  (SYS_GPA_MFPH_PA15MFP_UART0_RXD | SYS_GPA_MFPH_PA14MFP_UART0_TXD);

  SYS_LockReg();
}
//************************************************************************
void LED_GPIO_init(void) {
  GPIO_SetMode(PA, BIT0, GPIO_MODE_OUTPUT);
  GPIO_SetMode(PA, BIT1, GPIO_MODE_OUTPUT);
  GPIO_SetMode(PA, BIT2, GPIO_MODE_OUTPUT);
  GPIO_SetMode(PA, BIT3, GPIO_MODE_OUTPUT);
  PA0 = 1;
  PA1 = 1;
  PA2 = 1;
  PA3 = 1;
}

int main(void) {
  SYS_Init();
  LED_GPIO_init();

  while (1) {
    
  }
}

/****************************** End of file *****************************/