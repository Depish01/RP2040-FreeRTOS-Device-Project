#include "config.h"
#include "handles.h"
#include "task_adc.h"
#include <EEPROM.h>

void Config_SaveToEEPROM();
void Config_LoadFromEEPROM();
void indicators_init();
bool AddIndicator(const char* name, uint16_t* freqs, uint8_t count, 
                  int threshold_24, int threshold_58, int calibr_24, int calibr_58,
                  bool flagFPV, bool flagActive, bool flagConst);


SystemConfig_t gConfig;  // глобальная структура
Indicator_t indicators[MAX_INDICATORS];
size_t indicator_count = 0;

void vTaskConfigGatekeeper(void *pvParameters) {
  EEPROM.begin(128);
  Config_LoadFromEEPROM();
  indicators_init();

  ConfigMessage_t msg;
  TimerMessage_t tmsg;
  // SystemConfig_t msg;

  for (;;) {
    if (xQueueReceive(configQueue, &msg, portMAX_DELAY) == pdTRUE) {

      if (msg.type == CONFIG_GET && msg.replyQueue != NULL) {

        xQueueSend(msg.replyQueue, &gConfig, portMAX_DELAY);

      } else if (msg.type == CONFIG_SET) {

        switch (msg.field) {

          case CFG_LIMIT24:
            gConfig.limit24 = msg.value.limit24;

            break;

          case CFG_LIMIT58:
            gConfig.limit58 = msg.value.limit58;

            break;

          case CFG_BRIGHTNESS:
            gConfig.brightness = msg.value.brightness;

            break;

          case CFG_BLOCK_CHOICE:
            gConfig.blockChoice = msg.value.blockChoice;
            tmsg.type = TIMER_CONFIG_UPDATE;
            tmsg.timerId = TIMER_KEYLOCK;
            tmsg.newValue = gConfig.blockChoice;
            xQueueSend(timerQueue, &tmsg, portMAX_DELAY);

            break;

          case CFG_SLEEP_CHOICE:
            gConfig.sleepChoice = msg.value.sleepChoice;
            tmsg.type = TIMER_CONFIG_UPDATE;
            tmsg.timerId = TIMER_SLEEP;
            tmsg.newValue = gConfig.sleepChoice;
            xQueueSend(timerQueue, &tmsg, portMAX_DELAY);

            break;

          case CFG_LANGUAGE:
            gConfig.language = msg.value.language;

            break;

          case CFG_AVER_COEFF:
            gConfig.averCoeff = msg.value.averCoeff;

            break;

          case CFG_LOGO_FLAG:
            gConfig.logoFlag = msg.value.logoFlag;

            break;

          case CFG_SOUND_OFF_FLAG:
            gConfig.soundOffFlag = msg.value.soundOffFlag;

            break;

          case CFG_FPV_MIN:
            gConfig.FPV_Min = msg.value.FPV_Min;

            break;

          case CFG_FPV_MAX:
            gConfig.FPV_Max = msg.value.FPV_Max;

            break;

          case CFG_CALIBR_NUMBER:
            gConfig.calibrNumber = msg.value.calibrNumber;

            break;

          case CFG_CALIBR_COEFF24:
            memcpy(gConfig.calibrationCoeff24, msg.value.calibrationCoeff24, dronsNumber);
            break;
            
          case CFG_CALIBR_COEFF58:
            memcpy(gConfig.calibrationCoeff58, msg.value.calibrationCoeff58, dronsNumber);

            break;

          }
        
        Config_SaveToEEPROM();
      }
    }
  }
}


void GetConfig(SystemConfig_t *localconfig) {
  QueueHandle_t replyQueue = xQueueCreate(1, sizeof(SystemConfig_t));

  // Запрашиваем конфиг при старте
  ConfigMessage_t req = { .type = CONFIG_GET, .replyQueue = replyQueue };
  xQueueSend(configQueue, &req, portMAX_DELAY);
  xQueueReceive(replyQueue, localconfig, portMAX_DELAY);

  vQueueDelete(replyQueue);
}

void Config_SaveToEEPROM() {
  // мьютекс на ацп
  xSemaphoreTake( xMutexADC, portMAX_DELAY );

  ADC_Stop();

  EEPROM.put(0, gConfig);
  EEPROM.commit();

  ADC_Start();

  xSemaphoreGive( xMutexADC );
}

void Config_LoadFromEEPROM() {

  EEPROM.get(0, gConfig);

  if (gConfig.magic != CONFIG_MAGIC) {
    gConfig.magic = CONFIG_MAGIC;
    gConfig.limit24 = 50;
    gConfig.limit58 = 50;
    gConfig.brightness = 10;
    gConfig.blockChoice = 0;
    gConfig.sleepChoice = 0;
    gConfig.language = 0;
    gConfig.averCoeff = 80;
    gConfig.logoFlag = true;
    gConfig.soundOffFlag = false;
    gConfig.FPV_Min = 5000;
    gConfig.FPV_Max = 0;
    gConfig.calibrNumber = 30;
    for (int i = 0; i < dronsNumber; i++) {
      gConfig.calibrationCoeff24[i] = 100;
      gConfig.calibrationCoeff58[i] = 100;
    }
    // EEPROM.put(0, gConfig);
    // EEPROM.commit();
  }

}

uint16_t i_Ocu2[]  = {100, 200, 300, 500, 301, 501, 700, 701};
uint16_t i_Ocu3[]  = {200, 199, 201, 300, 299, 301, 400, 398, 399, 401, 402, 500, 498, 499, 501, 502};
uint16_t i_Autel[] = {44, 48, 200, 400, 600, 800, 1000};
uint16_t i_FPV[]   = {50, 100, 150, 60, 120, 180, 649, 709, 750};
uint16_t i_Fimi[]  = {71, 72, 143, 214, 286, 357, 428, 429, 643};
uint16_t i_Mini[]  = {30, 60, 90};

void indicators_init() {
  xSemaphoreTake( xMutexIndicators, portMAX_DELAY );

  AddIndicator("Ocu2",  i_Ocu2,   sizeof(i_Ocu2)/sizeof(uint16_t),  50, 50, gConfig.calibrationCoeff24[0], gConfig.calibrationCoeff58[0], false, true, true);
  AddIndicator("Ocu3",  i_Ocu3,   sizeof(i_Ocu3)/sizeof(uint16_t),  50, 50, gConfig.calibrationCoeff24[1], gConfig.calibrationCoeff58[1], false, true, true);
  AddIndicator("Aut",   i_Autel,  sizeof(i_Autel)/sizeof(uint16_t), 50, 50, gConfig.calibrationCoeff24[2], gConfig.calibrationCoeff58[2], false, true, true);
  AddIndicator("FPV",   i_FPV,    sizeof(i_FPV)/sizeof(uint16_t),   50, 50, gConfig.calibrationCoeff24[3], gConfig.calibrationCoeff58[3], true,  true, true);
  AddIndicator("Fimi",  i_Fimi,   sizeof(i_Fimi)/sizeof(uint16_t),  50, 50, gConfig.calibrationCoeff24[4], gConfig.calibrationCoeff58[4], false, true, true);
  AddIndicator("Mini",  i_Mini,   sizeof(i_Mini)/sizeof(uint16_t),  50, 50, gConfig.calibrationCoeff24[5], gConfig.calibrationCoeff58[5], false, true, true);
  
  xSemaphoreGive( xMutexIndicators );
}

bool AddIndicator(const char* name,  uint16_t* freqs, uint8_t count, 
                  int threshold_24, int threshold_58, int calibr_24, int calibr_58,
                  bool flagFPV, bool flagActive, bool flagConst) {

  if (indicator_count >= MAX_INDICATORS) return false;

  Indicator_t* ind = &indicators[indicator_count++];
  memset(ind, 0, sizeof(Indicator_t));

  strncpy(ind->name, name, sizeof(ind->name) - 1);
  ind->freq_count = min(count, MAX_FREQS_PER_INDICATOR);
  // memcpy(ind->freqs, freqs, ind->freq_count * sizeof(uint16_t));
  ind->freqs = freqs;

  ind->threshold_24 = threshold_24;
  ind->threshold_58 = threshold_58;
  ind->calibrationCoeff_24 = calibr_24;
  ind->calibrationCoeff_58 = calibr_58;
  ind->isActive = flagActive;
  ind->isFPV = flagFPV;
  ind->isConst = flagConst;

  return true;
}

// SaveToEEPROM
// delay(500);
// adc.end();
// delay(500);

// EEPROM.write(LIMITS_ADDRESS_24, (uint8_t)(limit1 >> 8));
// EEPROM.write(LIMITS_ADDRESS_24 + 1, (uint8_t)(limit1));
// EEPROM.write(LIMITS_ADDRESS_58, (uint8_t)(limit2 >> 8));
// EEPROM.write(LIMITS_ADDRESS_58 + 1, (uint8_t)(limit2));
// EEPROM.write(BRIGHTNESS_ADDRESS, (uint8_t)(brightness));
// EEPROM.write(BLOCK_ADDRESS, (uint8_t)(blockChoice));
// EEPROM.write(SLEEP_ADDRESS, (uint8_t)(sleepChoice));
// EEPROM.write(ADDRESS_LANGUAGE, (uint8_t)(language));
// EEPROM.write(ADDRESS_AVER, (uint8_t)(averCoeff));
// EEPROM.write(ADDRESS_LOGO, (uint8_t)(logoFlag));
// EEPROM.write(ADDRESS_SOUND, (uint8_t)(soundOffFlag));
// EEPROM.write(ADDRESS_CALIBR, (uint8_t)(calibrNumber));
// for (int i = 0; i < dronsNumber; i++) {
//   EEPROM.write(CALIBRATION_ADDRESS_24 + i, (uint8_t)(calibrationCoeff24[i]));
//   EEPROM.write(CALIBRATION_ADDRESS_58 + i, (uint8_t)(calibrationCoeff58[i]));
// }
// EEPROM.put(ADDRESS_ADC_MIN, FPV_MinMax[0]);
// EEPROM.put(ADDRESS_ADC_MAX, FPV_MinMax[1]);
// EEPROM.commit();


//checkEEPROM
  // for (int i = 0; i < dronsNumber; i++) {
  //   calibrationCoeff24[i] = EEPROM.read(CALIBRATION_ADDRESS_24 + i);
  //   calibrationCoeff58[i] = EEPROM.read(CALIBRATION_ADDRESS_58 + i);
  //   if ((calibrationCoeff24[i] == 0) || (calibrationCoeff24[i] > 100)) calibrationCoeff24[i] = 100;
  //   if ((calibrationCoeff58[i] == 0) || (calibrationCoeff58[i] > 100)) calibrationCoeff58[i] = 100;
  // }

  // brightness = EEPROM.read(BRIGHTNESS_ADDRESS);
  // if (brightness > 10) brightness = 10;
  // blockChoice = EEPROM.read(BLOCK_ADDRESS);
  // if (blockChoice > 10) blockChoice = 0;
  // sleepChoice = EEPROM.read(SLEEP_ADDRESS);
  // if (sleepChoice > 10) sleepChoice = 0;
  // // EEPROM.read(CHANNEL_ADDRESS);
  // limit1 = ((EEPROM.read(LIMITS_ADDRESS_24) << 8) | (EEPROM.read(LIMITS_ADDRESS_24 + 1)));
  // limit2 = ((EEPROM.read(LIMITS_ADDRESS_58) << 8) | (EEPROM.read(LIMITS_ADDRESS_58 + 1)));
  // if (limit1 > 10000) limit1 = 50;
  // if (limit2 > 10000) limit2 = 50;

  // language = EEPROM.read(ADDRESS_LANGUAGE);
  // if (language >= LANGUAGES_Q) language = 0;

  // averCoeff = EEPROM.read(ADDRESS_AVER);
  // if (averCoeff > 100) averCoeff = 80;

  // if (EEPROM.read(ADDRESS_LOGO) > 1) { logoFlag = true; }
  // else { logoFlag = (bool)(EEPROM.read(ADDRESS_LOGO)); }
  
  // calibrNumber = EEPROM.read(ADDRESS_CALIBR);
  // if (calibrNumber > 100) calibrNumber = 30;

  // if (EEPROM.read(ADDRESS_SOUND) > 1) { soundOffFlag = false; }
  // else { soundOffFlag = (bool)(EEPROM.read(ADDRESS_SOUND)); }

  // EEPROM.get(ADDRESS_ADC_MIN, FPV_MinMax[0]);
  // if (FPV_MinMax[0] == 0xffff) FPV_MinMax[0] = 5000;

  // EEPROM.get(ADDRESS_ADC_MAX, FPV_MinMax[1]);
  // if (FPV_MinMax[1] == 0xffff) FPV_MinMax[1] = 0;