#include "handles.h"

QueueHandle_t xQueueSerial;
QueueHandle_t serialQueue;
QueueHandle_t flagsQueue;    // флаги из TaskSerial → TaskFFT
QueueHandle_t buttonEventQueue;
QueueHandle_t displayEventQueue; // очередь дисплея
QueueHandle_t configQueue; // очередь обновления конфига
QueueHandle_t timerQueue; // очередь обновления конфига
QueueHandle_t xAlarmQueue; // очередь обновления конфига

xSemaphoreHandle xMutexADC; // мьютекс на использование АЦП и запись в EEPROM
xSemaphoreHandle xMutexIndicators; // мьютекс на работу с индикаторами
xSemaphoreHandle xMutexPWM;