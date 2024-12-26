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
 *  |        PC.1| ---->  LED PC.1  _|¯|_  Period=250ms +Width=125ms -Width=125ms
 *  |        PC.2| ---->  LED PC.2  _|¯|_  Period=250ms +Width=125ms -Width=125ms
 *  |        PC.3| ---->  LED PC.3  _|¯|_  Period=250ms +Width=125ms -Width=125ms
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
__IO uint32_t Nuvoton_ticks;

void systick_config(void);
void delay(uint32_t ms);
uint32_t getCurrentMillis(void);

//************************************************************************
void SysTick_Handler(void) {
  Nuvoton_ticks ++;
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
  systick_config();
  UART0_Init();
  printf("\n\nCPU @ %dHz\n", SystemCoreClock);
  while (1) {
    (i == 0) ? (i = Bit_SET, printf("Led Set\n")) : (i = Bit_RESET, printf("Led ReSet\n"));
    PC1 = i;
    PC2 = i;
    PC3 = i;
    delay(125);
  }
}
//*****************************************************************************************************
void systick_config(void) {
  /* setup systick timer for 1000Hz interrupts */
  if (SysTick_Config(SystemCoreClock / 1000U)) {//SysTick_Config(uint32_t ticks);  ↓↓↓↓↓↓↓↓↓↓ ниже описание  
    /* capture error-> ошибка захвата)) */
    while (1) {
    }
  }
  /* configure the systick handler priority */
  NVIC_SetPriority(SysTick_IRQn, 0x00U);
}
/******************************************************************************************************
//   core_cm0.h
// ##################################    SysTick function  ############################################ //
/**
  \ingroup  CMSIS_Core_FunctionInterface
  \defgroup CMSIS_Core_SysTickFunctions SysTick Functions
  \brief    Functions that configure the System.
  @{
//***************************************************************************************************** 
\details Initializes the System Timer and its interrupt, and starts the System Tick Timer.
           Counter is in free running mode to generate periodic interrupts.
\param [in]  ticks  Number of ticks between two interrupts.
\return          0  Function succeeded.
\return          1  Function failed.
\note    When the variable <b>__Vendor_SysTickConfig</b> is set to 1, then the
           function <b>SysTick_Config</b> is not included. In this case, the file <b><i>device</i>.h</b>
           must contain a vendor-specific implementation of this function.
//*****************************************************************************************************
__STATIC_INLINE uint32_t SysTick_Config(uint32_t ticks)
{
  if ((ticks - 1UL) > SysTick_LOAD_RELOAD_Msk)
  {
    return (1UL);                                                   // Reload value impossible
  }

  SysTick->LOAD  = (uint32_t)(ticks - 1UL);                         // set reload register
  NVIC_SetPriority (SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL); // set Priority for Systick Interrupt
  SysTick->VAL   = 0UL;                                             // Load the SysTick Counter Value
  SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
                   SysTick_CTRL_TICKINT_Msk   |
                   SysTick_CTRL_ENABLE_Msk;                         // Enable SysTick IRQ and SysTick Timer
  return (0UL);                                                     // Function successful
}
//-----------------------------------------------------------------------------------------------------
@} end of CMSIS_Core_SysTickFunctions ****************************************************************/

// аналог arduino → delay(time)
// Задержка на указанное количество миллисекунд (мс). 1 секунда = 1'000 мс
void delay(uint32_t ms) {
  if (ms != 0) {
    uint32_t start = getCurrentMillis();
    do {
      __NOP();
    } while (getCurrentMillis() - start < ms);
  }
}
//*****************************************************************************************************
uint32_t getCurrentMillis(void) {
  return Nuvoton_ticks;
}
/*************************** End of file ****************************/