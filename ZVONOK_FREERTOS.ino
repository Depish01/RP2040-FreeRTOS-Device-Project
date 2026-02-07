// /src
//  ├── main.c               // запуск FreeRTOS, handles_init(), создание задач
//  ├── handles.c            // все глобальные хендлы (очереди, мьютексы, события)
//  ├── handles.h
//  ├── tasks_buttons.c      // TaskButtonsScan + TaskButtonsProcess
//  ├── tasks_buttons.h
//  ├── tasks_keyhandler.c   // TaskKeyHandler
//  ├── tasks_keyhandler.h
//  ├── tasks_display.c      // TaskDisplay
//  ├── tasks_display.h
//  ├── tasks_adc.c          // TaskADC
//  ├── tasks_adc.h
//  ├── tasks_system.c       // TaskSystem (сон, блокировка)
//  ├── tasks_system.h
//  ├── tasks_alert.c        // TaskAlert (звуки, мигание)
//  ├── tasks_alert.h
//  └── utils/               // вспомогательные модули (например, драйвер дисплея)


#include "config.h"
#include "handles.h"
#include "graphics.h"
#include "task_display.h"
#include "task_keyboard.h"
#include "task_system.h"
#include "task_adc.h"
#include "task_alarm.h"


void setup() {
  pinMode(DISPLAY_LED, OUTPUT);
  digitalWrite(DISPLAY_LED, LOW);


  xQueueSerial      = xQueueCreate( 25, sizeof( String ) );
  serialQueue       = xQueueCreate( 10, sizeof( SerialMessage_t ) ); 
  flagsQueue        = xQueueCreate(  3, sizeof( FlagsMessage_t ) ); 
  buttonEventQueue  = xQueueCreate( 10, sizeof( ButtonEvent_t ) );
  displayEventQueue = xQueueCreate( 10, sizeof( DisplayEvent_t ) );
  configQueue       = xQueueCreate( 10, sizeof( ConfigMessage_t ) );
  timerQueue        = xQueueCreate( 10, sizeof( TimerMessage_t ) );
  xAlarmQueue       = xQueueCreate(  5, sizeof( AlarmMessage ) );

  xMutexADC = xSemaphoreCreateMutex();
  xMutexIndicators = xSemaphoreCreateMutex();
  xMutexPWM = xSemaphoreCreateMutex();

  // xTaskCreate(blink_task, "BLINK", 128, nullptr, 1, nullptr);
  xTaskCreate(serial_task, "SERIAL", 300, nullptr, 3, nullptr);

  xTaskCreate(vTaskConfigGatekeeper, "CONFIG", 300, nullptr, 3, nullptr);
  xTaskCreate(vTaskKeyboardScan, "KEYSCAN", 200, nullptr, 2, nullptr);
  xTaskCreate(vTaskButtonFilter, "KEYFILTER", 200, nullptr, 2, nullptr);
  xTaskCreate(vTaskDisplay, "DISPLAY", 300, nullptr, 2, nullptr);
  xTaskCreate(vTaskTimers, "TIMERS", 300, nullptr, 3, nullptr);
  xTaskCreate(vTaskADC, "ADC", 300, nullptr, 2, nullptr);
  xTaskCreate(vTaskFFT, "FFT", 300, nullptr, 2, nullptr);
  xTaskCreate(vTaskBattery, "BATTERY", 200, nullptr, 3, nullptr);
  xTaskCreate(vTaskAlarm, "ALARM", 200, nullptr, 3, nullptr);

  // xTaskCreate(tft_task, "TFT", 1000, nullptr, 3, nullptr);
  // xTaskCreate(tft_task_2, "TFT_2", 1000, nullptr, 3, nullptr);
}
char *pcWriteBuffer;
void loop() {
  // put your main code here, to run repeatedly:
  // vTaskList(pcWriteBuffer);
  // Serial.println(pcWriteBuffer);
  // ps();
  delay(2000);
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    Serial.printf("Stack overflow: %s\n", pcTaskName);
    while(1);
}

void blink_task(void *param) {
  pinMode(7, OUTPUT);
  for( ;; ) {
    digitalWrite(7, LOW);
    delay(750);
    digitalWrite(7, HIGH);
    delay(250);
    // xQueueSend(xQueueSerial, "Message", 0);
  }
}

#define androidAddr 0x0A
#define deviceAddr 0x0D
#define setRegimCipher 0xFB

void serial_task(void *param) {
  Serial.begin(115200);
  String Message;
  // char Message[10];
  ButtonEvent_t newkey;
  portBASE_TYPE xStatus;
  
  SerialMessage_t msg;
  byte incomingByte;
  byte raw[4];
  for( ;; ) {

    xStatus = xQueueReceive(xQueueSerial, &Message, 50);
    if (xStatus) Serial.println(Message);

    // ======== Приём данных от задач (на отправку) ========
    if (xQueueReceive(serialQueue, &msg, 50) == pdTRUE) { 
      switch (msg.type) {
        case MSG_TYPE_SAMPLES:
        
          (long &)raw = 0xFFFFFFFF;
          for (byte ii = 0; ii < 4; ii++) Serial.write(raw[ii]);
          
          Serial.write(2);
          
          (long &)raw = msg.length;  // samples;
          for (byte ii = 0; ii < 4; ii++) Serial.write(raw[ii]);
          
          Serial.write(msg.data, msg.length * sizeof(uint16_t));

          (long &)raw = 0x5A5A5A5A;  // Отправили признак завершения
          for (byte ii = 0; ii < 4; ii++) Serial.write(raw[ii]);

          break;

        case MSG_TYPE_FFT:

          (long &)raw = 0xFFFFFFFF;
          for (byte ii = 0; ii < 4; ii++) Serial.write(raw[ii]);

          Serial.write(3);

          (long &)raw = msg.length;
          for (byte ii = 0; ii < 4; ii++) Serial.write(raw[ii]);

          Serial.write(msg.data, msg.length * sizeof(float));

          (long &)raw = 0xA5A5A5A5;  //завершающие символы
          for (byte ii = 0; ii < 4; ii++) Serial.write(raw[ii]);
          (long &)raw = 0x5A5A5A5A;  // Отправили признак завершения
          for (byte ii = 0; ii < 4; ii++) Serial.write(raw[ii]);
          
          break;

        case MSG_TYPE_DRONS:

          Serial.print("Lvl24:\t");
          for (size_t i = 0; i < dronsNumber; i++) {
            int value = msg.data[i*4] | (msg.data[i*4+1] << 8) | (msg.data[i*4+2] << 16) | (msg.data[i*4+3] << 24);
            Serial.print(value);
            Serial.print(",\t");
          }

          Serial.print("\tLvl58:\t");
          size_t offset = dronsNumber * 4; // Lvl58 начинается после Lvl24
          for (size_t i = 0; i < dronsNumber; i++) {
            int value = msg.data[offset + i*4] | (msg.data[offset + i*4+1] << 8) | (msg.data[offset + i*4+2] << 16) | (msg.data[offset + i*4+3] << 24);
            Serial.print(value);
            Serial.print(",\t");
          }
          Serial.println();

          break;
      }
    }

    // ======== Приём данных из порта ========
    while (Serial.available() > 0) {
      incomingByte = Serial.read();
      if (incomingByte != androidAddr) continue;

      delay(5);
      incomingByte = Serial.read();
      if (incomingByte != deviceAddr) continue;

      delay(5);
      incomingByte = Serial.read();
      switch (incomingByte) {
        case setRegimCipher:
          delay(5);
          incomingByte = Serial.read();
          delay(5);
          incomingByte = Serial.read();

          delay(5);
          incomingByte = Serial.read();
          
          SetRegim(incomingByte);
          break;

        case 0xCC:
          SystemConfig_t serialConfig;
          GetConfig(&serialConfig);

          Serial.println("MinMax: " + String(serialConfig.FPV_Min) + ", " + String(serialConfig.FPV_Max));
          break;

        default:
          continue;
      }
      

      // char ch = Serial.read();
      // switch (ch) {
      //   case '1':
      //     F200 = true;
      //     break;
      //   case '0':
      //     F200 = false;
      //     break;
      //   case 'F':
      //     FFT_Send = !FFT_Send;  // переключаем
      //     break;
      //   case 'R':
      //     FR24to58 = !FR24to58;
      //     break;

      //   // Можно добавить и другие команды
      //   case '?':
      //     Serial.println("Commands: 1(on), 0(off), F(toggle FFT), R(toggle band)");
      //     break;
      // }
    }

  }
}

void SetRegim(byte mode) {
  static FlagsMessage_t flags = {false, false, false};

  flags.FFT_Send = (mode & 2);      // Выдавать FFT
  flags.Sampl_Send = (mode & 4);    // Выдавать сэмплы
  flags.FR24to58 = (mode & 8);      // Графики 5.8 вместо 2.4
  // F200 = (mode & 16);         // Графики FREQ_0-1
  
  xQueueSend(flagsQueue, &flags, portMAX_DELAY);
}




// void vTaskButtonHandler(void *pvParameters) {
//   ButtonEvent ev;
//   for (;;) {
//     if (xQueueReceive(cleanEventQueue, &ev, portMAX_DELAY) == pdTRUE) {
      
//     }
//   }
// }



// void tft_task(void *param) {
//   tft.init();
//   tft.setRotation(3);
//   tft.fillScreen(TFT_BLACK);
//   tft.setTextColor(TFT_WHITE, TFT_BLACK);
//   tft.setTextSize(2);
//   int text_x = 5;
//   int text_y = 10;
//   char Message;
//   ButtonEvent newkey;

//   for( ;; ) {
//     xQueueReceive(cleanEventQueue, &newkey, portMAX_DELAY);
//     if (newkey.buttonId < BASE_KEYS) {
//       int r = newkey.buttonId / COLS;
//       int c = newkey.buttonId % COLS;
//       Message = keyMap[r][c];
//     } else {
//       int extraIdx = newkey.buttonId - BASE_KEYS;
//       Message = extraKeyCode[extraIdx];
//     }
//     // Message = newkey.buttonId;

//     xSemaphoreTake( xMutexTFT, portMAX_DELAY );

//     if (text_y > 120) {
//       text_y = 10;
//       tft.fillScreen(TFT_BLACK);
//     }

//     tft.setCursor(text_x, text_y);
//     // tft.print((char)Message);
//     tft.print(Message);

//     text_x += 20;
//     if (text_x > 160) {
//       text_x = 5;
//       text_y += 20;
//     }

//     xSemaphoreGive( xMutexTFT );
//     delay(1);
//   }
// }


#include <map>
std::map<eTaskState, const char *> eTaskStateName { {eReady, "Ready"}, { eRunning, "Running" }, {eBlocked, "Blocked"}, {eSuspended, "Suspended"}, {eDeleted, "Deleted"} };
void ps() {
  int tasks = uxTaskGetNumberOfTasks();
  TaskStatus_t *pxTaskStatusArray = new TaskStatus_t[tasks];
  unsigned long runtime;
  tasks = uxTaskGetSystemState( pxTaskStatusArray, tasks, &runtime );
  Serial.printf("# Tasks: %d\n", tasks);
  Serial.println("ID, NAME, STATE, PRIO, CYCLES, STACK");
  for (int i=0; i < tasks; i++) {
    Serial.printf("%d: %-16s %-10s %d %-16lu %d\n", i, pxTaskStatusArray[i].pcTaskName, eTaskStateName[pxTaskStatusArray[i].eCurrentState], (int)pxTaskStatusArray[i].uxCurrentPriority, pxTaskStatusArray[i].ulRunTimeCounter, pxTaskStatusArray[i].usStackHighWaterMark);
  }
  delete[] pxTaskStatusArray;

  Serial.println();

  // size_t freeHeap = xPortGetFreeHeapSize();
  Serial.printf("Free heap: %u bytes\n", (unsigned int)rp2040.getFreeHeap());

  // Serial.printf("Free heap: %u bytes\n", (unsigned int)freeHeap);
}