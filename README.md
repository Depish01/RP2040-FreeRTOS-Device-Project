# RP2040-FreeRTOS-Device-Project
RP2040, Arduino-pico, FreeRTOS (Display, Keyboard, Buzzer, EEPROM, ADC)

=========================================================================

This repository contains a program project based on the RP2040 microcontroller. The project consists of the following FreeRTOS tasks:
- vTaskConfigGatekeeper -- writes data to EEPROM memory and transfers updated configurations to other tasks
- vTaskKeyboardScan -- polls buttons on the matrix keyboard
- vTaskButtonFilter -- required due to keyboard hardware features
- vTaskDisplay -- UI/UX
- vTaskTimers -- monitors timer status
- vTaskADC -- polls the ADC
- vTaskFFT -- processes data from the ADC
- vTaskBattery -- polls the battery value
- vTaskAlarm -- buzzer control

Most of the functionality is implemented, but there are still some things that need to be fixed and features that can be added.

=========================================================================

Этот репозиторий содержит проект программы на базе микроконтроллера RP2040. Проект состоит из FreeRTOS задач:
- vTaskConfigGatekeeper -- отвечает за запись данных в память EEPROM и передачу обновлённых конфигураций в другие задачи
- vTaskKeyboardScan -- опрос кнопок на матричной клавиатуре
- vTaskButtonFilter -- необходима из-за hardware особенностей клавиатуры
- vTaskDisplay -- UI/UX 
- vTaskTimers -- отслеживает состояние таймеров
- vTaskADC -- опрос АЦП
- vTaskFFT -- обработка данных от АЦП
- vTaskBattery -- опрос значение батареи
- vTaskAlarm -- управление буззером

Большая часть функционала реализована, но всё еще есть моменты, которые необходимо исправить, и функции, которые можно добавить.
