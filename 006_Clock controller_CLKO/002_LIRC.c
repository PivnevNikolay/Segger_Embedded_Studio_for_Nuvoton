/**-----------------------------------------------------------------------
 *\date  02.12.2023
 *\brief
 *
 * ** За основу взят примеры Level1_Training.          **
 * ** Clock controller.  M031EC1AE (TSSOP28).          **
 * ** Код написан в IDE SEGGER Embedded Studio for ARM **
 * ** LIRC: 38.4kHz
 *
 *     M031EC1AE
 *   ------------
 *  |            |
 *  |            |
 *  |            |
 *  |        PA.3| ----> CLKO
 *  |            |
 *  |            |
 *  |            |
 *  |      +3.3V |
 *  |        GND |
 *  |            |
 *  |            |
 *
 *\authors ScuratovaAnna, PivnevNikolay
 *\сode debugging PivnevNikolay
 *
 * NuTool-PinConfigure   (Web application) --> https://opennuvoton.github.io/NuTool-PinConfigure/
 * NuTool-ClockConfigure (Web application) --> https://opennuvoton.github.io/NuTool-ClockConfigure/
  **************************************************************************************************
 *\brief
 */
#include "NuMicro.h"
#include <stdio.h>

void SYS_Init(void) {
  // MCU:M031EC1AE(TSSOP28)
  // Base Clocks:
  // LIRC:38.4kHz
  // HCLK:38.4kHz
  // PCLK0:38.4kHz
  // PCLK1:38.4kHz
  // Enabled-Module Frequencies:
  // Clock(LIRC): Clock:38.4kHz  LIRC: 38.4kHz

  /*---------------------------------------------------------------*/
  /* Init System Clock                                             */
  /*---------------------------------------------------------------*/
  // CLK->PWRCTL   = (CLK->PWRCTL   & ~(0x0000000DUL)) | 0x0231001AUL;
  // CLK->CLKDIV0  = (CLK->CLKDIV0  & ~(0x00FFFFFFUL)) | 0x00000000UL;
  // CLK->CLKDIV4  = (CLK->CLKDIV4  & ~(0x00FFFFFFUL)) | 0x00000000UL;
  // CLK->PCLKDIV  = (CLK->PCLKDIV  & ~(0x00000077UL)) | 0x00000000UL;
  // CLK->CLKSEL0  = (CLK->CLKSEL0  & ~(0x0000013FUL)) | 0x0000003BUL;
  // CLK->CLKSEL1  = (CLK->CLKSEL1  & ~(0x7777777FUL)) | 0x4477774BUL;
  // CLK->CLKSEL2  = (CLK->CLKSEL2  & ~(0x0030033FUL)) | 0x0020032BUL;
  // CLK->CLKSEL3  = (CLK->CLKSEL3  & ~(0x77777700UL)) | 0x44444400UL;
  // CLK->AHBCLK   = (CLK->AHBCLK   & ~(0x0000009EUL)) | 0x00000000UL;
  // CLK->APBCLK0  = (CLK->APBCLK0  & ~(0x18FF33FFUL)) | 0x00000041UL;
  // CLK->APBCLK1  = (CLK->APBCLK1  & ~(0x000F0300UL)) | 0x00000000UL;
  // CLK->CLKOCTL  = (CLK->CLKOCTL  & ~(0x0000007FUL)) | 0x00000030UL;
  // SysTick->CTRL = (SysTick->CTRL & ~(0x00000005UL)) | 0x00000000UL;
  // RTC->LXTCTL   = (RTC->LXTCTL   & ~(0x00000080UL)) | 0x00000000UL;
  SYS_UnlockReg();
  /* Enable clock source */
  CLK_EnableXtalRC(CLK_PWRCTL_LIRCEN_Msk);
  /* Waiting for clock source ready */
  CLK_WaitClockReady(CLK_STATUS_LIRCSTB_Msk);
  /* Set HCLK clock */
  CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_LIRC, CLK_CLKDIV0_HCLK(1));
  /* Set PCLK-related clock */
  CLK->PCLKDIV = (CLK_PCLKDIV_APB0DIV_DIV1 | CLK_PCLKDIV_APB1DIV_DIV1);
  /* Output selected clock to CKO */
  CLK_EnableCKO(CLK_CLKSEL1_CLKOSEL_LIRC, MODULE_NoMsk, 1);

  //********* Init I/O (CLKO) Multi-function **********************//
  /* Set PA multi-function pins for CLKO(PA.3) */
  SYS->GPA_MFPL &= ~(SYS_GPA_MFPL_PA3MFP_Msk);
  SYS->GPA_MFPL |= (SYS_GPA_MFPL_PA3MFP_CLKO);

  SystemCoreClockUpdate();

  SYS_LockReg();
}

int main(void) {
	
  SYS_Init();

  while (1) {
  }
}