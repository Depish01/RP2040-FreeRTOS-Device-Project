#include "task_keyboard.h"
#include "config.h"
#include "handles.h"

// пины строк (выходы) и столбцов (входы)
const uint8_t rowPins[ROWS] = {KEYPAD_X1, KEYPAD_X2, KEYPAD_X3, KEYPAD_X4};
const uint8_t colPins[COLS] = {KEYPAD_Y1, KEYPAD_Y2, KEYPAD_Y3, KEYPAD_Y4};
const uint8_t extraPins[EXTRA_KEYS] = {KEYPAD_Y1, KEYPAD_Y2, KEYPAD_Y3};

// карта символов 4x4 (можешь заменить на enum)
const uint8_t keyMap[ROWS][COLS] = {
  { '1', '2', '3', '*' },
  { '4', '5', '6', '0' },
  { '7', '8', '9', '#' },
  { 'U', 'D', 'G', 'R' }
};


// Коды для 3-х дополнительных кнопок (можно заменить на enum)
const uint8_t extraKeyCode[EXTRA_KEYS] = { 'A', 'V', 'S' }; // пример




void keyboardInit() {
  for (int r=0; r<ROWS; r++) {
    pinMode(rowPins[r], OUTPUT);
    digitalWrite(rowPins[r], 1);
  }

  for (int c=0; c<COLS; c++) {
    pinMode(colPins[c], INPUT_PULLUP);
  }
}

static inline void setAllRowsHigh(void) {
  for (int rr=0; rr<ROWS; rr++) digitalWrite(rowPins[rr], 1);
}


void vTaskKeyboardScan(void *pvParameters) {
  keyboardInit();
  uint32_t prevMask = 0;

    for (;;) {
        uint32_t mask = 0;
        bool extraPressed = false;

        // --- читаем доп. кнопки ---
        for (int e = 0; e < EXTRA_KEYS; e++) {
          bool pressed = (digitalRead(extraPins[e]) == KEY_PRESSED);
          if (pressed) {
              extraPressed = true;
              mask |= (1UL << (ROWS * COLS + e));
          }
        }

        // --- читаем матрицу ---
        if (!extraPressed) {
          for (int r = 0; r < ROWS; r++) {
            // for (int rr = 0; rr < ROWS; rr++) digitalWrite(rowPins[rr], HIGH);
            digitalWrite(rowPins[r], LOW);
            vTaskDelay(pdMS_TO_TICKS(1));

            for (int c = 0; c < COLS; c++) {
              bool pressed = (digitalRead(colPins[c]) == KEY_PRESSED);
              if (pressed) {
                mask |= (1UL << (r * COLS + c));
              }
            }
            setAllRowsHigh();
          }
        }

        // --- сравнение с прошлым состоянием ---
        uint32_t diff = mask ^ prevMask;
        if (diff) {
          for (int i = 0; i < ROWS * COLS + EXTRA_KEYS; i++) {
            if (diff & (1UL << i)) {
              ButtonEvent_t ev = {i, (mask & (1UL << i)) != 0};
              xQueueSend(buttonEventQueue, &ev, 0);
            }
          }
        }

        prevMask = mask;
        vTaskDelay(SCAN_PERIOD);
    }
}

#define LONG_PRESS_TIME   700  // мс
#define REPEAT_INTERVAL   120  // мс

void vTaskButtonFilter(void *pvParameters) {
    ButtonEvent_t buf[10];
    int count = 0;

    const uint32_t waitWindow = pdMS_TO_TICKS(40);
    uint32_t start = 0;

    uint32_t isPressedMask = 0;
    uint32_t longSendMask = 0;
    TickType_t pressTime[TOTAL_KEYS] = {0}; 
    TickType_t nextRepeatTime[TOTAL_KEYS] = {0};

    for (;;) {
        // uint32_t toWait = (count == 0) ? portMAX_DELAY : waitWindow - (xTaskGetTickCount() - start);

        uint32_t toWait;

        if (count == 0 && isPressedMask == 0) { // Окно приёма не началось и кнопка не зажата
            toWait = portMAX_DELAY;
        } 
        else if (count > 0) {         // Окно началось
            uint32_t elapsed = xTaskGetTickCount() - start;
            toWait = (elapsed < waitWindow) ? (waitWindow - elapsed) : 0;
        } 
        else {  // Зажата кнопка
            toWait = 100;
        }


        // Принимаем данные из задачи сканера кнопок
        // При нажатии экстра кнопок возникают ложные сработки кнопко на той же линии
        // Поэтому если приняли - ждём еще 40 мс, чтобы успели дойти ложные
        ButtonEvent_t ev;
        if (xQueueReceive(buttonEventQueue, &ev, toWait)) { 
          if (count == 0) start = xTaskGetTickCount();
          buf[count++] = ev;
          continue;
        }
        // таймаут — закрываем окно

        if (count > 0) {
          bool hasExtra = false;
          for (int i = 0; i < count; i++)
            if (buf[i].buttonId >= ROWS*COLS) hasExtra = true; 
          
          // Если в списке кнопок были экстракнопки, то посылаем только их
          for (int i = 0; i < count; i++) {
            if (!hasExtra || buf[i].buttonId >= ROWS*COLS) {  
              int newid = buf[i].buttonId;

              ButtonEvent2_t newbtn;
              newbtn.buttonId = newid;

              if (buf[i].pressed) {
                isPressedMask |= (1UL << newid);  // Выставляем 1 в маске нажатых кнопок
                longSendMask &= ~(1UL << newid);  // Выставляем 0 в маске отправок долгих нажатий
                pressTime[newid] = xTaskGetTickCount();        // Запоминаем время, когда была нажата
                newbtn.type = BUTTON_EVENT_PRESS;
              }
              else {
                isPressedMask &= ~(1UL << newid); // Выставляем 0 в маске нажатых кнопок
                newbtn.type = BUTTON_EVENT_RELEASE;
              } 

              DisplayEvent_t ev = {
                .type = DISPLAY_EVENT_BUTTON,
                .button = newbtn   // структура кнопки
              };
              xQueueSend(displayEventQueue, &ev, 0);

            }
          }
          count = 0;
        }

        // --- Проверяем удержание кнопок ---
        if (isPressedMask > 0) {  // Если хоть одна кнопка сейчас нажата
          // Serial.println("KEY" + String(isPressedMask, BIN));
          TickType_t now = xTaskGetTickCount();

          for (int id = 0; id < TOTAL_KEYS; id++) {
            if (!(isPressedMask & (1UL << id))) continue;  // Если не нажата идем дальше
            // Serial.println("KEY ID" + String(id));
            
            // - Обработка долгих нажатий -
            if (!(longSendMask & (1UL << id))) {        
              // Serial.println("KEY LONG " + String(id));
              // Долгое нажатие еще не регистрировалось
              if ((now - pressTime[id]) >= LONG_PRESS_TIME) {
                // Serial.println("KEY LONG " + String(id));

                ButtonEvent2_t btn_ev = { id, BUTTON_EVENT_LONG };
                DisplayEvent_t ev2 = {
                  .type = DISPLAY_EVENT_BUTTON,
                  .button = btn_ev   // структура кнопки
                };
                xQueueSend(displayEventQueue, &ev2, 0);

                longSendMask |= (1UL << id);   // Выставляем 1 в маске отправок долгих нажатий

                // назначаем время первого повтора
                nextRepeatTime[id] = now + REPEAT_INTERVAL;
              }
            } else {                                    
              // Долгое нажатие уже было обнаружено -- Автоповтор
              if (now >= nextRepeatTime[id]) {
                // Serial.println("KEY REPEAT " + String(id));

                ButtonEvent2_t btn_ev = { id, BUTTON_EVENT_REPEAT };
                DisplayEvent_t ev2 = {
                  .type = DISPLAY_EVENT_BUTTON,
                  .button = btn_ev   // структура кнопки
                };
                xQueueSend(displayEventQueue, &ev2, 0);

                nextRepeatTime[id] = now + REPEAT_INTERVAL;
              }
            }
          }
        }
    }
}
