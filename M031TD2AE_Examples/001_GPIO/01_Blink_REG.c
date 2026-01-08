/**-----------------------------------------------------------------------
 \date  21.11.2025
 *
 *    M031TD2AE
 *   ------------
 *  |            |
 *  |            |
 *  |            |
 *  |        PA.1| ---->  LED
 *  |            |
 *  |            |
 *  |            |
 *  |      +3.3V |
 *  |        GND |
 *
 *
 *\ author         ScuratovaAnna 
 *\ сode debugging PivnevNikolay
 *
/******************************************************************************/
#include "NuMicro.h"
#include <stdio.h>

#define SET_BIT(REG, BIT) ((REG) |= (BIT))
#define CLEAR_BIT(REG, BIT) ((REG) &= ~(BIT))
#define READ_BIT(REG, BIT) ((REG) & (BIT))
#define CLEAR_REG(REG) ((REG) = (0x0))
#define WRITE_REG(REG, VAL) ((REG) = (VAL))
#define READ_REG(REG) ((REG))
#define MODIFY_REG(REG, CLEARMASK, SETMASK) WRITE_REG((REG), (((READ_REG(REG)) & (~(CLEARMASK))) | (SETMASK)))

void delay_ms(uint32_t ms);

void SYS_Init(void) {
  SYS_UnlockReg();
  //********* Enable HIRC clock (Internal RC 48MHz)  **************//
  SET_BIT(CLK->PWRCTL, CLK_PWRCTL_HIRCEN_Msk);

  /* Waiting for HIRC clock ready */
  while (READ_BIT(CLK->STATUS, CLK_STATUS_HIRCSTB_Msk != CLK_STATUS_HIRCSTB_Msk));

  /* Switch HCLK clock source to HIRC */
  MODIFY_REG(CLK->CLKSEL0, CLK_CLKSEL0_HCLKSEL_Msk, CLK_CLKSEL0_HCLKSEL_HIRC);

  MODIFY_REG(CLK->CLKDIV0, CLK_CLKDIV0_HCLKDIV_Msk, CLK_CLKDIV0_HCLK(1));

  /* Waiting for HIRC clock ready */
  while (READ_BIT(CLK->STATUS, CLK_STATUS_HIRCSTB_Msk != CLK_STATUS_HIRCSTB_Msk));

  SystemCoreClockUpdate();

  MODIFY_REG(SYS->GPA_MFPL, SYS_GPA_MFPL_PA1MFP_Msk, SYS_GPA_MFPL_PA1MFP_GPIO);// PA1 setting for GPIO

  SYS_LockReg();
}

void LED_GPIO_init(void) {
  MODIFY_REG(PA->MODE, GPIO_MODE_MODE1_Msk, GPIO_MODE_OUTPUT << GPIO_MODE_MODE1_Pos);

  SET_BIT(PA->DOUT, GPIO_DOUT_DOUT1_Msk);// PA1 = 1;

  __NOP();
}

int main(void) {
  SYS_Init();
  LED_GPIO_init();
  
  while (1) {
    delay_ms(300);
    WRITE_REG(PA->DOUT, READ_REG(PA->DOUT) ^ GPIO_DOUT_DOUT1_Msk);
  }
}

void delay_ms(uint32_t ms) {
  for (uint32_t i = 0; i < ms * 1000; i++) {
    __NOP(); // No operation instruction
  }
}
/******************************** End of file *********************************/