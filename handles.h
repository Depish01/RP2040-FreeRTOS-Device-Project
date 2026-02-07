#pragma once

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>

// Очереди
extern QueueHandle_t xQueueSerial;
extern QueueHandle_t serialQueue;
extern QueueHandle_t flagsQueue;
extern QueueHandle_t buttonEventQueue;
extern QueueHandle_t displayEventQueue; // очередь дисплея
extern QueueHandle_t configQueue; // очередь обновления конфига
extern QueueHandle_t timerQueue; // очередь обновления конфига
extern QueueHandle_t xAlarmQueue; // очередь обновления конфига

// Мьютексы
extern xSemaphoreHandle xMutexADC;
extern xSemaphoreHandle xMutexIndicators;
extern xSemaphoreHandle xMutexPWM;