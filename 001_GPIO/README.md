* [Обзор](#Обзор)
* [Основные возможности портов ввода вывода](#Основные-возможности-портов-ввода-вывода)
* [Register Map Port A](#Register-Map-Port-A)
* [Mode Control](#Mode-Control)  
### Обзор   
Чип серии M031(М032) может имееть до 111 контактов ввода-вывода общего назначения, которые могут использоваться совместно
с другими функциональными контактами в зависимости от конфигурации чипа. Эти 111 контактов расположены в 5 портах, названных
*PA, PB, PC, PD, PE, PF, PG и PH.*
***
# Основные возможности портов ввода вывода  
- Каждый порт и каждый бит порта может быть настроен независимо от настроек других портов, выводов.
- Режим ввода: плавающий вход.
- Режим вывода: выход push-pull или open-drain.
- Отдельные регистры для входных и выходных данных.
- Возможность настройки отдельных выводов как источник внешних прерываний.
- Альтернативные функции ввода - вывода для встроенных в микроконтроллер переферийных узлов.
- Quasi-Bidirectional Квазидвунаправленный тип вывода порта может использоваться в качестве функции вывода и ввода без
  перенастройки состояния вывода порта.
- Отключение входного триггера Шмидта при использовании вывода для АЦП.  
- При сбросе микроконтроллера, все порты настроены в зависимости от CONFIG0(CIOINI[10]).  
*0 = All GPIO set as Quasi-bidirectional mode.*  
*1 = All GPIO set as input tri-state mode.*
  
*Примечание* : Блок конфигурации пользователя - это внутренняя программируемая область конфигурации для параметров загрузки, таких как 
блокировка безопасности флэш-памяти, выбор загрузки, уровень напряжения отключения и базовый адрес флэш-памяти данных. Он работает как предохранитель для включения питания. Он загружается из флэш-памяти в соответствующие управляющие регистры при включении питания микросхемы. Пользователь может установить эти биты в соответствии с различными требованиями приложения.
Блок конфигурации пользователя может быть обновлен функцией ISP, и его адрес, расположенный в 0x0030_0000, состоит из трех 32-битных слов (CONFIG0, CONFIG1 и CONFIG2). Любое изменение в блоке конфигурации пользователя вступит в силу после перезагрузки системы.  
***
# Register Map Port A  
R: read only, W: write only, R/W: both read and write
|Register   |    Offset   | R/W   |          Description                |   Reset Value  |
|:---------:|:-----------:|:-----:|:-----------------------------------:|:--------------:|
|PA_MODE    |GPIO_BA+0x000| R/W   |PA I/O Mode Control                  |   0xXXXX_XXXX  |
|PA_DINOFF  |GPIO_BA+0x004| R/W   |PA Digital Input Path Disable Control|   0x0000_0000  |
|PA_DOUT    |GPIO_BA+0x008| R/W   |PA Data Output Value                 |   0x0000_FFFF  |
|PA_DATMSK  |GPIO_BA+0x00C| R/W   |PA Data Output Write Mask            |   0x0000_0000  |
|PA_PIN     |GPIO_BA+0x010| R     |PA Pin Value                         |   0x0000_XXXX  |
|PA_DBEN    |GPIO_BA+0x014| R/W   |PA De-bounce Enable Control Register |   0x0000_0000  |
|PA_INTTYPE |GPIO_BA+0x018| R/W   |PA Interrupt Trigger Type Control    |   0x0000_0000  |
|PA_INTEN   |GPIO_BA+0x01C| R/W   |PA Interrupt Enable Control Register |   0x0000_0000  |
|PA_INTSRC  |GPIO_BA+0x020| R/W   |PA Interrupt Source Flag             |   0x0000_XXXX  |  

Карта регистров идентична для остальных портов микроконтроллера *(PB, PC, PD, PE, PF, PG и PH)*.  
***
#  Mode Control
Port A-H I/O Mode Control (Px_MODE)  
 | Register|    Offset   |R/W|    Description    | Reset Value |
 |---------|-------------|---|-------------------|-------------|
 | PA_MODE |GPIO_BA+0x000|R/W|PA I/O Mode Control| 0xXXXX_XXXX |
 | PB_MODE |GPIO_BA+0x040|R/W|PB I/O Mode Control| 0xXXXX_XXXX |
 | PC_MODE |GPIO_BA+0x080|R/W|PC I/O Mode Control| 0xXXXX_XXXX |
 | PD_MODE |GPIO_BA+0x0C0|R/W|PD I/O Mode Control| 0xXXXX_XXXX |
 | PE_MODE |GPIO_BA+0x100|R/W|PE I/O Mode Control| 0xXXXX_XXXX |
 | PF_MODE |GPIO_BA+0x140|R/W|PF I/O Mode Control| 0xXXXX_XXXX |
 | PG_MODE |GPIO_BA+0x180|R/W|PG I/O Mode Control| 0xXXXX_XXXX |
 | PH_MODE |GPIO_BA+0x1C0|R/W|PH I/O Mode Control| 0xXXXX_XXXX |

 | 31 30 | 29 28 | 27 26 | 25 24 |
 |:-----:|:-----:|:-----:|:-----:|
 |MODE15 |MODE14 |MODE13 |MODE12 |
 | 23 22 | 21 20 | 19 18 | 17 16 |
 |MODE11 |MODE10 |MODE9  |MODE8  |
 |15 14  |13 12  |11 10  | 9 8   |
 |MODE7  |MODE6  |MODE5  |MODE4  |
 |  7 6  |  5 4  |  3 2  |  1 0  |
 | MODE3 | MODE2 | MODE1 | MODE0 |  

 |   Bits  |Field     |Descriptions                                                                                                   |
 |---------|----------|:--------------------------------------------------------------------------------------------------------------|
 |[2n+1:2n]|MODEn     |**_Port A-F I/O Pin[n] Mode Control_**                                                                           |
 |n=0,1..15|          |*Determine each I/O mode of Px.n pins.*                                                                          |
 |         |          |*00* = Px.n is in Input mode.                                                                                    |
 |         |          |*01* = Px.n is in Push-pull Output mode.                                                                         |
 |         |          |*10* = Px.n is in Open-drain Output mode.                                                                        |
 |         |          |*11* = Px.n is in Quasi-bidirectional mode.                                                                      |
 
 **Note1:**  
 *The initial value of this field is defined by CIOINI (CONFIG0 [10]). If CIOINI is set to 0, the default value is 0xFFFF_FFFF and all pins will be quasi-bidirectional mode after 
 chip powered on. If CIOINI is set to 1, the default value is 0x0000_0000 and all pins will be input mode after chip   
 powered on.*  
 **Note2:**  
 *The PC.8-13,15/PD.4-14/PF.7-13 pin is ignored.*
 ***
