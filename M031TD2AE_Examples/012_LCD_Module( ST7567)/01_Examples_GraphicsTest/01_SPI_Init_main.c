/**
 * @file SPI_Init.c
 * @brief Инициализация монохромного дисплея ST7567 (SPI) на Nuvoton M031TD2AE.
 *        в примере реализована горизонтальная и вертикальная зебра 
 *
 * Микроконтроллер: Nuvoton M031TD2AE
 * Дисплей:         ST7567, 128 x 64, SPI0
 * Среда (IDE):     SEGGER Embedded Studio.
 *
 *   PA0 = SPI0_MOSI (MFP function 4)
 *   PA2 = SPI0_CLK  (MFP function 4)
 *   PB0 = LCD RESET
 *   PB1 = LCD A0/DC
 *   PB2 = LCD CS
 *
 *
 *    M031TD2AE             ST7567 
 *   ------------        ---------
 *  |            |      |
 *  |            |      |
 *  |            |      |
 *  |        PA.0| ---->| SPI0_MOSI
 *  |        PA.2| ---->| SPI0_CLK
 *  |        PB.0| ---->| RES
 *  |        PB.1| ---->| D/C
 *  |        PB.2| ---->| CS
 *  |            |      |
 *  |            |      |
 *  |      +3.3V | ---> | BL
 *  |      +3.3V | ---> | VCC
 *  |        GND | ---> | GND
 *
 *   VCC 3.3-5V 
 *   I/O 3.3V
 *
 *
 *\ author         ScuratovaAnna 
 *\ сode debugging PivnevNikolay
 */

#include "NuMicro.h"
#include <stdio.h>

uint8_t toggle = 0; 

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

  // HIRC = 48 MHz.
  SET_BIT(CLK->PWRCTL, CLK_PWRCTL_HIRCEN_Msk);
  while (READ_BIT(CLK->STATUS, CLK_STATUS_HIRCSTB_Msk) != CLK_STATUS_HIRCSTB_Msk) {
  }

  MODIFY_REG(CLK->CLKSEL0,
      CLK_CLKSEL0_HCLKSEL_Msk,
      CLK_CLKSEL0_HCLKSEL_HIRC);
  MODIFY_REG(CLK->CLKDIV0,
      CLK_CLKDIV0_HCLKDIV_Msk,
      CLK_CLKDIV0_HCLK(1));

  // Оставляем выбранный в рабочем проекте источник SPI0 — PCLK1.
  MODIFY_REG(CLK->CLKSEL2,
      CLK_CLKSEL2_SPI0SEL_Msk,
      CLK_CLKSEL2_SPI0SEL_PCLK1);

  // Важно: включаем тактирование SPI0, не затирая остальные биты APBCLK0.
  SET_BIT(CLK->APBCLK0, CLK_APBCLK0_SPI0CKEN_Msk);

  SystemCoreClockUpdate();

  MODIFY_REG(SYS->GPA_MFPL,
      SYS_GPA_MFPL_PA0MFP_Msk | SYS_GPA_MFPL_PA2MFP_Msk,
      (4UL << SYS_GPA_MFPL_PA0MFP_Pos) |
          (4UL << SYS_GPA_MFPL_PA2MFP_Pos));

  SYS_LockReg();
}

void LCD_GPIO_Init(void) {
    // PB0 = RESET, PB1 = DC/A0, PB2 = CS.
    MODIFY_REG(PB->MODE,
               GPIO_MODE_MODE0_Msk | GPIO_MODE_MODE1_Msk | GPIO_MODE_MODE2_Msk,
               (1UL << GPIO_MODE_MODE0_Pos) |
               (1UL << GPIO_MODE_MODE1_Pos) |
               (1UL << GPIO_MODE_MODE2_Pos));

    // RESET=1, DC=1, CS=1.
    SET_BIT(PB->DOUT,
            GPIO_DOUT_DOUT0_Msk |
            GPIO_DOUT_DOUT1_Msk |
            GPIO_DOUT_DOUT2_Msk);
}

void SPI0_Init(void) {
    // Аппаратный сброс SPI0.
    SET_BIT(SYS->IPRST1, SYS_IPRST1_SPI0RST_Msk);
    CLEAR_BIT(SYS->IPRST1, SYS_IPRST1_SPI0RST_Msk);

    // Master, 8 бит, передача по спаду (TXNEG), SPI включён.
    WRITE_REG(SPI0->CTL,
              (8UL << SPI_CTL_DWIDTH_Pos) |
              SPI_CTL_TXNEG_Msk |
              SPI_CTL_SPIEN_Msk);

    // SCLK = f_SPI0 / (2 * (DIVIDER + 1)).
       При источнике SPI0 = 48 MHz и DIVIDER = 11 получаем 2 MHz. */
    WRITE_REG(SPI0->CLKDIV, 11UL << SPI_CLKDIV_DIVIDER_Pos);
}

// Отправка байта по аппаратному SPI с ожиданием окончания
void SPI_SendByte(uint8_t data) {
  WRITE_REG(SPI0->TX, (uint32_t)data); // Запись данных в буфер TX
  while (READ_BIT(SPI0->STATUS, SPI_STATUS_BUSY_Msk))
    ; // Ожидание снятия флага BUSY
}

// Отправка команды (DC = 0)
void LCD_WriteCmd(uint8_t cmd) {
  CLEAR_BIT(PB->DOUT, GPIO_DOUT_DOUT2_Msk); // CS = 0
  CLEAR_BIT(PB->DOUT, GPIO_DOUT_DOUT1_Msk); // DC = 0
  SPI_SendByte(cmd);
  SET_BIT(PB->DOUT, GPIO_DOUT_DOUT2_Msk);   // CS = 1
}

// Отправка данных (DC = 1)
void LCD_WriteData(uint8_t data) {
  CLEAR_BIT(PB->DOUT, GPIO_DOUT_DOUT2_Msk); // CS = 0
  SET_BIT(PB->DOUT, GPIO_DOUT_DOUT1_Msk);   // DC = 1
  SPI_SendByte(data);
  SET_BIT(PB->DOUT, GPIO_DOUT_DOUT2_Msk);   // CS = 1
}

void LCD_Init(void) {
     // Аппаратный сброс дисплея через PB0.
    CLEAR_BIT(PB->DOUT, GPIO_DOUT_DOUT0_Msk);
    delay_ms(50U);
    SET_BIT(PB->DOUT, GPIO_DOUT_DOUT0_Msk);
    delay_ms(50U);

    // Последовательность инициализации
    LCD_WriteCmd(0xE2U); 
    delay_ms(10U);

    LCD_WriteCmd(0xA2U); 
    LCD_WriteCmd(0xA0U); 
    LCD_WriteCmd(0xC8U); 

    LCD_WriteCmd(0x2FU); 
    LCD_WriteCmd(0x24U); 

    LCD_WriteCmd(0x81U); 
    LCD_WriteCmd(0x20U); 

    LCD_WriteCmd(0x40U); 
    LCD_WriteCmd(0xAFU); 
}

int main(void) {
  SYS_Init();      // Инициализация тактирования и мультиплексоров пинов
  LCD_GPIO_Init(); // Инициализация служебных выводов CS, DC, RST
  SPI0_Init();     // Инициализация модуля SPI0
  LCD_Init();      // Инициализация дисплея

  while (1) {
    for (uint8_t page = 0; page < 8; page++) {
      LCD_WriteCmd(0xB0 + page); // Выбор страницы (от 0 до 7)
      LCD_WriteCmd(0x00);        // Младшие 4 бита столбца (0)
      LCD_WriteCmd(0x10);        // Старшие 4 бита столбца (0)

      for (uint8_t col = 0; col < 128; col++) {
        if (toggle == 0) {
          // Горизонтальная "зебра" (чередование пикселей внутри столбца)
          LCD_WriteData(0xAA);
        } else {
          // Вертикальная "зебра" (чередование самих столбцов: черный / белый)
          if (col % 2 == 0) {
            LCD_WriteData(0xFF); // Весь столбец (8 пикселей) черный
          } else {
            LCD_WriteData(0x00); // Весь столбец (8 пикселей) белый
          }
        }
      }
    }

    // Меняем состояние на противоположное (0 на 1, 1 на 0)
    toggle = !toggle;

    // Задержка перед следующей сменой узора (используем функцию из вашего Blink)
    delay_ms(1000);
  }
}
/* -------------------------------------------------------------------------- */
/* Задержка на SysTick: 1 мс при SystemCoreClock, рассчитанном выше.          */
/* -------------------------------------------------------------------------- */
void delay_ms(uint32_t ms) {
  if (ms == 0U) {
    return;
  }

  SysTick->LOAD = (uint32_t)((SystemCoreClock / 1000UL) - 1UL);
  SysTick->VAL = 0UL;
  SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;

  while (ms > 0U) {
    while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0U) {
    }
    ms--;
  }

  SysTick->CTRL = 0UL;
}
 //------------------- End of file ----------------------