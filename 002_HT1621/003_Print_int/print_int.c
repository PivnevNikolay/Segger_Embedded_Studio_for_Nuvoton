/**-------------------------------------------------------------------
 *\date  13.06.2023
 *\brief
 *
 ***********************************************************************************************
 * В данном примере подключим, инициализируем дисплей и выведем на дисплей значения 
 * в виде счётчика от -1555 до 3000.
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
 *
 *
 * HT1621 – это LCD драйвер, способный управлять 128 элементным (32х4) индикатором.
 * Возможность конфигурирования делает HT1620 пригодным для использования во множестве LCD устройств, включая LCD модули и системы дисплеев.
 * Микросхема при формировании напряжения смещения для питания LCD потребляет совсем небольшой ток.
 *
 * ** Код написан в IDE SEGGER Embedded Studio for ARM **
 **********************************************************************
 *\ author PivnevNikolay
 *\ сode debugging ScuratovaAnna
 *\brief
 */
#include "NuMicro.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WR   1
#define CS   0
#define DATA 2

#define BIAS    0x52
#define SYSDIS  0X00
#define SYSEN   0X02
#define LCDOFF  0X04
#define LCDON   0X06
#define XTAL    0x28
#define RC256   0X30
#define TONEON  0X12
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
int x = -1555;

//********************************************************************
void wrDATA(unsigned char data, unsigned char len);
void wrCMD(unsigned char CMD);
void config(void);
void clear(void);
void wrCLR(unsigned char len);
void wrclrdata(unsigned char addr, unsigned char sdata);
void update(void);
void wrone(char addr, char sdata);
void setBatteryLevel(int level);
char charToSegBits(char character);
void setdecimalseparator(int decimaldigits);
void print_str(const char *str, bool leftPadded);
void setdecimalseparator(int decimaldigits);
void print_Int(int num);

//********************************************************************
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
//********************************************************************
void lcd_begin(void) {
  GPIO_SetMode(PA, BIT0, GPIO_MODE_OUTPUT); // PA.0 --> CS
  GPIO_SetMode(PA, BIT1, GPIO_MODE_OUTPUT); // PA.1 --> WR
  GPIO_SetMode(PA, BIT2, GPIO_MODE_OUTPUT); // PA.2 --> Data
  config();
}
//********************************************************************
int main(void) {
  SYS_Init();
  TIMER_Delay(TIMER0, 10);
  lcd_begin();
  clear();
//********************************************************************
  while (1) {
    print_Int(x++);
    TIMER_Delay(TIMER0, 5000);
    if (x > 3000) {
      x = -1100;
    }
  }
}
//********************************************************************
void wrDATA(unsigned char data, unsigned char len) {
  for (uint8_t i = 0; i < len; i++) {
    PA->DOUT &= ~(1 << WR);
    PA->DOUT = (data & 0x80) ? PA->DOUT | (1 << DATA) : PA->DOUT & ~(1 << DATA);
    // PA->DOUT &= ~(1 << WR);
    PA->DOUT |= (1 << WR);
    data = (data << 1);
  }
}

void wrCMD(unsigned char CMD) {
  PA->DOUT &= ~(1 << CS);
  wrDATA(0x80, 4);
  wrDATA(CMD, 8);
  PA->DOUT |= (1 << CS);
}

void config(void) {
  wrCMD(BIAS);
  wrCMD(RC256);
  wrCMD(SYSDIS);
  wrCMD(WDTDIS1);
  wrCMD(SYSEN);
  wrCMD(LCDON);
}

void clear(void) {
  wrCLR(16);
}

void wrCLR(unsigned char len) {
  uint8_t addr = 0;
  uint8_t i;
  for (i = 0; i < len; i++) {
    wrclrdata(addr, 0x00);
    addr = addr + 2;
  }
}

void wrclrdata(unsigned char addr, unsigned char sdata) {
  addr <<= 2;
  PA->DOUT &= ~(1 << CS);
  wrDATA(0xa0, 3);
  wrDATA(addr, 6);
  wrDATA(sdata, 8);
  PA->DOUT |= (1 << CS);
}

void update(void) {
  wrone(0, _buffer[5]);
  wrone(2, _buffer[4]);
  wrone(4, _buffer[3]);
  wrone(6, _buffer[2]);
  wrone(8, _buffer[1]);
  wrone(10, _buffer[0]);
}

void wrone(char addr, char sdata) {
  addr <<= 2;
  PA->DOUT &= ~(1 << CS);
  wrDATA(0xa0, 3);
  wrDATA(addr, 6);
  wrDATA(sdata, 8);
  PA->DOUT |= (1 << CS);
}

void setBatteryLevel(int level) {

  _buffer[0] &= 0x7F;
  _buffer[1] &= 0x7F;
  _buffer[2] &= 0x7F;

  switch (level) {
  case 3: // индикатор заряда батареи включен горят все 3 сегмента
    _buffer[0] |= 0x80;
  case 2: // индикатор заряда батареи включен  горит 2 сегмента
    _buffer[1] |= 0x80;
  case 1: // индикатор заряда батареи включен  горит 1 сегмент
    _buffer[2] |= 0x80;
  case 0: // индикатор заряда батареи выключен
  default:
    break;
  }
  update();
}

void print_str(const char *str, bool leftPadded) {
  int chars = strlen(str);
  int padding = 6 - chars;

  for (uint8_t i = 0; i < 6; i++) {
    _buffer[i] &= 0x80;
    uint8_t character = leftPadded
                            ? i < padding ? ' ' : str[i - padding]
                        : i >= chars ? ' '
                                     : str[i];
    _buffer[i] |= charToSegBits(character);
  }
  setdecimalseparator(0);
  update();
}

void setdecimalseparator(int decimaldigits) {
  _buffer[3] &= 0x7F;
  _buffer[4] &= 0x7F;
  _buffer[5] &= 0x7F;

  if (decimaldigits <= 0 || decimaldigits > 3) {
    return;
  }
  _buffer[6 - decimaldigits] |= 0x80;
}

void print_Int(int num) {
  if (num > 32767)
    num = 32767;
  if (num < -32767)
    num = -32767;
  char localbuffer[7];
  sprintf(localbuffer, "%d      ", num); // convert the decimal into string !!! 6 пробелов
  uint8_t precision = 0;
  if (precision > 0 && (num) < pow(10, precision)) {
    for (uint8_t i = 0; i < (5 - precision); i++) {
      if (localbuffer[i + 1] == '0' && localbuffer[i] != '-') {
        localbuffer[i] = ' ';
      }
    }
  }
  for (uint8_t i = 0; i < 6; i++) {
    _buffer[i] &= 0x80;
    _buffer[i] |= charToSegBits(localbuffer[i]);
  }
  update();
}

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