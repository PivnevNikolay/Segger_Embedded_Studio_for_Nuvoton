/**-----------------------------------------------------------------------
 *
 | ** WH1202A 4 бит 2 строки                                    ** |
 | ** Код написан в IDE SEGGER Embedded Studio for ARM 7.32a    ** |
 | ** J-Link Software and Documentation pack V7.90 [2023-08-02] ** |
 *   
 *     M031EC1AE            WH1202A
 *   ------------         -------------
 *  |            |       |
 *  |        PB.0| ----> |RS
 *  |        PB.1| ----> |E
 *  |            |       |
 *  |            |       |
 *  |        PA.0| ----> |DB4
 *  |        PA.1| ----> |DB5
 *  |        PA.2| ----> |DB6
 *  |        PA.3| ----> |DB7
 *  |            |       |
 *  |            |       |
 *  |        +5V |       | +5V
 *  |        GND | <---> | GND
 *
 *  вывод RW дисплея по умолчанию притянут к логическому нулю(GND).
 * -------------------------------------------------------------------
 * ccылка на экран :
 * https://www.chipdip.ru/product/wh1202a-ygh-ct
 * Демонстрация десятичного, шестнадцатеричного и двоичного форматов.
 * -------------------------------------------------------------------
 * подсматривали здесь: https://kit.alexgyver.ru/tutorials/lcd1602/
 * -------------------------------------------------------------------
 *  author: PivnevNikolay 
 *  сode debugging: ScuratovaAnna
 */
/******************************************************************************/
#include "NuMicro.h"
#include "stdbool.h"
#include <stdio.h>
#include <string.h>

#define SYSCLOCK 24000000U     // макрос нашей системной частоты
__IO uint32_t SysTick_CNT = 0; // обьявляем и иницализируем в 0 значение нашего счетчика SysTick

char s1[] = "Hello, world!";
char s2[] = "M031EC1AE";

volatile int counter = 0;

#define LCD_D4 0
#define LCD_D5 1
#define LCD_D6 2
#define LCD_D7 3

#define RS1 PB0 = 1
#define RS0 PB0 = 0
#define E1 PB1  = 1
#define E0 PB1  = 0

void SYS_Init(void);
void delay_1ms(uint32_t mS);
void GPIO_Init(void);
void lcd_pulse_enable(void);
void lcd_send_data(uint8_t data);
void lcd_com(uint8_t cmd);
void lcd_init_4bit(void);
void lcd_SetPos(uint8_t x, uint8_t y);
void lcd_print(const char *s);
void lcd_string(char *p, uint8_t x, uint8_t y);
void LCD_PrintNumber(int num);
void LCD_PrintHex(uint8_t num);
void lcd_printBinary(uint8_t num);
void lcds_creen_left(uint8_t n);
void lcds_creen_right(uint8_t n);
void cleaning_lcd(void);

void SysTick_Init(void) {
  SysTick->LOAD &= ~SysTick_LOAD_RELOAD_Msk; // сбрасываем возможные старые значения интервала счета в 0
  SysTick->LOAD = SYSCLOCK / 1000 - 1;
  SysTick->VAL &= ~SysTick_VAL_CURRENT_Msk;                                                         // сбрасываем текущие значения счетчика в 0
  SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk; // запуск счетчика
}

void SysTick_Handler(void) {
  if (SysTick_CNT > 0)
    SysTick_CNT--;
}

int main(void) {
  SYS_Init();
  GPIO_Init();
  SysTick_Init();
  lcd_init_4bit();
  lcd_SetPos(0, 0);
  /*lcd_send_data('H');
  lcd_send_data('i');
  lcd_send_data('_');
  lcd_send_data('N');
  lcd_send_data('u');
  lcd_send_data('v');
  lcd_send_data('o');
  lcd_send_data('t');
  lcd_send_data('o');
  lcd_send_data('n');
  delay_1ms(1500);
  cleaning_lcd();
  lcd_SetPos(0, 0);
  lcd_string("HI BABY", 1, 1);
  delay_1ms(500);
  lcds_creen_right(5); // Подвигаем туда
  lcds_creen_left(5);  // Подвигаем сюда
  delay_1ms(500);
  cleaning_lcd();
  lcd_SetPos(0, 0);
  lcd_print("Binary : ");
  for (uint8_t i = 0; i < 255; i++) {
   lcd_SetPos(0, 1);
   lcd_printBinary(i);
   delay_1ms(500);
  }*/
  lcd_send_data(126);
  lcd_print("Temp:");
  lcd_print("25.6C");
  lcd_send_data(223);
  delay_1ms(3000);
  cleaning_lcd();
  lcd_SetPos(0, 0);
  for (int i = 0; i < strlen(s1); i++) {
    lcd_send_data(s1[i]);
    delay_1ms(500);
  }
  lcd_SetPos(0, 1);
  for (int i = 0; i < strlen(s2); i++) {
    lcd_send_data(s2[i]);
    delay_1ms(500);
  }
  delay_1ms(1000);
  cleaning_lcd();
  lcd_SetPos(0, 0);
  lcd_print("Dec:  Hex: ");
  while (1) {
    lcd_SetPos(0, 1);
    LCD_PrintNumber(counter);
    lcd_SetPos(6, 1);
    LCD_PrintHex(counter);
    counter++;
    delay_1ms(400);
    if (counter == 255) {
      counter = 0;
      cleaning_lcd();
      lcd_SetPos(0, 0);
      lcd_print("Dec:  Hex: ");
    }
  }
}

void SYS_Init(void) {
  SYS_UnlockReg();
  CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);
  CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);
  CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));
  CLK->PCLKDIV = (CLK_PCLKDIV_APB0DIV_DIV1 | CLK_PCLKDIV_APB1DIV_DIV1);
  CLK_EnableModuleClock(UART0_MODULE);
  CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));

  CLK_EnableSysTick(CLK_CLKSEL0_STCLKSEL_HCLK_DIV2, 0);

  SystemCoreClockUpdate();

  SYS->GPA_MFPL = (SYS->GPA_MFPL & ~(SYS_GPA_MFPL_PA0MFP_Msk)) | (SYS_GPA_MFPL_PA0MFP_GPIO); // PA.0
  SYS->GPA_MFPL = (SYS->GPA_MFPL & ~(SYS_GPA_MFPL_PA1MFP_Msk)) | (SYS_GPA_MFPL_PA1MFP_GPIO); // PA.1
  SYS->GPA_MFPL = (SYS->GPA_MFPL & ~(SYS_GPA_MFPL_PA2MFP_Msk)) | (SYS_GPA_MFPL_PA2MFP_GPIO); // PA.2
  SYS->GPA_MFPL = (SYS->GPA_MFPL & ~(SYS_GPA_MFPL_PA3MFP_Msk)) | (SYS_GPA_MFPL_PA3MFP_GPIO); // PA.3

  SYS->GPB_MFPL = (SYS->GPB_MFPL & ~(SYS_GPB_MFPL_PB0MFP_Msk)) | (SYS_GPB_MFPL_PB0MFP_GPIO); // PB.0
  SYS->GPB_MFPL = (SYS->GPB_MFPL & ~(SYS_GPB_MFPL_PB1MFP_Msk)) | (SYS_GPB_MFPL_PB1MFP_GPIO); // PB.1

  SYS->GPA_MFPH = (SYS->GPA_MFPH & ~(SYS_GPA_MFPH_PA14MFP_Msk | SYS_GPA_MFPH_PA15MFP_Msk)) |
                  (SYS_GPA_MFPH_PA15MFP_UART0_RXD | SYS_GPA_MFPH_PA14MFP_UART0_TXD);

  SYS_LockReg();
}

void delay_1ms(uint32_t mS) {
  SysTick->VAL &= ~SysTick_VAL_CURRENT_Msk; // сбрасываем старые возможные значения текущего счета в 0
  SysTick->VAL = SYSCLOCK / 1000 - 1;
  SysTick_CNT = mS;
  while (SysTick_CNT) {
  } // как только будет 0, то выходим из цикла и задержка заканчивается
}

void GPIO_Init(void) {
  GPIO_SetMode(PB, BIT0, GPIO_MODE_OUTPUT); // PB.0 --> RS_Pin
  GPIO_SetMode(PB, BIT1, GPIO_MODE_OUTPUT); // PB.1 --> E_Pin
  GPIO_SetMode(PA, BIT0, GPIO_MODE_OUTPUT); // PA.0
  GPIO_SetMode(PA, BIT1, GPIO_MODE_OUTPUT); // PA.1
  GPIO_SetMode(PA, BIT2, GPIO_MODE_OUTPUT); // PA.2
  GPIO_SetMode(PA, BIT3, GPIO_MODE_OUTPUT); // PA.3
}

void lcd_pulse_enable(void) {
  E1;
  delay_1ms(1);
  E0;
  delay_1ms(1);
}
/******************************************************************************
 *                  Функция вывода одного символа (char)                       *
 *******************************************************************************/
void lcd_send_data(uint8_t data) {
  RS1; // RS = 1 (данные)
  PA->DOUT = (data & 0x10) ? PA->DOUT | (1 << LCD_D4) : PA->DOUT & ~(1 << LCD_D4);
  PA->DOUT = (data & 0x20) ? PA->DOUT | (1 << LCD_D5) : PA->DOUT & ~(1 << LCD_D5);
  PA->DOUT = (data & 0x40) ? PA->DOUT | (1 << LCD_D6) : PA->DOUT & ~(1 << LCD_D6);
  PA->DOUT = (data & 0x80) ? PA->DOUT | (1 << LCD_D7) : PA->DOUT & ~(1 << LCD_D7);
  lcd_pulse_enable();

  PA->DOUT = (data & 0x01) ? PA->DOUT | (1 << LCD_D4) : PA->DOUT & ~(1 << LCD_D4);
  PA->DOUT = (data & 0x02) ? PA->DOUT | (1 << LCD_D5) : PA->DOUT & ~(1 << LCD_D5);
  PA->DOUT = (data & 0x04) ? PA->DOUT | (1 << LCD_D6) : PA->DOUT & ~(1 << LCD_D6);
  PA->DOUT = (data & 0x08) ? PA->DOUT | (1 << LCD_D7) : PA->DOUT & ~(1 << LCD_D7);
  lcd_pulse_enable();
}
/******************************************************************************
 *                  Функция записи команды в LCD                               *
 *******************************************************************************/
void lcd_com(uint8_t cmd) {
  RS0;
  PA->DOUT = (cmd & 0x10) ? PA->DOUT | (1 << LCD_D4) : PA->DOUT & ~(1 << LCD_D4);
  PA->DOUT = (cmd & 0x20) ? PA->DOUT | (1 << LCD_D5) : PA->DOUT & ~(1 << LCD_D5);
  PA->DOUT = (cmd & 0x40) ? PA->DOUT | (1 << LCD_D6) : PA->DOUT & ~(1 << LCD_D6);
  PA->DOUT = (cmd & 0x80) ? PA->DOUT | (1 << LCD_D7) : PA->DOUT & ~(1 << LCD_D7);
  lcd_pulse_enable();

  PA->DOUT = (cmd & 0x01) ? PA->DOUT | (1 << LCD_D4) : PA->DOUT & ~(1 << LCD_D4);
  PA->DOUT = (cmd & 0x02) ? PA->DOUT | (1 << LCD_D5) : PA->DOUT & ~(1 << LCD_D5);
  PA->DOUT = (cmd & 0x04) ? PA->DOUT | (1 << LCD_D6) : PA->DOUT & ~(1 << LCD_D6);
  PA->DOUT = (cmd & 0x08) ? PA->DOUT | (1 << LCD_D7) : PA->DOUT & ~(1 << LCD_D7);
  lcd_pulse_enable();
}

/******************************************************************************
 *                   Функция инициализации LCD                                 *
 *******************************************************************************/
void lcd_init_4bit(void) {
  delay_1ms(10); // Ожидание после подачи питания
  // Шаг 1: Установка 4-битного режима
  lcd_com(0x33); // Переход в 8-битный режим (временный)
  lcd_com(0x32); // Переход в 4-битный режим
  // Шаг 2: Основные настройки
  lcd_com(0x28); // 4-бит, 2 строки, шрифт 5x8
  lcd_com(0x0C); // Дисплей включен, курсор выключен
  lcd_com(0x06); // Автоинкремент адреса
  lcd_com(0x01); // Очистка дисплея
  delay_1ms(2);  // Ожидание завершения очистки
}

/******************************************************************************
 *                   Функция установки позиции курсора                         *
 *******************************************************************************/
void lcd_SetPos(uint8_t x, uint8_t y) {
  switch (y) {
  case 0:
    lcd_com(x | 0x80);
    delay_1ms(1);
    break;
  case 1:
    lcd_com((0x40 + x) | 0x80);
    delay_1ms(1);
    break;
  case 2:
    lcd_com((0x14 + x) | 0x80);
    delay_1ms(1);
    break;
  case 3:
    lcd_com((0x54 + x) | 0x80);
    delay_1ms(1);
    break;
  }
}
/******************************************************************************
 *                   Функция вывода строки на экран LCD                        *
 *******************************************************************************/
void lcd_print(const char *s) {
  unsigned int len = strlen(s);
  unsigned int i = 0;
  for (i = 0; i < len; i++) {
    lcd_send_data(s[i]);
  }
}
/******************************************************************************
 *         Функция вывода строка на экран LCD с заданными координатами         *
 *******************************************************************************/
void lcd_string(char *p, uint8_t x, uint8_t y) // Вывести строку на дисплей X,Y
/// lcd_string("HI BABY",4,1) ; выведется сторка по конкретным координатам
{
  lcd_SetPos(x, y);
  while (*p) {
    lcd_send_data(*p++);
  }
}
/******************************************************************************
 *         Функция вывода на экран LCD целого число типа int                   *
 *******************************************************************************/
void LCD_PrintNumber(int num) {
  char buffer[16];
  snprintf(buffer, sizeof(buffer), "%d", num);
  lcd_print(buffer);
}
/******************************************************************************
 *       Функция вывода целого числа  в шестнадцатеричном формате              *
 *******************************************************************************/
void LCD_PrintHex(uint8_t num) {
  char hex[3];
  sprintf(hex, "%X", num);
  lcd_print(hex);
}
/******************************************************************************
 *       Функция вывода целого числа  в бинарном формате                       *
 *******************************************************************************/
void lcd_printBinary(uint8_t num) {
  for (int i = 7; i >= 0; i--) {
    lcd_send_data((num & (1 << i)) ? '1' : '0');
  }
}

/******************************************************************************
 *                   Функция смещения экрана LCD влево на n символов           *
 *******************************************************************************/
void lcds_creen_left(uint8_t n) {
  for (uint8_t i = 0; i < n; i++) {
    lcd_com(0x18);
    delay_1ms(500);
  }
}
/******************************************************************************
 *                  Функция смещения экрана LCD вправо на n символов           *
 *******************************************************************************/
void lcds_creen_right(uint8_t n) {
  for (uint8_t i = 0; i < n; i++) {
    lcd_com(0x1C);
    delay_1ms(500);
  }
}
/******************************************************************************
 *                  Функция очистки экрана LCD                                 *
 *******************************************************************************/
void cleaning_lcd(void) {
  lcd_com(0x01);
  delay_1ms(2);
}
/******************************* End of file ***********************************/