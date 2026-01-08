/**-----------------------------------------------------------------------
 \date  24.11.2025
 *
 *    M031TD2AE
 *   ------------
 *  |            |
 *  |            |
 *  |            |
 *  |        PA.1| ---->  IIC_SCL
 *  |        PA.2| ---->  IIC_SDA
 *  |            |
 *  |            |
 *  |            |
 *  |      +3.3V | ---> +3.3V
 *  |        GND | ---> GND
 * 
 * Cсылка на модуль: https://aliexpress.ru/item/1005004373244103.html?spm=a2g2w.favourites.mywishlist.23.3b1e4aa6RhaJqq&sku_id=12000028949384173
 * Cсылка на видео:  https://www.youtube.com/watch?v=Qpi0Y5bcvlQ&list=LL&index=17
 * Source Files:     https://github.com/upiir/arduino_uno_r4_led_display
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

__IO uint32_t Nuvoton_ticks;

uint8_t data_line = 0;
uint8_t delay_count = 0;

uint8_t table[22][16] = {//logo
    {0x00,0x00,0x00,0x00,0x00,0x00,0x8, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},//1
    {0x00,0x00,0x00,0x00,0x00,0x4, 0x8, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},//2
    {0x00,0x4, 0x2, 0x2, 0x2, 0x4, 0x8, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},//3
    {0x18,0x24,0x42,0x42,0x42,0x24,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},//4
    {0x18,0x24,0x42,0x42,0x42,0x24,0x18,0x8, 0x4, 0x2, 0x2, 0x2, 0x00,0x00,0x00,0x00},//5
    {0x18,0x24,0x42,0x42,0x42,0x24,0x18,0x8, 0x4, 0x2, 0x2, 0x2, 0x24,0x18,0x00,0x00},//6
    {0x18,0x24,0x42,0x42,0x42,0x24,0x18,0x8, 0x4, 0x42,0x42,0x42,0x24,0x18,0x00,0x00},//7
    {0x18,0x24,0x42,0x42,0x42,0x24,0x18,0x8, 0x24,0x42,0x42,0x42,0x24,0x18,0x00,0x00},//8
    {0x18,0x24,0x42,0x42,0x42,0x24,0x18,0x18,0x24,0x42,0x42,0x42,0x24,0x18,0x00,0x00},//9
    {0x18,0x24,0x42,0x42,0x42,0x24,0x18,0x18,0x24,0x42,0x42,0x42,0x24,0x18,0x00,0x00},//10
    {0x18,0x24,0x42,0x5A,0x42,0x24,0x18,0x18,0x24,0x42,0x5A,0x42,0x24,0x18,0x00,0x00},//11
    {0x18,0x24,0x42,0x18,0x18,0x24,0x00,0x00,0x24,0x18,0x18,0x42,0x24,0x18,0x00,0x00},//12
    {0x00,0x18,0x3C,0x3C,0x3C,0x18,0x00,0x00,0x18,0x18,0x18,0x18,0x18,0x18,0x00,0x00},//13
    {0x18,0x18,0x7E,0x7E,0x18,0x18,0x00,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x00,0x00},//14
    {0x18,0x18,0x7E,0x7E,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x00},//15
    {0x18,0x18,0xFF,0xFF,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18},//16
    {0x18,0x18,0xFF,0xFF,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18},//17
    {0x18,0x18,0xE7,0xE7,0x18,0x18,0x18,0x00,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x00},//18
    {0x18,0x00,0xC3,0xC3,0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x18,0x18,0x18,0x00,0x00},//19
    {0x00,0x00,0x81,0x81,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x00},//20
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},//21
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}};

void SYS_Init(void);
void LED_GPIO_init(void);
void systick_config(void);
void delay_ms(uint32_t ms);
void delayMicroseconds(unsigned int us);
uint32_t getCurrentMillis(void);
void IIC_start(void);
void IIC_send(uint8_t send_data);
void IIC_end(void);

void SysTick_Handler(void) {
  Nuvoton_ticks++;
}

int main(void) {
  SYS_Init();
  systick_config();
  LED_GPIO_init();
  while (1) {
    //------------------- start ----------------------------
    IIC_start();
    IIC_send(0x40); // set the address plus 1 automatically
    IIC_end();
    //------- end the process of address plus 1 ------------

    //------------- set the data display -------------------
    IIC_start();
    IIC_send(0xc0); // set the initial address as 0
    for (char i = 0; i < 16; i++) {
      IIC_send(table[data_line][i]); // send the display data
    }
      data_line++;
      if (data_line >= 22) {
        data_line = 0;
      }
    IIC_end();
    //------------- end the data display -------------------
    //------------- set the brightness display -------------
    IIC_start();
    IIC_send(0x89);//(0x88); // set the brightness display--IIC_send(0xC3);
    IIC_end();
    delay_ms(60);

    //------------ end the brightness display --------------
  }
}

void SYS_Init(void) {
  SYS_UnlockReg();
  //--------- Enable HIRC clock (Internal RC 48MHz)  ----------------
  SET_BIT(CLK->PWRCTL, CLK_PWRCTL_HIRCEN_Msk);

  // Waiting for HIRC clock ready
  while (READ_BIT(CLK->STATUS, CLK_STATUS_HIRCSTB_Msk != CLK_STATUS_HIRCSTB_Msk));

  // Switch HCLK clock source to HIRC
  MODIFY_REG(CLK->CLKSEL0, CLK_CLKSEL0_HCLKSEL_Msk, CLK_CLKSEL0_HCLKSEL_HIRC);

  MODIFY_REG(CLK->CLKDIV0, CLK_CLKDIV0_HCLKDIV_Msk, CLK_CLKDIV0_HCLK(1));

  // Waiting for HIRC clock ready
  while (READ_BIT(CLK->STATUS, CLK_STATUS_HIRCSTB_Msk != CLK_STATUS_HIRCSTB_Msk))
    ;

  SystemCoreClockUpdate();

  MODIFY_REG(SYS->GPA_MFPL, SYS_GPA_MFPL_PA1MFP_Msk, SYS_GPA_MFPL_PA1MFP_GPIO); // PA1 setting for GPIO --> IIC_SCL
  MODIFY_REG(SYS->GPA_MFPL, SYS_GPA_MFPL_PA2MFP_Msk, SYS_GPA_MFPL_PA2MFP_GPIO); // PA2 setting for GPIO --> IIC_SDA

  SYS_LockReg();
}

void LED_GPIO_init(void) {
  MODIFY_REG(PA->MODE, GPIO_MODE_MODE1_Msk, GPIO_MODE_OUTPUT << GPIO_MODE_MODE1_Pos); // --> IIC_SCL
  MODIFY_REG(PA->MODE, GPIO_MODE_MODE2_Msk, GPIO_MODE_OUTPUT << GPIO_MODE_MODE2_Pos); // --> IIC_SDA

  CLEAR_BIT(PA->DOUT, GPIO_DOUT_DOUT1_Msk); // PA1 = 0; --> IIC_SCL
  CLEAR_BIT(PA->DOUT, GPIO_DOUT_DOUT2_Msk); // PA2 = 0; --> IIC_SDA

  __NOP();
}

void systick_config(void) {
  // setup systick timer for 1000Hz interrupts
  if (SysTick_Config(SystemCoreClock / 1000U)) {
    // capture error-> ошибка захвата))
    while (1) {
    }
  }
  // configure the systick handler priority
  NVIC_SetPriority(SysTick_IRQn, 0x00U);
}

void delay_ms(uint32_t ms) {
  if (ms != 0) {
    uint32_t start = getCurrentMillis();
    do {
      __NOP();
    } while (getCurrentMillis() - start < ms);
  }
}

uint32_t getCurrentMillis(void) {
  return Nuvoton_ticks;
}

void delayMicroseconds(unsigned int us) {// Задержка в микросекундах (точная)
  __IO uint32_t currentTicks = SysTick->VAL;
  // Number of ticks per millisecond
  const uint32_t tickPerMs = SysTick->LOAD + 1;
  // Number of ticks to count
  const uint32_t nbTicks = ((us - ((us > 0) ? 1 : 0)) * tickPerMs) / 1000;
  // Number of elapsed ticks
  uint32_t elapsedTicks = 0;
  __IO uint32_t oldTicks = currentTicks;
  do {
    currentTicks = SysTick->VAL;
    elapsedTicks += (oldTicks < currentTicks) ? tickPerMs + oldTicks - currentTicks : oldTicks - currentTicks;
    oldTicks = currentTicks;
  } while (nbTicks > elapsedTicks);
}

void IIC_start(void) {
  // (IIC_SCL,LOW);
  CLEAR_BIT(PA->DOUT, GPIO_DOUT_DOUT1_Msk); // PA1 = 0; --> IIC_SCL
  delayMicroseconds(3);
  // (IIC_SDA,HIGH);
  SET_BIT(PA->DOUT, GPIO_DOUT_DOUT2_Msk);   // PA2 = 1; --> IIC_SDA
  delayMicroseconds(3);
  // (IIC_SCL,HIGH);
  SET_BIT(PA->DOUT, GPIO_DOUT_DOUT1_Msk);   // PA1 = 1; --> IIC_SCL
  delayMicroseconds(3);
  // (IIC_SDA,LOW);
  CLEAR_BIT(PA->DOUT, GPIO_DOUT_DOUT2_Msk); // PA2 = 0; --> IIC_SDA
  delayMicroseconds(3);
}

void IIC_send(uint8_t send_data) {
  for (uint8_t i = 0; i < 8; i++) {
    // (IIC_SCL,LOW);
    CLEAR_BIT(PA->DOUT, GPIO_DOUT_DOUT1_Msk);    // PA1 = 0; --> IIC_SCL
    delayMicroseconds(3);
    if (send_data & 0x01) {
    // (IIC_SDA,HIGH);
      SET_BIT(PA->DOUT, GPIO_DOUT_DOUT2_Msk);    // PA2 = 1; --> IIC_SDA
    } else {
    // (IIC_SDA,LOW);
      CLEAR_BIT(PA->DOUT, GPIO_DOUT_DOUT2_Msk); // PA2 = 0; --> IIC_SDA
    }
    delayMicroseconds(3);
    // (IIC_SCL,HIGH);
    SET_BIT(PA->DOUT, GPIO_DOUT_DOUT1_Msk);     // PA1 = 1; --> IIC_SCL
    delayMicroseconds(3);
    send_data = send_data >> 1;
  }
}

void IIC_end(void) {
  // (IIC_SCL,LOW);
  CLEAR_BIT(PA->DOUT, GPIO_DOUT_DOUT1_Msk); // PA1 = 0; --> IIC_SCL
  delayMicroseconds(3);
  // (IIC_SDA,LOW);
  CLEAR_BIT(PA->DOUT, GPIO_DOUT_DOUT2_Msk); // PA2 = 0; --> IIC_SDA
  delayMicroseconds(3);
  // (IIC_SCL,HIGH);
  SET_BIT(PA->DOUT, GPIO_DOUT_DOUT1_Msk);   // PA1 = 1; --> IIC_SCL
  delayMicroseconds(3);
  // (IIC_SDA,HIGH);
  SET_BIT(PA->DOUT, GPIO_DOUT_DOUT2_Msk);   // PA2 = 1; --> IIC_SDA
  delayMicroseconds(3);
}
  //------------------- End of file ----------------------
