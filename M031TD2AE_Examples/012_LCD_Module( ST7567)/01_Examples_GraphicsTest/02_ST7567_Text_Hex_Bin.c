/**
 * @file ST7567_Text_Hex_Bin.c
 * @brief Вывод текста на ST7567 (SPI) на Nuvoton M031TD2AE.
 *        Вывод чисел разными способами: float, десятичный, шестнадцатеричный и двоичный формат.
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
#include <stdint.h>

#define SET_BIT(REG, BIT)       ((REG) |= (BIT))
#define CLEAR_BIT(REG, BIT)     ((REG) &= ~(BIT))
#define READ_BIT(REG, BIT)      ((REG) & (BIT))
#define CLEAR_REG(REG)          ((REG) = (0x0))
#define WRITE_REG(REG, VAL)     ((REG) = (VAL))
#define READ_REG(REG)           ((REG))
#define MODIFY_REG(REG, CLR, SET) WRITE_REG((REG), ((READ_REG(REG) & ~(CLR)) | (SET)))

#define LCD_WIDTH       128U
#define LCD_PAGES       8U
#define LCD_FB_SIZE     (LCD_WIDTH * LCD_PAGES)
#define LCD_CHAR_WIDTH  6U       /* 5 пикселей символ + 1 пустой столбец */

static uint8_t g_lcdBuffer[LCD_FB_SIZE];

void delay_ms(uint32_t ms);
void SYS_Init(void);
void LCD_GPIO_Init(void);
void SPI0_Init(void);
void SPI_SendByte(uint8_t data);
void LCD_WriteCmd(uint8_t cmd);
void LCD_WriteData(uint8_t data);
void LCD_Init(void);

/* -------------------------------------------------------------------------- */
/* Тактирование и выводы                                                      */
/* -------------------------------------------------------------------------- */

void SYS_Init(void)
{
    SYS_UnlockReg();

    /* HIRC = 48 MHz. */
    SET_BIT(CLK->PWRCTL, CLK_PWRCTL_HIRCEN_Msk);
    while (READ_BIT(CLK->STATUS, CLK_STATUS_HIRCSTB_Msk) != CLK_STATUS_HIRCSTB_Msk)
    {
    }

    MODIFY_REG(CLK->CLKSEL0,
               CLK_CLKSEL0_HCLKSEL_Msk,
               CLK_CLKSEL0_HCLKSEL_HIRC);
    MODIFY_REG(CLK->CLKDIV0,
               CLK_CLKDIV0_HCLKDIV_Msk,
               CLK_CLKDIV0_HCLK(1));

    /* Оставляем выбранный в рабочем проекте источник SPI0 — PCLK1. */
    MODIFY_REG(CLK->CLKSEL2,
               CLK_CLKSEL2_SPI0SEL_Msk,
               CLK_CLKSEL2_SPI0SEL_PCLK1);

    /* Важно: включаем тактирование SPI0, не затирая остальные биты APBCLK0. */
    SET_BIT(CLK->APBCLK0, CLK_APBCLK0_SPI0CKEN_Msk);

    SystemCoreClockUpdate();

    /* PA0 = SPI0_MOSI, PA2 = SPI0_CLK — функция 4, как в рабочем проекте. */
    MODIFY_REG(SYS->GPA_MFPL,
               SYS_GPA_MFPL_PA0MFP_Msk | SYS_GPA_MFPL_PA2MFP_Msk,
               (4UL << SYS_GPA_MFPL_PA0MFP_Pos) |
               (4UL << SYS_GPA_MFPL_PA2MFP_Pos));

    SYS_LockReg();
}

void LCD_GPIO_Init(void)
{
    /* PB0 = RESET, PB1 = DC/A0, PB2 = CS. */
    MODIFY_REG(PB->MODE,
               GPIO_MODE_MODE0_Msk | GPIO_MODE_MODE1_Msk | GPIO_MODE_MODE2_Msk,
               (1UL << GPIO_MODE_MODE0_Pos) |
               (1UL << GPIO_MODE_MODE1_Pos) |
               (1UL << GPIO_MODE_MODE2_Pos));

    /* RESET=1, DC=1, CS=1. */
    SET_BIT(PB->DOUT,
            GPIO_DOUT_DOUT0_Msk |
            GPIO_DOUT_DOUT1_Msk |
            GPIO_DOUT_DOUT2_Msk);
}

/* -------------------------------------------------------------------------- */
/* SPI0                                                                       */
/* -------------------------------------------------------------------------- */

void SPI0_Init(void)
{
    /* Аппаратный сброс SPI0. */
    SET_BIT(SYS->IPRST1, SYS_IPRST1_SPI0RST_Msk);
    CLEAR_BIT(SYS->IPRST1, SYS_IPRST1_SPI0RST_Msk);

    /* Master, 8 бит, передача по спаду (TXNEG), SPI включён. */
    WRITE_REG(SPI0->CTL,
              (8UL << SPI_CTL_DWIDTH_Pos) |
              SPI_CTL_TXNEG_Msk |
              SPI_CTL_SPIEN_Msk);

    /* SCLK = f_SPI0 / (2 * (DIVIDER + 1)).
       При источнике SPI0 = 48 MHz и DIVIDER = 3 получаем 6 MHz. */
    WRITE_REG(SPI0->CLKDIV, 3UL << SPI_CLKDIV_DIVIDER_Pos);
}

void SPI_SendByte(uint8_t data)
{
    /* Не записываем следующий байт, пока передающий буфер занят. */
    while (READ_BIT(SPI0->STATUS, SPI_STATUS_TXFULL_Msk) != 0U)
    {
    }

    WRITE_REG(SPI0->TX, (uint32_t)data);

    /* Дожидаемся окончания передачи последнего бита. */
    while (READ_BIT(SPI0->STATUS, SPI_STATUS_BUSY_Msk) != 0U)
    {
    }
}

/* -------------------------------------------------------------------------- */
/* Низкоуровневые функции ST7567                                              */
/* -------------------------------------------------------------------------- */

void LCD_WriteCmd(uint8_t cmd)
{
    CLEAR_BIT(PB->DOUT, GPIO_DOUT_DOUT2_Msk);  /* CS = 0 */
    CLEAR_BIT(PB->DOUT, GPIO_DOUT_DOUT1_Msk);  /* DC = 0: команда */
    SPI_SendByte(cmd);
    SET_BIT(PB->DOUT, GPIO_DOUT_DOUT2_Msk);    /* CS = 1 */
}

void LCD_WriteData(uint8_t data)
{
    CLEAR_BIT(PB->DOUT, GPIO_DOUT_DOUT2_Msk);  /* CS = 0 */
    SET_BIT(PB->DOUT, GPIO_DOUT_DOUT1_Msk);    /* DC = 1: данные */
    SPI_SendByte(data);
    SET_BIT(PB->DOUT, GPIO_DOUT_DOUT2_Msk);    /* CS = 1 */
}

void LCD_Init(void)
{
    /* Аппаратный сброс дисплея через PB0. */
    CLEAR_BIT(PB->DOUT, GPIO_DOUT_DOUT0_Msk);
    delay_ms(50U);
    SET_BIT(PB->DOUT, GPIO_DOUT_DOUT0_Msk);
    delay_ms(50U);

    /* Последовательность инициализации из рабочего SPI_main.txt. */
    LCD_WriteCmd(0xE2U); /* Software reset */
    delay_ms(10U);

    LCD_WriteCmd(0xA2U); /* LCD bias = 1/9 */
    LCD_WriteCmd(0xA0U); /* SEG direction: normal */
    LCD_WriteCmd(0xC8U); /* COM direction: reverse */

    LCD_WriteCmd(0x2FU); /* Booster, regulator, follower ON */
    LCD_WriteCmd(0x24U); /* V0 */

    LCD_WriteCmd(0x81U); /* Electronic volume mode */
    LCD_WriteCmd(0x20U); /* Contrast value */

    LCD_WriteCmd(0x40U); /* Display start line = 0 */
    LCD_WriteCmd(0xAFU); /* Display ON */
}

/* -------------------------------------------------------------------------- */
/* Шрифт 5x7: заглавные латинские буквы, цифры и нужные знаки                 */
/* Каждый байт описывает вертикальную колонку символа; бит 0 — верхний пиксель. */
/* -------------------------------------------------------------------------- */

static const uint8_t *Font5x7(char ch)
{
    static const uint8_t blank[5] = {0x00U, 0x00U, 0x00U, 0x00U, 0x00U};

    static const uint8_t letters[26][5] = {
        {0x7EU, 0x11U, 0x11U, 0x11U, 0x7EU}, /* A */
        {0x7FU, 0x49U, 0x49U, 0x49U, 0x36U}, /* B */
        {0x3EU, 0x41U, 0x41U, 0x41U, 0x22U}, /* C */
        {0x7FU, 0x41U, 0x41U, 0x22U, 0x1CU}, /* D */
        {0x7FU, 0x49U, 0x49U, 0x49U, 0x41U}, /* E */
        {0x7FU, 0x09U, 0x09U, 0x09U, 0x01U}, /* F */
        {0x3EU, 0x41U, 0x49U, 0x49U, 0x7AU}, /* G */
        {0x7FU, 0x08U, 0x08U, 0x08U, 0x7FU}, /* H */
        {0x00U, 0x41U, 0x7FU, 0x41U, 0x00U}, /* I */
        {0x20U, 0x40U, 0x41U, 0x3FU, 0x01U}, /* J */
        {0x7FU, 0x08U, 0x14U, 0x22U, 0x41U}, /* K */
        {0x7FU, 0x40U, 0x40U, 0x40U, 0x40U}, /* L */
        {0x7FU, 0x02U, 0x0CU, 0x02U, 0x7FU}, /* M */
        {0x7FU, 0x04U, 0x08U, 0x10U, 0x7FU}, /* N */
        {0x3EU, 0x41U, 0x41U, 0x41U, 0x3EU}, /* O */
        {0x7FU, 0x09U, 0x09U, 0x09U, 0x06U}, /* P */
        {0x3EU, 0x41U, 0x51U, 0x21U, 0x5EU}, /* Q */
        {0x7FU, 0x09U, 0x19U, 0x29U, 0x46U}, /* R */
        {0x46U, 0x49U, 0x49U, 0x49U, 0x31U}, /* S */
        {0x01U, 0x01U, 0x7FU, 0x01U, 0x01U}, /* T */
        {0x3FU, 0x40U, 0x40U, 0x40U, 0x3FU}, /* U */
        {0x1FU, 0x20U, 0x40U, 0x20U, 0x1FU}, /* V */
        {0x3FU, 0x40U, 0x38U, 0x40U, 0x3FU}, /* W */
        {0x63U, 0x14U, 0x08U, 0x14U, 0x63U}, /* X */
        {0x07U, 0x08U, 0x70U, 0x08U, 0x07U}, /* Y */
        {0x61U, 0x51U, 0x49U, 0x45U, 0x43U}  /* Z */
    };

    static const uint8_t digits[10][5] = {
        {0x3EU, 0x51U, 0x49U, 0x45U, 0x3EU}, /* 0 */
        {0x00U, 0x42U, 0x7FU, 0x40U, 0x00U}, /* 1 */
        {0x42U, 0x61U, 0x51U, 0x49U, 0x46U}, /* 2 */
        {0x21U, 0x41U, 0x45U, 0x4BU, 0x31U}, /* 3 */
        {0x18U, 0x14U, 0x12U, 0x7FU, 0x10U}, /* 4 */
        {0x27U, 0x45U, 0x45U, 0x45U, 0x39U}, /* 5 */
        {0x3CU, 0x4AU, 0x49U, 0x49U, 0x30U}, /* 6 */
        {0x01U, 0x71U, 0x09U, 0x05U, 0x03U}, /* 7 */
        {0x36U, 0x49U, 0x49U, 0x49U, 0x36U}, /* 8 */
        {0x06U, 0x49U, 0x49U, 0x29U, 0x1EU}  /* 9 */
    };

    static const uint8_t colon[5]  = {0x00U, 0x36U, 0x36U, 0x00U, 0x00U};
    static const uint8_t period[5] = {0x00U, 0x60U, 0x60U, 0x00U, 0x00U};
    static const uint8_t plus[5]   = {0x08U, 0x08U, 0x3EU, 0x08U, 0x08U};
    static const uint8_t minus[5]  = {0x08U, 0x08U, 0x08U, 0x08U, 0x08U};

    if ((ch >= 'a') && (ch <= 'z'))
    {
        ch = (char)(ch - 'a' + 'A');
    }

    if ((ch >= 'A') && (ch <= 'Z'))
    {
        return letters[(uint8_t)(ch - 'A')];
    }

    if ((ch >= '0') && (ch <= '9'))
    {
        return digits[(uint8_t)(ch - '0')];
    }

    switch (ch)
    {
        case ':': return colon;
        case '.': return period;
        case '+': return plus;
        case '-': return minus;
        case ' ': return blank;
        default:  return blank;
    }
}

static void LCD_ClearBuffer(void)
{
    uint16_t i;
    for (i = 0U; i < LCD_FB_SIZE; i++)
    {
        g_lcdBuffer[i] = 0x00U;
    }
}

/* page = номер вертикальной страницы 0..7; одна страница — 8 пикселей высотой. */
static void LCD_DrawString(uint8_t x, uint8_t page, const char *text)
{
    uint16_t index;
    const uint8_t *glyph;
    uint8_t col;

    if (page >= LCD_PAGES)
    {
        return;
    }

    while ((*text != '\0') && (x <= (LCD_WIDTH - LCD_CHAR_WIDTH)))
    {
        glyph = Font5x7(*text);
        index = (uint16_t)(((uint16_t)page * (uint16_t)LCD_WIDTH) + x);

        for (col = 0U; col < 5U; col++)
        {
            g_lcdBuffer[index + col] = glyph[col];
        }
        g_lcdBuffer[index + 5U] = 0x00U; /* межсимвольный интервал */

        text++;
        x = (uint8_t)(x + LCD_CHAR_WIDTH);
    }
}

static void LCD_Update(void)
{
    uint8_t page;
    uint8_t col;

    for (page = 0U; page < LCD_PAGES; page++)
    {
        LCD_WriteCmd((uint8_t)(0xB0U | page)); /* выбор страницы */
        LCD_WriteCmd(0x10U);                   /* старшие 4 бита колонки */
        LCD_WriteCmd(0x00U);                   /* младшие 4 бита колонки */

        for (col = 0U; col < LCD_WIDTH; col++)
        {
            LCD_WriteData(g_lcdBuffer[((uint16_t)page * LCD_WIDTH) + col]);
        }
    }
}

/* -------------------------------------------------------------------------- */
/* Форматирование чисел без printf/sprintf                                    */
/* -------------------------------------------------------------------------- */

static char *AppendUnsigned(char *dst, uint32_t value)
{
    char temp[10];
    uint8_t count = 0U;

    do
    {
        temp[count] = (char)('0' + (value % 10U));
        count++;
        value /= 10U;
    } while ((value != 0U) && (count < sizeof(temp)));

    while (count > 0U)
    {
        count--;
        *dst++ = temp[count];
    }

    *dst = '\0';
    return dst;
}

static char *AppendSignedInteger(char *dst, int32_t value)
{
    uint32_t magnitude;

    if (value < 0)
    {
        *dst++ = '-';
        magnitude = (uint32_t)(-value);
    }
    else
    {
        *dst++ = '+';
        magnitude = (uint32_t)value;
    }

    return AppendUnsigned(dst, magnitude);
}

static char *AppendSignedFixed2(char *dst, int32_t value100)
{
    uint32_t magnitude;

    if (value100 < 0)
    {
        *dst++ = '-';
        magnitude = (uint32_t)(-value100);
    }
    else
    {
        *dst++ = '+';
        magnitude = (uint32_t)value100;
    }

    dst = AppendUnsigned(dst, magnitude / 100U);
    *dst++ = '.';
    *dst++ = (char)('0' + ((magnitude / 10U) % 10U));
    *dst++ = (char)('0' + (magnitude % 10U));
    *dst = '\0';
    return dst;
}

static int32_t FloatToHundredths(float value)
{
    float scaled = value * 100.0f;

    /* Округление к ближайшей сотой, в том числе для отрицательных значений. */
    if (scaled < 0.0f)
    {
        return -(int32_t)((-scaled) + 0.5f);
    }
    return (int32_t)(scaled + 0.5f);
}

static void FormatUnsignedLine(char *out, uint32_t value)
{
    out[0] = 'U';
    out[1] = 'i';
    out[2] = 'n';
    out[3] = 't';
    out[4] = ':';
    out[5] = ' ';
    (void)AppendUnsigned(&out[6], value);
}

static void FormatIntegerLine(char *out, int32_t value)
{
    out[0] = 'I';
    out[1] = 'N';
    out[2] = 'T';
    out[3] = ':';
    out[4] = ' ';
    (void)AppendSignedInteger(&out[5], value);
}

static void FormatFloatLine(char *out, float value)
{
    out[0] = 'F';
    out[1] = 'L';
    out[2] = 'O';
    out[3] = 'A';
    out[4] = 'T';
    out[5] = ':';
    out[6] = ' ';
    (void)AppendSignedFixed2(&out[7], FloatToHundredths(value));
}

/* -------------------------------------------------------------------------- */
/* Вывод одного числа двумя способами: шестнадцатеричный и двоичный формат.   */
/* Оба способа работают с uint8_t, без printf и дополнительных библиотек.    */
/* -------------------------------------------------------------------------- */

/* Таблица символов для преобразования 4-битного значения в HEX-цифру. */
static char HexDigit(uint8_t nibble)
{
    static const char digits[] = "0123456789ABCDEF";
    return digits[nibble & 0x0FU];
}

/* 6-я строка экрана: HEX: 0x00 ... HEX: 0x64 (0..100 десятично). */
static void FormatHexLine(char *out, uint8_t value)
{
    out[0] = 'H';
    out[1] = 'E';
    out[2] = 'X';
    out[3] = ':';
    out[4] = ' ';
    out[5] = '0';
    out[6] = 'x';

    /* Старшая и младшая тетрады байта (по четыре бита). */
    out[7] = HexDigit((uint8_t)(value >> 4U));
    out[8] = HexDigit(value);
    out[9] = '\0';
}

/* 7-я строка экрана: BIN: 00000000 ... BIN: 01100100. */
static void FormatBinaryLine(char *out, uint8_t value)
{
    uint8_t bit;

    out[0] = 'B';
    out[1] = 'I';
    out[2] = 'N';
    out[3] = ':';
    out[4] = ' ';

    /* Выводим восемь битов слева направо: от bit7 до bit0. */
    for (bit = 0U; bit < 8U; bit++)
    {
        uint8_t shift = (uint8_t)(7U - bit);
        out[5U + bit] = ((value >> shift) & 1U) != 0U ? '1' : '0';
    }

    out[13] = '\0';
}

/* -------------------------------------------------------------------------- */
/* Основная программа                                                         */
/* -------------------------------------------------------------------------- */

int main(void)
{
    uint8_t counterU = 0U;        /* uint8_t: 0..255..0 */
    uint8_t counterUDirection = 1U;

    int32_t counterI = -100;      /* int32_t: -100..+100..-100 */
    uint8_t counterIDirection = 1U;

    int16_t floatHundredths = -500; /* -5.00..+5.00..-5.00 */
    uint8_t floatDirection = 1U;
    float floatValue = -5.0f;

    /* Общий счётчик для HEX и BIN: 0 -> 100 -> 0. */
    uint8_t counterHexBin = 0U;
    uint8_t counterHexBinDirection = 1U;

    char lineU[22];
    char lineI[22];
    char lineF[22];
    char lineHex[22];
    char lineBin[22];

    SYS_Init();
    LCD_GPIO_Init();
    SPI0_Init();
    LCD_Init();

    while (1)
    {
        /* Строка 1 — постоянный текст. */
        FormatUnsignedLine(lineU, counterU);
        FormatIntegerLine(lineI, counterI);

        /* Значение хранится и обрабатывается как float, формат — две цифры. */
        floatValue = (float)floatHundredths / 100.0f;
        FormatFloatLine(lineF, floatValue);

        /* Строки 6 и 7: одно число 0..100 в двух системах счисления. */
        FormatHexLine(lineHex, counterHexBin);
        FormatBinaryLine(lineBin, counterHexBin);

        LCD_ClearBuffer();
        LCD_DrawString(0U, 0U, "NUVOTON M031TD2AE");
        LCD_DrawString(0U, 1U, "DISPLAY ST7567");
        LCD_DrawString(0U, 2U, lineU);
        LCD_DrawString(0U, 3U, lineI);
        LCD_DrawString(0U, 4U, lineF);
        LCD_DrawString(0U, 5U, lineHex);
        LCD_DrawString(0U, 6U, lineBin);
        LCD_Update();

        /* Счётчик uint: 0 -> 255 -> 0. */
        if (counterUDirection != 0U)
        {
            if (counterU < 255U)
            {
                counterU++;
            }
            else
            {
                counterUDirection = 0U;
                counterU--;
            }
        }
        else
        {
            if (counterU > 0U)
            {
                counterU--;
            }
            else
            {
                counterUDirection = 1U;
                counterU++;
            }
        }

        /* Счётчик int: -100 -> +100 -> -100. */
        if (counterIDirection != 0U)
        {
            if (counterI < 100)
            {
                counterI++;
            }
            else
            {
                counterIDirection = 0U;
                counterI--;
            }
        }
        else
        {
            if (counterI > -100)
            {
                counterI--;
            }
            else
            {
                counterIDirection = 1U;
                counterI++;
            }
        }

        /* float: -5.00 -> +5.00 -> -5.00 с шагом 0.05. */
        if (floatDirection != 0U)
        {
            if (floatHundredths < 500)
            {
                floatHundredths = (int16_t)(floatHundredths + 5);
            }
            else
            {
                floatDirection = 0U;
                floatHundredths = (int16_t)(floatHundredths - 5);
            }
        }
        else
        {
            if (floatHundredths > -500)
            {
                floatHundredths = (int16_t)(floatHundredths - 5);
            }
            else
            {
                floatDirection = 1U;
                floatHundredths = (int16_t)(floatHundredths + 5);
            }
        }

        /* Общий счётчик HEX/BIN изменяется так же, как uint:
           сначала возрастает до 100, затем уменьшается до 0.
           На обоих строках всегда отображается одно и то же значение. */
        if (counterHexBinDirection != 0U)
        {
            if (counterHexBin < 100U)
            {
                counterHexBin++;
            }
            else
            {
                counterHexBinDirection = 0U;
                counterHexBin--;
            }
        }
        else
        {
            if (counterHexBin > 0U)
            {
                counterHexBin--;
            }
            else
            {
                counterHexBinDirection = 1U;
                counterHexBin++;
            }
        }

        delay_ms(750U);
    }
}

/* -------------------------------------------------------------------------- */
/* Задержка на SysTick: 1 мс при SystemCoreClock, рассчитанном выше.          */
/* -------------------------------------------------------------------------- */

void delay_ms(uint32_t ms)
{
    if (ms == 0U)
    {
        return;
    }

    SysTick->LOAD = (uint32_t)((SystemCoreClock / 1000UL) - 1UL);
    SysTick->VAL = 0UL;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;

    while (ms > 0U)
    {
        while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0U)
        {
        }
        ms--;
    }

    SysTick->CTRL = 0UL;
}
