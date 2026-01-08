/**-----------------------------------------------------------------------
 \date  21.11.2024
 *
 | ** Bерсия драйверов V7.90 [2023-08-02]                              ** |
 | ** Код написан в IDE SEGGER Embedded Studio for ARM V7.32a (64-bit) ** |
 *    
 * 
 *   M031LE3AE  
 *   ------------ 
 *  |            |
 *  |            |
 *  |            |
 *  |        PC.1| ---->  LED PC.1  _|¯|_  Period=400ms +Width=200ms -Width=200ms
 *  |        PC.2| ---->  LED PC.2  _|¯|_  Period=400ms +Width=200ms -Width=200ms
 *  |        PC.3| ---->  LED PC.3  _|¯|_  Period=400ms +Width=200ms -Width=200ms
 *  |            |
 *  |      +3.3V |
 *  |        GND | 
 *
 *
 *\authors ScuratovaAnna, PivnevNikolay
 *\ сode debugging PivnevNikolay
 *
 ********************************************************************/
#include "NuMicro.h"
#include "stdbool.h"
#include <stdio.h>

#define Bit_SET 1
#define Bit_RESET 0

__IO bool i = 0;
__IO uint32_t SYSTEM_MS;

uint32_t millis(void);
void system_msdelay(uint32_t delay_ms);
void systick_setup(void);

void SysTick_Handler(void) {
  SYSTEM_MS++;
}

//************************************************************************
void SYS_Init(void) {
  SYS_UnlockReg();
  /* Enable clock source */
  CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);
  /* Waiting for clock source ready */
  CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);
  /* Set HCLK clock */
  CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));
  /* Set PCLK-related clock */
  CLK->PCLKDIV = (CLK_PCLKDIV_APB0DIV_DIV1 | CLK_PCLKDIV_APB1DIV_DIV1);
  CLK_EnableModuleClock(ISP_MODULE);
  CLK_EnableSysTick(CLK_CLKSEL0_STCLKSEL_HIRC_DIV2, 0);
  SystemCoreClockUpdate();
  //********* GPIO PC.1 PC.2 PC.3 ****************************//
  SYS->GPC_MFPL = (SYS->GPC_MFPL & ~(SYS_GPC_MFPL_PC1MFP_Msk)) | (SYS_GPC_MFPL_PC1MFP_GPIO); // PC.1
  SYS->GPC_MFPL = (SYS->GPC_MFPL & ~(SYS_GPC_MFPL_PC2MFP_Msk)) | (SYS_GPC_MFPL_PC2MFP_GPIO); // PC.2
  SYS->GPC_MFPL = (SYS->GPC_MFPL & ~(SYS_GPC_MFPL_PC3MFP_Msk)) | (SYS_GPC_MFPL_PC3MFP_GPIO); // PC.3

  //********* UART0 RXD=PA.15 and TXD=PA.14 ***********************//
  SYS->GPA_MFPH = (SYS->GPA_MFPH & ~(SYS_GPA_MFPH_PA14MFP_Msk | SYS_GPA_MFPH_PA15MFP_Msk)) |
                  (SYS_GPA_MFPH_PA15MFP_UART0_RXD | SYS_GPA_MFPH_PA14MFP_UART0_TXD);

  SYS_LockReg();
}
//************************************************************************
void LED_GPIO_init(void) {
  GPIO_SetMode(PC, BIT1, GPIO_MODE_OUTPUT);
  GPIO_SetMode(PC, BIT2, GPIO_MODE_OUTPUT);
  GPIO_SetMode(PC, BIT3, GPIO_MODE_OUTPUT);
  PC1 = 1;
  PC2 = 1;
  PC3 = 1;
}
//************************************************************************
void UART0_Init(void) {
  /* Reset UART0 */
  SYS->IPRST1 |= SYS_IPRST1_UART0RST_Msk;
  SYS->IPRST1 &= ~SYS_IPRST1_UART0RST_Msk;
  /* Configure UART0 and set UART0 baud rate */
  UART0->BAUD = UART_BAUD_MODE2 | UART_BAUD_MODE2_DIVIDER(__HIRC, 115200);
  UART0->LINE = UART_WORD_LEN_8 | UART_PARITY_NONE | UART_STOP_BIT_1;
}
//************************************************************************
int main(void) {
  SYS_Init();
  LED_GPIO_init();
  systick_setup();
  UART0_Init();
  printf("\n\nCPU @ %dHz\n", SystemCoreClock);
  while (1) {
    (i == 0) ? (i = Bit_SET, printf("Led Set\n")) : (i = Bit_RESET, printf("Led ReSet\n"));
    PC1 = i;
    PC2 = i;
    PC3 = i;
    system_msdelay(200);
  }
}
//************************************************************************
void systick_setup(void) {
  SysTick->LOAD &= ~SysTick_LOAD_RELOAD_Msk;
  SysTick->LOAD = (SystemCoreClock / 1000U) - 1U;                                         // 1ms tick
  SysTick->VAL = 0x00;                                                                    // explicitly set start value (undefined on reset)
  SysTick->CTRL |= (1U << SysTick_CTRL_TICKINT_Pos) | (1U << SysTick_CTRL_CLKSOURCE_Pos); // enable systick interrupt, source processor clock
  SysTick->CTRL |= (1U << SysTick_CTRL_ENABLE_Pos);                                       // enable SysTick
}
//************************************************************************
uint32_t millis(void) {
  return SYSTEM_MS;
}
//************************************************************************
// Задержка на указанное количество миллисекунд (мс). 1 секунда = 1'000 мс.
void system_msdelay(uint32_t delay_ms) {
  uint32_t end = millis() + delay_ms;
  while (millis() != end);
}
/****************************** End of file *****************************/