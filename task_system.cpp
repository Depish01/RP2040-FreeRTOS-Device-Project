#include "task_system.h"
#include "config.h"
#include "handles.h"

typedef struct {
    bool active;
    uint32_t timeoutMs;    // сколько должен работать
    TickType_t startTick;  // отметка времени старта
} TimerState_t;



byte timers_values [TIME_STRINGS_Q] = {
  0,  5,  10,  15,  30    //Seconds
};

#define BATTERY_TIME 30000

void vTaskTimers(void *pvParameters) {

  SystemConfig_t timersConfig;
  GetConfig(&timersConfig);
  
  TimerState_t timers[TIMER_COUNT];
  
  // Инициализация таймеров
  for (int i = 0; i < TIMER_COUNT; i++) {
    timers[i].active = false;
    timers[i].timeoutMs = 0;
    timers[i].startTick = 0;
  }

  if (timersConfig.blockChoice != 0) {
    timers[TIMER_KEYLOCK].active    = true;
    timers[TIMER_KEYLOCK].timeoutMs = timers_values[timersConfig.blockChoice] * 1000;
  }

  if (timersConfig.sleepChoice != 0) {
    timers[TIMER_SLEEP].active    = true;
    timers[TIMER_SLEEP].timeoutMs = timers_values[timersConfig.sleepChoice] * 1000;
  }

  timers[TIMER_BATTERY].active    = true;
  timers[TIMER_BATTERY].timeoutMs = BATTERY_TIME;

  TimerMessage_t msg;

  for (;;) {
    // Ожидание сообщений с таймаутом (например, 100 мс)
    if (xQueueReceive(timerQueue, &msg, pdMS_TO_TICKS(100)) == pdPASS) {
      switch (msg.type) {
        case TIMER_CONFIG_UPDATE:
          // обновляем значения из конфига
          timers[msg.timerId].timeoutMs = timers_values[msg.newValue] * 1000 ;
          break;

        case TIMER_RESET:
          if (timers[msg.timerId].timeoutMs == 0) break;
          timers[msg.timerId].startTick = xTaskGetTickCount();
          timers[msg.timerId].active = true;
          break;

        case TIMER_STOP:
          timers[msg.timerId].active = false;
          break;
      }
    }

    // Проверяем активные таймеры
    TickType_t now = xTaskGetTickCount();
    for (int i = 0; i < TIMER_COUNT; i++) {
      if (timers[i].active) {
        if ((now - timers[i].startTick) >= pdMS_TO_TICKS(timers[i].timeoutMs)) {
          timers[i].active = false;

          TimerMessage_t msg = {
            .timerId = (TimerId_t)i
          };
          
          // Отправляем сообщение дисплею о смене режима
          DisplayEvent_t dmsg = {
            .type = DISPLAY_EVENT_TIMER,
            .timer = msg   // или что у тебя определяет режим
          };
          xQueueSend(displayEventQueue, &dmsg, 0);

          // String message = "Timer" + String(i);
          // xQueueSend(xQueueSerial, &message, 0);
        }
      }
    }
  }
}
