/**-----------------------------------------------------------------------
 \date  09.01.2025
 *
 | ** Bерсия драйверов V7.92 [2023-08-11]                              ** |
 | ** Код написан в IDE SEGGER Embedded Studio for ARM V7.32a (64-bit) ** |
 *    
 * 
 *     M031LE3AE        encoder
 *   ------------ 
 *  |            |
 *  |            |
 *  |       PA.12| <---  _|¯|_ (S1)
 *  |            |
 *  |       PC.14| <---  _|¯|_ (S2)
 *  |            |
 *  |      +3.3V | --->   +5V
 *  |        GND | --->   GND 
 *
 *  https://alexgyver.ru/encoder/  весь код для примеров взят отсюда 
 *  Для тестирования и отладки использован "круглый энкодер" с RC цепями для аппаратного подавления дребезга контактов.
 *
 *\authors ScuratovaAnna, PivnevNikolay
 *\ сode debugging PivnevNikolay
 *
 *************************************************************************/
// ----------------------------   ПРИМЕР 1   ----------------------------//
// ---------------- Высокоточная обработка энкодера ---------------------//
// алгоритм с "таблицей", позволяющий увеличить точность энкодера в 4 раза
 #include <NuMicro.h>
#include <stdbool.h>
#include <stdio.h>

typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

u8 condition_1 = FALSE;
u8 condition_2 = FALSE;
u8 flag        = FALSE;

int pos               = 0;
u8 lastState          = 0;
const int8_t increment[16] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};

void SYS_Init(void) {
  SYS_UnlockReg();
  CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);
  CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);
  CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));
  CLK->PCLKDIV = (CLK_PCLKDIV_APB0DIV_DIV1 | CLK_PCLKDIV_APB1DIV_DIV1);
  SystemCoreClockUpdate();
  //-------- GPIO PC.1 PC.2 PC.3 ----------------------------
  SYS->GPA_MFPH = (SYS->GPA_MFPH & ~(SYS_GPA_MFPH_PA12MFP_Msk)) | (SYS_GPA_MFPH_PA12MFP_GPIO);    // PA.12
  SYS->GPC_MFPH = (SYS->GPC_MFPH & ~(SYS_GPC_MFPH_PC14MFP_Msk)) | (SYS_GPC_MFPH_PC14MFP_GPIO);    // PC.14

  //-------- UART0 RXD=PA.15 and TXD=PA.14 ------------------
  SYS->GPA_MFPH = (SYS->GPA_MFPH & ~(SYS_GPA_MFPH_PA14MFP_Msk | SYS_GPA_MFPH_PA15MFP_Msk)) |
      (SYS_GPA_MFPH_PA15MFP_UART0_RXD | SYS_GPA_MFPH_PA14MFP_UART0_TXD);

  SYS_LockReg();
}

void ENC_GPIO_init(void) {
  GPIO_SetMode(PA, BIT12, GPIO_MODE_INPUT);
  GPIO_SetMode(PC, BIT14, GPIO_MODE_INPUT);
}

int main(void) {
  SYS_Init();
  ENC_GPIO_init();
  while (1) {
    condition_1 = PA12;
    condition_2 = PC14;
    // uint8_t state = condition_1 | (condition_2 << 1);
    uint8_t state = condition_2 | (condition_1 << 1);
    if (state != lastState) {
      flag ++;
      pos += increment[state | (lastState << 2)];
      lastState = state;
      printf("Val = %d\n", pos / 4);
    }
    }
  }
// ********************************************************************** /
// ----------------------------   ПРИМЕР 2   ----------------------------//
/*
 | ** Bерсия драйверов V7.92 [2023-08-11]                              ** |
 | ** Код написан в IDE SEGGER Embedded Studio for ARM V7.32a (64-bit) ** |
 *    
 *     M031LE3AE         
 *     M031LE3AE        encoder   
 *   ------------ 
 *  |            |
 *  |            |
 *  |       PA.12| <---  _|¯|_ (S1)
 *  |            |
 *  |       PC.14| <---  _|¯|_ (S2)
 *  |            |
 *  |      +3.3V | --->   +5V
 *  |        GND | --->   GND 
 *
 * ----------- Быстрая обработка энкодера digitalRead() -----------------
 * Максимально быстрый универсальный код для обработки энкодера работает
 * на перывании (используется одно прерывание!!!)
 * https://alexgyver.ru/encoder/  весь код для примеров взят отсюда !!!*/
#include <NuMicro.h>
#include <stdbool.h>
#include <stdio.h>

typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

__IO bool i = 0;
__IO u32 Nuvoton_ticks;

#define ENC_TYPE 1

volatile int encCounter = 0;
volatile bool state0    = 0;
volatile bool state1    = 0;
volatile bool lastState = 0;
volatile bool turnFlag  = 0;

void systick_config(void);
void delay(u32 ms);
u32 getCurrentMillis(void);

void SysTick_Handler(void) {
  Nuvoton_ticks++;
}

void GPABGH_IRQHandler(void) {
  if (GPIO_GET_INT_FLAG(PA, BIT12)) {
    state0 = PC14;
    state1 = PA12;
    if (state1 != lastState) {
#if (ENC_TYPE == 1)
      turnFlag = !turnFlag;
      if (turnFlag) {
        encCounter += (state0 != lastState) ? -1 : 1;
      }
#else
      encCounter += (state0 != lastState) ? -1 : 1;
#endif
      // lastState = state0;
      lastState = state1;
    }
    GPIO_CLR_INT_FLAG(PA, BIT12);
  } else {
    volatile u32 temp;
    temp       = PA->INTSRC;
    PA->INTSRC = temp;
    printf("Un-expected interrupts.\n");
  }
}

void SYS_Init(void) {
  SYS_UnlockReg();
  CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);
  CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);
  CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));
  CLK->PCLKDIV = (CLK_PCLKDIV_APB0DIV_DIV1 | CLK_PCLKDIV_APB1DIV_DIV1);
  CLK_EnableModuleClock(ISP_MODULE);
  CLK_EnableSysTick(CLK_CLKSEL0_STCLKSEL_HIRC_DIV2, 0);
  SystemCoreClockUpdate();
  //********* GPIO PC.1 PC.2 PC.3 ****************************
  SYS->GPA_MFPH = (SYS->GPA_MFPH & ~(SYS_GPA_MFPH_PA12MFP_Msk)) | (SYS_GPA_MFPH_PA12MFP_GPIO);    // PA.12
  SYS->GPC_MFPH = (SYS->GPC_MFPH & ~(SYS_GPC_MFPH_PC14MFP_Msk)) | (SYS_GPC_MFPH_PC14MFP_GPIO);    // PC.14

  //********* UART0 RXD=PA.15 and TXD=PA.14 ******************
  SYS->GPA_MFPH = (SYS->GPA_MFPH & ~(SYS_GPA_MFPH_PA14MFP_Msk | SYS_GPA_MFPH_PA15MFP_Msk)) |
      (SYS_GPA_MFPH_PA15MFP_UART0_RXD | SYS_GPA_MFPH_PA14MFP_UART0_TXD);

  SYS_LockReg();
}

void ENC_GPIO_init(void) {
  GPIO_SetMode(PA, BIT12, GPIO_MODE_INPUT);
  GPIO_SetMode(PC, BIT14, GPIO_MODE_INPUT);
  GPIO_EnableInt(PA, 12, GPIO_INT_BOTH_EDGE);
  NVIC_EnableIRQ(GPIO_PAPBPGPH_IRQn);
}

int main(void) {
  SYS_Init();
  ENC_GPIO_init();
  systick_config();
  printf("\n\nCPU @ %dHz\n", SystemCoreClock);
  while (1) {
    delay(100);
    printf("Val = %d\n", encCounter);
  }
}

void systick_config(void) {
  if (SysTick_Config(SystemCoreClock / 1000U)) {
    while (1) {
    }
  }
  NVIC_SetPriority(SysTick_IRQn, 0x00U);
}

void delay(u32 ms) {
  if (ms != 0) {
    u32 start = getCurrentMillis();
    do {
      __NOP();
    } while (getCurrentMillis() - start < ms);
  }
}

u32 getCurrentMillis(void) {
  return Nuvoton_ticks;
}
// ********************************************************************** /