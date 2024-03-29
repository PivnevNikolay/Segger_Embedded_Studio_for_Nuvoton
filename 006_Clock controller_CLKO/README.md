* [Обзор](#Обзор)
* [High Speed External Crystal Oscillator](#High-Speed-External-Crystal-Oscillator)

### Обзор  
Блок управления тактовой частотой *Clock Controller*. Обеспечивает функции высокоскоростного внутреннего RC-генератора (48 MHz internal high speed RC oscillator (*HIRC*)), высокоскоростного внешнего кварцевого генератора (4~32 MHz external high speed crystal oscillator (*HXT*)), низкоскоростного внутреннего RC-генератора (38.4 kHz internal low speed RC oscillator (*LIRC*)). Тактовые частоты *PCLK0*, *PCLK1* и *HCLK* являются производными от системных часов, которые могут поступать от *HIRC*, *HXT* или *LIRC*.  
***
### High Speed External Crystal Oscillator  
Высокоскоростной внешний кварцевый генератор – HXT. Высокоскоростной внешний кварцевый генератор с частотой от 4 до 32 МГц (HSE) обеспечивает высокоточный
источник тактовых импульсов для системных часов. Кристалл с определенной частотой должен быть размещен на двух выводах XT1 (XT1_IN/XT1_OUT), и для обеспечения его надлежащих колебаний необходимы внешние компоненты, такие как резисторы и конденсаторы.  
Приведены следующие рекомендации по повышению стабильности схемы кристалла на печатной плате:  
* Кварцевый генератор должен располагаться как можно ближе к микроконтроллеру,чтобы длина трассировки была как можно короче для уменьшения любых паразитных емкостей.  
* Экранируйте все линии вблизи кристалла, используя плоскость заземления для изоляции сигналов и уменьшения шума.  
* Часто переключаемые сигнальные линии должны распологаться вдали от области кристалла, чтобы предотвратить перекрестные помехи.  
