/**
 * @file SPI_main_rotating_cube.c
 * @brief Единственная демонстрационная сцена: неподвижный по центру,
 *        непрерывно вращающийся каркасный 3D-куб с индикатором осей.
 *
 * Микроконтроллер: Nuvoton M031TD2AE
 * Дисплей:        ST7567, 128 x 64, SPI0
 * Среда:          SEGGER Embedded Studio
 *
 * СОХРАНЁННОЕ ПОДКЛЮЧЕНИЕ ИЗ РАБОЧЕГО SPI_main.txt:
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
 * ГРАФИКА:
 *   - используется кадровый буфер 1024 байта;
 *   - произвольные пиксели хранятся в постраничном формате ST7567;
 *   - линии строятся алгоритмом Брезенхэма;
 *   - куб преобразуется из 3D в 2D ортографической проекцией;
 *   - куб вращается вокруг X, Y и Z без перемещения центра и изменения размера;
 *   - вращающийся куб гарантированно помещается в экран;
 *   - в левом верхнем углу отображаются координаты центра куба;
 *   - в правом нижнем углу отображается триада X/Y/Z, которая использует
 *     тот же поворот, что и сам куб, поэтому направления осей меняются вместе
 *     с ориентацией куба.
 *
 * ВАЖНО:
 *   Animation_RotatingCube() является единственной сценой и выполняется
 *   бесконечно. После её запуска main() больше не возвращается к другим
 *   действиям.
 *
 *\ author + idea        ScuratovaAnna 
 *\ сode debugging       PivnevNikolay
 */

#include "NuMicro.h"
#include <stdint.h>

/* Макросы битовых операций, используемые в исходных файлах проекта. */
#define SET_BIT(REG, BIT)          ((REG) |= (BIT))
#define CLEAR_BIT(REG, BIT)        ((REG) &= ~(BIT))
#define READ_BIT(REG, BIT)         ((REG) & (BIT))
#define CLEAR_REG(REG)             ((REG) = (0x0))
#define WRITE_REG(REG, VAL)        ((REG) = (VAL))
#define READ_REG(REG)              (REG)
#define MODIFY_REG(REG, CLR, SET)  WRITE_REG((REG), ((READ_REG(REG) & ~(CLR)) | (SET)))

/* -------------------------------------------------------------------------- */
/* Геометрия дисплея и внутренние параметры                                   */
/* -------------------------------------------------------------------------- */

#define LCD_WIDTH                  128U
#define LCD_HEIGHT                 64U
#define LCD_PAGES                  8U
#define LCD_FB_SIZE                (LCD_WIDTH * LCD_PAGES)

/*
 * Координатная система экрана:
 *   X = 0 находится слева, X = 127 справа;
 *   Y = 0 находится сверху, Y = 63 снизу.
 *
 * Центр выбран в той же точке, что и в предыдущем проекте.
 */
#define CUBE_CENTER_X              63
#define CUBE_CENTER_Y              31

/*
 * Половина ребра куба в условных координатных единицах.
 *
 * Ортографическая проекция не меняет масштаб в зависимости от глубины.
 * Для куба с вершинами (+/-H, +/-H, +/-H) максимальное удаление любой
 * проекции вершины от центра экрана не превышает sqrt(3) * H.
 * При H=11 это около 19.1 пикселя.
 *
 * Поэтому весь куб находится в прямоугольнике примерно:
 *   X: 44..82
 *   Y: 12..50
 *
 * Это оставляет свободную верхнюю область под координаты и правую нижнюю
 * область под индикатор трёх осей. Куб не выходит за границы дисплея
 * ни при каком угле поворота.
 */
#define CUBE_HALF_SIZE             11L

/* Формат синуса/косинуса: Q15, единица приблизительно равна 32767. */
#define Q15_ONE                    32767L

/* Положение индикатора локальных осей в правом нижнем углу. */
#define AXIS_ORIGIN_X              105
#define AXIS_ORIGIN_Y              40
#define AXIS_LENGTH                15L

/* Угол хранится в четвертях градуса: 4 единицы = 1 градус; 1440 = 360°. */
#define ANGLE_FULL_TURN            1440U

/* Кадровый буфер ST7567: 8 страниц по 128 байт. */
static uint8_t g_lcdBuffer[LCD_FB_SIZE];

/* Системный счётчик миллисекунд увеличивается из SysTick_Handler(). */
static volatile uint32_t g_msTicks = 0UL;

/* -------------------------------------------------------------------------- */
/* Прототипы базовых функций                                                  */
/* -------------------------------------------------------------------------- */

static void SYS_Init(void);
static void LCD_GPIO_Init(void);
static void SPI0_Init(void);
static void SPI_SendByte(uint8_t data);
static void LCD_WriteCmd(uint8_t cmd);
static void LCD_WriteData(uint8_t data);
static void LCD_Init(void);
static void SysTick_Init(void);
static void delay_ms(uint32_t ms);

static void LCD_ClearBuffer(void);
static void LCD_DrawPixel(int32_t x, int32_t y, uint8_t color);
static void LCD_DrawLine(int32_t x0, int32_t y0,
                         int32_t x1, int32_t y1, uint8_t color);
static void LCD_Update(void);

static void LCD_DrawChar(int32_t x, int32_t y, char ch);
static void LCD_DrawStringXY(int32_t x, int32_t y, const char *text);
static void LCD_DrawLinePattern(int32_t x0, int32_t y0,
                                int32_t x1, int32_t y1,
                                uint8_t pattern);
static void LCD_DrawAxisLine(int32_t x0, int32_t y0,
                             int32_t x1, int32_t y1,
                             uint8_t pattern);

static int32_t SinQ15(uint16_t angleQuarterDeg);
static int32_t CosQ15(uint16_t angleQuarterDeg);
static void Cube_RotatePoint(int32_t x, int32_t y, int32_t z,
                             int32_t sinX, int32_t cosX,
                             int32_t sinY, int32_t cosY,
                             int32_t sinZ, int32_t cosZ,
                             int32_t *outX, int32_t *outY, int32_t *outZ);
static void Animation_RotatingCube(void);

/* -------------------------------------------------------------------------- */
/* Тактирование и GPIO                                                        */
/* -------------------------------------------------------------------------- */

static void SYS_Init(void)
{
    SYS_UnlockReg();

    /* Включаем внутренний высокочастотный генератор HIRC (48 MHz). */
    SET_BIT(CLK->PWRCTL, CLK_PWRCTL_HIRCEN_Msk);

    /* Ждём, пока генератор HIRC выйдет на стабильный режим. */
    while (READ_BIT(CLK->STATUS, CLK_STATUS_HIRCSTB_Msk) !=
           CLK_STATUS_HIRCSTB_Msk)
    {
    }

    /* HCLK берёт частоту непосредственно от HIRC, делитель равен 1. */
    MODIFY_REG(CLK->CLKSEL0,
               CLK_CLKSEL0_HCLKSEL_Msk,
               CLK_CLKSEL0_HCLKSEL_HIRC);

    MODIFY_REG(CLK->CLKDIV0,
               CLK_CLKDIV0_HCLKDIV_Msk,
               CLK_CLKDIV0_HCLK(1));

    /* Источник SPI0 — PCLK1, как в проверенном рабочем SPI_main.txt. */
    MODIFY_REG(CLK->CLKSEL2,
               CLK_CLKSEL2_SPI0SEL_Msk,
               CLK_CLKSEL2_SPI0SEL_PCLK1);

    /*
     * Включаем только бит тактирования SPI0.
     * Не присваиваем всё поле APBCLK0 целиком, чтобы не отключить
     * случайно тактирование других периферийных блоков.
     */
    SET_BIT(CLK->APBCLK0, CLK_APBCLK0_SPI0CKEN_Msk);

    SystemCoreClockUpdate();

    /*
     * PA0 = MOSI и PA2 = CLK. Номер функции MFP=4 сохранён из твоего
     * рабочего проекта, где SPI0 уже управлял дисплеем.
     */
    MODIFY_REG(SYS->GPA_MFPL,
               SYS_GPA_MFPL_PA0MFP_Msk | SYS_GPA_MFPL_PA2MFP_Msk,
               (4UL << SYS_GPA_MFPL_PA0MFP_Pos) |
               (4UL << SYS_GPA_MFPL_PA2MFP_Pos));

    SYS_LockReg();
}

static void LCD_GPIO_Init(void)
{
    /* PB0, PB1 и PB2 переводим в режим GPIO-выходов. */
    MODIFY_REG(PB->MODE,
               GPIO_MODE_MODE0_Msk | GPIO_MODE_MODE1_Msk | GPIO_MODE_MODE2_Msk,
               (1UL << GPIO_MODE_MODE0_Pos) |
               (1UL << GPIO_MODE_MODE1_Pos) |
               (1UL << GPIO_MODE_MODE2_Pos));

    /* Начальные уровни: RESET=1, DC=1, CS=1. */
    SET_BIT(PB->DOUT,
            GPIO_DOUT_DOUT0_Msk |
            GPIO_DOUT_DOUT1_Msk |
            GPIO_DOUT_DOUT2_Msk);
}

/* -------------------------------------------------------------------------- */
/* SPI0                                                                       */
/* -------------------------------------------------------------------------- */

static void SPI0_Init(void)
{
    /* Сбрасываем SPI0, чтобы настройки начинались с известного состояния. */
    SET_BIT(SYS->IPRST1, SYS_IPRST1_SPI0RST_Msk);
    CLEAR_BIT(SYS->IPRST1, SYS_IPRST1_SPI0RST_Msk);

    /*
     * SPI master, 8 бит на слово, TXNEG и включённый SPI.
     * Эта конфигурация соответствует исходному рабочему SPI_main.txt.
     */
    WRITE_REG(SPI0->CTL,
              (8UL << SPI_CTL_DWIDTH_Pos) |
              SPI_CTL_TXNEG_Msk |
              SPI_CTL_SPIEN_Msk);

    /* SCLK = f_SPI0 / (2 * (DIVIDER + 1)).
       При источнике SPI0 = 48 MHz и DIVIDER = 3 получаем 6 MHz. */
    WRITE_REG(SPI0->CLKDIV, 3UL << SPI_CLKDIV_DIVIDER_Pos);
}

static void SPI_SendByte(uint8_t data)
{
    /* Не пишем новые данные, пока передающий FIFO заполнен. */
    while (READ_BIT(SPI0->STATUS, SPI_STATUS_TXFULL_Msk) != 0U)
    {
    }

    /* Передаём один байт. */
    WRITE_REG(SPI0->TX, (uint32_t)data);

    /* Дожидаемся, пока SPI закончит передачу текущего байта. */
    while (READ_BIT(SPI0->STATUS, SPI_STATUS_BUSY_Msk) != 0U)
    {
    }
}

/* -------------------------------------------------------------------------- */
/* Команды и данные дисплея ST7567                                            */
/* -------------------------------------------------------------------------- */

static void LCD_WriteCmd(uint8_t cmd)
{
    /* CS активен низким уровнем, DC=0 означает передачу команды. */
    CLEAR_BIT(PB->DOUT, GPIO_DOUT_DOUT2_Msk);
    CLEAR_BIT(PB->DOUT, GPIO_DOUT_DOUT1_Msk);

    SPI_SendByte(cmd);

    /* Завершаем транзакцию дисплея. */
    SET_BIT(PB->DOUT, GPIO_DOUT_DOUT2_Msk);
}

static void LCD_WriteData(uint8_t data)
{
    /* CS активен низким уровнем, DC=1 означает передачу данных дисплея. */
    CLEAR_BIT(PB->DOUT, GPIO_DOUT_DOUT2_Msk);
    SET_BIT(PB->DOUT, GPIO_DOUT_DOUT1_Msk);

    SPI_SendByte(data);

    /* Завершаем транзакцию дисплея. */
    SET_BIT(PB->DOUT, GPIO_DOUT_DOUT2_Msk);
}

static void LCD_Init(void)
{
    /* Аппаратный сброс ST7567 через PB0. */
    CLEAR_BIT(PB->DOUT, GPIO_DOUT_DOUT0_Msk);
    delay_ms(50UL);
    SET_BIT(PB->DOUT, GPIO_DOUT_DOUT0_Msk);
    delay_ms(50UL);

    /* Последовательность команд сохранена из рабочего SPI_main.txt. */
    LCD_WriteCmd(0xE2U); /* Software reset. */
    delay_ms(10UL);

    LCD_WriteCmd(0xA2U); /* LCD bias = 1/9. */
    LCD_WriteCmd(0xA0U); /* Направление SEG — normal. */
    LCD_WriteCmd(0xC8U); /* Направление COM — reverse. */
    LCD_WriteCmd(0x2FU); /* Booster, regulator, follower ON. */
    LCD_WriteCmd(0x24U); /* Настройка V0. */
    LCD_WriteCmd(0x81U); /* Электронная регулировка контрастности. */
    LCD_WriteCmd(0x20U); /* Значение контрастности. */
    LCD_WriteCmd(0x40U); /* Начальная строка дисплея = 0. */
    LCD_WriteCmd(0xAFU); /* Display ON. */
}

/* -------------------------------------------------------------------------- */
/* SysTick: монотонный счётчик миллисекунд                                    */
/* -------------------------------------------------------------------------- */

void SysTick_Handler(void)
{
    /* Вызывается аппаратно один раз в миллисекунду. */
    g_msTicks++;
}

static void SysTick_Init(void)
{
    /* Сбрасываем программный счётчик перед включением SysTick. */
    g_msTicks = 0UL;

    /* Один прерывающий тик на 1 ms при SystemCoreClock. */
    SysTick->LOAD = (uint32_t)((SystemCoreClock / 1000UL) - 1UL);
    SysTick->VAL = 0UL;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk |
                    SysTick_CTRL_ENABLE_Msk;
}

static void delay_ms(uint32_t ms)
{
    uint32_t start = g_msTicks;

    /* Вычитание uint32_t безопасно и при переполнении счётчика. */
    while ((uint32_t)(g_msTicks - start) < ms)
    {
    }
}

/* -------------------------------------------------------------------------- */
/* Кадровый буфер                                                             */
/* -------------------------------------------------------------------------- */

static void LCD_ClearBuffer(void)
{
    uint16_t i;

    /*
     * Очищаем все 1024 байта RAM-буфера.
     * В формате ST7567 каждый байт соответствует вертикальной группе
     * из восьми пикселей одной колонки.
     */
    for (i = 0U; i < LCD_FB_SIZE; i++)
    {
        g_lcdBuffer[i] = 0x00U;
    }
}

static void LCD_DrawPixel(int32_t x, int32_t y, uint8_t color)
{
    uint16_t index;
    uint8_t mask;

    /*
     * Обрезка по границам дисплея.
     * Благодаря этой проверке любая линия может выходить за диапазон,
     * но фактически в буфере будут изменяться только видимые пиксели.
     */
    if ((x < 0) || (x >= (int32_t)LCD_WIDTH) ||
        (y < 0) || (y >= (int32_t)LCD_HEIGHT))
    {
        return;
    }

    /*
     * Страница — y/8; номер вертикального бита — y%8.
     * Адрес байта: page*128 + x.
     */
    index = (uint16_t)(((uint16_t)y >> 3) * LCD_WIDTH + (uint16_t)x);
    mask = (uint8_t)(1U << ((uint16_t)y & 7U));

    if (color != 0U)
    {
        /* color=1: включить пиксель. */
        g_lcdBuffer[index] |= mask;
    }
    else
    {
        /* color=0: выключить пиксель. */
        g_lcdBuffer[index] &= (uint8_t)(~mask);
    }
}

static void LCD_DrawLine(int32_t x0, int32_t y0,
                         int32_t x1, int32_t y1, uint8_t color)
{
    /*
     * Алгоритм Брезенхэма.
     * Он строит прямую линию целочисленными операциями и не требует
     * float/double, что удобно для небольшого микроконтроллера.
     */
    int32_t dx = (x1 >= x0) ? (x1 - x0) : (x0 - x1);
    int32_t sx = (x0 < x1) ? 1 : -1;
    int32_t dy = -((y1 >= y0) ? (y1 - y0) : (y0 - y1));
    int32_t sy = (y0 < y1) ? 1 : -1;
    int32_t err = dx + dy;
    int32_t e2;

    for (;;)
    {
        /* LCD_DrawPixel отсекает точки вне видимой области. */
        LCD_DrawPixel(x0, y0, color);

        /* Последняя точка линии уже обработана — выходим. */
        if ((x0 == x1) && (y0 == y1))
        {
            break;
        }

        e2 = 2 * err;

        if (e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }

        if (e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

static void LCD_Update(void)
{
    uint8_t page;
    uint8_t col;

    /*
     * Передаём весь кадровый буфер на дисплей.
     * Каждая страница содержит 128 байт, всего страниц 8.
     */
    for (page = 0U; page < LCD_PAGES; page++)
    {
        /* Выбор страницы ST7567. */
        LCD_WriteCmd((uint8_t)(0xB0U | page));

        /* Устанавливаем адрес колонки 0. */
        LCD_WriteCmd(0x10U); /* Старшие 4 бита. */
        LCD_WriteCmd(0x00U); /* Младшие 4 бита. */

        for (col = 0U; col < LCD_WIDTH; col++)
        {
            LCD_WriteData(g_lcdBuffer[((uint16_t)page * LCD_WIDTH) + col]);
        }
    }
}

/* -------------------------------------------------------------------------- */
/* Растровый шрифт 5x7                                                       */
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


/*
 * Рисует один символ в произвольной точке кадрового буфера.
 *
 * В отличие от записи целой страницы байтами, этот вариант использует
 * LCD_DrawPixel() для каждого установленного бита. Поэтому текст можно
 * размещать поверх графики, не очищая соседние пиксели.
 */
static void LCD_DrawChar(int32_t x, int32_t y, char ch)
{
    const uint8_t *glyph = Font5x7(ch);
    uint8_t column;
    uint8_t row;

    for (column = 0U; column < 5U; column++)
    {
        for (row = 0U; row < 7U; row++)
        {
            if ((glyph[column] & (uint8_t)(1U << row)) != 0U)
            {
                LCD_DrawPixel(x + column, y + row, 1U);
            }
        }
    }
}

/* Рисует строку слева направо; ширина одного символа с интервалом — 6 px. */
static void LCD_DrawStringXY(int32_t x, int32_t y, const char *text)
{
    while (*text != '\0')
    {
        LCD_DrawChar(x, y, *text);
        x += 6;
        text++;
    }
}

/* -------------------------------------------------------------------------- */
/* Тригонометрия Q15                                                          */
/* -------------------------------------------------------------------------- */

/*
 * Таблица синуса от 0 до 90 градусов включительно, с шагом 5 градусов.
 * Значения хранятся в Q15: sin(angle)*32767.
 */
static const uint16_t g_sinQuarterQ15[19] =
{
    0U, 2856U, 5690U, 8481U, 11207U,
    13848U, 16383U, 18794U, 21062U, 23170U,
    25101U, 26841U, 28377U, 29697U, 30791U,
    31650U, 32269U, 32642U, 32767U
};

static int32_t SinQuarterQ15(uint16_t angleQuarterDeg)
{
    /*
     * Табличный шаг 5 градусов соответствует 20 четвертям градуса.
     * index выбирает соседние записи, remainder — положение между ними.
     */
    uint16_t index = (uint16_t)(angleQuarterDeg / 20U);
    uint16_t remainder = (uint16_t)(angleQuarterDeg % 20U);
    int32_t y0;
    int32_t y1;

    if (index >= 18U)
    {
        return Q15_ONE;
    }

    y0 = (int32_t)g_sinQuarterQ15[index];
    y1 = (int32_t)g_sinQuarterQ15[index + 1U];

    /* Линейная интерполяция внутри интервала таблицы. */
    return y0 + (((y1 - y0) * (int32_t)remainder) / 20L);
}

static int32_t SinQ15(uint16_t angleQuarterDeg)
{
    /* Сводим любой угол к диапазону 0..359.75 градусов. */
    angleQuarterDeg = (uint16_t)(angleQuarterDeg % ANGLE_FULL_TURN);

    /* Первый квадрант: 0..90°, синус положительный и растёт. */
    if (angleQuarterDeg <= 360U)
    {
        return SinQuarterQ15(angleQuarterDeg);
    }

    /* Второй квадрант: отражаем угол относительно 180°. */
    if (angleQuarterDeg <= 720U)
    {
        return SinQuarterQ15((uint16_t)(720U - angleQuarterDeg));
    }

    /* Третий квадрант: синус отрицательный. */
    if (angleQuarterDeg <= 1080U)
    {
        return -SinQuarterQ15((uint16_t)(angleQuarterDeg - 720U));
    }

    /* Четвёртый квадрант: отрицательное зеркальное значение. */
    return -SinQuarterQ15((uint16_t)(1440U - angleQuarterDeg));
}

static int32_t CosQ15(uint16_t angleQuarterDeg)
{
    /* cos(a) = sin(a + 90°); 90° = 360 четвертей градуса. */
    return SinQ15((uint16_t)(angleQuarterDeg + 360U));
}

/* -------------------------------------------------------------------------- */
/* Линии индикатора осей: сплошная, штриховая и точечная                     */
/* -------------------------------------------------------------------------- */

static void LCD_DrawLinePattern(int32_t x0, int32_t y0,
                                int32_t x1, int32_t y1,
                                uint8_t pattern)
{
    int32_t dx = (x1 >= x0) ? (x1 - x0) : (x0 - x1);
    int32_t sx = (x0 < x1) ? 1 : -1;
    int32_t dy = -((y1 >= y0) ? (y1 - y0) : (y0 - y1));
    int32_t sy = (y0 < y1) ? 1 : -1;
    int32_t err = dx + dy;
    int32_t e2;
    uint16_t step = 0U;
    uint8_t draw;

    for (;;)
    {
        /*
         * pattern=0: сплошная линия (каждая точка);
         * pattern=1: штриховая (3 пикселя рисуем, 2 пропускаем);
         * pattern=2: точечная (рисуем каждый второй пиксель).
         */
        if (pattern == 0U)
        {
            draw = 1U;
        }
        else if (pattern == 1U)
        {
            draw = ((step % 5U) < 3U) ? 1U : 0U;
        }
        else
        {
            draw = ((step & 1U) == 0U) ? 1U : 0U;
        }

        if (draw != 0U)
        {
            LCD_DrawPixel(x0, y0, 1U);
        }

        if ((x0 == x1) && (y0 == y1))
        {
            break;
        }

        e2 = 2 * err;
        if (e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }

        step++;
    }
}

/*
 * Рисует одну линию индикатора локальной оси X, Y или Z.
 *
 * Наконечники намеренно отсутствуют: на небольшом дисплее 128x64
 * они занимают много места и при близком расположении осей могут
 * сливаться друг с другом.
 *
 * Направление оси определяется:
 *   1) ориентацией самой линии;
 *   2) буквой X, Y или Z, которая рисуется отдельно около её конца.
 *
 * pattern задаёт стиль линии:
 *   0 — сплошная;
 *   1 — штриховая;
 *   2 — точечная.
 *
 * Координаты x0/y0 — общее начало триады.
 * Координаты x1/y1 — конец соответствующей оси после поворота.
 */
static void LCD_DrawAxisLine(int32_t x0, int32_t y0,
                             int32_t x1, int32_t y1,
                             uint8_t pattern)
{
    /* Никакой дополнительной геометрии не строим — только саму ось. */
    LCD_DrawLinePattern(x0, y0, x1, y1, pattern);
}

/* -------------------------------------------------------------------------- */
/* Общий поворот 3D-точки вокруг X, затем Y, затем Z                          */
/* -------------------------------------------------------------------------- */

static void Cube_RotatePoint(int32_t x, int32_t y, int32_t z,
                             int32_t sinX, int32_t cosX,
                             int32_t sinY, int32_t cosY,
                             int32_t sinZ, int32_t cosZ,
                             int32_t *outX, int32_t *outY, int32_t *outZ)
{
    int32_t y1;
    int32_t z1;
    int32_t x2;
    int32_t z2;
    int32_t x3;
    int32_t y3;

    /*
     * Поворот вокруг X.
     *
     * x1 = x
     * y1 = y*cos(X) - z*sin(X)
     * z1 = y*sin(X) + z*cos(X)
     *
     * sin/cos представлены в Q15, поэтому делим произведения на 32767.
     */
    y1 = (y * cosX - z * sinX) / Q15_ONE;
    z1 = (y * sinX + z * cosX) / Q15_ONE;

    /*
     * Поворот вокруг Y.
     *
     * x2 = x*cos(Y) + z1*sin(Y)
     * y2 = y1
     * z2 = -x*sin(Y) + z1*cos(Y)
     */
    x2 = (x * cosY + z1 * sinY) / Q15_ONE;
    z2 = (-x * sinY + z1 * cosY) / Q15_ONE;

    /*
     * Поворот вокруг Z.
     *
     * x3 = x2*cos(Z) - y1*sin(Z)
     * y3 = x2*sin(Z) + y1*cos(Z)
     * z3 = z2
     */
    x3 = (x2 * cosZ - y1 * sinZ) / Q15_ONE;
    y3 = (x2 * sinZ + y1 * cosZ) / Q15_ONE;

    *outX = x3;
    *outY = y3;
    *outZ = z2;
}

/* -------------------------------------------------------------------------- */
/* Единственная сцена проекта: неподвижный каркасный куб с вращением          */
/* -------------------------------------------------------------------------- */

static void Animation_RotatingCube(void)
{
    /*
     * Вершины куба в локальной системе координат.
     * Каждая координата равна -CUBE_HALF_SIZE или +CUBE_HALF_SIZE.
     * Центр куба находится в локальной точке (0,0,0).
     */
    static const int8_t vertices[8][3] =
    {
        { -CUBE_HALF_SIZE, -CUBE_HALF_SIZE, -CUBE_HALF_SIZE }, /* 0 — задняя/нижняя/левая. */
        {  CUBE_HALF_SIZE, -CUBE_HALF_SIZE, -CUBE_HALF_SIZE }, /* 1 — задняя/нижняя/правая. */
        {  CUBE_HALF_SIZE,  CUBE_HALF_SIZE, -CUBE_HALF_SIZE }, /* 2 — задняя/верхняя/правая. */
        { -CUBE_HALF_SIZE,  CUBE_HALF_SIZE, -CUBE_HALF_SIZE }, /* 3 — задняя/верхняя/левая. */
        { -CUBE_HALF_SIZE, -CUBE_HALF_SIZE,  CUBE_HALF_SIZE }, /* 4 — передняя/нижняя/левая. */
        {  CUBE_HALF_SIZE, -CUBE_HALF_SIZE,  CUBE_HALF_SIZE }, /* 5 — передняя/нижняя/правая. */
        {  CUBE_HALF_SIZE,  CUBE_HALF_SIZE,  CUBE_HALF_SIZE }, /* 6 — передняя/верхняя/правая. */
        { -CUBE_HALF_SIZE,  CUBE_HALF_SIZE,  CUBE_HALF_SIZE }  /* 7 — передняя/верхняя/левая. */
    };

    /*
     * Каждое ребро — пара индексов вершин.
     * Первые четыре образуют грань z=-11, следующие четыре — грань z=+11,
     * последние четыре соединяют соответствующие вершины двух граней.
     */
    static const uint8_t edges[12][2] =
    {
        { 0U, 1U }, { 1U, 2U }, { 2U, 3U }, { 3U, 0U },
        { 4U, 5U }, { 5U, 6U }, { 6U, 7U }, { 7U, 4U },
        { 0U, 4U }, { 1U, 5U }, { 2U, 6U }, { 3U, 7U }
    };

    /* Экранные координаты восьми вершин, вычисляемые заново для каждого кадра. */
    int32_t screenX[8];
    int32_t screenY[8];

    /*
     * Углы вращения вокруг трёх локальных осей.
     * Формат такой же, как у SinQ15/CosQ15: 4 единицы = 1 градус.
     */
    uint16_t angleX = 0U;
    uint16_t angleY = 0U;
    uint16_t angleZ = 0U;

    /* Предварительно вычисленные значения синуса и косинуса на текущем кадре. */
    int32_t sinX;
    int32_t cosX;
    int32_t sinY;
    int32_t cosY;
    int32_t sinZ;
    int32_t cosZ;

    /* Результирующие координаты преобразованной 3D-точки. */
    int32_t rx;
    int32_t ry;
    int32_t rz;

    /* Координаты конца трёх осей в окошке индикатора справа снизу. */
    int32_t axisEndX[3];
    int32_t axisEndY[3];

    /* Временные переменные для индексов массивов. */
    uint8_t vertex;
    uint8_t edge;

    /* Временные координаты для положения букв X, Y и Z возле концов линий. */
    int32_t labelX;
    int32_t labelY;

    /*
     * Центр и размер куба неизменны на всём протяжении работы:
     * нет перемещения, масштабирования, пульсации или сжатия.
     */
    const int32_t centerX = CUBE_CENTER_X;
    const int32_t centerY = CUBE_CENTER_Y;

    /* Один раз стираем дисплей перед первым кадром. */
    LCD_ClearBuffer();
    LCD_Update();
    delay_ms(200UL);

    /*
     * Бесконечный цикл анимации.
     *
     * Каждый проход — один полный кадр:
     *   1. вычислить тригонометрию текущего поворота;
     *   2. очистить предыдущий кадр в RAM;
     *   3. повернуть все 8 вершин куба;
     *   4. спроецировать их на экран без перспективного изменения размера;
     *   5. соединить вершины 12 рёбрами;
     *   6. повернуть и нарисовать индикатор осей теми же углами;
     *   7. вывести координаты неподвижного центра;
     *   8. отправить готовый кадр на дисплей.
     *
     * Никакой финальной заливки, тайм-аута, движения куба или изменения размера
     * нет. Старый рисунок очищается в кадровом буфере перед построением нового,
     * поэтому следы предыдущих положений не накапливаются.
     */
    while (1)
    {
        /* -------------------------------------------------------------- */
        /* 1. Тригонометрия текущего кадра                                */
        /* -------------------------------------------------------------- */

        sinX = SinQ15(angleX);
        cosX = CosQ15(angleX);
        sinY = SinQ15(angleY);
        cosY = CosQ15(angleY);
        sinZ = SinQ15(angleZ);
        cosZ = CosQ15(angleZ);

        /* -------------------------------------------------------------- */
        /* 2. Удаляем предыдущий кадр из буфера                            */
        /* -------------------------------------------------------------- */

        LCD_ClearBuffer();

        /* -------------------------------------------------------------- */
        /* 3. Поворачиваем все 8 вершин куба и выполняем проекцию          */
        /* -------------------------------------------------------------- */

        for (vertex = 0U; vertex < 8U; vertex++)
        {
            /*
             * Исходная точка локального куба. Координаты уже равны +/-11,
             * то есть размер куба в каждом измерении постоянен.
             */
            Cube_RotatePoint((int32_t)vertices[vertex][0],
                             (int32_t)vertices[vertex][1],
                             (int32_t)vertices[vertex][2],
                             sinX, cosX,
                             sinY, cosY,
                             sinZ, cosZ,
                             &rx, &ry, &rz);

            /*
             * Ортографическая проекция.
             *
             * X экрана = центрX + X после поворотов.
             * Y экрана = центрY - Y после поворотов.
             *
             * Минус перед ry нужен потому, что ось Y дисплея направлена вниз.
             * Координата rz не участвует в размере проекции — это принципиально:
             * у ортографической проекции нет перспективного увеличения/уменьшения.
             */
            screenX[vertex] = centerX + rx;
            screenY[vertex] = centerY - ry;

            /*
             * rz намеренно вычисляется, даже если не нужен экранной проекции:
             * та же функция преобразования используется ниже для направлений
             * локальных осей X/Y/Z, чтобы триада отражала ориентацию куба.
             */
            (void)rz;
        }

        /* -------------------------------------------------------------- */
        /* 4. Соединяем вершины — получаем каркас куба                     */
        /* -------------------------------------------------------------- */

        for (edge = 0U; edge < 12U; edge++)
        {
            uint8_t first = edges[edge][0];
            uint8_t second = edges[edge][1];

            LCD_DrawLine(screenX[first], screenY[first],
                         screenX[second], screenY[second], 1U);
        }

        /*
         * Небольшой крестик обозначает центр вращения.
         * Он рисуется после рёбер, чтобы оставаться видимым поверх каркаса.
         */
        LCD_DrawLine(centerX - 2, centerY, centerX + 2, centerY, 1U);
        LCD_DrawLine(centerX, centerY - 2, centerX, centerY + 2, 1U);

        /* -------------------------------------------------------------- */
        /* 5. Поворачиваем три базисных вектора X, Y и Z                   */
        /* -------------------------------------------------------------- */

        /*
         * Базисные векторы задаются по отдельности:
         *   локальный +X = (AXIS_LENGTH, 0, 0);
         *   локальный +Y = (0, AXIS_LENGTH, 0);
         *   локальный +Z = (0, 0, AXIS_LENGTH).
         *
         * Каждый вектор пропускается через ТОЧНО ТУ ЖЕ последовательность
         * поворотов X -> Y -> Z, что и вершины куба.
         * В результате индикатор в углу показывает текущую ориентацию
         * локальных осей вращающегося куба, а не неподвижные экранные оси.
         */

        /* Ось X: (9,0,0). */
        Cube_RotatePoint(AXIS_LENGTH, 0L, 0L,
                         sinX, cosX, sinY, cosY, sinZ, cosZ,
                         &rx, &ry, &rz);
        axisEndX[0] = AXIS_ORIGIN_X + rx;
        axisEndY[0] = AXIS_ORIGIN_Y - ry;

        /* Ось Y: (0,9,0). */
        Cube_RotatePoint(0L, AXIS_LENGTH, 0L,
                         sinX, cosX, sinY, cosY, sinZ, cosZ,
                         &rx, &ry, &rz);
        axisEndX[1] = AXIS_ORIGIN_X + rx;
        axisEndY[1] = AXIS_ORIGIN_Y - ry;

        /* Ось Z: (0,0,9). */
        Cube_RotatePoint(0L, 0L, AXIS_LENGTH,
                         sinX, cosX, sinY, cosY, sinZ, cosZ,
                         &rx, &ry, &rz);
        axisEndX[2] = AXIS_ORIGIN_X + rx;
        axisEndY[2] = AXIS_ORIGIN_Y - ry;

        /* -------------------------------------------------------------- */
        /* 6. Рисуем триаду в правом нижнем углу                          */
        /* -------------------------------------------------------------- */

        /*
         * Все три линии начинаются в одной точке — начале локальных осей.
         * В отличие от куба, триада является маленьким информационным
         * индикатором и имеет собственный масштаб и постоянное положение.
         * Меняются только направления концов осей.
         *
         * Разные стили помогают различать оси на чёрно-белом экране:
         *   X — сплошная;
         *   Y — штриховая;
         *   Z — точечная.
         */
        LCD_DrawAxisLine(AXIS_ORIGIN_X, AXIS_ORIGIN_Y,
                          axisEndX[0], axisEndY[0], 0U);
        LCD_DrawAxisLine(AXIS_ORIGIN_X, AXIS_ORIGIN_Y,
                          axisEndX[1], axisEndY[1], 1U);
        LCD_DrawAxisLine(AXIS_ORIGIN_X, AXIS_ORIGIN_Y,
                          axisEndX[2], axisEndY[2], 2U);

        /* Отмечаем общий ноль координат триады небольшим крестом. */
        LCD_DrawLine(AXIS_ORIGIN_X - 1, AXIS_ORIGIN_Y,
                     AXIS_ORIGIN_X + 1, AXIS_ORIGIN_Y, 1U);
        LCD_DrawLine(AXIS_ORIGIN_X, AXIS_ORIGIN_Y - 1,
                     AXIS_ORIGIN_X, AXIS_ORIGIN_Y + 1, 1U);

        /*
         * Буквы рисуются около концов соответствующих линий.
         * Их позиции привязаны к концам повёрнутых осей, поэтому буквы тоже
         * перемещаются вслед за направлением осей.
         */

        /* Метка X: немного правее и выше конца линии. */
        labelX = axisEndX[0] + 1;
        labelY = axisEndY[0] - 3;
        if (labelX > ((int32_t)LCD_WIDTH - 5)) labelX = (int32_t)LCD_WIDTH - 5;
        if (labelX < 0) labelX = 0;
        if (labelY > ((int32_t)LCD_HEIGHT - 7)) labelY = (int32_t)LCD_HEIGHT - 7;
        if (labelY < 0) labelY = 0;
        LCD_DrawChar(labelX, labelY, 'X');

        /* Метка Y: немного левее и выше конца линии. */
        labelX = axisEndX[1] - 5;
        labelY = axisEndY[1] - 6;
        if (labelX > ((int32_t)LCD_WIDTH - 5)) labelX = (int32_t)LCD_WIDTH - 5;
        if (labelX < 0) labelX = 0;
        if (labelY > ((int32_t)LCD_HEIGHT - 7)) labelY = (int32_t)LCD_HEIGHT - 7;
        if (labelY < 0) labelY = 0;
        LCD_DrawChar(labelX, labelY, 'Y');

        /* Метка Z: немного правее и ниже конца линии. */
        labelX = axisEndX[2] + 1;
        labelY = axisEndY[2];
        if (labelX > ((int32_t)LCD_WIDTH - 5)) labelX = (int32_t)LCD_WIDTH - 5;
        if (labelX < 0) labelX = 0;
        if (labelY > ((int32_t)LCD_HEIGHT - 7)) labelY = (int32_t)LCD_HEIGHT - 7;
        if (labelY < 0) labelY = 0;
        LCD_DrawChar(labelX, labelY, 'Z');

        /* -------------------------------------------------------------- */
        /* 7. Выводим постоянные координаты центра в левом верхнем углу   */
        /* -------------------------------------------------------------- */

        /*
         * Центр куба не перемещается, поэтому координаты всегда одни:
         * X=063, Y=031.
         * Текст рисуется после куба, чтобы оставаться читаемым.
         */
        LCD_DrawStringXY(1, 1, "CX:063 CY:031");

        /* -------------------------------------------------------------- */
        /* 8. Передаём готовый кадр на ST7567                             */
        /* -------------------------------------------------------------- */

        LCD_Update();

        /* -------------------------------------------------------------- */
        /* 9. Увеличиваем углы перед следующим кадром                     */
        /* -------------------------------------------------------------- */

        /*
         * Угол измеряется в четвертях градуса:
         *   +4 = +1 градус;
         *   +2 = +0,5 градуса.
         *
         * Разные скорости вокруг трёх осей дают сложное непрерывное вращение.
         * Операция modulo удерживает углы в диапазоне 0..1439 и не допускает
         * переполнения переменных.
         */
        angleX = (uint16_t)((angleX + 5U) % ANGLE_FULL_TURN); /* +1,25°/кадр */
        angleY = (uint16_t)((angleY + 3U) % ANGLE_FULL_TURN); /* +0,75°/кадр */
        angleZ = (uint16_t)((angleZ + 2U) % ANGLE_FULL_TURN); /* +0,50°/кадр */

        /*
         * Небольшая пауза задаёт темп анимации.
         * Реальная частота кадров также зависит от времени LCD_Update(),
         * поскольку каждый байт передаётся через SPI.
         */
        delay_ms(25UL);
    }
}

/* -------------------------------------------------------------------------- */
/* main — инициализация оборудования и запуск единственной анимации          */
/* -------------------------------------------------------------------------- */

int main(void)
{
    // Настройка источника тактирования и выводов SPI.
    SYS_Init();

    // SysTick должен работать до delay_ms(), в том числе при LCD_Init().
    SysTick_Init();

    // Инициализируем управляющие GPIO дисплея и SPI0.
    LCD_GPIO_Init();
    SPI0_Init();

    // Сбрасываем и конфигурируем ST7567.
    LCD_Init();

    // Чистый экран перед первым кадром.
    LCD_ClearBuffer();
    LCD_Update();

    // Единственная демонстрационная функция. Она работает бесконечно.
    Animation_RotatingCube();

    // Сюда выполнение не дойдёт, так как анимация бесконечная.
    while (1)
    {
    }
}
