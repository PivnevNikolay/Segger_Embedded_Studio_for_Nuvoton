 /**-------------------------------------------------------------------
 * ЖК-дисплей (модуль) GMG12864-06D Ver:02 представляет собой матричный дисплей разрешением 128х64 
 * с размерами окна 58х28 мм. В дисплее применен драйвер ST7565R (интерфейс SPI). Напряжение питания 3,3 В.
 *
 *      M031EC1AE              GMG12864 
 *   ---------------         ------------
 *  |               |       |
 *  |               |       |
 *  |               |       |
 *  |          PB.0 | ----> | CS  (CS)
 *  |          PB.1 | ----> | RSE (RST(Reset))
 *  |          PB.2 | ----> | RS  (DC(Data/Command) 
 *  |               |       |
 *  |          PB.3 | ----> | SCL (SCK)
 *  |          PB.4 | ----> | SI  (MOSI)
 *  |               |       |
 *  |         +3.3V | <---> | +3.3V
 *  |           GND | <---> | GND
 *  |         +3.3V | <---> | +3.3V 
 *  |           GND | <---> | GND
 *
 * \сode debugging ScuratovaAnna, PivnevNikolay
 *
 *************************************************************************/
#include "NuMicro.h"
#include "stdbool.h"
#include "stdio.h"
#include "string.h"

#define font3x5 0
#define font5x7 1

#define LATCH_Pin 0 // PB.0 --> LATCH_Pin    CS
#define CLK_Pin 3   // PB.3 --> CLK_Pin      WR
#define DATA_Pin 4  // PB.4 --> DATA_Pin     DATA

#define cs_set() PB0 = 0    // CS_Pin --> LOW
#define cs_reset() PB0 = 1  // CS_Pin --> HIGH
#define RST_set() PB1 = 0   // RST_Pin --> LOW
#define RST_reset() PB1 = 1 // RST_Pin --> HIGH
#define DC_set() PB2 = 0    // DC_Pin --> LOW
#define DC_reset() PB2 = 1  // DC_Pin --> HIGH

uint8_t GMG12864_width = 128; // Display width in pixels
uint8_t GMG12864_height = 64; // Display height in pixels

char tx_buffer[128] = {0,}; // Buffer for sending text to the display
uint8_t Frame_buffer[1024] = {0,}; // Frame Buffer

//-------------------------------------- Font 5*7 ------------------------------------
const uint8_t Font_5x7[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, // 0/ --> space     20
    0x00, 0x4F, 0x00, 0x00, 0x00, //(1) --> !
    0x07, 0x00, 0x07, 0x00, 0x00, //(2) --> "
    0x14, 0x7F, 0x14, 0x7F, 0x14, //(3) --> #
    0x24, 0x2A, 0x7F, 0x2A, 0x12, //(4) --> $
    0x23, 0x13, 0x08, 0x64, 0x62, //(5) --> %
    0x36, 0x49, 0x55, 0x22, 0x40, //(6) --> &
    0x00, 0x05, 0x03, 0x00, 0x00, //(7) --> '
    0x1C, 0x22, 0x41, 0x00, 0x00, //(8) --> (
    0x41, 0x22, 0x1C, 0x00, 0x00, //(9) --> )
    0x14, 0x08, 0x3E, 0x08, 0x14, //(10) --> *
    0x08, 0x08, 0x3E, 0x08, 0x08, //(11) --> +
    0xA0, 0x60, 0x00, 0x00, 0x00, //(12) --> ,
    0x08, 0x08, 0x08, 0x08, 0x08, //(13) --> -
    0x60, 0x60, 0x00, 0x00, 0x00, //(14) --> .
    0x20, 0x10, 0x08, 0x04, 0x02, //(15) --> /
    0x3E, 0x51, 0x49, 0x45, 0x3E, //(16) --> 0
    0x00, 0x42, 0x7F, 0x40, 0x00, //(17) --> 1
    0x42, 0x61, 0x51, 0x49, 0x46, //(18) --> 2
    0x21, 0x41, 0x45, 0x4B, 0x31, //(19) --> 3
    0x18, 0x14, 0x12, 0x7F, 0x10, //(20) --> 4
    0x27, 0x45, 0x45, 0x45, 0x39, //(21) --> 5
    0x3C, 0x4A, 0x49, 0x49, 0x30, //(22) --> 6
    0x01, 0x71, 0x09, 0x05, 0x03, //(23) --> 7
    0x36, 0x49, 0x49, 0x49, 0x36, //(24) --> 8
    0x06, 0x49, 0x49, 0x29, 0x1E, //(25) --> 9
    0x6C, 0x6C, 0x00, 0x00, 0x00, //(26) --> :
    0x00, 0x56, 0x36, 0x00, 0x00, //(27) --> ;
    0x08, 0x14, 0x22, 0x41, 0x00, //(28) --> <
    0x24, 0x24, 0x24, 0x24, 0x24, //(29) --> =
    0x00, 0x41, 0x22, 0x14, 0x08, //(30) --> >
    0x02, 0x01, 0x51, 0x09, 0x06, //(31) --> ?
    0x32, 0x49, 0x79, 0x41, 0x3E, //(32) --> @
    0x7E, 0x11, 0x11, 0x11, 0x7E, //(33) --> A
    0x7F, 0x49, 0x49, 0x49, 0x36, //(34) --> B
    0x3E, 0x41, 0x41, 0x41, 0x22, //(35) --> C
    0x7F, 0x41, 0x41, 0x22, 0x1C, //(36) --> D
    0x7F, 0x49, 0x49, 0x49, 0x41, //(37) --> E
    0x7F, 0x09, 0x09, 0x09, 0x01, //(38) --> F
    0x3E, 0x41, 0x49, 0x49, 0x3A, //(39) --> G
    0x7F, 0x08, 0x08, 0x08, 0x7F, //(40) --> H
    0x00, 0x41, 0x7F, 0x41, 0x00, //(41) --> I
    0x20, 0x40, 0x41, 0x3F, 0x01, //(42) --> J
    0x7F, 0x08, 0x14, 0x22, 0x41, //(43) --> K
    0x7F, 0x40, 0x40, 0x40, 0x40, //(44) --> L
    0x7F, 0x02, 0x0C, 0x02, 0x7F, //(45) --> M
    0x7F, 0x04, 0x08, 0x10, 0x7F, //(46) --> N
    0x3E, 0x41, 0x41, 0x41, 0x3E, //(47) --> O
    0x7F, 0x09, 0x09, 0x09, 0x06, //(48) --> P
    0x3E, 0x41, 0x51, 0x21, 0x5E, //(49) --> Q
    0x7F, 0x09, 0x19, 0x29, 0x46, //(50) --> R
    0x46, 0x49, 0x49, 0x49, 0x31, //(51) --> S
    0x01, 0x01, 0x7F, 0x01, 0x01, //(52) --> T
    0x3F, 0x40, 0x40, 0x40, 0x3F, //(53) --> U
    0x1F, 0x20, 0x40, 0x20, 0x1F, //(54) --> V
    0x3F, 0x40, 0x60, 0x40, 0x3F, //(55) --> W
    0x63, 0x14, 0x08, 0x14, 0x63, //(56) --> X
    0x07, 0x08, 0x70, 0x08, 0x07, //(57) --> Y
    0x61, 0x51, 0x49, 0x45, 0x43, //(58) --> Z
    0x7F, 0x41, 0x41, 0x00, 0x00, //(59) --> [
    0x15, 0x16, 0x7C, 0x16, 0x15, //(60) --> '\'
    0x41, 0x41, 0x7F, 0x00, 0x00, //(61) --> ]
    0x04, 0x02, 0x01, 0x02, 0x04, //(62) --> ^
    0x40, 0x40, 0x40, 0x40, 0x40, //(63) --> _
    0x01, 0x02, 0x04, 0x00, 0x00, //(64) --> `
    0x20, 0x54, 0x54, 0x54, 0x78, //(65) --> a
    0x7F, 0x44, 0x44, 0x44, 0x38, //(66) --> b
    0x38, 0x44, 0x44, 0x44, 0x00, //(67) --> c
    0x38, 0x44, 0x44, 0x48, 0x7F, //(68) --> d
    0x38, 0x54, 0x54, 0x54, 0x18, //(69) --> e
    0x10, 0x7E, 0x11, 0x01, 0x02, //(70) --> f
    0x0C, 0x52, 0x52, 0x52, 0x3E, //(71) --> g
    0x7F, 0x08, 0x04, 0x04, 0x78, //(72) --> h
    0x00, 0x44, 0x7D, 0x40, 0x00, //(73) --> i
    0x20, 0x40, 0x40, 0x3D, 0x00, //(74) --> j
    0x7F, 0x10, 0x28, 0x44, 0x00, //(75) --> k
    0x00, 0x41, 0x7F, 0x40, 0x00, //(76) --> l
    0x7C, 0x04, 0x18, 0x04, 0x78, //(77) --> m
    0x7C, 0x08, 0x04, 0x04, 0x78, //(78) --> n
    0x38, 0x44, 0x44, 0x44, 0x38, //(79) --> o
    0x7C, 0x14, 0x14, 0x14, 0x08, //(80) --> p
    0x08, 0x14, 0x14, 0x18, 0x7C, //(81) --> q
    0x7C, 0x08, 0x04, 0x04, 0x08, //(82) --> r
    0x48, 0x54, 0x54, 0x54, 0x20, //(83) --> s
    0x04, 0x3F, 0x44, 0x40, 0x20, //(84) --> t
    0x3C, 0x40, 0x40, 0x20, 0x7C, //(85) --> u
    0x1C, 0x20, 0x40, 0x20, 0x1C, //(86) --> v
    0x1E, 0x20, 0x10, 0x20, 0x1E, //(87) --> w
    0x22, 0x14, 0x08, 0x14, 0x22, //(88) --> x
    0x06, 0x48, 0x48, 0x48, 0x3E, //(89) --> y
    0x44, 0x64, 0x54, 0x4C, 0x44, //(90) --> z
    0x08, 0x36, 0x41, 0x00, 0x00, //(91) --> {
    0x00, 0x00, 0x7F, 0x00, 0x00, //(92) --> |
    0x41, 0x36, 0x08, 0x00, 0x00, //(93) --> }
    0x08, 0x08, 0x2A, 0x1C, 0x08, //(94) --> ~
    0x7E, 0x11, 0x11, 0x11, 0x7E, //(95) --> А
    0x7F, 0x49, 0x49, 0x49, 0x33, //(96) --> Б
    0x7F, 0x49, 0x49, 0x49, 0x36, //(97) --> В
    0x7F, 0x01, 0x01, 0x01, 0x03, //(98) --> Г
    0xE0, 0x51, 0x4F, 0x41, 0xFF, //(99) --> Д
    0x7F, 0x49, 0x49, 0x49, 0x41, //(100) --> Е
    0x77, 0x08, 0x7F, 0x08, 0x77, //(101) --> Ж
    0x41, 0x49, 0x49, 0x49, 0x36, //(102) --> З
    0x7F, 0x10, 0x08, 0x04, 0x7F, //(103) --> И
    0x7C, 0x21, 0x12, 0x09, 0x7C, //(104) --> Й
    0x7F, 0x08, 0x14, 0x22, 0x41, //(105) --> К
    0x20, 0x41, 0x3F, 0x01, 0x7F, //(106) --> Л
    0x7F, 0x02, 0x0C, 0x02, 0x7F, //(107) --> М
    0x7F, 0x08, 0x08, 0x08, 0x7F, //(108) --> Н
    0x3E, 0x41, 0x41, 0x41, 0x3E, //(109) --> О
    0x7F, 0x01, 0x01, 0x01, 0x7F, //(110) --> П
    0x7F, 0x09, 0x09, 0x09, 0x06, //(111) --> Р
    0x3E, 0x41, 0x41, 0x41, 0x22, //(112) --> С
    0x01, 0x01, 0x7F, 0x01, 0x01, //(113) --> Т
    0x47, 0x28, 0x10, 0x08, 0x07, //(114) --> У
    0x1C, 0x22, 0x7F, 0x22, 0x1C, //(115) --> Ф
    0x63, 0x14, 0x08, 0x14, 0x63, //(116) --> Х
    0x7F, 0x40, 0x40, 0x40, 0xFF, //(117) --> Ц
    0x07, 0x08, 0x08, 0x08, 0x7F, //(118) --> Ч
    0x7F, 0x40, 0x7F, 0x40, 0x7F, //(119) --> Ш
    0x7F, 0x40, 0x7F, 0x40, 0xFF, //(120) --> Щ
    0x01, 0x7F, 0x48, 0x48, 0x30, //(121) --> Ъ
    0x7F, 0x48, 0x30, 0x00, 0x7F, //(122) --> Ы
    0x00, 0x7F, 0x48, 0x48, 0x30, //(123) --> Э
    0x22, 0x41, 0x49, 0x49, 0x3E, //(124) --> Ь
    0x7F, 0x08, 0x3E, 0x41, 0x3E, //(125) --> Ю
    0x46, 0x29, 0x19, 0x09, 0x7f, //(126) --> Я
    0x20, 0x54, 0x54, 0x54, 0x78, //(127) --> a
    0x3c, 0x4a, 0x4a, 0x49, 0x31, //(128) --> б
    0x7c, 0x54, 0x54, 0x28, 0x00, //(129) --> в
    0x7c, 0x04, 0x04, 0x04, 0x0c, //(130) --> г
    0xe0, 0x54, 0x4c, 0x44, 0xfc, //(131) --> д
    0x38, 0x54, 0x54, 0x54, 0x18, //(132) --> е
    0x6c, 0x10, 0x7c, 0x10, 0x6c, //(133) --> ж
    0x44, 0x44, 0x54, 0x54, 0x28, //(134) --> з
    0x7c, 0x20, 0x10, 0x08, 0x7c, //(135) --> и
    0x7c, 0x41, 0x22, 0x11, 0x7c, //(136) --> й
    0x7c, 0x10, 0x28, 0x44, 0x00, //(137) --> к
    0x20, 0x44, 0x3c, 0x04, 0x7c, //(138) --> л
    0x7c, 0x08, 0x10, 0x08, 0x7c, //(139) --> м
    0x7c, 0x10, 0x10, 0x10, 0x7c, //(140) --> н
    0x38, 0x44, 0x44, 0x44, 0x38, //(141) --> о
    0x7c, 0x04, 0x04, 0x04, 0x7c, //(142) --> п
    0x7C, 0x14, 0x14, 0x14, 0x08, //(143) --> р
    0x38, 0x44, 0x44, 0x44, 0x28, //(144) --> с
    0x04, 0x04, 0x7c, 0x04, 0x04, //(145) --> т
    0x0C, 0x50, 0x50, 0x50, 0x3C, //(146) --> у
    0x30, 0x48, 0xfc, 0x48, 0x30, //(147) --> ф
    0x44, 0x28, 0x10, 0x28, 0x44, //(148) --> х
    0x7c, 0x40, 0x40, 0x40, 0xfc, //(149) --> ц
    0x0c, 0x10, 0x10, 0x10, 0x7c, //(150) --> ч
    0x7c, 0x40, 0x7c, 0x40, 0x7c, //(151) --> ш
    0x7c, 0x40, 0x7c, 0x40, 0xfc, //(152) --> щ
    0x04, 0x7c, 0x50, 0x50, 0x20, //(153) --> ъ
    0x7c, 0x50, 0x50, 0x20, 0x7c, //(154) --> ы
    0x7c, 0x50, 0x50, 0x20, 0x00, //(155) --> э
    0x28, 0x44, 0x54, 0x54, 0x38, //(156) --> ь
    0x7c, 0x10, 0x38, 0x44, 0x38, //(157) --> ю
    0x08, 0x54, 0x34, 0x14, 0x7c, //(158) --> я
    0x7E, 0x4B, 0x4A, 0x4B, 0x42, //(159) --> Ё
    0x38, 0x55, 0x54, 0x55, 0x18, //(160) --> ё
    0x00, 0x06, 0x09, 0x09, 0x06, //(161) --> °
};
//------------------------------------------------------------------------------------
//-------------------------------------- Font 3*5 ------------------------------------
const uint8_t Font_3x5[] = {
    0x00, 0x00, 0x00, // 0/ --> space     20
    0x17, 0x00, 0x00, // 1/ --> !
    0x03, 0x00, 0x03, // 2/ --> "
    0xFF, 0x0A, 0xFF, // 3/ --> #
    0x17, 0xFF, 0x1D, // 4/ --> $
    0x09, 0x04, 0x12, // 5/ --> %
    0x0E, 0x1B, 0x0A, // 6/ --> &
    0x03, 0x00, 0x00, // 7/ --> '
    0x0E, 0x11, 0x00, // 8/ --> (
    0x11, 0x0E, 0x00, // 9/ --> )
    0x04, 0x0A, 0x04, // 10/ --> *
    0x04, 0x0E, 0x04, // 11/ --> +
    0x10, 0x18, 0x00, // 12/ --> ,
    0x04, 0x04, 0x04, // 13/ --> -
    0x10, 0x00, 0x00, // 14/ --> .
    0x08, 0x04, 0x02, // 15/ --> /
    0xFF, 0x11, 0xFF, // 16/ --> 0
    0x11, 0xFF, 0x10, // 17/ --> 1
    0x1D, 0x15, 0x17, // 18/ --> 2
    0x15, 0x15, 0xFF, // 19/ --> 3
    0x07, 0x04, 0xFF, // 20/ --> 4
    0x17, 0x15, 0x1D, // 21/ --> 5
    0xFF, 0x15, 0x1D, // 22/ --> 6
    0x01, 0x01, 0xFF, // 23/ --> 7
    0xFF, 0x15, 0xFF, // 24/ --> 8
    0x17, 0x15, 0x1F, // 25/ --> 9
    0x0A, 0x00, 0x00, // 26/ --> :
    0x10, 0x0A, 0x00, // 27/ --> ;
    0x04, 0x0A, 0x00, // 28/ --> <
    0x0A, 0x0A, 0x0A, // 29/ --> =
    0x0A, 0x04, 0x00, // 30/ --> >
    0x01, 0x15, 0x07, // 31/ --> ?
    0x1F, 0x11, 0x17, // 32/ --> @
    0x1F, 0x05, 0x1F, // 33/ --> A
    0x1F, 0x15, 0x1B, // 34/ --> B
    0x1F, 0x11, 0x11, // 35/ --> C
    0x1F, 0x11, 0x0E, // 36/ --> D
    0x1F, 0x15, 0x15, // 37/ --> E
    0x1F, 0x05, 0x05, // 38/ --> F
    0x1F, 0x11, 0x1D, // 39/ --> G
    0x1F, 0x04, 0x1F, // 40/ --> H
    0x11, 0x1F, 0x11, // 41/ --> I
    0x18, 0x10, 0x1F, // 42/ --> J
    0x1F, 0x04, 0x1B, // 43/ --> K
    0x1F, 0x10, 0x10, // 44/ --> L
    0x1F, 0x02, 0x1F, // 45/ --> M
    0x1F, 0x01, 0x1F, // 46/ --> N
    0x1F, 0x11, 0x1F, // 47/ --> O
    0x1F, 0x05, 0x07, // 48/ --> P
    0x0F, 0x19, 0x0F, // 49/ --> Q
    0x1F, 0x05, 0x1B, // 50/ --> R
    0x17, 0x15, 0x1D, // 51/ --> S
    0x01, 0x1F, 0x01, // 52/ --> T
    0x1F, 0x10, 0x1F, // 53/ --> U
    0x0F, 0x10, 0x0F, // 54/ --> V
    0x1F, 0x08, 0x1F, // 55/ --> W
    0x1B, 0x04, 0x1B, // 56/ --> X
    0x03, 0x1C, 0x03, // 57/ --> Y
    0x19, 0x15, 0x13, // 58/ --> Z
    0x1F, 0x11, 0x00, // 59/ --> [
    0x02, 0x04, 0x08, // 60/ --> '\'
    0x11, 0x1F, 0x00, // 61/ --> ]
    0x02, 0x01, 0x02, // 62/ --> ^
    0x10, 0x10, 0x10, // 63/ --> _
    0x01, 0x02, 0x00, // 64/ --> `
    0x0C, 0x12, 0x1E, // 65/ --> a
    0x1E, 0x14, 0x08, // 66/ --> b
    0x1C, 0x14, 0x14, // 67/ --> c
    0x0C, 0x12, 0x1F, // 68/ --> d
    0x0C, 0x1A, 0x14, // 69/ --> e
    0x04, 0x1F, 0x05, // 70/ --> f
    0x17, 0x15, 0x0F, // 71/ --> g
    0x1F, 0x02, 0x1C, // 72/ --> h
    0x00, 0x1D, 0x00, // 73/ --> i
    0x10, 0x0D, 0x00, // 74/ --> j
    0x1F, 0x04, 0x1A, // 75/ --> k
    0x01, 0x1F, 0x00, // 76/ --> l
    0x1E, 0x04, 0x1E, // 77/ --> m
    0x1E, 0x02, 0x1E, // 78/ --> n
    0x1E, 0x12, 0x1E, // 79/ --> o
    0x1E, 0x0A, 0x04, // 80/ --> p
    0x04, 0x0A, 0x1E, // 81/ --> q
    0x1E, 0x02, 0x06, // 82/ --> r
    0x14, 0x12, 0x0A, // 83/ --> s
    0x02, 0x1F, 0x12, // 84/ --> t
    0x1E, 0x10, 0x1E, // 85/ --> u
    0x0E, 0x10, 0x0E, // 86/ --> v
    0x1E, 0x08, 0x1E, // 87/ --> w
    0x1A, 0x04, 0x1A, // 88/ --> x
    0x13, 0x14, 0x0F, // 89/ --> y
    0x1A, 0x12, 0x16, // 90/ --> z
    0x04, 0x1F, 0x11, // 91/ --> {
    0x00, 0x1F, 0x00, // 92/ --> |
    0x11, 0x1F, 0x04, // 93/ --> }
    0x0C, 0x04, 0x06, // 94/ --> ~
    0x1F, 0x05, 0x1F, // 95/ --> А
    0x1F, 0x15, 0x1D, // 96/ --> Б
    0x1F, 0x15, 0x1B, // 97/ --> В
    0x1F, 0x01, 0x01, // 98/ --> Г
    0x1E, 0x11, 0x1E, // 99/ --> Д
    0x1F, 0x15, 0x15, // 100/ --> Е
    0x1B, 0x1F, 0x1B, // 101/ --> Ж
    0x15, 0x15, 0x1B, // 102/ --> З
    0x1F, 0x10, 0x1F, // 103/ --> И
    0x1D, 0x11, 0x1D, // 104/ --> Й
    0x1F, 0x04, 0x1B, // 105/ --> К
    0x1E, 0x01, 0x1F, // 106/ --> Л
    0x1F, 0x02, 0x1F, // 107/ --> М
    0x1F, 0x04, 0x1F, // 108/ --> Н
    0x1F, 0x11, 0x1F, // 109/ --> О
    0x1F, 0x01, 0x1F, // 110/ --> П
    0x1F, 0x05, 0x07, // 111/ --> Р
    0x1F, 0x11, 0x11, // 112/ --> С
    0x01, 0x1F, 0x01, // 113/ --> Т
    0x13, 0x14, 0x1F, // 114/ --> У
    0x0E, 0x1B, 0x0E, // 115/ --> Ф
    0x1B, 0x04, 0x1B, // 116/ --> Х
    0x0F, 0x08, 0x1F, // 117/ --> Ц
    0x07, 0x04, 0x1F, // 118/ --> Ч
    0x1F, 0x18, 0x1F, // 119/ --> Ш
    0x0F, 0x0C, 0x1F, // 120/ --> Щ
    0x01, 0x1F, 0x1C, // 121/ --> Ъ
    0x1F, 0x14, 0x1F, // 122/ --> Ы
    0x15, 0x15, 0x0E, // 123/ --> Э
    0x1F, 0x14, 0x08, // 124/ --> Ь
    0x1F, 0x0E, 0x1F, // 125/ --> Ю
    0x1B, 0x05, 0x1F, // 126/ --> Я
    0x0C, 0x12, 0x1E, // 127/ --> a
    0x1E, 0x15, 0x1D, // 128/ --> б
    0x1E, 0x16, 0x0C, // 129/ --> в
    0x1E, 0x02, 0x02, // 130/ --> г
    0x1C, 0x12, 0x1C, // 131/ --> д
    0x0C, 0x1A, 0x14, // 132/ --> е
    0x1A, 0x1E, 0x1A, // 133/ --> ж
    0x12, 0x16, 0x1E, // 134/ --> з
    0x1E, 0x10, 0x1E, // 135/ --> и
    0x1A, 0x12, 0x1A, // 136/ --> й
    0x1E, 0x04, 0x1A, // 137/ --> к
    0x1C, 0x02, 0x1E, // 138/ --> л
    0x1E, 0x04, 0x1E, // 139/ --> м
    0x1E, 0x08, 0x1E, // 140/ --> н
    0x1E, 0x12, 0x1E, // 141/ --> о
    0x1E, 0x02, 0x1E, // 142/ --> п
    0x1E, 0x0A, 0x04, // 143/ --> р
    0x1E, 0x12, 0x12, // 144/ --> с
    0x02, 0x1E, 0x02, // 145/ --> т
    0x16, 0x14, 0x0E, // 146/ --> у
    0x0C, 0x1E, 0x0C, // 147/ --> ф
    0x1A, 0x0C, 0x1A, // 148/ --> х
    0x0E, 0x08, 0x1E, // 149/ --> ц
    0x06, 0x04, 0x1E, // 150/ --> ч
    0x1E, 0x18, 0x1E, // 151/ --> ш
    0x0E, 0x0C, 0x1E, // 152/ --> щ
    0x02, 0x1E, 0x18, // 153/ --> ъ
    0x1E, 0x14, 0x1E, // 154/ --> ы
    0x1E, 0x18, 0x00, // 155/ --> ь
    0x12, 0x16, 0x0C, // 156/ --> э
    0x1E, 0x0C, 0x1E, // 157/ --> ю
    0x14, 0x0A, 0x1E, // 158/ --> я
    0x1F, 0x15, 0x15, // 159/ --> Ё
    0x0C, 0x1A, 0x14, // 160/ --> ё
    0x03, 0x03, 0x00, // 161/ --> °
};
//------------------------------------------------------------------------------------
void GMG12864_Send_command(uint8_t Command);
void GMG12864_Send_data(uint8_t Dat);
void transfer(uint8_t data);
void GMG12864_Init(void);
void ST7565_SetX(uint8_t x);
void ST7565_SetY(uint8_t y);
void GMG12864_Update(void);
void GMG12864_Clean_Frame_buffer(void);
void GMG12864_Draw_pixel(int16_t x, int16_t y, uint8_t color);
void GMG12864_Print_symbol_3x5(uint8_t x, uint8_t y, uint16_t symbol, uint8_t inversion);
void GMG12864_Print_symbol_5x7(uint8_t x, uint8_t y, uint16_t symbol, uint8_t inversion);
void GMG12864_Decode_UTF8(uint8_t x, uint8_t y, uint8_t font, bool inversion, char *tx_buffer);
void GMG12864_Inversion(uint16_t x_start, uint16_t x_end);

void SYS_Init(void) {
  SYS_UnlockReg();
  CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);
  CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);
  CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(11)); // The clock of HCLK --> 4MHz
  // CLK->PCLKDIV = (CLK_PCLKDIV_APB0DIV_DIV4 | CLK_PCLKDIV_APB1DIV_DIV4);
  //********* TMR0_MODULE  ****************************************
  CLK_EnableModuleClock(TMR0_MODULE);
  CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_HIRC, 0);
  SystemCoreClockUpdate();
  SYS_LockReg();
}
//------------------------------------------------------------------------------------
void GPIO_Init(void) {
  GPIO_SetMode(PB, BIT0, GPIO_MODE_OUTPUT); // PB.0 --> CS_Pin
  GPIO_SetMode(PB, BIT1, GPIO_MODE_OUTPUT); // PB.1 --> RST_PIn
  GPIO_SetMode(PB, BIT2, GPIO_MODE_OUTPUT); // PB.2 --> DC_Pin
  GPIO_SetMode(PB, BIT3, GPIO_MODE_OUTPUT); // PB.3 --> CLK_Pin
  GPIO_SetMode(PB, BIT4, GPIO_MODE_OUTPUT); // PB.4 --> Data_Pin
}
//------------------------------------------------------------------------------------
int main(void) {
  SYS_Init();
  GPIO_Init();
  GMG12864_Init();
  TIMER_Delay(TIMER0, 1000);
  // GMG12864_Clean_Frame_buffer();
  // GMG12864_Draw_pixel(1, 1,1);
  // GMG12864_Update();
  // TIMER_Delay(TIMER0, 1000000);
  // GMG12864_Print_symbol_3x5(1, 1, 49, 0);
  // GMG12864_Print_symbol_5x7(5, 5, 50, 0);
  // TIMER_Delay(TIMER0, 1000000);
  // GMG12864_Update();
  GMG12864_Clean_Frame_buffer();
  TIMER_Delay(TIMER0, 1000000);
  sprintf(tx_buffer, "ПривеТ! Hello!");
  GMG12864_Decode_UTF8(1, 1, 1, 0, tx_buffer);
  sprintf(tx_buffer, "Nuvoton");
  GMG12864_Decode_UTF8(1, 13, 1, 0, tx_buffer);
  sprintf(tx_buffer, "M031EC1AE");
  GMG12864_Decode_UTF8(1, 23, 1, 0, tx_buffer);
  sprintf(tx_buffer, "SEGGER");
  GMG12864_Decode_UTF8(1, 33, 1, 0, tx_buffer);
  sprintf(tx_buffer, "Embedded Studio");
  GMG12864_Decode_UTF8(1, 43, 1, 0, tx_buffer);
  // GMG12864_Inversion(1, 82);
  GMG12864_Update();
  while (1) {
  }
}

//------------ The function of sending a command to the display ----------------------
void GMG12864_Send_command(uint8_t Command) {
  DC_set();
  transfer(Command);
  DC_reset();
}
//------------------------------------------------------------------------------------
//--------------- The function of sending data to the display ------------------------
void GMG12864_Send_data(uint8_t Dat) {
  transfer(Dat);
}
//------------------------------------------------------------------------------------
void transfer(uint8_t data) {
  for (uint8_t i = 8; i >= 1; i--) {
    PB->DOUT &= ~(1 << CLK_Pin);
    PB->DOUT = (data & 0x80) ? PB->DOUT | (1 << DATA_Pin) : PB->DOUT & ~(1 << DATA_Pin);
    PB->DOUT |= (1 << CLK_Pin);
    data = (data << 1);
  }
}
//------------------------------------------------------------------------------------
void GMG12864_Init(void) {
  cs_set();
  RST_set();
  RST_reset();
  GMG12864_Send_command(0xA2); 
  GMG12864_Send_command(0xA0);
  GMG12864_Send_command(0xC8);
  GMG12864_Send_command(0x20 | 0x7);
  GMG12864_Send_command(0x28 | 0x7);
  //****** enter dynamic contrast mode ******//
  GMG12864_Send_command(0x81); // Electronic Volume
  GMG12864_Send_command(15);   // Contrast adjustment. Adjust on your display. Maximum 63.
  GMG12864_Send_command(0x40);
  //****** CMD_DISPLAY_ON  CMD_DISPLAY_OFF ******//
  GMG12864_Send_command(0xAF); // Display on
  //****** Inverting the screen ******//
  GMG12864_Send_command(0xA6); // 0xA6 - nomal, 0xA7 - revers
  cs_reset();
}
//------------------------------------------------------------------------------------
void ST7565_SetX(uint8_t x) {
  GMG12864_Send_command(((x)&0xF0) >> 4 | 0x10);
  GMG12864_Send_command((x)&0x0F);
}
//------------------------------------------------------------------------------------
void ST7565_SetY(uint8_t y) {
  GMG12864_Send_command(((y)&0x07) | 0xB0);
}
//------------------------------------------------------------------------------------
void GMG12864_Update(void) {
  cs_set();
  for (uint8_t y = 0; y < 8; y++) {
    ST7565_SetX(0);
    ST7565_SetY((int16_t)y);
    for (uint8_t x = 0; x < 128; x++) {
      GMG12864_Send_data(Frame_buffer[x + 128 * y]);
    }
  }
  cs_reset();
}
//------------------------------------------------------------------------------------

void GMG12864_Clean_Frame_buffer(void) {
  memset(Frame_buffer, 0x00, sizeof(Frame_buffer));
}

void GMG12864_Draw_pixel(int16_t x, int16_t y, uint8_t color) {
  if (x >= GMG12864_width || x < 0 || y >= GMG12864_height || y < 0) {
    return;
  } else {
    uint16_t array_pos = x + ((y / 8) * GMG12864_width);
    if (color) {
      Frame_buffer[array_pos] |= 1 << (y % 8);
    } else {
      Frame_buffer[array_pos] &= 0xFF ^ 1 << (y % 8);
    }
  }
}
//-------------------- Функция вывода символа на дисплей -----------------------------
void GMG12864_Print_symbol_5x7(uint8_t x, uint8_t y, uint16_t symbol, uint8_t inversion) {
  uint8_t x_start = x;    // начальное положение по x
  uint8_t x_stop = x + 5; // конечное положение по x с учетом межсимвольного интервала
  for (uint8_t x = x_start; x <= x_stop; x++) {
    if (x == x_stop) {                   // Заполняем межсимвольные интервалы
      for (uint8_t i = 0; i <= 6; i++) { // от 0 до 6, т.к. шрифт высотой 7 пикселей
        if (0x00 & (1 << i)) {
          if (inversion) {
            GMG12864_Draw_pixel(x, y + i, 0); // Закрасить пиксель
          } else {
            GMG12864_Draw_pixel(x, y + i, 1); // Закрасить пиксель
          }
        } else {
          if (inversion) {
            GMG12864_Draw_pixel(x, y + i, 1); // Очистить пиксель
          } else {
            GMG12864_Draw_pixel(x, y + i, 0); // Очистить пиксель
          }
        }
      }
    } else {                             // Заполняем полезной информацией
      for (uint8_t i = 0; i <= 6; i++) { // от 0 до 6, т.к. шрифт высотой 7 пикселей
        if (Font_5x7[(symbol * 5) + x - x_start] & (1 << i)) {
          if (inversion) {
            GMG12864_Draw_pixel(x, y + i, 0); // Закрасить пиксель
          } else {
            GMG12864_Draw_pixel(x, y + i, 1); // Закрасить пиксель
          }
        } else {
          if (inversion) {
            GMG12864_Draw_pixel(x, y + i, 1); // Очистить пиксель
          } else {
            GMG12864_Draw_pixel(x, y + i, 0); // Очистить пиксель
          }
        }
      }
    }
  }
}
//------------------------------------------------------------------------------------
//-------------------- Функция вывода символа на дисплей -----------------------------
void GMG12864_Print_symbol_3x5(uint8_t x, uint8_t y, uint16_t symbol, uint8_t inversion) {
  /// Функция вывода символа на дисплей
  uint8_t x_start = x;    // начальное положение по x
  uint8_t x_stop = x + 3; // конечное положение по x с учетом межсимвольного интервала
  for (uint8_t x = x_start; x <= x_stop; x++) {
    if (x == x_stop) {                   // Заполняем межсимвольные интервалы
      for (uint8_t i = 0; i <= 4; i++) { // от 0 до 6, т.к. шрифт высотой 7 пикселей
        if (0x00 & (1 << i)) {
          if (inversion) {
            GMG12864_Draw_pixel(x, y + i, 0); // Закрасить пиксель
          } else {
            GMG12864_Draw_pixel(x, y + i, 1); // Закрасить пиксель
          }
        } else {
          if (inversion) {
            GMG12864_Draw_pixel(x, y + i, 1); // Очистить пиксель
          } else {
            GMG12864_Draw_pixel(x, y + i, 0); // Очистить пиксель
          }
        }
      }
    } else {                             // Заполняем полезной информацией
      for (uint8_t i = 0; i <= 4; i++) { // от 0 до 6, т.к. шрифт высотой 7 пикселей
        if (Font_3x5[(symbol * 3) + x - x_start] & (1 << i)) {
          if (inversion) {
            GMG12864_Draw_pixel(x, y + i, 0); // Закрасить пиксель
          } else {
            GMG12864_Draw_pixel(x, y + i, 1); // Закрасить пиксель
          }
        } else {
          if (inversion) {
            GMG12864_Draw_pixel(x, y + i, 1); // Очистить пиксель
          } else {
            GMG12864_Draw_pixel(x, y + i, 0); // Очистить пиксель
          }
        }
      }
    }
  }
}
//------------------------------------------------------------------------------------
//--------------- Функция декодирования UTF-8 в набор символов -----------------------
void GMG12864_Decode_UTF8(uint8_t x, uint8_t y, uint8_t font, bool inversion, char *tx_buffer) {
  /// Функция декодирования UTF-8 в набор символов и последующее занесение в буфер кадра
  /// \param x - координата по х. От 0 до 127
  /// \param y - координата по y. от 0 до 7
  /// \param font - шрифт. 0 - 3x5, 1 - 5x7
  uint16_t symbol = 0;
  bool flag_block = 0;
  for (int i = 0; i < strlen(tx_buffer); i++) {
    if (tx_buffer[i] < 0xC0) { // Английский текст и символы. Если до русского текста, то [i] <0xD0. Но в font добавлен знак "°"
      if (flag_block) {
        flag_block = 0;
      } else {
        symbol = tx_buffer[i];
        if (font == font3x5) { // Если выбран шрифт размера 3x5
          if (inversion) {
            GMG12864_Print_symbol_3x5(x, y, symbol - 32, 1); // Таблица UTF-8. Basic Latin. С "пробел" до "z". Инверсия вкл.
          } else {
            GMG12864_Print_symbol_3x5(x, y, symbol - 32, 0); // Таблица UTF-8. Basic Latin. С "пробел" до "z". Инверсия выкл.
          }
          x = x + 4;
        } else if (font == font5x7) { // Если выбран шрифт размера 5x7
          if (inversion) {
            GMG12864_Print_symbol_5x7(x, y, symbol - 32, 1); // Таблица UTF-8. Basic Latin. С "пробел" до "z". Инверсия вкл.
          } else {
            GMG12864_Print_symbol_5x7(x, y, symbol - 32, 0); // Таблица UTF-8. Basic Latin. С "пробел" до "z". Инверсия выкл.
          }
          x = x + 6;
        }
      }
    }

    else { // Кириллица
      symbol = tx_buffer[i] << 8 | tx_buffer[i + 1];
      if (symbol < 0xD180 && symbol > 0xD081) {
        if (font == font3x5) { // Если выбран шрифт размера 3x5
          if (inversion) {
            GMG12864_Print_symbol_3x5(x, y, symbol - 53297, 1); // Таблица UTF-8. Кириллица. С буквы "А" до "п". Инверсия вкл.
          } else {
            GMG12864_Print_symbol_3x5(x, y, symbol - 53297, 0); // Таблица UTF-8. Кириллица. С буквы "А" до "п". Инверсия выкл.
          }
          x = x + 4;
        } else if (font == font5x7) { // Если выбран шрифт размера 5x7
          if (inversion) {
            GMG12864_Print_symbol_5x7(x, y, symbol - 53297, 1); // Таблица UTF-8. Кириллица. С буквы "А" до "п". Инверсия вкл.
          } else {
            GMG12864_Print_symbol_5x7(x, y, symbol - 53297, 0); // Таблица UTF-8. Кириллица. С буквы "А" до "п". Инверсия выкл.
          }
          x = x + 6;
        }
      } else if (symbol == 0xD081) {
        if (font == font3x5) { // Если выбран шрифт размера 3x5
          if (inversion) {
            GMG12864_Print_symbol_3x5(x, y, 159, 1); ////Таблица UTF-8. Кириллица. Буква "Ё". Инверсия вкл.
          } else {
            GMG12864_Print_symbol_3x5(x, y, 159, 0); ////Таблица UTF-8. Кириллица. Буква "Ё". Инверсия выкл.
          }
          x = x + 4;
        } else if (font == font5x7) { // Если выбран шрифт размера 5x7
          if (inversion) {
            GMG12864_Print_symbol_5x7(x, y, 159, 1); ////Таблица UTF-8. Кириллица. Буква "Ё". Инверсия вкл.
          } else {
            GMG12864_Print_symbol_5x7(x, y, 159, 0); ////Таблица UTF-8. Кириллица. Буква "Ё". Инверсия выкл.
          }
          x = x + 6;
        }
      } else if (symbol == 0xD191) {
        if (font == font3x5) { // Если выбран шрифт размера 3x5
          if (inversion) {
            GMG12864_Print_symbol_3x5(x, y, 160, 1); ////Таблица UTF-8. Кириллица. Буква "ё". Инверсия вкл.
          } else {
            GMG12864_Print_symbol_3x5(x, y, 160, 0); ////Таблица UTF-8. Кириллица. Буква "ё". Инверсия выкл.
          }
          x = x + 4;
        } else if (font == font5x7) { // Если выбран шрифт размера 5x7
          if (inversion) {
            GMG12864_Print_symbol_5x7(x, y, 160, 1); ////Таблица UTF-8. Кириллица. Буква "ё". Инверсия вкл.
          } else {
            GMG12864_Print_symbol_5x7(x, y, 160, 0); ////Таблица UTF-8. Кириллица. Буква "ё". Инверсия выкл.
          }
          x = x + 6;
        }
      } else if (symbol == 0xC2B0) {
        if (font == font3x5) { // Если выбран шрифт размера 3x5
          if (inversion) {
            GMG12864_Print_symbol_3x5(x, y, 161, 1); ////Таблица UTF-8. Basic Latin. Символ "°". Инверсия вкл.
          } else {
            GMG12864_Print_symbol_3x5(x, y, 161, 0); ////Таблица UTF-8. Basic Latin. Символ "°". Инверсия выкл.
          }
          x = x + 4;
        } else if (font == font5x7) { // Если выбран шрифт размера 5x7
          if (inversion) {
            GMG12864_Print_symbol_5x7(x, y, 161, 1); ////Таблица UTF-8. Basic Latin. Символ "°". Инверсия вкл.
          } else {
            GMG12864_Print_symbol_5x7(x, y, 161, 0); ////Таблица UTF-8. Basic Latin. Символ "°". Инверсия выкл.
          }
          x = x + 6;
        }
      }

      else {
        if (font == font3x5) { // Если выбран шрифт размера 3x5
          if (inversion) {
            GMG12864_Print_symbol_3x5(x, y, symbol - 53489, 1); // Таблица UTF-8. Кириллица. С буквы "р", начинается сдвиг. Инверсия вкл.
          } else {
            GMG12864_Print_symbol_3x5(x, y, symbol - 53489, 0); // Таблица UTF-8. Кириллица. С буквы "р", начинается сдвиг. Инверсия выкл.
          }
          x = x + 4;
        } else if (font == font5x7) { // Если выбран шрифт размера 5x7
          if (inversion) {
            GMG12864_Print_symbol_5x7(x, y, symbol - 53489, 1); // Таблица UTF-8. Кириллица. С буквы "р", начинается сдвиг. Инверсия вкл.
          } else {
            GMG12864_Print_symbol_5x7(x, y, symbol - 53489, 0); // Таблица UTF-8. Кириллица. С буквы "р", начинается сдвиг. Инверсия выкл.
          }
          x = x + 6;
        }
      }
      flag_block = 1;
    }
  }
}
//------------------------------------------------------------------------------------
//-------------------- Inversion function of any place in the buffer -----------------
void GMG12864_Inversion(uint16_t x_start, uint16_t x_end) {
  // Parameter --> x_start - начальная точка по х от 0 до 1024
  // Parameter --> x_end - конечная точка по y от 0 до 1024
  for (; x_start < x_end; x_start++) {
    Frame_buffer[x_start] = ~(Frame_buffer[x_start]);
  }
}
//------------------------------------------------------------------------------------
