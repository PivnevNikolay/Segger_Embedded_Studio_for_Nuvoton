/**-------------------------------------------------------------------
 \date  02.06.2025
 *
 *    M031EC1AE    
 *   ------------ 
 *  |            |
 *  |            |
 *  |            |
 *  |        PB.2| ---->  LED
 *  |            | 
 *  |       PB.14| ---->  LED
 *  |            |
 *  |      +3.3V |
 *  |        GND |
 *
 *\ authors ScuratovaAnna, PivnevNikolay
 *\ сode debugging PivnevNikolay 
 */
//******************************  Пример первый **************************
#include "NuMicro.h"
#include "stdbool.h"
#include <stdio.h>

#define SET_BIT(REG, BIT) ((REG) |= (BIT))
#define CLEAR_BIT(REG, BIT) ((REG) &= ~(BIT))
#define READ_BIT(REG, BIT) ((REG) & (BIT))
#define CLEAR_REG(REG) ((REG) = (0x0))
#define WRITE_REG(REG, VAL) ((REG) = (VAL))
#define READ_REG(REG) ((REG))
#define MODIFY_REG(REG, CLEARMASK, SETMASK) WRITE_REG((REG), (((READ_REG(REG)) & (~(CLEARMASK))) | (SETMASK)))

void SYS_Init(void) {
 SYS_UnlockReg();
 CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);
 CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);
 CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));

 CLK_EnableModuleClock(UART0_MODULE);
 CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));

 CLK_EnableModuleClock(TMR0_MODULE);
 CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_HIRC, 0);

 SystemCoreClockUpdate();
 SYS->GPB_MFPL = (SYS->GPB_MFPL & ~(SYS_GPB_MFPL_PB0MFP_Msk))  | (SYS_GPB_MFPL_PB0MFP_GPIO); // PB.2
 SYS->GPB_MFPH = (SYS->GPB_MFPH & ~(SYS_GPB_MFPH_PB14MFP_Msk ))| (SYS_GPB_MFPH_PB14MFP_GPIO); // PB.14
 SYS_LockReg();
}

int main(void) {
 SYS_Init();
 // GPIO_SetMode(PA, BIT0, GPIO_MODE_OUTPUT);
  MODIFY_REG(PB->MODE,GPIO_MODE_MODE0_Msk, GPIO_MODE_OUTPUT << GPIO_MODE_MODE0_Pos);
  MODIFY_REG(PB->MODE,GPIO_MODE_MODE14_Msk,GPIO_MODE_OUTPUT << GPIO_MODE_MODE14_Pos);
  SET_BIT(PB->DOUT, GPIO_DOUT_DOUT0_Msk);
  SET_BIT(PB->DOUT, GPIO_DOUT_DOUT14_Msk);
  TIMER_Delay(TIMER0, 1000000);
  while (1) {
  CLEAR_BIT(PB->DOUT, GPIO_DOUT_DOUT0_Msk);
  CLEAR_BIT(PB->DOUT, GPIO_DOUT_DOUT14_Msk);
  TIMER_Delay(TIMER0, 900000);
  SET_BIT(PB->DOUT, GPIO_DOUT_DOUT0_Msk);
  SET_BIT(PB->DOUT, GPIO_DOUT_DOUT14_Msk);
  TIMER_Delay(TIMER0, 900000);
  }
} 
/*************************** End of file ****************************/


/**-----------------------------------------------------------------------
 *\date  28.05.2023
 *\brief
 *
 * ** За основу взят примеры Level1_Training.          **
 * **        пример поморгаем светодиодами.            **
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
 * ARDUINO MULTI-FUNCTION SHIELD
 * Идея и примеры взяты отсюда:
 * http://publicatorbar.ru/2017/12/21/arduino-multi-function-shield/
 * Все примеры реализованы с применением ARDUINO MULTI-FUNCTION SHIELD
//************************************************************************
 *\brief
 */
//******************************  Пример второй **************************
//******************************  StdDriver  *****************************

#include "NuMicro.h"
#include "stdbool.h"
#include <stdio.h>

#define Bit_SET 1
#define Bit_RESET 0

volatile bool i = 0;

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
void UART0_Init(void)
{ // Reset UART0 
  SYS_ResetModule(UART0_RST);
  // Configure UART0 and set UART0 baud rate 
  UART_Open(UART0, 115200);
}
//************************************************************************
int main(void) {
  SYS_Init();
  LED_GPIO_init();
  UART0_Init();
  while (1) {
    (i == 0) ? (i = Bit_SET, printf("Led Set\n")) : (i = Bit_RESET, printf("Led ReSet\n"));
    PA0 = i;
    PA1 = i;
    PA2 = i;
    PA3 = i;
    TIMER_Delay(TIMER0, 1000000);
  }
}
/****************************** End of file *****************************/



//******************************  Пример третий **************************
//******************************  RegBased  ******************************
#include "NuMicro.h"
#include "stdbool.h"
#include <stdio.h>

#define Bit_SET 1
#define Bit_RESET 0

volatile bool i = 0;

//************************************************************************
void SYS_Init(void) {
  SYS_UnlockReg();
  //********* Enable HIRC clock (Internal RC 48MHz)  **************//
  // Enable HIRC clock (Internal RC 48MHz)
  // CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);
  CLK->PWRCTL |= CLK_PWRCTL_HIRCEN_Msk;
  /* Waiting for HIRC clock ready */
  while ((CLK->STATUS & CLK_STATUS_HIRCSTB_Msk) != CLK_STATUS_HIRCSTB_Msk);
  /* Switch HCLK clock source to HIRC */
  CLK->CLKSEL0 = (CLK->CLKSEL0 & ~CLK_CLKSEL0_HCLKSEL_Msk) | CLK_CLKSEL0_HCLKSEL_HIRC;
  CLK->CLKDIV0 = (CLK->CLKDIV0 & ~CLK_CLKDIV0_HCLKDIV_Msk) | CLK_CLKDIV0_HCLK(1);
  /* Waiting for HIRC clock ready */
  while((CLK->STATUS & CLK_STATUS_HIRCSTB_Msk) != CLK_STATUS_HIRCSTB_Msk);
  //********* UART0_MODULE  ***************************************//
  CLK->CLKSEL1 = (CLK->CLKSEL1 & ~CLK_CLKSEL1_UART0SEL_Msk) | CLK_CLKSEL1_UART0SEL_HIRC;
  CLK->CLKDIV0 = (CLK->CLKDIV0 & ~CLK_CLKDIV0_UART0DIV_Msk) | CLK_CLKDIV0_UART0(1);
  /* Enable UART0 peripheral clock */
  CLK->APBCLK0 |= (CLK_APBCLK0_UART0CKEN_Msk | CLK_APBCLK0_TMR0CKEN_Msk);
  //********* TMR0_MODULE  ****************************************//
  CLK->CLKSEL1 = (CLK->CLKSEL1 & ~CLK_CLKSEL1_TMR0SEL_Msk) | CLK_CLKSEL1_TMR0SEL_HIRC;

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
  PA->MODE = (PA->MODE & ~(GPIO_MODE_MODE0_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE0_Pos);
/*-----------------------------------------------------------------------*/
/* обнуляем значения регистра MODE0 --> MODE0_0 =0 MODE0_1 = 0           */
/* PA->MODE &= ~((0x1UL << 1)|(0x1UL << 0));                             */
/* устанавливаем значение MODE0 = 01 (Push-pull Output mode)             */
/* PA->MODE |=  (0x1UL << 0);                                            */
/*-----------------------------------------------------------------------*/
  PA->MODE = (PA->MODE & ~(GPIO_MODE_MODE1_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE1_Pos);
  PA->MODE = (PA->MODE & ~(GPIO_MODE_MODE2_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE2_Pos);
  PA->MODE = (PA->MODE & ~(GPIO_MODE_MODE3_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE3_Pos);
  PA0 = 1;
//PA->DOUT |=  (0x1UL << 0);
  PA1 = 1;
//PA->DOUT |=  (0x1UL << 1);
  PA2 = 1;
//PA->DOUT |=  (0x1UL << 2);
  PA3 = 1;
//PA->DOUT |=  (0x1UL << 3);
}
/*----------------------------------------------------------------------*/
/* Init UART0                                                           */
/*----------------------------------------------------------------------*/
//************************************************************************
void UART0_Init(void)
{
    /* Reset UART0 */
    SYS->IPRST1 |=  SYS_IPRST1_UART0RST_Msk;
    SYS->IPRST1 &= ~SYS_IPRST1_UART0RST_Msk;
    /* Configure UART0 and set UART0 baud rate */
    UART0->BAUD = UART_BAUD_MODE2 | UART_BAUD_MODE2_DIVIDER(__HIRC, 115200);
    UART0->LINE = UART_WORD_LEN_8 | UART_PARITY_NONE | UART_STOP_BIT_1;
}
//************************************************************************
int main(void) {
  SYS_Init();
  LED_GPIO_init();
  UART0_Init();
  while (1) {
    (i == 0) ? (i = Bit_SET, printf("Led Set\n")) : (i = Bit_RESET, printf("Led ReSet\n"));
    PA0 = i;
    PA1 = i;
    PA2 = i;
    PA3 = i;
    TIMER_Delay(TIMER0, 1000000);
  }
}
/****************************** End of file *****************************/
