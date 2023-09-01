

#include "NuMicro.h"
#include "stdbool.h"
#include <stdio.h>

typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

#define ALONE

const u8 Port_Bit[] = {1UL, 2UL, 4UL, 8UL};
const char k4x4[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

char GetKey4x4(void);

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

  SystemCoreClockUpdate();
  //********* GPIO PA.0 PA.1 PA.2 PA.3 ****************************//output
  SYS->GPA_MFPL = (SYS->GPA_MFPL & ~(SYS_GPA_MFPL_PA0MFP_Msk)) | (SYS_GPA_MFPL_PA0MFP_GPIO); // PA.0
  SYS->GPA_MFPL = (SYS->GPA_MFPL & ~(SYS_GPA_MFPL_PA1MFP_Msk)) | (SYS_GPA_MFPL_PA1MFP_GPIO); // PA.1
  SYS->GPA_MFPL = (SYS->GPA_MFPL & ~(SYS_GPA_MFPL_PA2MFP_Msk)) | (SYS_GPA_MFPL_PA2MFP_GPIO); // PA.2
  SYS->GPA_MFPL = (SYS->GPA_MFPL & ~(SYS_GPA_MFPL_PA3MFP_Msk)) | (SYS_GPA_MFPL_PA3MFP_GPIO); // PA.3
  //********* GPIO PB.0 PB.1 PB.2 PB.3 ****************************//input
  SYS->GPB_MFPL = (SYS->GPB_MFPL & ~(SYS_GPB_MFPL_PB0MFP_Msk)) | (SYS_GPB_MFPL_PB0MFP_GPIO); // PB.0
  SYS->GPB_MFPL = (SYS->GPB_MFPL & ~(SYS_GPB_MFPL_PB1MFP_Msk)) | (SYS_GPB_MFPL_PB1MFP_GPIO); // PB.1
  SYS->GPB_MFPL = (SYS->GPB_MFPL & ~(SYS_GPB_MFPL_PB2MFP_Msk)) | (SYS_GPB_MFPL_PB2MFP_GPIO); // PB.2
  SYS->GPB_MFPL = (SYS->GPB_MFPL & ~(SYS_GPB_MFPL_PB3MFP_Msk)) | (SYS_GPB_MFPL_PB3MFP_GPIO); // PB.3
  //********* UART0 RXD=PA.15 and TXD=PA.14 ***********************//
  SYS->GPA_MFPH = (SYS->GPA_MFPH & ~(SYS_GPA_MFPH_PA14MFP_Msk | SYS_GPA_MFPH_PA15MFP_Msk)) |
                  (SYS_GPA_MFPH_PA15MFP_UART0_RXD | SYS_GPA_MFPH_PA14MFP_UART0_TXD);
  SYS_LockReg();
}

void GPIO_init(void) {
  for (u8 i = 0; i <= 3; i++) {
    GPIO_SetMode(PB, Port_Bit[i], GPIO_MODE_OUTPUT); // PB.0-3 output
    GPIO_SetMode(PA, Port_Bit[i], GPIO_MODE_INPUT);  // PA.0-3 input
    PB->DOUT |= (0x1UL << i);
  }

  UART_Open(UART0, 115200);
}

int main(void) {
  SYS_Init();
  GPIO_init();
  printf("\n\nCPU @ %dHz\n", SystemCoreClock);
  while (1) {
    char b = GetKey4x4();
    if (b != 0) {
    printf("%c\n", b);
    }
  }
}

char GetKey4x4(void) {
  static char olda;
  char a = 0;
  TIMER_Delay(TIMER0, 100000);
  for (u8 p = 0; p <= 3; p++) {
    PB->DOUT &= ~(0x1UL << p);
    for (u8 x = 0; x <= 3; x++) {
      if ((PA->PIN & Port_Bit[x])== 0x0) {
        a = k4x4[p][x];
      }
    }
    PB->DOUT |= (0x1UL << p);
  }
#ifdef ALONE
  if (a == olda)
    return 0;
  olda = a;
#endif
  return a;
}

/*************************** End of file ****************************/