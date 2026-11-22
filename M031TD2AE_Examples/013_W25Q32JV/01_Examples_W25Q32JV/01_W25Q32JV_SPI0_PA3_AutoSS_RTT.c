/**
 * @file W25Q32JV_SPI0_PA3_AutoSS_RTT.c
 *
 * M031TD2AE + W25Q32JV: аппаратный SPI0_SS на PA.3, вывод в SEGGER RTT.
 * Только ЧТЕНИЕ. Записи / стирания / изменения Status Register здесь НЕТ.
 *
 * Подключение W25Q32JV (SOIC-8):
 *     PA.0  SPI0_MOSI  -> DI / IO0    (вывод 5)
 *     PA.1  SPI0_MISO  <- DO / IO1    (вывод 2)
 *     PA.2  SPI0_CLK   -> CLK         (вывод 6)
 *     PA.3  SPI0_SS    -> /CS         (вывод 1)
 *     3.3V             -> VCC         (вывод 8)
 *     GND              -> GND         (вывод 4)
 *     /WP / IO2        -> 3.3V        (вывод 3)
 *     /HOLD / IO3      -> 3.3V        (вывод 7)
 *
 *
 * Нужны NuMicro.h, SEGGER_RTT.h и SEGGER_RTT.c в проекте.
 * SEGGER_RTT_printf.c не нужен: используется SEGGER_RTT_WriteString().
 *
 * Обоснование 32-битного обмена:
 *   SPI0->SSCTL = AUTOSS | SS (именно 0x09, как в рабочем тесте PA.3).
 *   SPI0->CTL.DWIDTH = 0 -> один кадр из 32 бит (см. spi_reg.h).
 *   MOSI: [9F] [FF] [FF] [FF].  MISO: [XX] [EF] [40] [16].
 *   Аппаратный /CS остаётся в нуле на протяжении всех 32 тактов.
 *
 *\ author + idea        ScuratovaAnna 
 *\ сode debugging       PivnevNikolay
 */

#include <stdint.h>
#include "NuMicro.h"
#include "SEGGER_RTT.h"

/* Макросы битовых операций, используемые в исходных файлах проекта. */
#define SET_BIT(REG, BIT)          ((REG) |= (BIT))
#define CLEAR_BIT(REG, BIT)        ((REG) &= ~(BIT))
#define READ_BIT(REG, BIT)         ((REG) & (BIT))
#define CLEAR_REG(REG)             ((REG) = (0x0))
#define WRITE_REG(REG, VAL)        ((REG) = (VAL))
#define READ_REG(REG)              (REG)
#define MODIFY_REG(REG, CLR, SET)  WRITE_REG((REG), ((READ_REG(REG) & ~(CLR)) | (SET)))

#define SPI0_DIVIDER       (239UL)      /* 48 МГц / (239+1) = 200 кГц */
#define SPI_TIMEOUT       (1000000UL)  /* Тайм-аут цикла опроса STATUS */
#define TEST_READ_COUNT   (1000UL)     /* Измерения в одной серии */
#define REPORT_EVERY      (100UL)      /* Промежуточный отчёт */

/* Для наблюдения через Watch в SEGGER Embedded Studio. */
volatile uint32_t g_spi_rx32     = 0UL;
volatile uint32_t g_spi_status   = 0UL;
volatile uint32_t g_total_ok32   = 0UL;
volatile uint32_t g_total_errors = 0UL;

/* ------------------------------------------------------------------------- */
/* Небольшие функции вывода в SEGGER RTT, без printf / malloc / UART.        */
/* ------------------------------------------------------------------------- */
static void RTT_Text(const char *s)
{
    SEGGER_RTT_WriteString(0U, s);
}

static char HexDigit(uint32_t n)
{
    static const char hex[] = "0123456789ABCDEF";
    return hex[n & 15UL];
}

static void RTT_Hex8(uint8_t value)
{
    char buf[3];
    buf[0] = HexDigit((uint32_t)value >> 4);
    buf[1] = HexDigit(value);
    buf[2] = '\0';
    RTT_Text(buf);
}

static void RTT_Hex32(uint32_t value)
{
    char buf[11];
    uint32_t i;
    buf[0] = '0';
    buf[1] = 'x';
    for (i = 0UL; i < 8UL; ++i)
    {
        buf[2UL + i] = HexDigit(value >> (28UL - 4UL * i));
    }
    buf[10] = '\0';
    RTT_Text(buf);
}

static void RTT_U32(uint32_t value)
{
    char reversed[10];
    char buf[11];
    uint32_t n = 0UL;
    uint32_t i;

    do
    {
        reversed[n++] = (char)('0' + (value % 10UL));
        value /= 10UL;
    } while ((value != 0UL) && (n < 10UL));

    for (i = 0UL; i < n; ++i)
    {
        buf[i] = reversed[n - i - 1UL];
    }
    buf[n] = '\0';
    RTT_Text(buf);
}

/* Задержка через регистры SysTick, без библиотечных функций CLK_*. */
static void Delay_ms(uint32_t ms)
{
    if (ms == 0UL) return;

    /* В SYS_Init() HCLK задан 48 МГц. 48000 тиков = 1 мс. */
    SysTick->LOAD = (48000000UL / 1000UL) - 1UL;
    SysTick->VAL = 0UL;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;

    while (ms-- != 0UL)
    {
        while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0UL) { }
    }

    SysTick->CTRL = 0UL;
}

/* Ожидание сброса указанных битов STATUS. 0 = тайм-аут. */
static uint8_t SPI_WaitClear(uint32_t mask)
{
    uint32_t timeout = SPI_TIMEOUT;
    while (((SPI0->STATUS & mask) != 0UL) && (timeout != 0UL))
    {
        --timeout;
    }
    return (uint8_t)(timeout != 0UL);
}

/* Ожидание установки указанных битов STATUS. 0 = тайм-аут. */
static uint8_t SPI_WaitSet(uint32_t mask)
{
    uint32_t timeout = SPI_TIMEOUT;
    while (((SPI0->STATUS & mask) != mask) && (timeout != 0UL))
    {
        --timeout;
    }
    return (uint8_t)(timeout != 0UL);
}

/* ------------------------------------------------------------------------- */
/* Настройка тактирования и мультиплексирования PA.0..PA.3.                   */
/* ------------------------------------------------------------------------- */
static void SYS_Init_Registers(void)
{
    SYS_UnlockReg();

    /* Включить внутренний HIRC 48 MHz. */
    CLK->PWRCTL |= CLK_PWRCTL_HIRCEN_Msk;
    while ((CLK->STATUS & CLK_STATUS_HIRCSTB_Msk) == 0UL) { }

    /* HCLK = HIRC / 1 = 48 МГц. */
    CLK->CLKSEL0 = (CLK->CLKSEL0 & ~CLK_CLKSEL0_HCLKSEL_Msk) |
                    CLK_CLKSEL0_HCLKSEL_HIRC;
    CLK->CLKDIV0 = (CLK->CLKDIV0 & ~CLK_CLKDIV0_HCLKDIV_Msk) |
                    CLK_CLKDIV0_HCLK(1);

    /* PCLK1 = HCLK / 1 = 48 МГц (поле APB1DIV действительно находится
       в CLK->PCLKDIV, см. предоставленный заголовок регистров CLK). */
    CLK->PCLKDIV &= ~CLK_PCLKDIV_APB1DIV_Msk;

    /* Источник SPI0 = PCLK1, включить модуль SPI0. */
    CLK->CLKSEL2 = (CLK->CLKSEL2 & ~CLK_CLKSEL2_SPI0SEL_Msk) |
                   CLK_CLKSEL2_SPI0SEL_PCLK1;
    CLK->APBCLK0 |= CLK_APBCLK0_SPI0CKEN_Msk;

    /* Все четыре вывода SPI0, включая *аппаратный* SS на PA.3.
       Символьные определения MFP берём из NuMicro.h, без магических 4. */
    SYS->GPA_MFPL =
        (SYS->GPA_MFPL & ~(SYS_GPA_MFPL_PA0MFP_Msk |
                           SYS_GPA_MFPL_PA1MFP_Msk |
                           SYS_GPA_MFPL_PA2MFP_Msk |
                           SYS_GPA_MFPL_PA3MFP_Msk)) |
        SYS_GPA_MFPL_PA0MFP_SPI0_MOSI |
        SYS_GPA_MFPL_PA1MFP_SPI0_MISO |
        SYS_GPA_MFPL_PA2MFP_SPI0_CLK  |
        SYS_GPA_MFPL_PA3MFP_SPI0_SS;

    SYS_LockReg();
}

/* ------------------------------------------------------------------------- */
/* SPI0: аппаратный Auto-SS, Mode 0, 32 бит, MSB first.                      */
/* ------------------------------------------------------------------------- */
static uint8_t SPI0_Init_Registers(void)
{
    SYS_UnlockReg();

    /* Сбросить весь SPI0, чтобы не наследовать старые состояния FIFO. */
    SYS->IPRST1 |= SYS_IPRST1_SPI0RST_Msk;
    SYS->IPRST1 &= ~SYS_IPRST1_SPI0RST_Msk;

    /* Порядок из spi_reg.h: отключить SPI и проверить SPIENSTS=0. */
    SPI0->CTL = 0UL;
    if (!SPI_WaitClear(SPI_STATUS_SPIENSTS_Msk))
    {
        SYS_LockReg();
        return 0U;
    }


    /* SCLK = f_SPI0 / (2 * (DIVIDER + 1)).
    При источнике SPI0 = 48 MHz и DIVIDER = 3 получаем 6 MHz. */
    WRITE_REG(SPI0->CLKDIV, 3UL << SPI_CLKDIV_DIVIDER_Pos);

    /* Это *твоя* проверенная анализатором комбинация:
       SS=1, AUTOSS=1, SSACTPOL=0 -> активный уровень PA.3 = 0. */
    SPI0->SSCTL = SPI_SSCTL_AUTOSS_Msk | SPI_SSCTL_SS_Msk;

    /* Сброс FIFO строго ДО включения SPI. */
    SPI0->FIFOCTL = SPI_FIFOCTL_TXRST_Msk | SPI_FIFOCTL_RXRST_Msk;
    if (!SPI_WaitClear(SPI_STATUS_TXRXRST_Msk))
    {
        SYS_LockReg();
        return 0U;
    }

    /* 32 бит: DWIDTH=0. Master: SLAVE=0. Mode 0: CLKPOL=0, RXNEG=0,
       TXNEG=1. FULLDPX: HALFDPX=0. MSB first: LSB=0. */
    SPI0->CTL = SPI_CTL_TXNEG_Msk | SPI_CTL_SPIEN_Msk;

    if (!SPI_WaitSet(SPI_STATUS_SPIENSTS_Msk))
    {
        SYS_LockReg();
        return 0U;
    }

    SYS_LockReg();
    return 1U;
}

/* ------------------------------------------------------------------------- */
/* Одно 32-битное слово = один импульс аппаратного CS на PA.3.                */
/* 0: OK, 1: SPI занят, 2: RX FIFO содержит старые данные,                  */
/* 3: тайм-аут RX, 4: ошибка RX FIFO/переполнение, 5: SPI не включён.       */
/* ------------------------------------------------------------------------- */
static uint8_t SPI0_Transfer32(uint32_t tx_word, uint32_t *rx_word)
{
    uint32_t result;

    if ((SPI0->STATUS & SPI_STATUS_SPIENSTS_Msk) == 0UL)
        return 5U;

    if (!SPI_WaitClear(SPI_STATUS_BUSY_Msk | SPI_STATUS_TXFULL_Msk))
        return 1U;

    /* После каждой операции ниже RX читается ровно ОДИН раз. Нельзя
       читать SPI0->RX дважды: второй доступ может нарушить соответствие
       переданных и принятых данных! */
    if ((SPI0->STATUS & SPI_STATUS_RXEMPTY_Msk) == 0UL)
        return 2U;

    SPI0->TX = tx_word;

    /* Как только RXEMPTY=0, завершены все 32 входных такта кадра. */
    if (!SPI_WaitClear(SPI_STATUS_RXEMPTY_Msk))
        return 3U;

    result = SPI0->RX;  /* ОДИН раз за один кадр */

    if (!SPI_WaitClear(SPI_STATUS_BUSY_Msk))
        return 1U;

    if ((SPI0->STATUS & SPI_STATUS_RXOVIF_Msk) != 0UL)
        return 4U;

    *rx_word = result;
    return 0U;
}

/* ------------------------------------------------------------------------- */
/* W25Q32JV: Read JEDEC ID (0x9F).                                           */
/* ------------------------------------------------------------------------- */
static uint8_t Flash_ReadJEDEC(uint8_t id[3], uint32_t *raw)
{
    uint32_t rx_word = 0UL;
    uint8_t error;

    id[0] = id[1] = id[2] = 0xFFU;
    *raw = 0UL;

    /* Один кадр 32 бита:
       TX: 9F FF FF FF
       RX: XX EF 40 16 (при исправном W25Q32JV)
       Байт RX во время команды 9F неопределён, поэтому не проверяется. */
    error = SPI0_Transfer32(0x9FFFFFFFUL, &rx_word);
    if (error != 0U) return error;

    *raw = rx_word;
    id[0] = (uint8_t)(rx_word >> 16);
    id[1] = (uint8_t)(rx_word >> 8);
    id[2] = (uint8_t)rx_word;
    return 0U;
}

/* Чтение SR1/SR2/SR3. В каждом кадре команда занимает первые 8 бит,
   затем Flash выдаёт статус; берём только первый байт после команды. */
static uint8_t Flash_ReadStatus(uint8_t cmd, uint8_t *status)
{
    uint32_t rx_word = 0UL;
    uint8_t error = SPI0_Transfer32(((uint32_t)cmd << 24) | 0x00FFFFFFUL,
                                    &rx_word);
    if (error != 0U) return error;

    *status = (uint8_t)(rx_word >> 16);
    return 0U;
}

static void RTT_PrintID(uint8_t id[3], uint32_t raw)
{
    RTT_Text(" RAW="); RTT_Hex32(raw);
    RTT_Text(" JEDEC=");
    RTT_Hex8(id[0]); RTT_Text(" ");
    RTT_Hex8(id[1]); RTT_Text(" ");
    RTT_Hex8(id[2]);
}

/* ------------------------------------------------------------------------- */
/* Основной диагностический тест.                                            */
/* ------------------------------------------------------------------------- */
int main(void)
{
    uint8_t id[3], sr1, sr2, sr3;
    uint8_t spi_error;
    uint32_t raw;
    uint32_t sample;
    uint32_t series = 0UL;
    uint32_t count_ok, count_ff, count_00, count_other, count_spi_err;

    SYS_Init_Registers();
    SEGGER_RTT_Init();

    RTT_Text("\r\n===============================================\r\n");
    RTT_Text(" M031TD2AE / W25Q32JV / SPI0 / SEGGER RTT\r\n");
    RTT_Text(" PA0 MOSI, PA1 MISO, PA2 CLK, PA3 HW SPI0_SS\r\n");
    RTT_Text(" SSCTL = AUTOSS | SS (0x09), CS active LOW\r\n");
    RTT_Text(" SPI mode 0, MSB-first, 32-bit frame, 200 kHz\r\n");
    RTT_Text(" TX: 9F FF FF FF; EXPECT JEDEC: EF 40 16\r\n");
    RTT_Text(" READ ONLY -- NO PROGRAM, NO ERASE\r\n");
    RTT_Text("===============================================\r\n");

    if (!SPI0_Init_Registers())
    {
        RTT_Text("ERROR: SPI0 INIT TIMEOUT; CHECK CLOCK SETTINGS\r\n");
        for (;;) { }
    }

    RTT_Text("REG: SPI0_CTL="); RTT_Hex32(SPI0->CTL);
    RTT_Text(" SSCTL="); RTT_Hex32(SPI0->SSCTL);
    RTT_Text(" CLKDIV="); RTT_U32(SPI0->CLKDIV & SPI_CLKDIV_DIVIDER_Msk);
    RTT_Text(" GPA_MFPL="); RTT_Hex32(SYS->GPA_MFPL);
    RTT_Text(" PCLKDIV="); RTT_Hex32(CLK->PCLKDIV);
    RTT_Text("\r\n");

    /* После включения питания даём Flash время перед первым чтением.
       Команда 0xAB (Release Power Down) здесь НЕ посылается: это тест
       только базовой функции JEDEC, без побочных управляющих команд. */
    Delay_ms(10UL);

    for (;;)
    {
        ++series;
        count_ok = 0UL;
        count_ff = 0UL;
        count_00 = 0UL;
        count_other = 0UL;
        count_spi_err = 0UL;

        RTT_Text("\r\nSERIES #"); RTT_U32(series);
        RTT_Text(" / 1000 READ JEDEC\r\n");

        for (sample = 1UL; sample <= TEST_READ_COUNT; ++sample)
        {
            spi_error = Flash_ReadJEDEC(id, &raw);

            if (spi_error != 0U)
            {
                ++count_spi_err;
                ++g_total_errors;
                if (count_spi_err <= 5UL)
                {
                    RTT_Text("  SPI ERROR #"); RTT_U32(sample);
                    RTT_Text(" code="); RTT_U32(spi_error);
                    RTT_Text(" STATUS="); RTT_Hex32(SPI0->STATUS);
                    RTT_Text("\r\n");
                }
                /* При тайм-ауте или нарушении FIFO не продолжаем поток
                   ложных чтений: сохранить текущие регистры для DEBUG. */
                RTT_Text("STOP: SPI ERROR, CHECK DEBUG / SIGNALS\r\n");
                for (;;) { }
            }
            else if ((id[0] == 0xEFU) &&
                     (id[1] == 0x40U) &&
                     (id[2] == 0x16U))
            {
                ++count_ok;
                ++g_total_ok32;
            }
            else if ((id[0] == 0xFFU) &&
                     (id[1] == 0xFFU) &&
                     (id[2] == 0xFFU))
            {
                ++count_ff;
                ++g_total_errors;
            }
            else if ((id[0] == 0x00U) &&
                     (id[1] == 0x00U) &&
                     (id[2] == 0x00U))
            {
                ++count_00;
                ++g_total_errors;
            }
            else
            {
                ++count_other;
                ++g_total_errors;
            }

            g_spi_rx32 = raw;
            g_spi_status = SPI0->STATUS;

            /* Первое чтение и несколько ошибок -- со значением *всех* 32 бит. */
            if (sample == 1UL ||
                ((count_ff + count_00 + count_other) <= 3UL &&
                 (id[0] != 0xEFU || id[1] != 0x40U || id[2] != 0x16U)))
            {
                RTT_Text("  #"); RTT_U32(sample);
                RTT_PrintID(id, raw);
                RTT_Text("\r\n");
            }

            if ((sample % REPORT_EVERY) == 0UL)
            {
                RTT_Text("  PROGRESS "); RTT_U32(sample);
                RTT_Text(" OK32="); RTT_U32(count_ok);
                RTT_Text(" FFFFFF="); RTT_U32(count_ff);
                RTT_Text(" 000000="); RTT_U32(count_00);
                RTT_Text(" OTHER="); RTT_U32(count_other);
                RTT_Text(" SPI_ERR="); RTT_U32(count_spi_err);
                RTT_Text("\r\n");
            }

            Delay_ms(5UL);
        }

        /* Читаем регистры состояния после 1000 JEDEC чтений. */
        sr1 = sr2 = sr3 = 0xFFU;
        spi_error  = Flash_ReadStatus(0x05U, &sr1);
        spi_error |= Flash_ReadStatus(0x35U, &sr2);
        spi_error |= Flash_ReadStatus(0x15U, &sr3);

        RTT_Text("STATUS: SR1="); RTT_Hex8(sr1);
        RTT_Text(" SR2="); RTT_Hex8(sr2);
        RTT_Text(" SR3="); RTT_Hex8(sr3);
        if (spi_error != 0U) RTT_Text(" [SPI ERROR]");
        RTT_Text("\r\nRESULT: ");
        RTT_Text((count_ok == TEST_READ_COUNT) ?
                 "STABLE W25Q32JV (1000/1000)\r\n" :
                 "UNSTABLE / NO RESPONSE\r\n");

        Delay_ms(1000UL);
    }
}
