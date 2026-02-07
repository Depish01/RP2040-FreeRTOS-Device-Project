#include <Arduino.h>
#include "task_adc.h"
#include "handles.h"
#include "config.h"

// ================= Пины =================
#define PIN_BUZZER   6
#define PIN_LED_RED  7
#define PIN_LED_GREEN 8

// ================= Константы =================
constexpr uint16_t PWM_TIME_FPV    = 500;
constexpr uint16_t PWM_TIME_OTHER  = 500;
constexpr uint16_t LED_TIME        = 500;

constexpr uint8_t BUZZER_DUTY_FPV   = 40;
constexpr uint8_t BUZZER_DUTY_OTHER = 50;

constexpr uint16_t FREQ_FPV   = 1500;
constexpr uint16_t FREQ_OTHER = 800;

constexpr uint8_t COEFF_MIN = 1;
constexpr uint8_t COEFF_MAX = 10;

// // ================= Типы =================
// enum AlarmMode {
//   ALARM_NONE = 0,
//   ALARM_FPV,
//   ALARM_OTHER
// };

// struct AlarmMessage {
//   AlarmMode mode;
//   uint8_t coeff;
//   bool soundOff;
// };

// ================= Глобальные объекты =================
// static QueueHandle_t xAlarmQueue = nullptr;

// ================= Утилиты =================
inline uint8_t clampCoeff(uint8_t c) {
  return (c < COEFF_MIN) ? COEFF_MIN : (c > COEFF_MAX ? COEFF_MAX : c);
}

inline uint32_t halfPeriodMs(AlarmMode mode, uint8_t coeff) {
  coeff = clampCoeff(coeff);
  return (mode == ALARM_FPV) ? (PWM_TIME_FPV / coeff) : (PWM_TIME_OTHER / coeff);
}

// ================= Низкоуровневое управление =================
void buzzerOff() {
  digitalWrite(PIN_BUZZER, LOW);
}

void buzzerOn(AlarmMode mode) {
  if (xSemaphoreTake(xMutexPWM, portMAX_DELAY)) {
    const uint8_t duty = (mode == ALARM_FPV) ? BUZZER_DUTY_FPV : BUZZER_DUTY_OTHER;
    const uint16_t freq = (mode == ALARM_FPV) ? FREQ_FPV : FREQ_OTHER;
    analogWriteFreq(freq);
    analogWrite(PIN_BUZZER, duty);
    xSemaphoreGive(xMutexPWM);
  }
}

void ledsOff() {
  digitalWrite(PIN_LED_RED, LOW);
  digitalWrite(PIN_LED_GREEN, LOW);
}

void ledsSet(AlarmMode mode, bool on) {
  if (mode == ALARM_FPV) {
    digitalWrite(PIN_LED_GREEN, LOW);
    digitalWrite(PIN_LED_RED, on ? HIGH : LOW);
  } else if (mode == ALARM_OTHER) {
    digitalWrite(PIN_LED_RED, LOW);
    digitalWrite(PIN_LED_GREEN, on ? HIGH : LOW);
  }
}

// ================= Инициализация =================
void setupAlarmSystem() {
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  buzzerOff();
  ledsOff();
}

// ================= Основная задача =================
void vTaskAlarm(void *param) {
  setupAlarmSystem();
  AlarmMessage msg{ALARM_NONE, 1, false};
  AlarmMessage newMsg;
  bool ledState = true;

  for (;;) {

    while(xQueueReceive(xAlarmQueue, &newMsg, 0) == pdTRUE) {
      msg = newMsg;
      msg.coeff = clampCoeff(msg.coeff);
    }

    if (msg.mode == ALARM_NONE) {
      buzzerOff();
      ledsOff();
      xQueueReceive(xAlarmQueue, &msg, portMAX_DELAY);
      msg.coeff = clampCoeff(msg.coeff);
      continue;
    }

    // Установить текущее состояние
    ledsSet(msg.mode, ledState);
    if (!msg.soundOff && ledState) buzzerOn(msg.mode);
    else buzzerOff();

    delay(halfPeriodMs(msg.mode, msg.coeff));

    ledState = !ledState;

    // delay(1000);
  }
}



// ================= Отправка =================
bool sendAlarm(AlarmMode mode, uint8_t coeff, bool soundOff) {
  if (!xAlarmQueue) return false;
  AlarmMessage m{mode, clampCoeff(coeff), soundOff};
  return xQueueSend(xAlarmQueue, &m, 10) == pdTRUE;
}

// ================= Пример использования =================
// void setup() {
//   Serial.begin(115200);
//   setupAlarmSystem();

//   sendAlarm(ALARM_NONE, 1, false);
//   delay(2000);
//   sendAlarm(ALARM_FPV, 5, false);
//   delay(8000);
//   sendAlarm(ALARM_OTHER, 2, false);
//   delay(8000);
//   sendAlarm(ALARM_OTHER, 4, true);
//   delay(8000);
//   sendAlarm(ALARM_NONE, 1, false);
// }

