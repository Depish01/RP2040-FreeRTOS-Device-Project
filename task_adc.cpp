#include "task_adc.h"
#include "config.h"
#include "handles.h"

#include "ADCInput.h"
ADCInput adc(A0, A1);

#include "arduinoFFT.h"
arduinoFFT FFT = arduinoFFT(); 

#define FREQ_1 200000

//ANCHOR - ЧАСТОТЫ
#define freqsNum2048 6
uint16_t Ocusinc2[] = { 100, 200, 300, 500, 301, 501, 700, 701};
uint16_t Ocu3[] =     { 200, 199, 201, 300, 299, 301, 400, 398, 399, 401, 402, 500, 498, 499, 501, 502};
uint16_t Phantom4[] = { 44, 48, 200, 400, 600, 800, 1000};
uint16_t FPV[] =      { 50, 100, 150, 60, 120, 180, 649, 709, 750};
uint16_t Fimi[] =     { 71, 72, 143, 214, 286, 357, 428, 429, 643};
uint16_t Mini[] =     { 30, 60, 90};

#define freqsNum200 2
uint16_t Phantom4_200[] = { 8, 10, 20, 21, 31, 41, 51 };
uint16_t FPV_200[] = { 160, 320, 480, 640, 161, 322, 483, 484, 644, 645 };

uint16_t* FREQUENCIES[freqsNum2048 + freqsNum200] = { 
  Ocusinc2, Ocu3, Phantom4, FPV, Fimi, Mini, Phantom4_200, FPV_200
};

size_t FREQUENCIES_LEN[freqsNum2048 + freqsNum200] = {
  sizeof(Ocusinc2) / sizeof(uint16_t),
  sizeof(Ocu3) / sizeof(uint16_t),
  sizeof(Phantom4) / sizeof(uint16_t),
  sizeof(FPV) / sizeof(uint16_t),
  sizeof(Fimi) / sizeof(uint16_t),
  sizeof(Mini) / sizeof(uint16_t),
  
  sizeof(Phantom4_200) / sizeof(uint16_t),
  sizeof(FPV_200) / sizeof(uint16_t)
};

SystemConfig_t adcConfig;
QueueHandle_t freeQueue; // очередь свободных буферов
QueueHandle_t readyQueue; // очередь готовых буферов

#define BUF_COUNT 2  // можно 2, 3 или больше

// Отправлю в config.h

// #define BUF_SIZE 2048

// typedef struct {
//     uint16_t ch24[BUF_SIZE];
//     uint16_t ch58[BUF_SIZE];
//     size_t length;  // сколько реально отсэмплировали
// } Buffer_t;

Buffer_t buffers[BUF_COUNT];


bool FFT_Send = false;
bool Sampl_Send = false;
bool FR24to58 = false;
bool F200 = false;

void process_fft(uint16_t*, uint16_t*);
void message_to_display();
void process_fft_band(uint16_t*, int*, bool);
void process_fpv_indicator(uint16_t*);
void minmaxFPV();
void ADC_Start();
void ADC_Stop();
void SendSamples(uint16_t* samples);
void SendFFT(float* fft_data);


void vTaskADC(void *pvParameters) {
  freeQueue  = xQueueCreate(BUF_COUNT, sizeof(Buffer_t*));
  readyQueue = xQueueCreate(BUF_COUNT, sizeof(Buffer_t*));

  for (int i = 0; i < BUF_COUNT; i++) {
    Buffer_t *buf = &buffers[i];
    xQueueSend(freeQueue, &buf, 0);
  }
  GetConfig(&adcConfig);

  Buffer_t *buf;

  // xSemaphoreTake( xMutexADC, portMAX_DELAY );
  // adc.setPins(A0, A1);
  // adc.setBuffers(6, 32);
  // adc.begin(FREQ_0);
  // xSemaphoreGive( xMutexADC );

  String message;
  int counter = 0;

  for (;;) {
    // 1. Ждём свободный буфер
    if (xQueueReceive(freeQueue, &buf, portMAX_DELAY) == pdTRUE) {

      xSemaphoreTake( xMutexADC, portMAX_DELAY );

      // 2. Заполняем буфер с двух пинов
      // СДЕЛАТЬ КОЛЬЦЕВОЙ БУФЕР?
      buf->length = BUF_SIZE;
      for (size_t i = 0; i < BUF_SIZE; i++) {
        while (adc.available() < 2) delay(10); 

        buf->ch24[i] = adc.read(); // первый пин
        buf->ch58[i] = adc.read(); // второй пин
      }

      // 3. Отправляем буфер на обработку
      xQueueSend(readyQueue, &buf, portMAX_DELAY);

      DisplayEvent_t dmsg = {
        .type = DISPLAY_EVENT_OSCIL,
        .adcbuffer = buf
      };
      xQueueSend(displayEventQueue, &dmsg, portMAX_DELAY);

      xSemaphoreGive( xMutexADC );
      // delay(100);
      // message = "ADC " + String(counter++);
      // xQueueSend(xQueueSerial, &message, portMAX_DELAY);
    }
  }
}

void vTaskFFT(void *pvParameters) {
  Buffer_t *buf;
  FlagsMessage_t flags = {false, false, false};
  DronsMessage_t drons;

  for (;;) {
    // 1. Ждём готовый буфер
    if (xQueueReceive(readyQueue, &buf, portMAX_DELAY) == pdTRUE) {

      // 2. Проверяем режим отправки доп данных
      if (xQueueReceive(flagsQueue, &flags, 0) == pdTRUE) {
        // обновлены флаги
        Sampl_Send = flags.Sampl_Send;
        FFT_Send = flags.FFT_Send;
        FR24to58 = flags.FR24to58;
      }

      // 3. Обработка данных
      process_fft(buf->ch24, buf->ch58);

      // 4. Отправка данных на дисплей
      message_to_display();

      // 5. Возвращаем буфер в пул
      xQueueSend(freeQueue, &buf, portMAX_DELAY);
    }
  }
}

int dron_24[dronsNumber];
int dron_58[dronsNumber];

int Values24[dronsNumber];
int Values58[dronsNumber];

double vReal[BUF_SIZE];
double vImag[BUF_SIZE];
float FreqAverage[BUF_SIZE/2];

uint16_t FPV_MinMax[2] = {5000, 0};

int adder = 0;
void process_fft(uint16_t* data24, uint16_t* data58) {

  // Обработка FPV индикатора
  process_fpv_indicator(data58);

  // Обработка обеих частотных полос
  process_fft_band(data24, dron_24, false);
  process_fft_band(data58, dron_58, true);

  for (int i = 0; i < dronsNumber; i++) {
    Values24[i] = dron_24[i] / 4000;
    if (i != 3) Values58[i] = dron_58[i] / 1000;

    // Values24[i] += adder;
    // Values58[i] += adder;
  }
  // adder++; 

}

void message_to_display() {
  DronsMessage_t drons;
  memcpy(drons.drons_24, Values24, sizeof(Values24));
  memcpy(drons.drons_58, Values58, sizeof(Values58));

  // drons.drons_24 = dron_24;
  // drons.drons_58 = dron_58;

  DisplayEvent_t dmsg = {
    .type = DISPLAY_EVENT_DRONS,
    .drons = drons
  };

  // Serial.println("/////");
  // if (xSemaphoreTake(xMutexIndicators, portMAX_DELAY)) {
  //   for (size_t i = 0; i < indicator_count; i++) {
  //     Indicator_t* ind = &indicators[i];
  //     Serial.println(String(ind->name) + "\t" + String(ind->value_24) + "\t" + String(ind->value_58) + "\t" + String(Values24[i]) + "\t" + String(Values58[i]));
      
  //     // Serial.println("Freq\tValue\tSum");
  //     // Serial.println("Freq count: " + String(ind->freq_count));
  //   }
  //   xSemaphoreGive(xMutexIndicators);
  // }

  xQueueSend(displayEventQueue, &dmsg, portMAX_DELAY);
}

void process_fft_band(uint16_t* data, int* dron_array, bool is_58_band) {
  for (int c = 0; c < BUF_SIZE; c++) {
    vReal[c] = (double)(data[c]);
    vImag[c] = 0;
  }

  // Отправка сэмплов если нужно
  if (Sampl_Send && (FR24to58 == is_58_band)) {
  // if (!F200 && Sampl_Send && (FR24to58 == is_58_band)) {
  // if (!is_58_band) {
    SendSamples(data);
  // }
  }

  // Выполнение FFT
  FFT.Compute(vReal, vImag, BUF_SIZE, FFT_FORWARD);

  // Вычисление амплитуд
  for (int i = 1; i < BUF_SIZE / 2; i++) {
    FreqAverage[i] = (float)sqrt(vReal[i] * vReal[i] + vImag[i] * vImag[i]);
  }

  // Отправка FFT если нужно
  if (FFT_Send && (FR24to58 == is_58_band)) {
  // if (!F200 && FFT_Send && (FR24to58 == is_58_band)) {
  // if (!is_58_band) {
    SendFFT(FreqAverage);
  }

    // Обработка частотных диапазонов
  for (int k = 0; k < freqsNum2048; k++) {
    float tempSum = 0;
    for (uint16_t f = 0; f < FREQUENCIES_LEN[k]; f++) {
      uint16_t freq_idx = FREQUENCIES[k][f];
      if (freq_idx < BUF_SIZE/2) {
        tempSum += FreqAverage[freq_idx];
      }
    }
    dron_array[k] = (dron_array[k] * adcConfig.averCoeff + tempSum * (100 - adcConfig.averCoeff)) / 100;
  }

  if (xSemaphoreTake(xMutexIndicators, portMAX_DELAY)) {
    for (size_t i = 0; i < indicator_count; i++) {
      Indicator_t* ind = &indicators[i];
      // Serial.println(String(ind->name));
      if (ind->isFPV) continue;
      
      float tempSum = 0;
      // Serial.println("Freq\tValue\tSum");
      // Serial.println("Freq count: " + String(ind->freq_count));
      for (size_t j = 0; j < ind->freq_count; j++) {
        uint16_t f = ind->freqs[j];
        // Serial.println(String(f));
        if (f < BUF_SIZE/2) {
          tempSum += FreqAverage[f];
          // Serial.println("\t" + String(FreqAverage[f]) + "\t" + String(tempSum));
        }
      }

      tempSum = tempSum / (!is_58_band ? 4000 : 1000);

      int* value = (!is_58_band) ? &ind->value_24 : &ind->value_58;
      // Serial.println("Initial val = " + String(*value));
      *value = (*value * adcConfig.averCoeff + tempSum * (100 - adcConfig.averCoeff)) / 100;
      // Serial.println("Aver val = " + String(*value));

      // *value = *value / (!is_58_band ? 4000 : 1000);
      // Serial.println("Divided val = " + String(*value));
    }
    xSemaphoreGive(xMutexIndicators);
  }
}

int fpv_indicator_58 = 0;
int fpv_sum_58 = 0;
double fpv_disp_58 = 0;
double fpv_dispersia_58_ended = 0;

void process_fpv_indicator(uint16_t* data) {
  fpv_indicator_58 = fpv_sum_58 = fpv_disp_58 = 0;

  for (int c = 0; c < BUF_SIZE; c++) {
    fpv_indicator_58 += data[c];
  }

  fpv_indicator_58 /= BUF_SIZE;

  for (int c = 0; c < BUF_SIZE; c++) {
    fpv_sum_58 = fpv_sum_58 + pow((data[c] - fpv_indicator_58), 2);
  }

  fpv_disp_58 = sqrt(fpv_sum_58 / (BUF_SIZE - 1));
  if (fpv_disp_58 == 0) fpv_disp_58 = 1;
  fpv_dispersia_58_ended = 0.8 * fpv_dispersia_58_ended + 0.2 * fpv_disp_58;

  minmaxFPV();

  int newValue58 = abs(fpv_indicator_58 - adcConfig.FPV_Max) * signalMaxLevel / (adcConfig.FPV_Max - adcConfig.FPV_Min);
  Values58[3] = (0.8 * (double)Values58[3]) + (0.2 * (double)(newValue58));

  
  if (xSemaphoreTake(xMutexIndicators, portMAX_DELAY)) {
    for (size_t i = 0; i < indicator_count; i++) {
      Indicator_t* ind = &indicators[i];
      if (!ind->isFPV) continue;
      
      ind->value_58 = (ind->value_58 * adcConfig.averCoeff + newValue58 * (100 - adcConfig.averCoeff)) / 100;
    }
    xSemaphoreGive(xMutexIndicators);
  }

}

void minmaxFPV() {
  if (fpv_indicator_58 < adcConfig.FPV_Min) {
    adcConfig.FPV_Min = fpv_indicator_58;

    ConfigMessage_t change = { .type = CONFIG_SET, .field = CFG_FPV_MIN};
    change.value.FPV_Min = adcConfig.FPV_Min;
    xQueueSend(configQueue, &change, portMAX_DELAY);
  }

  if (fpv_indicator_58 > adcConfig.FPV_Max) {
    adcConfig.FPV_Max = fpv_indicator_58;

    ConfigMessage_t change = { .type = CONFIG_SET, .field = CFG_FPV_MAX};
    change.value.FPV_Max = adcConfig.FPV_Max;
    xQueueSend(configQueue, &change, portMAX_DELAY);
  }
}

void SendSamples(uint16_t* samples) {
  static SerialMessage_t msg;
  msg.type = MSG_TYPE_SAMPLES;
  msg.length = BUF_SIZE;
  msg.data = (uint8_t*)samples; // указатель на буфер, который будет отправлен

  xQueueSend(serialQueue, &msg, portMAX_DELAY);
  delay(100);
}

void SendFFT(float* fft_data) {
  static SerialMessage_t msg;
  msg.type = MSG_TYPE_FFT;
  msg.length = BUF_SIZE / 2;
  msg.data = (uint8_t*)fft_data;

  xQueueSend(serialQueue, &msg, portMAX_DELAY);
  delay(100);
}


#define BATTERY_CHECK_TIME 10000

//ADC max val 4096 -- 3.3 V // max voltage from battery 3.05 V // min v 1.97
const uint16_t battery_values[7] = {
  // 3724,  //image_battery_full_bit
  // 3561,  //image_battery_83_bits,
  // 3338,  //image_battery_67_bits,
  // 3114,  //image_battery_50_bits,
  // 2891,  //image_battery_33_bits,
  // 2667,  //image_battery_17_bits,
  // 2444  //image_battery_0_bits

  3300,
  3154,
  3018,
  2872,
  2726,
  2590,
  2444
};

void vTaskBattery(void *pvParameters) {
  xSemaphoreTake( xMutexADC, portMAX_DELAY );
  adc.setPins(A3);
  adc.setBuffers(6, 32);
  adc.begin(FREQ_0);
  xSemaphoreGive( xMutexADC );

  String message;
  for (;;) {

    xSemaphoreTake(xMutexADC, portMAX_DELAY);
    
    ADC_Stop();
    adc.setPins(A3);
    adc.begin(FREQ_0);

    while(adc.available() > 1) {
      adc.read();
    }

    uint16_t value = adc.read();
    uint8_t grade = 6;
    for (uint8_t i = 0; i < 7; i++) {
      if (value >= battery_values[i]) {
        grade = i;
        break;
      }
    }

    BatteryMessage_t msg = {
      .value = value,
      .grade = grade
    };

    DisplayEvent_t dmsg = {
      .type = DISPLAY_EVENT_BATTERY,
      .battery = msg  
    };

    xQueueSend(displayEventQueue, &dmsg, portMAX_DELAY);

    ADC_Stop();
    ADC_Start();
    

    xSemaphoreGive( xMutexADC );

    // message = "Bat val " + String(value);
    // xQueueSend(xQueueSerial, &message, portMAX_DELAY);

    // message = "Bat gr " + String(grade);
    // xQueueSend(xQueueSerial, &message, portMAX_DELAY);

    delay(10000);
  }
}

void ADC_Stop() {
  adc.end();
}

void ADC_Start() {
  adc.setPins(A0, A1);
  adc.setBuffers(6, 32);
  adc.begin(FREQ_0);
}
