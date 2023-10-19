* [Обзор](#Обзор)
* [Основные возможности портов ввода вывода](#Основные-возможности-портов-ввода-вывода)
* [Register Map Port A](#Register-Map-Port-A)
* [Mode Control](#Mode-Control)
* [Digital Input Path Disable Control](#Digital-Input-Path-Disable-Control)
* [Data Output Value](#Data-Output-Value)
* [Data Output Write Mask](#Data-Output-Write-Mask)
### Обзор   
Чип серии M031(М032) может имееть до 111 контактов ввода-вывода общего назначения, которые могут использоваться совместно
с другими функциональными контактами в зависимости от конфигурации чипа. Эти 111 контактов расположены в 5 портах, названных
*PA, PB, PC, PD, PE, PF, PG и PH.*
***
### Основные возможности портов ввода вывода  
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
### Register Map Port A  
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
### Mode Control
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
 ### Digital Input Path Disable Control  
 Port A-H Digital Input Path Disable Control (Px_DINOFF)  
| Register|Offset        |R/W|Description                           |Reset Value |
|---------|-------------|----|--------------------------------------|------------|
|PA_DINOFF|GPIO_BA+0x004 |R/W| PA Digital Input Path Disable Control| 0x0000_0000|
|PB_DINOFF|GPIO_BA+0x044 |R/W| PB Digital Input Path Disable Control| 0x0000_0000|
|PC_DINOFF|GPIO_BA+0x084 |R/W| PC Digital Input Path Disable Control| 0x0000_0000|
|PD_DINOFF|GPIO_BA+0x0C4 |R/W| PD Digital Input Path Disable Control| 0x0000_0000|
|PE_DINOFF|GPIO_BA+0x104 |R/W| PE Digital Input Path Disable Control| 0x0000_0000|
|PF_DINOFF|GPIO_BA+0x144 |R/W| PF Digital Input Path Disable Control| 0x0000_0000|
|PG_DINOFF|GPIO_BA+0x184 |R/W| PG Digital Input Path Disable Control| 0x0000_0000|
|PH_DINOFF|GPIO_BA+0x1C4 |R/W| PH Digital Input Path Disable Control| 0x0000_0000|  

|31    |30    |29    |28    |27    |26    |25    | 24   |
|:----:|:----:|:----:|:----:|:----:|:----:|:----:|:----:|
|DINOFF|DINOFF|DINOFF|DINOFF|DINOFF|DINOFF|DINOFF|DINOFF|
|23    |22    |21    |20    |19    |18    |17    |16    |
|DINOFF|DINOFF|DINOFF|DINOFF|DINOFF|DINOFF|DINOFF|DINOFF|
|15    |14    |13    |12    |11    |10    |9     |8     |
|Reserved|Reserved|Reserved|Reserved|Reserved|Reserved|Reserved|Reserved|
|7     |6     |5     |4     |3     |2      |1    |0     |
|Reserved|Reserved|Reserved|Reserved|Reserved|Reserved|Reserved|Reserved|  

|Bits    |Description |Description|
|:------:|:----------:|:--------------------------------------------------------------------------------------------|
|[n+16]  | DINOFF[n]  |Port A-H Pin[n] Digital Input Path Disable Bit|                                              |
|n=0,1..15|            |Each of these bits is used to control if the digital input path of corresponding Px.n pin is |
|        |            |disabled. If input is analog signal, users can disable Px.n digital input path to avoid input|
|        |            |current leakage.                                                                             |
|        |            |*0* = Px.n digital input path Enabled.                                                       |
|        |            |*1* = Px.n digital input path Disabled (digital input tied to low).                          |
|[15:0]  |Reserved    |Reserved                                                                                     |  

**Note1:**   
*The PC.15/PF.12-13/PG.0-1,5-8/PH.0-3,12-15 pin is ignored.*  
***  
### Data Output Value  
Port A-H Data Output Value (Px_DOUT)  
|Register | Offset      |R/W |Description                           | Reset Value|
|:-------:|:-----------:|:--:|:-------------------------------------|:----------:|
|PA_DOUT  |GPIO_BA+0x008|R/W | PA Data Output Value                 | 0x0000_FFFF|
|PB_DOUT  |GPIO_BA+0x048|R/W | PB Data Output Value                 | 0x0000_FFFF|
|PC_DOUT  |GPIO_BA+0x088|R/W | PC Data Output Value                 | 0x0000_7FFF|
|PD_DOUT  |GPIO_BA+0x0C8|R/W | PD Data Output Value                 | 0x0000_FFFF|
|PE_DOUT  |GPIO_BA+0x108|R/W | PE Data Output Value                 | 0x0000_FFFF|
|PF_DOUT  |GPIO_BA+0x148|R/W | PF Data Output Value                 | 0x0000_CFFF|
|PG_DOUT  |GPIO_BA+0x188|R/W | PG Data Output Value                 | 0x0000_FE1C|
|PH_DOUT  |GPIO_BA+0x1C8|R/W | PH Data Output Value                 | 0x0000_0FF0|  

|  31  | 30   |  29  |  28  |  27  |  26  |  25  |  24  |
|:----:|:----:|:----:|:----:|:----:|:----:|:----:|:----:|
|Reserved|Reserved|Reserved|Reserved|Reserved|Reserved|Reserved|Reserved|
|23    |22    |21    |20    |19    |18    |17    |16    |
|Reserved|Reserved|Reserved|Reserved|Reserved|Reserved|Reserved|Reserved|
|15    |14    |13    |12    |11    |10    |9     |8     |
| DOUT | DOUT | DOUT | DOUT | DOUT | DOUT | DOUT | DOUT |
|7     |6     |5     |4     |3     |2      |1    |0     |
| DOUT | DOUT | DOUT | DOUT | DOUT | DOUT | DOUT | DOUT |  

|Bits     |Description |Description|
|:-------:|:----------:|:--------------------------------------------------------------------------------------------|
|[31:16]  |Reserved    |Reserved                                                                                     |
|         |            |**Port A-H Pin[n] Output Value**                                                             |
|         |            |Each of these bits controls the status of a Px.n pin when the Px.n is configured as          |
|         |            |Pushpull output, Open-drain output or Quasi-bidirectional mode.                              |
|[n]      | DOUT[n]    |*0* = Px.n will drive Low if the Px.n pin is configured as Push-pull output, Open-drain      |
|n=0,1..15|            |output or Quasi-bidirectional mode.                                                          |
|         |            |*1* = Px.n will drive High if the Px.n pin is configured as Push-pull output or              |  
|         |            |Quasibidirectional mode.

**Note1:**   
*The PC.15/PF.12-13/PG.0-1,5-8/PH.0-3,12-15 pin is ignored.*  
***  
### Data Output Write Mask  
Port A-H Data Output Write Mask (Px_DATMSK)  
|Register | Offset      |R/W |Description                           | Reset Value|
|:-------:|:-----------:|:--:|:-------------------------------------|:----------:|
|PA_DATMSK|GPIO_BA+0x00C| R/W| PA Data Output Write Mask            | 0x0000_0000|
|PB_DATMSK|GPIO_BA+0x04C| R/W| PB Data Output Write Mask            | 0x0000_0000|
|PC_DATMSK|GPIO_BA+0x08C| R/W| PC Data Output Write Mask            | 0x0000_0000|
|PD_DATMSK|GPIO_BA+0x0CC| R/W| PD Data Output Write Mask            | 0x0000_0000|
|PE_DATMSK|GPIO_BA+0x10C| R/W| PE Data Output Write Mask            | 0x0000_0000|
|PF_DATMSK|GPIO_BA+0x14C| R/W| PF Data Output Write Mask            | 0x0000_0000|
|PG_DATMSK|GPIO_BA+0x18C| R/W| PG Data Output Write Mask            | 0x0000_0000|
|PH_DATMSK|GPIO_BA+0x1CC| R/W| PH Data Output Write Mask            | 0x0000_0000|
