/**-----------------------------------------------------------------------
 *\date  08.09.2023
 *\brief
 *
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
 *  |      +3.3V | <---> | +5V
 *  |        GND | <---> |GND 
 *
 *\authors ScuratovaAnna, PivnevNikolay
 *\ сode debugging PivnevNikolay
 *
 *
 *****************  SysTick — Системный таймер  *************************
 * https://easyelectronics.ru/arm-uchebnyj-kurs-systick-sistemnyj-tajmer.html
 * https://github.com/nr-electronics/CMSIS_STM32/blob/main/STM32.%20CMSIS%20%233.%20SysTick%20-%20programming%20sys%20timer/test_CMSIS_KEIL%20with%20SysTick/main.c
 * https://www.youtube.com/watch?v=75AOI-hL7NY 
 * https://microcontrollerslab.com/systick-timer-tm4c123g-arm-cortex-m4-microcontroller/
 * Пример реализован с применением ARDUINO MULTI-FUNCTION SHIELD
 * http://publicatorbar.ru/2017/12/21/arduino-multi-function-shield/
 ************************************************************************
 *\brief
 */
#include "NuMicro.h"
#include "stdbool.h"
#include <stdio.h>

#define Bit_SET 1
#define Bit_RESET 0

__IO bool i = 0;

#define SYSCLOCK 48000000U // макрос нашей системной частоты

//    #define     __IO    volatile             /*!< Defines 'read / write' permissions */
__IO uint32_t SysTick_CNT = 0; //обьявляем и иницализируем в 0 значение нашего счетчика SysTick

void SysTick_Init(void)
{
  SysTick->LOAD &= ~SysTick_LOAD_RELOAD_Msk; //сбрасываем возможные старые значения интервала счета в 0
  SysTick->LOAD = SYSCLOCK/1000 - 1;
  SysTick->VAL &= ~SysTick_VAL_CURRENT_Msk; //сбрасываем текущие значения счетчика в 0
  SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;//запуск счетчика
}

void SysTick_Handler(void)
{
  if(SysTick_CNT > 0)  SysTick_CNT--;
}

void delay_mS(uint32_t mS)
{
  SysTick->VAL &= ~SysTick_VAL_CURRENT_Msk;//сбрасываем старые возможные значения текущего счета в 0
  SysTick->VAL = SYSCLOCK / 1000 - 1;
  SysTick_CNT = mS;
  while(SysTick_CNT) {}// как только будет 0, то выходим из цикла и задержка заканчивается
}


//************************************************************************
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
/*----------------------------------------------------------------------*/
/* Init UART0                                                           */
/*----------------------------------------------------------------------*/
//************************************************************************
void UART0_Init(void){
  SYS_ResetModule(UART0_RST);
  UART_Open(UART0, 115200);
}
//************************************************************************
int main(void) {
  SYS_Init();
  LED_GPIO_init();
  UART0_Init();
  SysTick_Init();
  while (1) {
    (i == 0) ? (i = Bit_SET, printf("Led Set\n")) : (i = Bit_RESET, printf("Led ReSet\n"));
    PA0 = i;
    PA1 = i;
    PA2 = i;
    PA3 = i;
    delay_mS(500);
  }
}
/****************************** End of file *****************************/
