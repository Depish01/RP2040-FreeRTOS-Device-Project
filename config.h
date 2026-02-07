#pragma once

#define FIRMWARE_VERSION "v1.00"

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>


#define KEYPAD_X1 10
#define KEYPAD_X2 11
#define KEYPAD_X3 12
#define KEYPAD_X4 13

#define KEYPAD_Y1 14
#define KEYPAD_Y2 15
#define KEYPAD_Y3 16
#define KEYPAD_Y4 17

#define KEY_PRESSED 0
#define KEY_UNPRESSED 1

#define ROWS 4
#define COLS 4
#define BASE_KEYS (ROWS*COLS)   // 16
#define EXTRA_KEYS 3            // прямые кнопки
#define TOTAL_KEYS (BASE_KEYS + EXTRA_KEYS)

#define SCAN_PERIOD pdMS_TO_TICKS(20)

#define CONFIG_MAGIC 0xEE // Значение для проверки памяти

#define dronsNumber 6
#define TIME_STRINGS_Q 5

#define signalMaxLevel 300      // Значение сигнала для максимальной шкалы
#define FREQ_0 2048

#define ALARM_COEFF_SCALE 50

// --- Структура события ---
typedef struct {
    uint8_t buttonId;   // номер кнопки (0..TOTAL_KEYS-1)
    bool pressed;       // true = нажата, false = отпущена
} ButtonEvent_t;

typedef enum {
  BUTTON_EVENT_RELEASE,
  BUTTON_EVENT_PRESS,
  BUTTON_EVENT_LONG,
  BUTTON_EVENT_REPEAT
} ButtonEventType;

typedef struct {
    uint8_t buttonId;     // номер кнопки (0..TOTAL_KEYS-1)
    ButtonEventType type; 
} ButtonEvent2_t;

typedef enum {
    TIMER_CONFIG_UPDATE,   // новые значения из конфига
    TIMER_RESET,           // сброс конкретного таймера
    TIMER_STOP             // остановка таймера
} TimerMsgType_t;

typedef enum {
    TIMER_KEYLOCK,   // таймер блокировки клавиатуры
    TIMER_SLEEP,         // таймер перехода в спящий режим
    TIMER_BATTERY,       // таймер проверки батареи
    TIMER_COUNT          // общее количество таймеров
} TimerId_t;

typedef struct {
    TimerMsgType_t type;
    TimerId_t timerId;   // какой именно таймер (если их несколько)
    uint8_t newValue; // для обновления/сброса
} TimerMessage_t;

typedef struct {
    uint16_t value;     // значение с АЦП
    uint8_t grade;      // оценка заряда
} BatteryMessage_t;

#define BUF_SIZE 2048
typedef struct {
    uint16_t ch24[BUF_SIZE];
    uint16_t ch58[BUF_SIZE];
    size_t length;  // сколько реально отсэмплировали
} Buffer_t;

typedef struct {
  int drons_24[dronsNumber];
  int drons_58[dronsNumber];
} DronsMessage_t;
    // int[2] Ocusinc2;
    // int[2] Ocu3;
    // int[2] Phantom4;
    // int[2] FPV;
    // int[2] Fimi;
    // int[2] Mini;

#define MAX_FREQS_PER_INDICATOR 16
#define MAX_INDICATORS 10

typedef struct {
    char name[16];        // Имя, например "Ocusinc2"
    uint16_t* freqs;      // Указатель на массив частот
    size_t freq_count;    // Кол-во частот
    int value_24;          // Текущее рассчитанное значение
    int value_58;          // Текущее рассчитанное значение
    int threshold_24;
    int threshold_58;
    int calibrationCoeff_24;
    int calibrationCoeff_58;
    bool isActive;
    bool isFPV;
    bool isConst;
} Indicator_t;

extern Indicator_t indicators[MAX_INDICATORS];
extern size_t indicator_count;

typedef enum {
    DISPLAY_EVENT_BUTTON,   // событие от кнопки
    DISPLAY_EVENT_TIMER,    // событие от таймера
    DISPLAY_EVENT_BATTERY,  // событие от батареи
    DISPLAY_EVENT_DRONS,  // событие от FFT
    DISPLAY_EVENT_OSCIL
    // можно добавлять новые события (например, сообщения от UART)
} DisplayEventType_t;

typedef struct {
    DisplayEventType_t type;
    union {
        ButtonEvent2_t button;   // событие кнопки
        TimerMessage_t timer;   // событие таймера
        BatteryMessage_t battery;
        DronsMessage_t drons;
        Buffer_t *adcbuffer;
    };
} DisplayEvent_t;

typedef enum {
  MODE_NORMAL,
  MODE_LOCKED,
  MODE_SLEEP,
  MODE_SETTINGS,
  MODE_SUBSETTINGS,
  MODE_SETTINGS_ADMIN,
  MODE_THRESHOLD,
  MODE_COUNT
} DeviceMode;

typedef enum {
    KEY_1, KEY_2, KEY_3, KEY_STAR,
    KEY_4, KEY_5, KEY_6, KEY_0,
    KEY_7, KEY_8, KEY_9, KEY_HASH,
    KEY_UP, KEY_DOWN, KEY_GREEN, KEY_RED,
    KEY_AB, KEY_VFO, KEY_BRACKETS,
    KEY_COUNT
} KeyCode;

typedef struct {
  uint8_t magic;
  // uint8_t version;
  // uint8_t versionMinor;
  // uint8_t versionBuild;
  // uint8_t versionDate;
  uint16_t limit24;
  uint16_t limit58;

  uint8_t brightness;
  uint8_t blockChoice;
  uint8_t sleepChoice;
  uint8_t language;
  uint8_t averCoeff;
  bool logoFlag;
  bool soundOffFlag;

  uint16_t FPV_Min;
  uint16_t FPV_Max;

  uint8_t calibrNumber;
  uint8_t calibrationCoeff24[dronsNumber];
  uint8_t calibrationCoeff58[dronsNumber];
} SystemConfig_t;

// extern SystemConfig_t gConfig;

void vTaskConfigGatekeeper(void *pvParameters);
void GetConfig(SystemConfig_t*);

typedef enum {
    CONFIG_GET,
    CONFIG_SET
} ConfigMsgType;

typedef enum {
    CFG_LIMIT24,
    CFG_LIMIT58,
    CFG_BRIGHTNESS,
    CFG_BLOCK_CHOICE,
    CFG_SLEEP_CHOICE,
    CFG_LANGUAGE,
    CFG_AVER_COEFF,
    CFG_LOGO_FLAG,
    CFG_SOUND_OFF_FLAG,
    CFG_FPV_MIN,
    CFG_FPV_MAX,
    CFG_CALIBR_NUMBER,
    CFG_CALIBR_COEFF24,
    CFG_CALIBR_COEFF58
} ConfigField;

typedef struct {
  ConfigMsgType type;
  ConfigField field;        // Измененное поле
  QueueHandle_t replyQueue;  // Очередь для ответа (для GET)
  union {
    uint16_t limit24;
    uint16_t limit58;

    uint8_t brightness;
    uint8_t blockChoice;
    uint8_t sleepChoice;
    uint8_t language;
    uint8_t averCoeff;
    bool logoFlag;
    bool soundOffFlag;

    uint16_t FPV_Min;
    uint16_t FPV_Max;

    uint8_t calibrNumber;
    uint8_t calibrationCoeff24[dronsNumber];
    uint8_t calibrationCoeff58[dronsNumber];
  } value;
} ConfigMessage_t;

typedef enum {
  MSG_TYPE_SAMPLES,
  MSG_TYPE_FFT,
  MSG_TYPE_DRONS
} SerialMsgType_t;

typedef struct {
  SerialMsgType_t type;
  size_t length;
  uint8_t* data; // или float*, если используешь float
} SerialMessage_t;

// ==== Структура для передачи флагов ====
typedef struct {
  bool Sampl_Send;
  bool FFT_Send;
  bool FR24to58;
} FlagsMessage_t;

// ================= Типы =================
enum AlarmMode {
  ALARM_NONE = 0,
  ALARM_FPV,
  ALARM_OTHER
};

struct AlarmMessage {
  AlarmMode mode;
  uint8_t coeff;
  bool soundOff;
};

