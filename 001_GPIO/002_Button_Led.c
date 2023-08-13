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
 *  |      +3.3V | <---> | +5V
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
*************************************************************************/
//******************************  Пример первый **************************
//******************************  StdDriver  *****************************
#include "NuMicro.h"
#include "stdbool.h"
#include <stdio.h>

typedef uint32_t  u32;
typedef uint16_t  u16;
typedef uint8_t   u8;

u8 condition = TRUE;

//#define TRUE      (1UL) --> Boolean true, define to use in API parameters or return value
//#define FALSE     (0UL) --> Boolean false, define to use in API parameters or return value

//#define ENABLE    (1UL) --> Enable, define to use in API parameters
//#define DISABLE   (0UL) --> Disable, define to use in API parameters

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
  //********* GPIO PA.0 PA.1 PA.2 PA.3 ****************************//output
  SYS->GPA_MFPL = (SYS->GPA_MFPL & ~(SYS_GPA_MFPL_PA0MFP_Msk)) | (SYS_GPA_MFPL_PA0MFP_GPIO); // PA.0
  SYS->GPA_MFPL = (SYS->GPA_MFPL & ~(SYS_GPA_MFPL_PA1MFP_Msk)) | (SYS_GPA_MFPL_PA1MFP_GPIO); // PA.1
  SYS->GPA_MFPL = (SYS->GPA_MFPL & ~(SYS_GPA_MFPL_PA2MFP_Msk)) | (SYS_GPA_MFPL_PA2MFP_GPIO); // PA.2

  //********* GPIO PB.0 PB.1 PB.2 ****************************//input
  SYS->GPB_MFPL = (SYS->GPB_MFPL & ~(SYS_GPB_MFPL_PB0MFP_Msk)) | (SYS_GPB_MFPL_PB0MFP_GPIO); // PB.0
  SYS->GPB_MFPL = (SYS->GPB_MFPL & ~(SYS_GPB_MFPL_PB1MFP_Msk)) | (SYS_GPB_MFPL_PB1MFP_GPIO); // PB.1
  SYS->GPB_MFPL = (SYS->GPB_MFPL & ~(SYS_GPB_MFPL_PB2MFP_Msk)) | (SYS_GPB_MFPL_PB2MFP_GPIO); // PB.2
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
  PA0 = 1;
  PA1 = 1;
  PA2 = 1;
  GPIO_SetMode(PB, BIT0, GPIO_MODE_INPUT);
  GPIO_SetMode(PB, BIT1, GPIO_MODE_INPUT);
  GPIO_SetMode(PB, BIT2, GPIO_MODE_INPUT);
  GPIO_SET_DEBOUNCE_TIME(GPIO_DBCTL_DBCLKSRC_LIRC,  GPIO_DBCTL_DBCLKSEL_512);
  GPIO_ENABLE_DEBOUNCE(PB, BIT0 | BIT1 | BIT2);
}

int main(void) {
  SYS_Init();
  LED_GPIO_init();

  while (1) {
  condition = PB0;
  (condition == FALSE)?(PA0 = DISABLE):(PA0 = ENABLE);  
  condition = PB1;
  (condition == FALSE)?(PA1 = DISABLE):(PA1 = ENABLE); 
  condition = PB2;
  (condition == FALSE)?(PA2 = DISABLE):(PA2 = ENABLE);
  }
}
/****************************** End of file *****************************/



//******************************  Пример второй **************************
//******************************  RegBased  ******************************
#include "NuMicro.h"
#include "stdbool.h"
#include <stdio.h>

typedef uint32_t  u32;
typedef uint16_t  u16;
typedef uint8_t   u8;

u8 condition = TRUE;

//#define TRUE      (1UL) --> Boolean true, define to use in API parameters or return value
//#define FALSE     (0UL) --> Boolean false, define to use in API parameters or return value

//#define ENABLE    (1UL) --> Enable, define to use in API parameters
//#define DISABLE   (0UL) --> Disable, define to use in API parameters

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
  //CLK->CLKSEL1 = (CLK->CLKSEL1 & ~CLK_CLKSEL1_UART0SEL_Msk) | CLK_CLKSEL1_UART0SEL_HIRC;
  //CLK->CLKDIV0 = (CLK->CLKDIV0 & ~CLK_CLKDIV0_UART0DIV_Msk) | CLK_CLKDIV0_UART0(1);
  /* Enable UART0 peripheral clock */
  //CLK->APBCLK0 |= (CLK_APBCLK0_UART0CKEN_Msk | CLK_APBCLK0_TMR0CKEN_Msk);
  //********* TMR0_MODULE  ****************************************//
  //CLK->CLKSEL1 = (CLK->CLKSEL1 & ~CLK_CLKSEL1_TMR0SEL_Msk) | CLK_CLKSEL1_TMR0SEL_HIRC;

  SystemCoreClockUpdate();
  //********* GPIO PA.0 PA.1 PA.2 PA.3 ****************************//output
  SYS->GPA_MFPL = (SYS->GPA_MFPL & ~(SYS_GPA_MFPL_PA0MFP_Msk)) | (SYS_GPA_MFPL_PA0MFP_GPIO); // PA.0
  SYS->GPA_MFPL = (SYS->GPA_MFPL & ~(SYS_GPA_MFPL_PA1MFP_Msk)) | (SYS_GPA_MFPL_PA1MFP_GPIO); // PA.1
  SYS->GPA_MFPL = (SYS->GPA_MFPL & ~(SYS_GPA_MFPL_PA2MFP_Msk)) | (SYS_GPA_MFPL_PA2MFP_GPIO); // PA.2

  //********* GPIO PB.0 PB.1 PB.2 ****************************//input
  SYS->GPB_MFPL = (SYS->GPB_MFPL & ~(SYS_GPB_MFPL_PB0MFP_Msk)) | (SYS_GPB_MFPL_PB0MFP_GPIO); // PB.0
  SYS->GPB_MFPL = (SYS->GPB_MFPL & ~(SYS_GPB_MFPL_PB1MFP_Msk)) | (SYS_GPB_MFPL_PB1MFP_GPIO); // PB.1
  SYS->GPB_MFPL = (SYS->GPB_MFPL & ~(SYS_GPB_MFPL_PB2MFP_Msk)) | (SYS_GPB_MFPL_PB2MFP_GPIO); // PB.2
  //********* UART0 RXD=PA.15 and TXD=PA.14 ***********************//
  //SYS->GPA_MFPH = (SYS->GPA_MFPH & ~(SYS_GPA_MFPH_PA14MFP_Msk | SYS_GPA_MFPH_PA15MFP_Msk)) |
  //                (SYS_GPA_MFPH_PA15MFP_UART0_RXD | SYS_GPA_MFPH_PA14MFP_UART0_TXD);

  SYS_LockReg();
}
//************************************************************************
void LED_GPIO_init(void) {
  PA->MODE = (PA->MODE & ~(GPIO_MODE_MODE0_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE0_Pos);
/*-----------------------------------------------------------------------*/
/* обнуляем значения регистра MODE0 --> MODE0_0 = 0 MODE0_1 = 0          */
/* PA->MODE &= ~((0x1UL << 1)|(0x1UL << 0));                             */
/* устанавливаем значение MODE0 = 01 (Push-pull Output mode)             */
/* PA->MODE |=  (0x1UL << 0);                                            */
/*-----------------------------------------------------------------------*/
  PA->MODE = (PA->MODE & ~(GPIO_MODE_MODE1_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE1_Pos);
  PA->MODE = (PA->MODE & ~(GPIO_MODE_MODE2_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE2_Pos);
  PA0 = 1;
//PA->DOUT |=  (0x1UL << 0);
  PA1 = 1;
//PA->DOUT |=  (0x1UL << 1);
  PA2 = 1;
//PA->DOUT |=  (0x1UL << 2);

/*-----------------------------------------------------------------------*/
/* set mode of pin PB.0 as general - purpose INPUT                       */
/* 00 = Digital input                01 = Push-pull Output mode          */
/* 01 = Open-drain Output mode       11 = Quasi-bidirectional mode       */
/*-----------------------------------------------------------------------*/

//PB->MODE &= ~3UL;// set mode as input (00)
  PB->MODE = (PB->MODE & ~(GPIO_MODE_MODE0_Msk)) | (GPIO_MODE_INPUT << GPIO_MODE_MODE0_Pos);
  PB->MODE = (PB->MODE & ~(GPIO_MODE_MODE1_Msk)) | (GPIO_MODE_INPUT << GPIO_MODE_MODE1_Pos);
  PB->MODE = (PB->MODE & ~(GPIO_MODE_MODE2_Msk)) | (GPIO_MODE_INPUT << GPIO_MODE_MODE2_Pos);
  GPIO->DBCTL = GPIO_DBCTL_ICLKON_Msk | GPIO_DBCTL_DBCLKSRC_LIRC | GPIO_DBCTL_DBCLKSEL_512;
  PB->DBEN |= (BIT0 | BIT1 | BIT2);
}

int main(void) {
  SYS_Init();
  LED_GPIO_init();

  while (1) {
  ((PB->PIN & 0x1) == 0x0)?(PA0 = DISABLE):(PA0 = ENABLE);//0x1 --> 0000 0001 (PB0) 
  ((PB->PIN & 0x2) == 0x0)?(PA1 = DISABLE):(PA1 = ENABLE);//0x2 --> 0000 0010 (PB1)
  ((PB->PIN & 0x4) == 0x0)?(PA2 = DISABLE):(PA2 = ENABLE);/*0x4 --> 0000 0100 (PB2);  0x8 --> 0000 1000 (PB3);
                                                              0x10 --> 0001 0000 (PB4); 0x20 --> 0010 0000 (PB5) ...*/
  }
}
/****************************** End of file *****************************/