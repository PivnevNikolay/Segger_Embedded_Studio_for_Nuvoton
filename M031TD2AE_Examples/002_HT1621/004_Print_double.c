 /**-------------------------------------------------------------------
 \date  05.10.2024
 *
 ***********************************************************************************************
 * В данном примере выведем вещественное число с плавающей запятой на экран LCD.
 ***********************************************************************************************
 *
 *     M031EC1AE          HT1624
 *   ------------         ------------
 *  |            |       |
 *  |            |       |
 *  |            |       |
 *  |        PA.0| ----> | CS
 *  |        PA.1| ----> | WR
 *  |        PA.2| ----> | Data
 *  |        PA.3| ----> | LED+ для работы данного вывода необходимо доработать плату экрана
 *  |            |       |
 *  |        +5V | <---> | Vcc
 *  |        GND | <---> | GND
 *
 * Код взят и адаптирован для M031EC1AE (Nuvoton) отсюда.
 * https://count-zero.ru/2022/ht1621/
 * LCD Arduino library https://github.com/valerionew/ht1621-7-seg
 * Видео про HT1621
 * https://www.youtube.com/watch?v=O1xRavRY38Y
 *
 * ** Код написан в IDE SEGGER Embedded Studio for ARM **
 **********************************************************************
 * \\ author PivnevNikolay 
 * \\ сode debugging ScuratovaAnna
 */
#include "NuMicro.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CS 0
#define WR 1
#define DATA 2

#define BIAS 0x52
#define SYSDIS 0X00
#define SYSEN 0X02
#define LCDOFF 0X04
#define LCDON 0X06
#define XTAL 0x28
#define RC256 0X30
#define TONEON 0X12
#define TONEOFF 0X10
#define WDTDIS1 0X0A

int _buffer[7] = {
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
};
/************************************************************************/
void wrDATA(uint8_t data, uint8_t len);
void wrCMD(uint8_t CMD);
void config(void);
void clear(void);
void wrCLR(uint8_t len);
void wrclrdata(uint8_t addr, uint8_t sdata);
void update(void);
void wrone(uint8_t addr, uint8_t sdata);
void setdecimalseparator(int decimaldigits);
char charToSegBits(char character);
void print_long(long num, const char* flags, int precision);
void print_double(double num, int precision);
/************************************************************************/
double num = 0.711;
/************************************************************************/
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
  CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_HIRC, 0);
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
/************************************************************************/
void lcd_begin(void) {
  GPIO_SetMode(PA, BIT0, GPIO_MODE_OUTPUT); // PA.0 --> CS
  GPIO_SetMode(PA, BIT1, GPIO_MODE_OUTPUT); // PA.1 --> WR
  GPIO_SetMode(PA, BIT2, GPIO_MODE_OUTPUT); // PA.2 --> Data
  config();
}
/************************************************************************/
int main(void) {
  SYS_Init();
  TIMER_Delay(TIMER0, 10);
  lcd_begin();
  clear();
/************************************************************************/
  while (1) {
    print_double(num, 3);
    TIMER_Delay(TIMER0, 5000);
    num  += 0.001;
    if(num >= 3)num = -2.711;
  }
}
/************************************************************************/
void wrDATA(unsigned char data, unsigned char len) {
  for (uint8_t i = 0; i < len; i++) {
    PA->DOUT &= ~(1 << WR);
    PA->DOUT = (data & 0x80) ? PA->DOUT | (1 << DATA) : PA->DOUT & ~(1 << DATA);
    // PA->DOUT &= ~(1 << WR);
    PA->DOUT |= (1 << WR);
    data = (data << 1);
  }
}
/************************************************************************/
void wrCMD(unsigned char CMD) {
  PA->DOUT &= ~(1 << CS);
  wrDATA(0x80, 4);
  wrDATA(CMD, 8);
  PA->DOUT |= (1 << CS);
}
/************************************************************************/
void config(void) {
  wrCMD(BIAS);
  wrCMD(RC256);
  wrCMD(SYSDIS);
  wrCMD(WDTDIS1);
  wrCMD(SYSEN);
  wrCMD(LCDON);
}
/************************************************************************/
void clear(void) {
  wrCLR(16);
}
/************************************************************************/
void wrCLR(uint8_t len) {
  uint8_t addr = 0;
  uint8_t i;
  for (i = 0; i < len; i++) {
    wrclrdata(addr, 0x00);
    addr = addr + 2;
  }
}
/************************************************************************/
void wrclrdata(unsigned char addr, unsigned char sdata) {
  addr <<= 2;
  PA->DOUT &= ~(1 << CS);
  wrDATA(0xa0, 3);
  wrDATA(addr, 6);
  wrDATA(sdata, 8);
  PA->DOUT |= (1 << CS);
}
/************************************************************************/
void update(void) {
  wrone(0, _buffer[5]);
  wrone(2, _buffer[4]);
  wrone(4, _buffer[3]);
  wrone(6, _buffer[2]);
  wrone(8, _buffer[1]);
  wrone(10, _buffer[0]);
}
/************************************************************************/
void wrone(char addr, char sdata) {
  addr <<= 2;
  PA->DOUT &= ~(1 << CS);
  wrDATA(0xa0, 3);
  wrDATA(addr, 6);
  wrDATA(sdata, 8);
  PA->DOUT |= (1 << CS);
}
/************************************************************************/
void setdecimalseparator(int decimaldigits) {
  _buffer[3] &= 0x7F;
  _buffer[4] &= 0x7F;
  _buffer[5] &= 0x7F;

  if (decimaldigits <= 0 || decimaldigits > 3) {
    return;
  }
  _buffer[6 - decimaldigits] |= 0x80;
}
/************************************************************************/
char charToSegBits(char character) {
  switch (character) {
    case '0':
      return 0x7D;
    case '1':
      return 0x60;
    case '2':
      return 0x3E;
    case '3':
      return 0x7A;
    case '4':
      return 0x63;
    case '5':
      return 0x5B;
    case '6':
      return 0x5F;
    case '7':
      return 0x70;
    case '8':
      return 0x7F;
    case '9':
      return 0x7B;
    case ' ':
      return 0x00;
    case '*':
      return 0x33;
    case '|':
      return 0x5;
    case '-':
      return 0x2;
    case '_':
      return 0x8;
  }
}
/************************************************************************/
void print_long(long num, const char* flags, int precision) {
  if (num > 999999)
    num = 999999;
  if (num < -99999)
    num = -99999;
  char localbuffer[7];
  snprintf(localbuffer, 7, flags, num);
  if (precision > 0 && (num) < pow(10, precision)) {
    for (int i = 0; i < (5 - precision); i++) {
      if (localbuffer[i + 1] == '0' && localbuffer[i] != '-') {
        localbuffer[i] = ' ';
      } else {
        break;
      }
    }
  }
  for (int i = 0; i < 6; i++) {
    _buffer[i] &= 0x80;
    _buffer[i] |= charToSegBits(localbuffer[i]);
  }
  update();
}
/************************************************************************/
void print_double(double num, int precision) {
  if (num > 999999)
    num = 999999;
  if (num < -99999)
    num = -99999;
  if (precision > 3 && num > 0)
    precision = 3;
  else if (precision > 2 && num < 0)
    precision = 3;//!?
  if (precision < 0)
    precision = 0;
  const char* flags = (precision > 0 && abs(num) < 1) ? "%06li" : "%6li";
  long integerpart;
  integerpart = ((long)(num * pow(10, precision)));
  print_long(integerpart, flags, precision);
  setdecimalseparator(precision);
  update();
}
/************************************************************************/
