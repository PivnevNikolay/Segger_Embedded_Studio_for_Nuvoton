/**---------------------------------------------
 *\date  15.07.2021
 *  http://publicatorbar.ru/2017/12/21/arduino-multi-function-shield/
 * В примере используется ускоренный bit bang режим (любые пины).
 *
 *    NuMaker UNO                  Arduino Shild
 *   ------------                ----------------
 *  |            |              |
 *  |     (PA.0) |------------->| Pin 4 (LATCH_Pin)
 *  |     (PA.1) |------------->| Pin 7 (CLK_Pin  )
 *  |     (PA.2) |------------->| Pin 8 (DATA_Pin )
 *  |            |              |
 *  |            |              |
 *  |        GND |<------------>| GND
 *  |       +5V  |<------------>| +5V
 *
 *\ author ScuratovaAnna
 *\ сode debugging PivnevNikolay
 */

#include "NuMicro.h"
#include "stdbool.h"
#include <stdio.h>

#define CLK_Pin     1
#define LATCH_Pin   0
#define DATA_Pin    2 

const uint8_t SEGMENT_SELECT[] = { 0xF7, 0xFB, 0xFD, 0xFE };
uint8_t symbol[4];
//int i = 0;
//float z = 0.0;
//bool variable = false;

void send(uint8_t data);
void WriteNumberToSegment(uint8_t Segment, uint8_t Value);
void SEND_SYMBOL(void);
void Display_the_value(uint8_t rg, uint32_t value);
void Display_the_Float_value(uint8_t rg, uint32_t value);
void Display_Send_int32_t(int32_t value);
void Display_Send_float(float value);

void SYS_Init(void) {
  SYS_UnlockReg();
  //********* Enable HIRC clock (Internal RC 48MHz)  **************//
  CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);
  CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);
  CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(11)); // The clock of HCLK --> 4MHz
  // CLK->PCLKDIV = (CLK_PCLKDIV_APB0DIV_DIV4 | CLK_PCLKDIV_APB1DIV_DIV4);
  //********* UART0_MODULE  ***************************************//
  CLK_EnableModuleClock(UART0_MODULE);
  CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));
  //********* TMR0_MODULE  ****************************************//
  CLK_EnableModuleClock(TMR0_MODULE);
  CLK_EnableModuleClock(TMR1_MODULE);
  CLK_EnableModuleClock(TMR2_MODULE);
  CLK_EnableModuleClock(TMR3_MODULE);
  CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_HIRC, 0);
  CLK_SetModuleClock(TMR1_MODULE, CLK_CLKSEL1_TMR1SEL_HIRC, 0);
  CLK_SetModuleClock(TMR2_MODULE, CLK_CLKSEL1_TMR2SEL_HIRC, 0);
  CLK_SetModuleClock(TMR3_MODULE, CLK_CLKSEL1_TMR3SEL_HIRC, 0);
  SystemCoreClockUpdate();
  //********* GPIO PA.0 PA.1 PA.2 PA.3 ****************************//
  SYS->GPA_MFPL = (SYS->GPA_MFPL & ~(SYS_GPA_MFPL_PA0MFP_Msk)) | (SYS_GPA_MFPL_PA0MFP_GPIO); // PA.0
  SYS->GPA_MFPL = (SYS->GPA_MFPL & ~(SYS_GPA_MFPL_PA1MFP_Msk)) | (SYS_GPA_MFPL_PA1MFP_GPIO); // PA.1
  SYS->GPA_MFPL = (SYS->GPA_MFPL & ~(SYS_GPA_MFPL_PA2MFP_Msk)) | (SYS_GPA_MFPL_PA2MFP_GPIO); // PA.2
  //********* UART0 RXD=PA.15 and TXD=PA.14 ***********************//
  SYS->GPA_MFPH = (SYS->GPA_MFPH & ~(SYS_GPA_MFPH_PA14MFP_Msk | SYS_GPA_MFPH_PA15MFP_Msk)) |
                  (SYS_GPA_MFPH_PA15MFP_UART0_RXD | SYS_GPA_MFPH_PA14MFP_UART0_TXD);

  SYS_LockReg();
}

void GPIO_Init(void) {
  GPIO_SetMode(PA, BIT0, GPIO_MODE_OUTPUT); // PA.0 --> LATCH_Pin
  GPIO_SetMode(PA, BIT1, GPIO_MODE_OUTPUT); // PA.1 --> CLK_Pin
  GPIO_SetMode(PA, BIT2, GPIO_MODE_OUTPUT); // PA.2 --> DATA_Pin
}

void TIMER_Init(void) {
 TIMER_Open(TIMER0, TIMER_PERIODIC_MODE, 4000000);
 TIMER_EnableInt(TIMER0);

}

int main(void) {
  SYS_Init();
  GPIO_Init();
  while (1) {
  Display_Send_float(-1000.0);
  }
}

void send(uint8_t data) {
  for (uint8_t i = 8; i >= 1; i--) {
    PA->DOUT &= ~(1 << CLK_Pin);
    PA->DOUT = (data & 0x80) ? PA->DOUT | (1 << DATA_Pin) : PA->DOUT & ~(1 << DATA_Pin);
    PA->DOUT |= (1 << CLK_Pin);
    data = (data << 1);
  }
}

void WriteNumberToSegment(uint8_t Segment, uint8_t Value) {
  PA->DOUT &= ~(1 << LATCH_Pin);
  send(symbol[Value]);
  send(SEGMENT_SELECT[Segment]);
  PA->DOUT |= (1 << LATCH_Pin);
}

void SEND_SYMBOL(void) {
  WriteNumberToSegment(0, 0);  //записываем в 0 разряд
  WriteNumberToSegment(1, 1);  //записываем в 1 разряд
  WriteNumberToSegment(2, 2);  //записываем в 2 разряд 
  WriteNumberToSegment(3, 3);  //записываем в 3 разряд 
}

void Display_the_value(uint8_t rg, uint32_t value) {
  switch (value) {
    case 0:
      symbol[rg] = 0x3F;  // 0
      break;
    case 1:
      symbol[rg] = 0x6;  // 1
      break;
    case 2:
      symbol[rg] = 0x5B;  // 2
      break;
    case 3:
      symbol[rg] = 0x4F;  // 3
      break;
    case 4:
      symbol[rg] = 0x66;  // 4
      break;
    case 5:
      symbol[rg] = 0x6D;  // 5
      break;
    case 6:
      symbol[rg] = 0x7D;  // 6
      break;
    case 7:
      symbol[rg] = 0x7;  // 7
      break;
    case 8:
      symbol[rg] = 0x7F;  // 8
      break;
    case 9:
      symbol[rg] = 0X6F;  // 9
      break;
    case 254:
      symbol[rg] = 0X40;  // - минус
      break;
    case 255:
      symbol[rg] = 0x00;  // - пустота
      break;
    case 256:
      symbol[rg] = 0X8;  //- подчеркивание
      break;
  }
}

void Display_the_Float_value(uint8_t rg, uint32_t value) {
  switch (value) {
    case 0:
      symbol[rg] = 0xBF;  // 0.
      break;
    case 1:
      symbol[rg] = 0x86;  // 1.
      break;
    case 2:
      symbol[rg] = 0xDB;  // 2.
      break;
    case 3:
      symbol[rg] = 0xCF;  // 3.
      break;
    case 4:
      symbol[rg] = 0xE6;  // 4.
      break;
    case 5:
      symbol[rg] = 0xED;  // 5.
      break;
    case 6:
      symbol[rg] = 0xFD;  // 6.
      break;
    case 7:
      symbol[rg] = 0x87;  // 7.
      break;
    case 8:
      symbol[rg] = 0xFF;  // 8.
      break;
    case 9:
      symbol[rg] = 0XEF;  // 9.
      break;
    case 254:
      symbol[rg] = 0X40;  // - минус
      break;
    case 255:
      symbol[rg] = 0x00;  // - пустота
      break;
    case 256:
      symbol[rg] = 0X8;  // - подчеркивание
      break;
  }
}

void Display_Send_int32_t(int32_t value) {
  if (value > 9999 || value < -999) {
    Display_the_value(0, 256);
    Display_the_value(1, 256);
    Display_the_value(2, 256);
    Display_the_value(3, 256);
    SEND_SYMBOL();
  } else {
    bool positive_value;
    if (value < 0) {
      positive_value = false;
      value = value * (-1);
    } else {
      positive_value = true;
    }
    uint8_t BCD_Arr[4] = {
      0,
    };
    uint32_t lenght;
    lenght = value;
    while (value >= 1000) {
      value -= 1000;
      BCD_Arr[3]++;
    }
    while (value >= 100) {
      value -= 100;
      BCD_Arr[2]++;
    }
    while (value >= 10) {
      value -= 10;
      BCD_Arr[1]++;
    }
    BCD_Arr[0] = (uint8_t)(value);
    if (lenght < 10) {
      Display_the_value(3, 255);
      Display_the_value(2, 255);
      Display_the_value(1, 255);
      if (positive_value == true) {
        Display_the_value(1, 255);
      } else {
        Display_the_value(1, 254);
      }
      Display_the_value(0, BCD_Arr[0]);
      SEND_SYMBOL();
    } else if (lenght < 100) {
      Display_the_value(3, 255);
      Display_the_value(2, 255);
      if (positive_value == true) {
        Display_the_value(2, 255);
      } else {
        Display_the_value(2, 254);
      }
      Display_the_value(1, BCD_Arr[1]);
      Display_the_value(0, BCD_Arr[0]);
      SEND_SYMBOL();
    } else if (lenght < 1000) {
      Display_the_value(3, 255);
      if (positive_value == true) {
        Display_the_value(3, 255);
      } else {
        Display_the_value(3, 254);
      }
      Display_the_value(2, BCD_Arr[2]);
      Display_the_value(1, BCD_Arr[1]);
      Display_the_value(0, BCD_Arr[0]);
      SEND_SYMBOL();
    } else if (lenght < 10000) {
      if (positive_value == false) {
        Display_the_value(0, 256);
        Display_the_value(1, 256);
        Display_the_value(2, 256);
        Display_the_value(3, 256);
      }
      Display_the_value(3, BCD_Arr[3]);
      Display_the_value(2, BCD_Arr[2]);
      Display_the_value(1, BCD_Arr[1]);
      Display_the_value(0, BCD_Arr[0]);
      SEND_SYMBOL();
    }
  }
}

void Display_Send_float(float value) {
  if (value > 999.9f || value < -99.9f) {
    Display_the_Float_value(0, 256);
    Display_the_Float_value(1, 256);
    Display_the_Float_value(2, 256);
    Display_the_Float_value(3, 256);
    SEND_SYMBOL();
  } else {
    //умножаем на 10 т.к количество знаков после запятой один!
    int32_t value_conv = value * 10;
    value = value * 10;
    //знак -------------------
    bool positive_value;
    if (value_conv < 0) {
      positive_value = false;
      value_conv = value_conv * (-1);
    } else {
      positive_value = true;
    }
    //массив ------------------
    uint8_t BCD_Arr[4] = {
      0,
    };
    int32_t lenght;
    lenght = value_conv;
    while (value_conv >= 1000) {
      value_conv -= 1000;
      BCD_Arr[3]++;
    }
    while (value_conv >= 100) {
      value_conv -= 100;
      BCD_Arr[2]++;
    }
    while (value_conv >= 10) {
      value_conv -= 10;
      BCD_Arr[1]++;
    }
    BCD_Arr[0] = (uint8_t)(value_conv);

    if (lenght < 10) {
      Display_the_value(3, 255);

      if (positive_value == true) {
        Display_the_value(2, 255);
      } else {
        Display_the_value(2, 254);
      }
      Display_the_Float_value(1, BCD_Arr[1]);
      Display_the_value(0, BCD_Arr[0]);
      SEND_SYMBOL();
    } else if (lenght < 100) {
      Display_the_value(3, 255);
      if (positive_value == true) {
        Display_the_value(2, 255);
      } else {
        Display_the_value(2, 254);
      }
      Display_the_Float_value(1, BCD_Arr[1]);
      Display_the_value(0, BCD_Arr[0]);
      SEND_SYMBOL();
    } else if (lenght < 1000) {
      if (positive_value == true) {
        Display_the_value(3, 255);
      } else {
        Display_the_value(3, 254);
      }
      Display_the_value(2, BCD_Arr[2]);
      Display_the_Float_value(1, BCD_Arr[1]);
      Display_the_value(0, BCD_Arr[0]);
      SEND_SYMBOL();
    } else if (lenght < 10000) {
      if (positive_value == false) {
        Display_the_value(0, 256);
        Display_the_value(1, 256);
        Display_the_value(2, 256);
        Display_the_value(3, 256);
      }
      Display_the_value(3, BCD_Arr[3]);
      Display_the_value(2, BCD_Arr[2]);
      Display_the_Float_value(1, BCD_Arr[1]);
      Display_the_value(0, BCD_Arr[0]);
      SEND_SYMBOL();
    }
  }
}
/*************************** End of file ****************************/