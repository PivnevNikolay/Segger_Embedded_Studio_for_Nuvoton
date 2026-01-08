/**-----------------------------------------------------------------------
 *\date  15.10.2023
 *
 * ** Clock controller.  M031EC1AE (TSSOP28).          **
 * ** Код написан в IDE SEGGER Embedded Studio for ARM **
 * 
 *     M031EC1AE
 *   ------------
 *  |            |
 *  |            |
 *  |            |
 *  |        PA.3| ----> CLKO
 *  |        PF.2| ----> XT1_OUT  (HXT 8 MHz)
 *  |        PF.3| ----> XT1_IN
 *  |            |
 *  |      +3.3V |
 *  |        GND |
 *  |            |
 *  |            |
 *\authors ScuratovaAnna, PivnevNikolay
 *\сode debugging PivnevNikolay
 *
 * Example Code --> [M480] Use External Signal as HXT Clock Source
 * http://forum.nuvoton.com/viewtopic.php?t=8630
 *
 * NuTool-PinConfigure   (Web application) --> https://opennuvoton.github.io/NuTool-PinConfigure/
 * NuTool-ClockConfigure (Web application) --> https://opennuvoton.github.io/NuTool-ClockConfigure/
 **************************************************************************************************
 *
 */
#include "NuMicro.h"
#include <stdio.h>

void SYS_Init(void) {
  // MCU:M031EC1AE(TSSOP28)
  // Base Clocks:
  // HXT:8MHz
  // HCLK:8MHz
  // PCLK0:8MHz
  // PCLK1:8MHz
  // Enabled-Module Frequencies:
  // CLKO=Bus Clock(HCLK):8MHz/Engine Clock:8MHz

  /*---------------------------------------------------------------*/
  /* Init System Clock                                             */
  /*---------------------------------------------------------------*/
  // CLK->PWRCTL   = (CLK->PWRCTL   & ~(0x0000000DUL)) | 0x02310013UL;
  // CLK->CLKDIV0  = (CLK->CLKDIV0  & ~(0x00FFFFFFUL)) | 0x00000000UL;
  // CLK->CLKDIV4  = (CLK->CLKDIV4  & ~(0x00FFFFFFUL)) | 0x00000000UL;
  // CLK->PCLKDIV  = (CLK->PCLKDIV  & ~(0x00000077UL)) | 0x00000000UL;
  // CLK->CLKSEL0  = (CLK->CLKSEL0  & ~(0x0000013FUL)) | 0x00000038UL;
  // CLK->CLKSEL1  = (CLK->CLKSEL1  & ~(0x7777777FUL)) | 0x4477770BUL;
  // CLK->CLKSEL2  = (CLK->CLKSEL2  & ~(0x0030033FUL)) | 0x0020032BUL;
  // CLK->CLKSEL3  = (CLK->CLKSEL3  & ~(0x77777700UL)) | 0x44444400UL;
  // CLK->AHBCLK   = (CLK->AHBCLK   & ~(0x0000009EUL)) | 0x00000000UL;
  // CLK->APBCLK0  = (CLK->APBCLK0  & ~(0x18FF33FFUL)) | 0x00000040UL;
  // CLK->APBCLK1  = (CLK->APBCLK1  & ~(0x000F0300UL)) | 0x00000000UL;
  // CLK->CLKOCTL  = (CLK->CLKOCTL  & ~(0x0000007FUL)) | 0x00000030UL;
  // SysTick->CTRL = (SysTick->CTRL & ~(0x00000005UL)) | 0x00000000UL;
  // RTC->LXTCTL   = (RTC->LXTCTL   & ~(0x00000080UL)) | 0x00000000UL;
  SYS_UnlockReg();
  //* Enable HXT clock (external high speed crystal oscillator (HXT)) *//

  /* Set GPF multi-function pins for XT1_IN and XT1_OUT */
  SYS->GPF_MFPL &= ~(SYS_GPF_MFPL_PF3MFP_Msk | SYS_GPF_MFPL_PF2MFP_Msk);
  SYS->GPF_MFPL |= (SYS_GPF_MFPL_PF3MFP_XT1_IN | SYS_GPF_MFPL_PF2MFP_XT1_OUT);
  /* Enable HXT clock */
  CLK_EnableXtalRC(CLK_PWRCTL_HXTEN_Msk);
  /* Wait for HXT clock ready */
  CLK_WaitClockReady(CLK_STATUS_HXTSTB_Msk);
  /* Set PCLK0/PCLK1 to HCLK/2 */
  CLK->PCLKDIV = (CLK_PCLKDIV_APB0DIV_DIV1 | CLK_PCLKDIV_APB1DIV_DIV1);

  /* Output selected clock to CKO */
  CLK_EnableCKO(CLK_CLKSEL1_CLKOSEL_HXT, MODULE_NoMsk, 1);
  /*---------------------------------------------------------------*/
  // MCU:M031EC1AE(TSSOP28)
  // Base Clocks:
  // HXT:8MHz
  // HCLK:8MHz
  // PCLK0:8MHz
  // PCLK1:8MHz
  // Enabled-Module Frequencies:
  // CLKO_Divider=Bus Clock(HCLK):8MHz/Engine Clock:1MHz

  // CLK_EnableCKO(CLK_CLKSEL1_CLKOSEL_HXT, 2, 0);//CLKO Divider --> 1 MHz
  /*---------------------------------------------------------------*/

  //********* Init I/O (CLKO) Multi-function **********************//
  /* Set PA multi-function pins for CLKO(PA.3) */
  SYS->GPA_MFPL &= ~(SYS_GPA_MFPL_PA3MFP_Msk);
  SYS->GPA_MFPL |= (SYS_GPA_MFPL_PA3MFP_CLKO);

  SystemCoreClockUpdate();

  //********* UART0 RXD=PA.15 and TXD=PA.14 ***********************//
  // SYS->GPA_MFPH = (SYS->GPA_MFPH & ~(SYS_GPA_MFPH_PA14MFP_Msk | SYS_GPA_MFPH_PA15MFP_Msk)) |
  //              (SYS_GPA_MFPH_PA15MFP_UART0_RXD | SYS_GPA_MFPH_PA14MFP_UART0_TXD);

  SYS_LockReg();
}

int main(void) {
  SYS_Init();

  while (1) {
  }
}
