#include "task_display.h"

#include "config.h"
#include "handles.h"
#include "graphics.h"
#include "languages.h"

#include "Org_01.h"

#include <SPI.h>
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();

#define TIME_LOGO 3000

#define battery_x 24
#define battery_y 13

// Menu icons coordinats  24x24 only
#define icons_x 6
#define icon1_y 30
#define icon2_y 65
#define icon3_y 100
// Menu strings coordinats
#define strings_x 36
#define string1_y 35
#define string2_y 70
#define string3_y 105

#define menu_number 9
#define admin_menu_number 4

void displayInitialization();

void mainDraw();
void mainDrawSimple();
void mainDrawComplex();

void drawBattery();

void levelsDraw(bool flag24, bool flag58);
void levelsDrawSimple(bool ch1, bool ch2);
void levelsDrawComplex(bool ch1, bool ch2);

void drawLevelsComplex(int val, int row, String baud, int color);
void drawLimitSimple(int val, bool row, bool clr);
void drawLimitComplx(int val, int row, bool clr);
void ind_drawLimitComplx(int val, int row, int clr);

void IND_levelsDraw(bool flag24, bool flag58);
void IND_levelsDrawSimple(bool ch1, bool ch2);
void IND_levelsDrawComplex(bool ch1, bool ch2);

void menuThresholdsSimple();
void menuThresholdsComplex();

void drawMenuSettings();
void drawMenuRect(byte position, uint16_t color);
void drawMenuRect_centr(uint16_t color);
void drawMenuRect_up(uint16_t color);
void drawMenuRect_down(uint16_t color);

void calibrate(int action, const DronsMessage_t* data );

void stopAlarm();

// const unsigned char *bitmap_icons[menu_number] = {
//   image_light,
//   image_calibration,
//   image_antenna_bits,
//   image_locked_icon_bits,
//   image_sleep_mode_icon_bits,
//   icon_resetreset_icon,
//   image_wave_sine_icon_bits,
//   image_info
// };

Icon MENU_ICONS[menu_number] = {
  icon_light,
  icon_calibr,
  icon_channel,
  icon_drons,
  icon_lock,
  icon_sleep,
  icon_reset,
  icon_sinus,
  icon_info
};
// #include <PWMAudio.h>
// PWMAudio pwm(6);

// const uint16_t icons_colors[menu_number] = {
//   ST7735_YELLOW,  //{"Подсветка"},
//   0x8e5f,         //{"Калибровка"},
//   0x1ff6,         //{"Каналы"},
//   ST7735_ORANGE,  //{"Автоблок"},
//   0xb47f,         //{"Спящий реж"},
//   0xf145,         //{"Сброс"},
//   ST7735_GREEN,    //{"Осцилограф"}
//   0x54bf          //{"Инфо"}
// };

#define background_color 0xE
#define grayClr 0x8430
#define greenClr 0x57EA
#define redClr 0xFBAE

// Ширина индикатора в пикселях
#define indicatorMaxWidthSimple 104
#define indicatorMaxWidthComp   100
// byte indicatorMaxWidth[2] = {indicatorMaxWidthSimple, indicatorMaxWidthComp};
byte indicatorMaxWidth = indicatorMaxWidthSimple;

bool LOCK_FLAG = false;
bool calibrFlag = false;        // Флаг наличия калибровки
bool simpleDimpleFlag = false;  // Флаг режима отображения сигналов
bool channel_24 = true;         // Флаг работы по 2.4
bool channel_58 = true;         // Флаг работы по 5.8
byte battery_index = 0;         // Индикатор заряда
bool thresholds_24_58 = false;

// Пороги
uint16_t indlimit1;
uint16_t indlimit2;
int indicator_24 = 0;
int indicator_58 = 0;
int newIndicator_24 = 0;
int newIndicator_58 = 0;


DeviceMode currentMode = MODE_NORMAL;

typedef void (*KeyHandler)(int arg);

typedef struct {
    KeyHandler handler;
    int arg;
} KeyAction;

// ==== Обработчики ====
void handle_timers_update(int);
void handle_lock_mode(int);
void handle_sleep_mode(int);
void handle_levels_mode(int);
void handle_sound_toggle(int);

void handle_menu_mode(int mode);

void handle_threshold_mode(int mode);
void handle_threshold_input(int input);
void handle_threshold_24_58(int mode_24_58);

void handle_menu_down_up(int dirrection);

void handle_sub_menu(int digit);
void handle_menu_admin(int digit);

KeyAction keyMatrix[MODE_COUNT][KEY_COUNT];
KeyAction keyMatrixLong[MODE_COUNT][KEY_COUNT];

void KeyActionInit() {
  keyMatrix[MODE_NORMAL][KEY_VFO]      = { handle_sleep_mode, 1 };
  keyMatrix[MODE_NORMAL][KEY_AB]       = { handle_levels_mode, 0 };
  keyMatrix[MODE_NORMAL][KEY_BRACKETS] = { handle_threshold_mode, 1 };
  keyMatrixLong[MODE_NORMAL][KEY_GREEN]    = { handle_menu_mode, 1 };
  // keyMatrix[MODE_NORMAL][KEY_GREEN]    = { handle_menu_mode, 1 };
  keyMatrix[MODE_NORMAL][KEY_RED]      = { handle_timers_update, 0 };
  keyMatrix[MODE_NORMAL][KEY_UP]       = { handle_timers_update, 0 };
  keyMatrix[MODE_NORMAL][KEY_DOWN]     = { handle_timers_update, 0 };
  keyMatrixLong[MODE_NORMAL][KEY_STAR]     = { handle_lock_mode, 1 };
  keyMatrixLong[MODE_NORMAL][KEY_HASH]     = { handle_sound_toggle, 0 };
  keyMatrix[MODE_NORMAL][KEY_0]        = { handle_timers_update, 0 };
  keyMatrix[MODE_NORMAL][KEY_1]        = { handle_timers_update, 0 };
  keyMatrix[MODE_NORMAL][KEY_2]        = { handle_timers_update, 0 };
  keyMatrix[MODE_NORMAL][KEY_3]        = { handle_timers_update, 0 };
  keyMatrix[MODE_NORMAL][KEY_4]        = { handle_timers_update, 0 };
  keyMatrix[MODE_NORMAL][KEY_5]        = { handle_timers_update, 0 };
  keyMatrix[MODE_NORMAL][KEY_6]        = { handle_timers_update, 0 };
  keyMatrix[MODE_NORMAL][KEY_7]        = { handle_timers_update, 0 };
  keyMatrix[MODE_NORMAL][KEY_8]        = { handle_timers_update, 0 };
  keyMatrix[MODE_NORMAL][KEY_9]        = { handle_timers_update, 0 };

  keyMatrixLong[MODE_LOCKED][KEY_STAR]     = { handle_lock_mode, 0 };
  keyMatrix[MODE_LOCKED][KEY_VFO]      = { handle_timers_update, 0 };
  keyMatrix[MODE_LOCKED][KEY_AB]       = { handle_timers_update, 0 };
  keyMatrix[MODE_LOCKED][KEY_BRACKETS] = { handle_timers_update, 0 };
  keyMatrix[MODE_LOCKED][KEY_GREEN]    = { handle_timers_update, 0 };
  keyMatrix[MODE_LOCKED][KEY_RED]      = { handle_timers_update, 0 };
  keyMatrix[MODE_LOCKED][KEY_UP]       = { handle_timers_update, 0 };
  keyMatrix[MODE_LOCKED][KEY_DOWN]     = { handle_timers_update, 0 };
  keyMatrix[MODE_LOCKED][KEY_HASH]     = { handle_timers_update, 0 };
  keyMatrix[MODE_LOCKED][KEY_0]        = { handle_timers_update, 0 };
  keyMatrix[MODE_LOCKED][KEY_1]        = { handle_timers_update, 0 };
  keyMatrix[MODE_LOCKED][KEY_2]        = { handle_timers_update, 0 };
  keyMatrix[MODE_LOCKED][KEY_3]        = { handle_timers_update, 0 };
  keyMatrix[MODE_LOCKED][KEY_4]        = { handle_timers_update, 0 };
  keyMatrix[MODE_LOCKED][KEY_5]        = { handle_timers_update, 0 };
  keyMatrix[MODE_LOCKED][KEY_6]        = { handle_timers_update, 0 };
  keyMatrix[MODE_LOCKED][KEY_7]        = { handle_timers_update, 0 };
  keyMatrix[MODE_LOCKED][KEY_8]        = { handle_timers_update, 0 };
  keyMatrix[MODE_LOCKED][KEY_9]        = { handle_timers_update, 0 };

  keyMatrix[MODE_SLEEP][KEY_STAR]     = { handle_sleep_mode, 0 };
  keyMatrix[MODE_SLEEP][KEY_HASH]     = { handle_sleep_mode, 0 };
  keyMatrix[MODE_SLEEP][KEY_GREEN]    = { handle_sleep_mode, 0 };
  keyMatrix[MODE_SLEEP][KEY_VFO]      = { handle_sleep_mode, 0 };
  keyMatrix[MODE_SLEEP][KEY_AB]       = { handle_sleep_mode, 0 };
  keyMatrix[MODE_SLEEP][KEY_BRACKETS] = { handle_sleep_mode, 0 };
  keyMatrix[MODE_SLEEP][KEY_0]        = { handle_sleep_mode, 0 };
  keyMatrix[MODE_SLEEP][KEY_1]        = { handle_sleep_mode, 0 };
  keyMatrix[MODE_SLEEP][KEY_2]        = { handle_sleep_mode, 0 };
  keyMatrix[MODE_SLEEP][KEY_3]        = { handle_sleep_mode, 0 };
  keyMatrix[MODE_SLEEP][KEY_4]        = { handle_sleep_mode, 0 };
  keyMatrix[MODE_SLEEP][KEY_5]        = { handle_sleep_mode, 0 };
  keyMatrix[MODE_SLEEP][KEY_6]        = { handle_sleep_mode, 0 };
  keyMatrix[MODE_SLEEP][KEY_7]        = { handle_sleep_mode, 0 };
  keyMatrix[MODE_SLEEP][KEY_8]        = { handle_sleep_mode, 0 };
  keyMatrix[MODE_SLEEP][KEY_9]        = { handle_sleep_mode, 0 };
  keyMatrix[MODE_SLEEP][KEY_RED]      = { handle_sleep_mode, 0 };
  keyMatrix[MODE_SLEEP][KEY_UP]       = { handle_sleep_mode, 0 };
  keyMatrix[MODE_SLEEP][KEY_DOWN]     = { handle_sleep_mode, 0 };

  keyMatrix[MODE_SETTINGS][KEY_DOWN]    = { handle_menu_down_up, 0 };
  keyMatrix[MODE_SETTINGS][KEY_UP]      = { handle_menu_down_up, 1 };
  keyMatrix[MODE_SETTINGS][KEY_GREEN]   = { handle_menu_mode, 2 };
  keyMatrix[MODE_SETTINGS][KEY_RED]     = { handle_menu_mode, 0 };
  keyMatrixLong[MODE_SETTINGS][KEY_HASH]    = { handle_menu_mode, 3 };

  keyMatrix[MODE_SUBSETTINGS][KEY_DOWN]    = { handle_sub_menu, 1 };
  keyMatrix[MODE_SUBSETTINGS][KEY_UP]      = { handle_sub_menu, 2 };
  keyMatrix[MODE_SUBSETTINGS][KEY_GREEN]   = { handle_sub_menu, 3 };
  keyMatrix[MODE_SUBSETTINGS][KEY_RED]     = { handle_sub_menu, 4 };
  // keyMatrix[MODE_SUBSETTINGS][KEY_RED]     = { handle_menu_mode, 1 };

  keyMatrix[MODE_SETTINGS_ADMIN][KEY_DOWN]    = { handle_menu_admin, 1 };
  keyMatrix[MODE_SETTINGS_ADMIN][KEY_UP]      = { handle_menu_admin, 2 };
  keyMatrix[MODE_SETTINGS_ADMIN][KEY_GREEN]   = { handle_menu_admin, 3 };
  keyMatrix[MODE_SETTINGS_ADMIN][KEY_RED]     = { handle_menu_admin, 4 };
  // keyMatrix[MODE_SETTINGS_ADMIN][KEY_RED]     = { handle_menu_mode, 1 };

  keyMatrix[MODE_THRESHOLD][KEY_0]         = { handle_threshold_input, 0 };
  keyMatrix[MODE_THRESHOLD][KEY_1]         = { handle_threshold_input, 1 };
  keyMatrix[MODE_THRESHOLD][KEY_2]         = { handle_threshold_input, 2 };
  keyMatrix[MODE_THRESHOLD][KEY_3]         = { handle_threshold_input, 3 };
  keyMatrix[MODE_THRESHOLD][KEY_4]         = { handle_threshold_input, 4 };
  keyMatrix[MODE_THRESHOLD][KEY_5]         = { handle_threshold_input, 5 };
  keyMatrix[MODE_THRESHOLD][KEY_6]         = { handle_threshold_input, 6 };
  keyMatrix[MODE_THRESHOLD][KEY_7]         = { handle_threshold_input, 7 };
  keyMatrix[MODE_THRESHOLD][KEY_8]         = { handle_threshold_input, 8 };
  keyMatrix[MODE_THRESHOLD][KEY_9]         = { handle_threshold_input, 9 };
  keyMatrix[MODE_THRESHOLD][KEY_DOWN]      = { handle_threshold_input, 10 };
  keyMatrix[MODE_THRESHOLD][KEY_UP]        = { handle_threshold_input, 11 };
  keyMatrix[MODE_THRESHOLD][KEY_GREEN]     = { handle_threshold_24_58, 0 };
  keyMatrix[MODE_THRESHOLD][KEY_RED]       = { handle_threshold_24_58, 1 };
  keyMatrix[MODE_THRESHOLD][KEY_BRACKETS]  = { handle_threshold_mode, 0 };
}

void lightMenu(int);
void calibrMenu(int);
void channelsMenu(int);
void indicatorsMenu(int);
void blockMenu(int);
void sleepMenu(int);
void resetMenu(int);
void oscillographMenu(int);
void infoMenu(int);

typedef void (* FuncPtr) (int arg);
FuncPtr menu_pages[menu_number] = {
  &lightMenu,
  &calibrMenu,
  &channelsMenu,
  &indicatorsMenu,
  &blockMenu,
  &sleepMenu,
  &resetMenu,
  &oscillographMenu,
  &infoMenu
};  

double fpv_disp_58_ended = 0;
// Звонок 049, 050, 052
int dispersia_porog = 250;

// Звонок 051
// int dispersia_porog = 150;

int Lvl24[dronsNumber];
int Lvl58[dronsNumber];

int Lvl24_calibr[dronsNumber];
int Lvl58_calibr[dronsNumber];

int lvlMax_24 = 0;
int lvlMax_58 = 0;

bool overLimitFlag = false;
bool overLimitFPVFlag = false;

byte blinkMpl_NoFPV = 1;
byte blinkMpl_FPV = 1;

enum MenuMode {
  MENU_DEFAULT,
  MENU_CALIBRATION,
  MENU_OSCILLOGRAPH,
  MENU_INDICATORS
};

MenuMode menu_mode = MENU_DEFAULT;
Buffer_t *oscill_buf;
SystemConfig_t displayConfig;

void vTaskDisplay(void *pvParameters) {
  KeyActionInit();
  
  // Запрашиваем конфиг при старте
  GetConfig(&displayConfig);

  set_language(displayConfig.language);

  displayInitialization();
  mainDraw();
  
  DisplayEvent_t ev;
  
  for (;;) {

    if(xQueueReceive(displayEventQueue, &ev, portMAX_DELAY) == pdTRUE) {
      // String message = "Event" + String(ev.type);
      // xQueueSend(xQueueSerial, &message, 0);
      switch (ev.type) {
        case DISPLAY_EVENT_BUTTON:
          if (ev.button.type == BUTTON_EVENT_PRESS) {
            KeyAction action = keyMatrix[currentMode][ev.button.buttonId];
            if (action.handler) {
              action.handler(action.arg);
            }
          } else if (ev.button.type == BUTTON_EVENT_LONG) {
            KeyAction action = keyMatrixLong[currentMode][ev.button.buttonId];
            if (action.handler) {
              action.handler(action.arg);
            }
          }
          break;

        case DISPLAY_EVENT_TIMER:
          if ((currentMode == MODE_NORMAL) || (currentMode == MODE_LOCKED)) {
            if (ev.timer.timerId == TIMER_KEYLOCK) {
              handle_lock_mode(1);
            } else if (ev.timer.timerId == TIMER_SLEEP) {
              handle_sleep_mode(1);
            }
          }

          break;

        case DISPLAY_EVENT_BATTERY:
          if (menu_mode == MENU_OSCILLOGRAPH) break;
          if (menu_mode == MENU_INDICATORS) break;
          battery_index = ev.battery.grade;
          drawBattery();
          break;

        case DISPLAY_EVENT_DRONS:
          memcpy(Lvl24, ev.drons.drons_24, sizeof(Lvl24));
          memcpy(Lvl58, ev.drons.drons_58, sizeof(Lvl58));

          // uint8_t temp_drons_bytes[dronsNumber * sizeof(int) * 2];

          // memcpy(temp_drons_bytes, Lvl24, sizeof(Lvl24));
          // memcpy(temp_drons_bytes + sizeof(Lvl24), Lvl58, sizeof(Lvl58));
          
          // static SerialMessage_t debug_msg;
          // debug_msg.type = MSG_TYPE_DRONS;
          // debug_msg.length = dronsNumber * sizeof(int) * 2;
          // debug_msg.data = temp_drons_bytes; // указатель на буфер, который будет отправлен

          // xQueueSend(serialQueue, &debug_msg, portMAX_DELAY);

          if ((currentMode == MODE_NORMAL) || (currentMode == MODE_LOCKED)){
            // levelsDraw(channel_24, channel_58);
            IND_levelsDraw(channel_24, channel_58);
          } else if (currentMode == MODE_THRESHOLD) {
            IND_levelsDraw(!thresholds_24_58, thresholds_24_58);
          } else if (currentMode == MODE_SUBSETTINGS) {
            if (menu_mode == MENU_CALIBRATION) {
              calibrate(1, &ev.drons);
            }
          }

          break;

        case DISPLAY_EVENT_OSCIL:
          if (menu_mode != MENU_OSCILLOGRAPH) break;
          oscill_buf = ev.adcbuffer;
          oscillographMenu(5);
          break;
      }
    }
  }
}

void displayInitialization() {
  pinMode(DISPLAY_LED, OUTPUT);
  analogWriteFreq(1000);
  analogWriteRange(256);

  tft.init();
  tft.setRotation(3);
  tft.fillScreen(background_color);

  if (displayConfig.brightness == 10) {
    digitalWrite(DISPLAY_LED, HIGH);
  } else {
    analogWrite(DISPLAY_LED, (displayConfig.brightness) * briStep + briMin);
  }
  
  if (displayConfig.logoFlag) {
    DRAW_ICON(icon_logotype);
    // tft.drawBitmap(0, 0, new_logotype, 160, 128, 0xe6b6);
    delay(TIME_LOGO);
    tft.fillScreen(background_color);
  }
  

  tft.drawLine(0, 23, 159, 23, 0xAD55);

  tft.setTextColor(0xFFFF, background_color);
  tft.setTextSize(2);
  tft.setTextWrap(false);

  for (int i = 0; i < dronsNumber; i++) {
    if ((displayConfig.calibrationCoeff24[i] != 100) || (displayConfig.calibrationCoeff58[i] != 100)) {
      calibrFlag = true;
      break;
    }
  }
}


void mainDraw() {
  tft.fillScreen(background_color);
  tft.drawLine(0, 23, 159, 23, 0xAD55);

  if (!simpleDimpleFlag) {
    indicatorMaxWidth = indicatorMaxWidthSimple;
    mainDrawSimple();
  } else {
    indicatorMaxWidth = indicatorMaxWidthComp;
    mainDrawComplex();
  }

  IND_levelsDraw(channel_24, channel_58);
}


// Simple mode
void mainDrawSimple() {
  int color24 = 0xffff;
  int color58 = 0xffff;
  if (!channel_24) {
    color24 = grayClr;
  }
  if (!channel_58) {
    color58 = grayClr;
  }
  tft.fillRect(0, 25, 160, 103, background_color);

  drawBattery();

  tft.setCursor(12, 6);
  tft.print(get_str(STR_SEARCH));
  
  if (calibrFlag) {
    tft.drawBitmap(113, 5, image_crosshairs_bits, 15, 15, 0xffff, background_color);
  } else{
    tft.drawBitmap(113, 5, image_crosshairs_bits, 15, 15, background_color);
  }

  if (displayConfig.soundOffFlag) {
    tft.drawBitmap(110 - (calibrFlag * 17), 5, image_volume_muted_bits, 18, 16, 0xFFFF, background_color);
  } else {
    tft.drawBitmap(110 - (calibrFlag * 17), 5, image_volume_muted_bits, 18, 16, background_color);
  }

  tft.setTextColor(color24, background_color);
  tft.setCursor(5, 43);
  tft.print("2.4");
  tft.setTextColor(0xffff, background_color);


  tft.setTextColor(color58, background_color);
  tft.setCursor(5, 94);
  tft.print("5.8");
  tft.setTextColor(0xffff, background_color);

  tft.drawRect(0, 75, 160, 2, 0xad55);

  //2.4
  tft.drawRect(46, 37, 112, 25, color24);

  indlimit1 = (displayConfig.limit24 * indicatorMaxWidth) / signalMaxLevel;
  if (indlimit1 > indicatorMaxWidth) indlimit1 = indicatorMaxWidth;

  tft.drawBitmap(50 + indlimit1 - 2, 33, image_SmallArrowDown_bits, 5, 3, color24);
  tft.drawBitmap(50 + indlimit1 - 2, 63, image_SmallArrowUp_bits, 5, 3, color24);
  tft.drawLine(50 + indlimit1, 37, 50 + indlimit1, 60, color24);

  //5.8
  tft.drawRect(46, 88, 112, 25, color58);

  indlimit2 = (displayConfig.limit58 * indicatorMaxWidth) / signalMaxLevel;
  if (indlimit2 > indicatorMaxWidth) indlimit2 = indicatorMaxWidth;

  tft.drawBitmap(50 + indlimit2 - 2, 84, image_SmallArrowDown_bits, 5, 3, color58);
  tft.drawBitmap(50 + indlimit2 - 2, 114, image_SmallArrowUp_bits, 5, 3, color58);
  tft.drawLine(50 + indlimit2, 88, 50 + indlimit2, 111, color58);

  indicator_24 = 0;
  indicator_58 = 0;

}

// Complex mode
void mainDrawComplex() {
  tft.setCursor(12, 6);
  tft.print(get_str(STR_SEARCH_PLUS));
  tft.fillRect(0, 25, 160, 103, background_color);
  drawBattery();
  
  if (calibrFlag) {
    tft.drawBitmap(113, 5, image_crosshairs_bits, 15, 15, 0xffff, background_color);
  } else {
    tft.drawBitmap(113, 5, image_crosshairs_bits, 15, 15, background_color);
  }
  if (displayConfig.soundOffFlag) {
    tft.drawBitmap(110 - (calibrFlag * 17), 5, image_volume_muted_bits, 18, 16, 0xFFFF, background_color);
  } else {
    tft.drawBitmap(110 - (calibrFlag * 17), 5, image_volume_muted_bits, 18, 16, background_color);
  }

  tft.setTextSize(1);

  tft.setCursor(3, 32);
  tft.print("Ocu2");
  tft.drawRect(52, 30, 104, 11, 0xFFFF);

  tft.setCursor(3, 48);
  tft.print("Ocu3");
  tft.drawRect(52, 46, 104, 11, 0xFFFF);

  tft.setCursor(3, 64);
  tft.print("Aut");
  tft.drawRect(52, 62, 104, 11, 0xFFFF);

  tft.setCursor(3, 80);
  tft.print("FPV");
  tft.drawRect(52, 78, 104, 11, 0xFFFF);

  tft.setCursor(3, 96);
  tft.print("Fimi");
  tft.drawRect(52, 94, 104, 11, 0xFFFF);

  tft.setCursor(3, 112);
  tft.print("Mini");
  tft.drawRect(52, 110, 104, 11, 0xFFFF);

  tft.setTextSize(2);
}

void drawBattery() {
  tft.drawBitmap(131, 7, battery_icons[battery_index], battery_x, battery_y, battery_colors[battery_index], background_color);
}

void levelsDraw(bool flag24, bool flag58) {
  if (!simpleDimpleFlag) {
    levelsDrawSimple(flag24, flag58);
  } else {
    levelsDrawComplex(flag24, flag58);
  }
}

void levelsDrawSimple(bool ch1, bool ch2) {
  for (int i = 0; i < dronsNumber; i++) {
    Lvl24_calibr[i] = (Lvl24[i] * displayConfig.calibrationCoeff24[i]) / 100;
    Lvl58_calibr[i] = (Lvl58[i] * displayConfig.calibrationCoeff58[i]) / 100;
  }
  overLimitFlag = false;
  overLimitFPVFlag = false;
  lvlMax_24 = 0;
  lvlMax_58 = 0;
  for (int i = 0; i < dronsNumber; i++) {
    if (Lvl24_calibr[i] > lvlMax_24) lvlMax_24 = Lvl24_calibr[i];
    if (Lvl58_calibr[i] > lvlMax_58) {
      if (i == 3) {
        if(fpv_disp_58_ended < dispersia_porog) {
          lvlMax_58 = Lvl58_calibr[i];
        }
      } else{
        lvlMax_58 = Lvl58_calibr[i];
      }
    }
    // Serial.println("lvlMax_58 " +String(lvlMax_58));
    // Serial.println("fpv_disp_58_ended " +String(fpv_disp_58_ended));
  }


  if (ch1 * channel_24 * lvlMax_24 > displayConfig.limit24) {
    overLimitFlag = true;
    blinkMpl_NoFPV = ((lvlMax_24 - displayConfig.limit24) / 50) + 1;
  } else if (ch2 * channel_58 * lvlMax_58 > displayConfig.limit58) {
    overLimitFlag = true;
    blinkMpl_NoFPV = ((lvlMax_58 - displayConfig.limit58) / 50) + 1;
  }
  
  if ((ch2 * channel_58 * Lvl58_calibr[3] > displayConfig.limit58) && (fpv_disp_58_ended < dispersia_porog)) {
    overLimitFPVFlag = true;
    blinkMpl_FPV = ((Lvl58_calibr[3] - displayConfig.limit58) / 50) + 1;
  }

  if (blinkMpl_NoFPV > 10) blinkMpl_NoFPV = 10;
  if (blinkMpl_FPV > 10) blinkMpl_FPV = 10;
  

  newIndicator_24 = (lvlMax_24 * indicatorMaxWidth) / signalMaxLevel;
  newIndicator_58 = (lvlMax_58 * indicatorMaxWidth) / signalMaxLevel;
  if (newIndicator_24 > indicatorMaxWidth) newIndicator_24 = indicatorMaxWidth;
  if (newIndicator_58 > indicatorMaxWidth) newIndicator_58 = indicatorMaxWidth;

  indlimit1 = (displayConfig.limit24 * indicatorMaxWidth) / signalMaxLevel;
  if (indlimit1 > indicatorMaxWidth) indlimit1 = indicatorMaxWidth;

  indlimit2 = (displayConfig.limit58 * indicatorMaxWidth) / signalMaxLevel;
  if (indlimit2 > indicatorMaxWidth) indlimit2 = indicatorMaxWidth;

  if (ch1) {
    if (lvlMax_24 >= displayConfig.limit24) {
      tft.fillRect(50, 41, indicatorMaxWidth, 17, background_color);
      tft.fillRect(50, 41, newIndicator_24, 17, redClr);
      indicator_24 = newIndicator_24;



    } else {
      tft.fillRect(50, 41, indicatorMaxWidth, 17, background_color);
      tft.fillRect(50, 41, newIndicator_24, 17, greenClr);
      indicator_24 = newIndicator_24;
    }
    tft.drawLine(50 + indlimit1, 38, 50 + indlimit1, 60, 0xFFFF);
  }
  if (ch2) {
    if (lvlMax_58 >= displayConfig.limit58) {
      tft.fillRect(50, 92, indicatorMaxWidth, 17, background_color);
      tft.fillRect(50, 92, newIndicator_58, 17, redClr);
      indicator_24 = newIndicator_24;

    } else {
      tft.fillRect(50, 92, indicatorMaxWidth, 17, background_color);
      tft.fillRect(50, 92, newIndicator_58, 17, greenClr);
      indicator_24 = newIndicator_24;
    }
    tft.drawLine(50 + indlimit2, 89, 50 + indlimit2, 111, 0xFFFF);
  }
}

void levelsDrawComplex(bool ch1, bool ch2) {
  for (int i = 0; i < dronsNumber; i++) {
    Lvl24_calibr[i] = (Lvl24[i] * displayConfig.calibrationCoeff24[i]) / 100;
    Lvl58_calibr[i] = (Lvl58[i] * displayConfig.calibrationCoeff58[i]) / 100;
  }

  overLimitFlag = false;
  overLimitFPVFlag = false;
  lvlMax_24 = 0;
  lvlMax_58 = 0;
  for (int i = 0; i < dronsNumber; i++) {
    if (Lvl24_calibr[i] > lvlMax_24) lvlMax_24 = Lvl24_calibr[i];
    if (Lvl58_calibr[i] > lvlMax_58) {
      if (i == 3) {
        if(fpv_disp_58_ended < dispersia_porog) {
          lvlMax_58 = Lvl58_calibr[i];
        }
      } else{
        lvlMax_58 = Lvl58_calibr[i];
      }
    }
  }

  if (ch1 * channel_24 * lvlMax_24 > displayConfig.limit24) {
    overLimitFlag = true;
    blinkMpl_NoFPV = ((lvlMax_24 - displayConfig.limit24) / 50) + 1;
  } else if (ch2 * channel_58 * lvlMax_58 > displayConfig.limit58) {
    overLimitFlag = true;
    blinkMpl_NoFPV = ((lvlMax_58 - displayConfig.limit58) / 50) + 1;
  }

  if (blinkMpl_NoFPV > 10) blinkMpl_NoFPV = 10;

  if ((ch2 * channel_58 * Lvl58_calibr[3] > displayConfig.limit58) && (fpv_disp_58_ended < dispersia_porog)) {
    overLimitFPVFlag = true;
    blinkMpl_FPV = ((Lvl58_calibr[3] - displayConfig.limit58) / 50) + 1;
  }
  if (blinkMpl_FPV > 10) blinkMpl_FPV = 10;
  

  indlimit1 = (displayConfig.limit24 * indicatorMaxWidth) / signalMaxLevel;
  if (indlimit1 > indicatorMaxWidth) indlimit1 = indicatorMaxWidth;

  indlimit2 = (displayConfig.limit58 * indicatorMaxWidth) / signalMaxLevel;
  if (indlimit2 > indicatorMaxWidth) indlimit2 = indicatorMaxWidth;

  if ((ch1) && (ch2)) {

    for (int i = 0; i < dronsNumber; i++) {
      if (channel_24 * Lvl24_calibr[i] > displayConfig.limit24) {
        drawLevelsComplex(Lvl24_calibr[i], i, "2.4", redClr);
        drawLimitComplx(indlimit1, i, true);
      } else if (channel_58 * Lvl58_calibr[i] > displayConfig.limit58) {
        drawLevelsComplex(Lvl58_calibr[i], i, "5.8", redClr);
        drawLimitComplx(indlimit2, i, true);
      } else if (channel_58 * Lvl58_calibr[i] >= channel_24 * Lvl24_calibr[i]) {
        drawLevelsComplex(Lvl58_calibr[i], i, "5.8", greenClr);
        drawLimitComplx(indlimit2, i, true);
      } else {
        drawLevelsComplex(Lvl24_calibr[i], i, "2.4", greenClr);
        drawLimitComplx(indlimit1, i, true);
      }
    }
  } else if (ch1) {
    for (int i = 0; i < dronsNumber; i++) {
      if (Lvl24_calibr[i] > displayConfig.limit24) {
        drawLevelsComplex(Lvl24_calibr[i], i, "2.4", redClr);
      } else {
        drawLevelsComplex(Lvl24_calibr[i], i, "2.4", greenClr);
      }
      drawLimitComplx(indlimit1, i, true);
    }
  } else {
    for (int i = 0; i < dronsNumber; i++) {
      if (Lvl58_calibr[i] > displayConfig.limit58) {
        drawLevelsComplex(Lvl58_calibr[i], i, "5.8", redClr);
        drawLimitComplx(indlimit2, i, true);
      } else {
        drawLevelsComplex(Lvl58_calibr[i], i, "5.8", greenClr);
        drawLimitComplx(indlimit2, i, true);
      }
    }
  }
}


void IND_levelsDraw(bool flag24, bool flag58) {
  if (!simpleDimpleFlag) {
    IND_levelsDrawSimple(flag24, flag58);
  } else {
    IND_levelsDrawComplex(flag24, flag58);
  }
}

void IND_levelsDrawSimple(bool ch1, bool ch2) {
  lvlMax_24 = 0;
  lvlMax_58 = 0;
  AlarmMessage m{ALARM_NONE, 1, displayConfig.soundOffFlag};
  bool FPV_indicator = false;

  if (xSemaphoreTake(xMutexIndicators, portMAX_DELAY)) {
    for (size_t i = 0; i < indicator_count; i++) {
      Indicator_t* ind = &indicators[i];
      if (!ind->isActive) continue;
      if (!ind->isFPV) {
        int val24 = (ind->value_24 * ind->calibrationCoeff_24 / 100);
        int val58 = (ind->value_58 * ind->calibrationCoeff_58 / 100);

        lvlMax_24 = max(val24, lvlMax_24);
        // lvlMax_58 = max(val58, lvlMax_58);

        if (val58 > lvlMax_58) {
          lvlMax_58 = val58;
          FPV_indicator = false;
        }
      }
      if (ind->isFPV) {
        if (fpv_disp_58_ended < dispersia_porog) {  
          // lvlMax_58 = max(ind->value_58, lvlMax_58);
          if (ind->value_58 > lvlMax_58) {
          lvlMax_58 = ind->value_58;
          FPV_indicator = true;
        }
        }
      }
      
    }
    xSemaphoreGive(xMutexIndicators);
  }

  int max_dif = max((ch1 * channel_24 * lvlMax_24 - displayConfig.limit24), (ch2 * channel_58 * lvlMax_58 - displayConfig.limit58));
  if (max_dif > 0) {
    m.mode = FPV_indicator ? ALARM_FPV : ALARM_OTHER;
    m.coeff = (uint8_t)(max_dif / ALARM_COEFF_SCALE + 1);
  }

  xQueueSend(xAlarmQueue, &m, 10);

 

  newIndicator_24 = (lvlMax_24 * indicatorMaxWidth) / signalMaxLevel;
  newIndicator_58 = (lvlMax_58 * indicatorMaxWidth) / signalMaxLevel;
  if (newIndicator_24 > indicatorMaxWidth) newIndicator_24 = indicatorMaxWidth;
  if (newIndicator_58 > indicatorMaxWidth) newIndicator_58 = indicatorMaxWidth;

  indlimit1 = (displayConfig.limit24 * indicatorMaxWidth) / signalMaxLevel;
  if (indlimit1 > indicatorMaxWidth) indlimit1 = indicatorMaxWidth;

  indlimit2 = (displayConfig.limit58 * indicatorMaxWidth) / signalMaxLevel;
  if (indlimit2 > indicatorMaxWidth) indlimit2 = indicatorMaxWidth;

  uint16_t temp_color;
  if (ch1) {
    temp_color = ((lvlMax_24 >= displayConfig.limit24) ? redClr : greenClr);
    tft.fillRect(50, 41, indicatorMaxWidth, 17, background_color);
    tft.fillRect(50, 41, newIndicator_24, 17, temp_color);
    tft.drawLine(50 + indlimit1, 38, 50 + indlimit1, 60, 0xFFFF);
  }
  if (ch2) {
    temp_color = ((lvlMax_58 >= displayConfig.limit58) ? redClr : greenClr);
    tft.fillRect(50, 92, indicatorMaxWidth, 17, background_color);
    tft.fillRect(50, 92, newIndicator_58, 17, temp_color);
    tft.drawLine(50 + indlimit2, 89, 50 + indlimit2, 111, 0xFFFF);
  }
}

typedef struct {
  char name[16];
  int index;      // исходный индекс дрона
  int lvl24;      // уровень 2.4
  int lvl58;      // уровень 5.8
  int threshold_24;
  int threshold_58;
  int diff24;     // превышение порога 2.4
  int diff58;     // превышение порога 5.8
  bool isActive;
  bool isFPV;
} DroneLevel;

DroneLevel sorted[dronsNumber];

void IND_levelsDrawComplex(bool ch1, bool ch2) {

  indlimit1 = (displayConfig.limit24 * indicatorMaxWidth) / signalMaxLevel;
  if (indlimit1 > indicatorMaxWidth) indlimit1 = indicatorMaxWidth;

  indlimit2 = (displayConfig.limit58 * indicatorMaxWidth) / signalMaxLevel;
  if (indlimit2 > indicatorMaxWidth) indlimit2 = indicatorMaxWidth;
  

  // --- Подготовка данных ---
  if (xSemaphoreTake(xMutexIndicators, portMAX_DELAY)) {
    for (size_t i = 0; i < indicator_count; i++) {
      Indicator_t* ind = &indicators[i];
      // if (!ind->isActive) con tinue;

      sorted[i].index = i;
      sorted[i].lvl24 = ch1 * channel_24 * (ind->value_24 * ind->calibrationCoeff_24 / 100);
      sorted[i].lvl58 = ch2 * channel_58 * (ind->value_58 * ind->calibrationCoeff_58 / 100);
      sorted[i].diff24 = sorted[i].lvl24 - displayConfig.limit24;
      sorted[i].diff58 = sorted[i].lvl58 - displayConfig.limit58;
      sorted[i].isActive = ind->isActive;
      sorted[i].isFPV = ind->isFPV;
      // sorted[i].name = ind->name;
      strncpy(sorted[i].name, ind->name, sizeof(ind->name) - 1);

    }
    xSemaphoreGive(xMutexIndicators);
  }

  // --- Сортировка по приоритету: сначала превышение, потом уровень ---
  for (int i = 0; i < indicator_count - 1; i++) {
    for (int j = i + 1; j < indicator_count; j++) {

      // 1) Активные должны идти раньше
      if (sorted[i].isActive != sorted[j].isActive) {
        if (!sorted[i].isActive) {
          DroneLevel t = sorted[i];
          sorted[i] = sorted[j];
          sorted[j] = t;
        }
        continue;
      }
      
      // 2) Сравнение по превышению порогов
      int pri_i = max(sorted[i].diff24, sorted[i].diff58);
      int pri_j = max(sorted[j].diff24, sorted[j].diff58);

      // если оба ниже порогов — сортируем по максимальному уровню
      if (pri_i <= 0 && pri_j <= 0) {
        pri_i = max(sorted[i].lvl24, sorted[i].lvl58);
        pri_j = max(sorted[j].lvl24, sorted[j].lvl58);
      }

      if (pri_j > pri_i) {
        DroneLevel tmp = sorted[i];
        sorted[i] = sorted[j];
        sorted[j] = tmp;
      }
    }
  }

  // --- Отправляем тревогу ---
  AlarmMessage m{ALARM_NONE, 1, displayConfig.soundOffFlag};

  for (int i = 0; i < indicator_count; i++) {
    DroneLevel *top = &sorted[i];

    if (!top->isActive) break;   // не надо тревожить по выключенным

    int max_dif = max(top->diff24, top->diff58);

    if (max_dif < 0) break; // Порог не превышен
    
    if (top->isFPV) {
      // Если дисперсия больше порога, то это не похоже на ФПВ сигнал, идем к следующему дрону
      if (fpv_disp_58_ended >= dispersia_porog) continue; 
    }

    m.mode = top->isFPV ? ALARM_FPV : ALARM_OTHER;
    m.coeff = (uint8_t)(max_dif / ALARM_COEFF_SCALE + 1);

    break;
  }

  xQueueSend(xAlarmQueue, &m, 10);

  // --- Отрисовка отсортированных индикаторов ---
  for (int k = 0; k < indicator_count; k++) {
    int i = sorted[k].index;
    int lvl24 = sorted[k].lvl24;
    int lvl58 = sorted[k].lvl58;
    int diff24 = sorted[k].diff24;
    int diff58 = sorted[k].diff58;
    char name[16];
    strncpy(name, sorted[k].name, sizeof(sorted[k].name) - 1);

    String band;
    uint16_t color;
    int limitIndex;
    int lvl;

    if (ch1 && ch2) {
      if (diff24 > 0 && diff58 > 0) {
        // оба выше порога → выбираем большее превышение
        if (diff24 >= diff58)
          band = "2.4", lvl = lvl24, color = redClr, limitIndex = indlimit1;
        else
          band = "5.8", lvl = lvl58, color = redClr, limitIndex = indlimit2;
      } 
      else if (diff24 > 0) band = "2.4", lvl = lvl24, color = redClr, limitIndex = indlimit1;
      else if (diff58 > 0) band = "5.8", lvl = lvl58, color = redClr, limitIndex = indlimit2;
      else if (lvl58 >= lvl24) band = "5.8", lvl = lvl58, color = greenClr, limitIndex = indlimit2;
      else band = "2.4", lvl = lvl24, color = greenClr, limitIndex = indlimit1;
    } 
    else if (ch1)
      band = "2.4", lvl = lvl24,
      color = (lvl > displayConfig.limit24) ? redClr : greenClr,
      limitIndex = indlimit1;
    else if (ch2)
      band = "5.8", lvl = lvl58,
      color = (lvl > displayConfig.limit58) ? redClr : greenClr,
      limitIndex = indlimit2;
    else
      continue;

    tft.setTextSize(1);
    tft.setCursor(3, 32 + 16*k);
    tft.print("    ");
    tft.setCursor(3, 32 + 16*k);
    tft.print(name);
    tft.setTextSize(2);
    
    drawLevelsComplex(lvl, k, band, color);  // k — уже отсортированная позиция
    drawLimitComplx(limitIndex, k, true);
  }

}

void drawLevelsComplex(int val, int row, String baud, int color) {
  int indVal = (val * indicatorMaxWidth) / signalMaxLevel;
  if (indVal > indicatorMaxWidth) indVal = indicatorMaxWidth;
  tft.setTextSize(1);
  tft.setCursor(31, 32 + (row * 16));
  tft.print(baud);
  tft.fillRect(53, 31 + (row * 16), 102, 9, background_color);
  tft.fillRect(54, 32 + (row * 16), indVal, 7, color);
  tft.setTextSize(2);
}

void ind_drawLimitComplx(int val, int row, int clr) {
  int x = 54 + val;
  int y1 = 31 + (row * 16);
  int y2 = y1 + 8;
  tft.drawLine(x, y1, x, y2, clr);
  tft.drawPixel(x - 1, y1, clr);
  tft.drawPixel(x + 1, y1, clr);
  tft.drawPixel(x - 1, y2, clr);
  tft.drawPixel(x + 1, y2, clr);
}

void drawLimitComplx(int val, int row, bool clr) {
  int x = 54 + val;
  int y1 = 31 + (row * 16);
  int y2 = y1 + 8;
  tft.drawLine(x, y1, x, y2, clr * (0xFFFF));
  tft.drawPixel(x - 1, y1, clr * (0xFFFF));
  tft.drawPixel(x + 1, y1, clr * (0xFFFF));
  tft.drawPixel(x - 1, y2, clr * (0xFFFF));
  tft.drawPixel(x + 1, y2, clr * (0xFFFF));
}

void drawLimitSimple(int val, bool row, bool clr) {
  tft.drawBitmap(50 + val - 2, 33 + (row * 51), image_SmallArrowDown_bits, 5, 3, clr * 0xffff);
  tft.drawBitmap(50 + val - 2, 63 + (row * 51), image_SmallArrowUp_bits, 5, 3, clr * 0xffff);
  tft.drawLine(50 + val, 38 + (row * 51), 50 + val, 60 + (row * 51), clr * 0xffff);
}


// состояние ввода порога
static bool editingThreshold = false;
static int editDigits[3] = { -1, -1, -1 };  // -1 = ещё не введено
static int editPos = 0;                     // сколько цифр введено
static int originalValue = 0;               // исходное значение

void menuThresholds() {
  if (!simpleDimpleFlag) {
    menuThresholdsSimple();
  } else {
    menuThresholdsComplex();
  }
}

void menuThresholdsSimple() {

  tft.fillRect(0, 25, 160, 45, background_color);
  tft.fillRect(0, 79, 160, 45, background_color);

  if (!thresholds_24_58) {
    tft.setCursor(5, 43);
    tft.print("2.4");
    tft.drawRect(46, 37, 112, 25, 0xFFFF);
    IND_levelsDraw(true, false);
    drawLimitSimple(indlimit1, false, true);

    tft.setCursor(21, 95);
    tft.print(get_str(STR_LIMIT));

    tft.drawBitmap(119, 114, image_SmallArrowDown_bits, 5, 3, 0xFFFF);
    tft.drawBitmap(119, 86, image_SmallArrowUp_bits, 5, 3, 0xFFFF);

    tft.setCursor(105, 95);
    // if ((displayConfig.limit24 / 100) == 0) {
    //   tft.print("0");
    //   tft.setCursor(117, 95);
    // }
    // if ((displayConfig.limit24 / 10) == 0) {
    //   tft.print("0");
    //   tft.setCursor(129, 95);
    // }
    // tft.print(String(displayConfig.limit24));

    if (editingThreshold) {
      for (int i = 0; i < 3; i++) {
        if (editDigits[i] >= 0) {
          tft.print(editDigits[i]);
        } else {
          tft.print("_"); // Раньше был белый фон при наборе
        }
      }
    } else {
      if (displayConfig.limit24 < 100) tft.print("0");
      if (displayConfig.limit24 < 10)  tft.print("0");
      tft.print(displayConfig.limit24);
    }
  } else {
    tft.setCursor(5, 94);
    tft.print("5.8");
    tft.drawRect(46, 88, 112, 25, 0xFFFF);

    IND_levelsDraw(false, true);
    drawLimitSimple(indlimit2, true, true);

    tft.setCursor(21, 42);
    tft.print(get_str(STR_LIMIT));

    tft.drawBitmap(119, 61, image_SmallArrowDown_bits, 5, 3, 0xFFFF);
    tft.drawBitmap(119, 33, image_SmallArrowUp_bits, 5, 3, 0xFFFF);

    tft.setCursor(105, 42);
    // if ((displayConfig.limit58 / 100) == 0) {
    //   tft.print("0");
    //   tft.setCursor(117, 42);
    // }
    // if ((displayConfig.limit58 / 10) == 0) {
    //   tft.print("0");
    //   tft.setCursor(129, 42);
    // }
    // tft.print(String(displayConfig.limit58));

    if (editingThreshold) {
      for (int i = 0; i < 3; i++) {
        if (editDigits[i] >= 0) {
          tft.print(editDigits[i]);
        } else {
          tft.print("_");
        }
      }
    } else {
      if (displayConfig.limit58 < 100) tft.print("0");
      if (displayConfig.limit58 < 10)  tft.print("0");
      tft.print(displayConfig.limit58);
    }
  }
}

void menuThresholdsComplex() {
  if (!thresholds_24_58) {
    tft.setCursor(12, 6);
    tft.setTextColor(background_color,background_color);
    tft.print(get_str(STR_SEARCH_PLUS));

    tft.setTextColor(0xffff,background_color);
    tft.setCursor(12, 6);
    tft.print(get_str(STR_LIMIT));

    tft.drawBitmap(83, 11, image_ButtonLeftSmall_bits, 3, 5, 0xFFFF);
    tft.drawBitmap(126, 11, image_ButtonRightSmall_bits, 3, 5, 0xFFFF);
    
    IND_levelsDraw(true, false);
    for (int i = 0; i < dronsNumber; i++) {
      drawLimitComplx(indlimit1, i, true);
    }

    tft.setCursor(89, 6);
    if (editingThreshold) {
      for (int i = 0; i < 3; i++) {
        if (editDigits[i] >= 0) {
          tft.print(editDigits[i]);
        } else {
          tft.print("_");
        }
      }
    } else {
      if (displayConfig.limit24 < 100) tft.print("0");
      if (displayConfig.limit24 < 10)  tft.print("0");
      tft.print(displayConfig.limit24);
    }

  } else {
    tft.setCursor(12, 6);
    tft.setTextColor(background_color,background_color);
    tft.print(get_str(STR_SEARCH_PLUS));
  
    tft.setTextColor(0xffff,background_color);
    tft.setCursor(12, 6);
    tft.print(get_str(STR_LIMIT));

    tft.drawBitmap(83, 11, image_ButtonLeftSmall_bits, 3, 5, 0xFFFF);
    tft.drawBitmap(126, 11, image_ButtonRightSmall_bits, 3, 5, 0xFFFF);
    
    IND_levelsDraw(false, true);
    for (int i = 0; i < dronsNumber; i++) {
      drawLimitComplx(indlimit2, i, true);
    }

    tft.setCursor(89, 6);
    if (editingThreshold) {
      for (int i = 0; i < 3; i++) {
        if (editDigits[i] >= 0) {
          tft.print(editDigits[i]);
        } else {
          tft.print("_");
        }
      }
    } else {
      if (displayConfig.limit58 < 100) tft.print("0");
      if (displayConfig.limit58 < 10)  tft.print("0");
      tft.print(displayConfig.limit58);
    }
  }
}

void clearMenu() {
  tft.fillScreen(background_color);
  tft.drawLine(0, 23, 159, 23, 0xAD55);
  drawBattery();
}

int indexxx = 1;
int indexxx0 = 0;
int indexxx2 = 0;

StringID textMenu[menu_number] = {
  STR_LIGHT,
  STR_CALIBRATION,
  STR_CHANNELS,
  STR_DRONS,
  STR_AUTOBLOCK,
  STR_SLEEP,
  STR_RESET,
  STR_OSCILLOGRAM,
  STR_INFO
};

void menu() {
  clearMenu();
  tft.setCursor(12, 6);
  tft.print(get_str(STR_SETTINGS));
  
  indexxx0 = (indexxx - 1) == -1 ? (menu_number - 1) : (indexxx - 1);
  indexxx2 = (indexxx + 1) == menu_number ? (0) : (indexxx + 1);

  drawMenuRect(1, 0xffff); // rect in middle
  drawMenuSettings();
}

void drawMenuSettings() {
  // tft.drawBitmap(icons_x, icon1_y, bitmap_icons[indexxx0], 24, 24, icons_colors[indexxx0], background_color);
  tft.drawBitmap(icons_x, icon1_y, MENU_ICONS[indexxx0].bitmap, MENU_ICONS[indexxx0].width, MENU_ICONS[indexxx0].height, MENU_ICONS[indexxx0].color, background_color);
  tft.setCursor(strings_x, string1_y);
  tft.print(get_str(textMenu[indexxx0]));

  // tft.drawBitmap(icons_x, icon2_y, bitmap_icons[indexxx], 24, 24, icons_colors[indexxx], background_color);
  tft.drawBitmap(icons_x, icon2_y, MENU_ICONS[indexxx].bitmap, MENU_ICONS[indexxx].width, MENU_ICONS[indexxx].height, MENU_ICONS[indexxx].color, background_color);
  tft.setCursor(strings_x, string2_y);
  tft.print(get_str(textMenu[indexxx]));

  // tft.drawBitmap(icons_x, icon3_y, bitmap_icons[indexxx2], 24, 24, icons_colors[indexxx2], background_color);
  tft.drawBitmap(icons_x, icon3_y, MENU_ICONS[indexxx2].bitmap, MENU_ICONS[indexxx2].width, MENU_ICONS[indexxx2].height, MENU_ICONS[indexxx2].color, background_color);
  tft.setCursor(strings_x, string3_y);
  tft.print(get_str(textMenu[indexxx2]));
}

void drawMenuRect(byte position, uint16_t color) {
  if (position == 0) {
    drawMenuRect_up(color);
  } else if (position == 1) {
    drawMenuRect_centr(color);
  } else if (position == 2) {
    drawMenuRect_down(color);
  }
}

void drawMenuRect_centr(uint16_t color) {     //Center
  tft.drawRect(2, 63, 155, 28, color);
  tft.drawLine(157, 65, 157, 89, color);
  tft.drawLine(4, 91, 155, 91, color);
  tft.drawPixel(2, 63, background_color);
  tft.drawPixel(2, 64, background_color);
  tft.drawPixel(2, 90, background_color);
  tft.drawPixel(3, 63, background_color);
  tft.drawPixel(3, 64, color);
  tft.drawPixel(156, 63, background_color);
}

void drawMenuRect_up(uint16_t color) {     //Up
  tft.drawRect(2, 28, 155, 28, color);
  tft.drawLine(157, 30, 157, 54, color);
  tft.drawLine(4, 56, 155, 56, color);
  tft.drawPixel(2, 28, background_color);
  tft.drawPixel(2, 29, background_color);
  tft.drawPixel(2, 55, background_color);
  tft.drawPixel(3, 28, background_color);
  tft.drawPixel(3, 29, color);
  tft.drawPixel(156, 28, background_color);
}

void drawMenuRect_down(uint16_t color) {     //Down
  tft.drawRect(2, 98, 155, 28, color);
  tft.drawLine(157, 100, 157, 124, color);
  tft.drawLine(4, 126, 155, 126, color);
  tft.drawPixel(2, 98, background_color);
  tft.drawPixel(2, 99, background_color);
  tft.drawPixel(2, 125, background_color);
  tft.drawPixel(3, 98, background_color);
  tft.drawPixel(3, 99, color);
  tft.drawPixel(156, 98, background_color);
}




void lightMenu(int action) {
  switch (action) {
    case 0:
      clearMenu();
      tft.setCursor(12, 6);
      tft.print(get_str(STR_LIGHT_2));

      tft.drawBitmap(68, 38, image_light, 24, 24, ST7735_YELLOW);
      tft.drawRect(28, 67, 104, 20, 0xFFFF);
      tft.drawBitmap(20, 73, arrow_left, 4, 8, 0xFFFF);
      tft.drawBitmap(136, 73, arrow_right, 4, 8, 0xFFFF);

      for (int i = 0; i < displayConfig.brightness; i++) {
        tft.fillRect(31 + (i*10), 70, 8, 14, 0xFFFF);
      }

      analogWriteFreq(1000);
      analogWriteRange(256);
      break;

    case 1: // down button
      if (displayConfig.brightness > 0) {
        displayConfig.brightness--;
        tft.fillRect(31 + ((displayConfig.brightness)*10), 70, 8, 14, background_color);
        analogWrite(DISPLAY_LED, (displayConfig.brightness) * briStep + briMin);
      }
      break;
    case 2: // up button
      if (displayConfig.brightness < 10) {
        displayConfig.brightness++;
        tft.fillRect(31 + ((displayConfig.brightness - 1)*10), 70, 8, 14, 0xFFFF);
        if (displayConfig.brightness == 10) {
          digitalWrite(DISPLAY_LED, HIGH);
        } else {
          analogWrite(DISPLAY_LED, (displayConfig.brightness) * briStep + briMin);
        }
      }
      break;
    case 3: // gr button
      break;
    
    case 4: // back button
      ConfigMessage_t change = { .type = CONFIG_SET, .field = CFG_BRIGHTNESS };
      change.value.brightness = displayConfig.brightness;
      xQueueSend(configQueue, &change, portMAX_DELAY);

      // uint32_t Message = 33;
      // xQueueSend(xQueueSerial, &Message, portMAX_DELAY);

      handle_menu_mode(1);
      break;
  }
}

void drawYesNoButtons(bool yes) {
  // Левая кнопка
  tft.fillRect(30, 89, 44, 24, yes ? background_color : 0xFFFF);
  if (yes) tft.drawRect(30, 89, 44, 24, 0xFFFF);
  tft.setTextColor(yes ? 0xFFFF : background_color, background_color);
  tft.setCursor(textStrX[2], 94);
  tft.print(get_str(STR_NO));

  // Правая кнопка  
  tft.fillRect(86, 89, 44, 24, yes ? 0xFFFF : background_color);
  if (!yes) tft.drawRect(86, 89, 44, 24, 0xFFFF);
  tft.setTextColor(yes ? background_color : 0xFFFF, background_color);
  tft.setCursor(textStrX[3], 94);
  tft.print(get_str(STR_YES));

  tft.setTextColor(0xFFFF, background_color);
}

void calibrate(int action, const DronsMessage_t* data ) {
  static int rects = 0;
  static int counter = 0;
  static float percents = 0.f;
  const float percentsPerData = (float)(100.f / (float)displayConfig.calibrNumber); // СКОЛЬКО ПРОЦЕНТОВ НА ОДИН ПРОГОН
  const float percentsPerRect = 100 / 25; // СКОЛЬКО ПРОЦЕНТОВ НА ОДИН КВАДРАТИК. ВСЕГО 25 КВАДРАТИКОВ НА ЗАГРУЗКЕ

  static int calibrationSet24[dronsNumber];
  static int calibrationSet58[dronsNumber];

  int calibrMin24;
  int calibrMin58;  

  String message;

  switch (action) {
    case 0:
      clearMenu();
      
      menu_mode = MENU_CALIBRATION;
      
      tft.setCursor(6, 6); 
      tft.print(get_str(STR_CALIBRATION));
      
      tft.drawBitmap(68, 38, image_calibration, 24, 24, 0x8e5f);
      tft.drawRect(29, 77, 103, 10, 0xFFFF);
      tft.fillRect(31, 79, 3, 6, 0xFFFF);
      
      rects = 0;
      counter = 0;
      percents = 0.f;
      memset(calibrationSet24, 0, sizeof(calibrationSet24));
      memset(calibrationSet58, 0, sizeof(calibrationSet58));
      
      break;

    case 1:
      if (data == NULL) break;
      if (counter < displayConfig.calibrNumber) {
        counter++;

        for (int i = 0; i < dronsNumber; i++) {
          calibrationSet24[i] += data->drons_24[i];
          calibrationSet58[i] += data->drons_58[i];
        }

        percents += percentsPerData;
        tft.setCursor(63, 95);
        tft.print(String((int)percents) + "%");

        while (((rects + 1) * percentsPerRect) < percents) {
          if (rects < 25) {
            rects++;
            tft.fillRect(31 + (4 * rects), 79, 3, 6, 0xFFFF);
          }
        }
      } else {
        for (int i = 0; i < dronsNumber; i++) {
          calibrationSet24[i] /= displayConfig.calibrNumber;
          calibrationSet58[i] /= displayConfig.calibrNumber;
        }
        
        calibrMin24 = calibrationSet24[0];
        calibrMin58 = calibrationSet58[0];
      
        // if (i != 3) для значений FPV

        for (int i = 0; i < dronsNumber; i++) {
          if (calibrationSet24[i] < calibrMin24) {
            if (i != 3) calibrMin24 = calibrationSet24[i];
          }
          if (calibrationSet58[i] < calibrMin58) {
            if (i != 3) calibrMin58 = calibrationSet58[i];
          }
        }
      
        for (int i = 0; i < dronsNumber; i++) {
          if (i != 3 && calibrationSet24[i] != 0) displayConfig.calibrationCoeff24[i] = ((calibrMin24 * 100) / calibrationSet24[i]);
          if (i != 3 && calibrationSet58[i] != 0) displayConfig.calibrationCoeff58[i] = ((calibrMin58 * 100) / calibrationSet58[i]);
        }
        
        ConfigMessage_t change = { .type = CONFIG_SET, .field = CFG_CALIBR_COEFF24 };
        memcpy(change.value.calibrationCoeff24, displayConfig.calibrationCoeff24, dronsNumber);
        xQueueSend(configQueue, &change, portMAX_DELAY);
        
        change = { .type = CONFIG_SET, .field = CFG_CALIBR_COEFF58 };
        memcpy(change.value.calibrationCoeff58, displayConfig.calibrationCoeff58, dronsNumber);
        xQueueSend(configQueue, &change, portMAX_DELAY);
        
        calibrFlag = true;
        menu_mode = MENU_DEFAULT;

        handle_menu_mode(1);
        
      }
      break;
    
    case 2: // ОТМЕНА КАЛИБРОВКИ
      if (menu_mode == MENU_CALIBRATION) {
        menu_mode = MENU_DEFAULT;
        handle_menu_mode(1);
        return;
      }
      break;

  }
}

void calibrMenu(int action) {
  static bool calibrChoise = false;

  if (menu_mode == MENU_DEFAULT) {
    switch (action) {
      case 0:
        clearMenu();
        tft.setCursor(6, 6); 
        tft.print(get_str(STR_CALIBRATION));
        
        tft.setCursor(textStrX[0], 37);
        tft.print(get_str(STR_PERFORM));

        tft.setCursor(textStrX[1], 59);
        tft.print(get_str(STR_CALIBRATION_Q));

        calibrChoise = false;
        drawYesNoButtons(calibrChoise);

        break;

      case 1: // down button
        calibrChoise = !calibrChoise;
        drawYesNoButtons(calibrChoise);

        break;

      case 2: // up button
        calibrChoise = !calibrChoise;
        drawYesNoButtons(calibrChoise);

        break;

      case 3: // gr button
        calibrChoise ? calibrate(0, NULL) : handle_menu_mode(1);

        break;

      case 4: // red button
        handle_menu_mode(1);
        break;
    }
  }
  else if (menu_mode == MENU_CALIBRATION) {
    switch (action) {
      case 4: // red button
        calibrate(2, NULL);

        break;
    }
  }
}


#define bullets_x 10
#define bullets_y1 34
#define bullets_y2 69
#define bullets_y3 104

void channelsMenu(int action) {
  static byte channelsIndex = 0;
  const int bullets_y[] = {bullets_y1, bullets_y2, bullets_y3};
  const int string_y[] = {string1_y, string2_y, string3_y};
  const bool conditions[] = {channel_24 && channel_58, !channel_58, !channel_24};
  const char* labels[] = {"2.4 + 5.8", "2.4 ", "5.8 "};
  const bool ch24_states[] = {true, true, false};
  const bool ch58_states[] = {true, false, true};


  switch (action) {
    case 0:
      clearMenu();
      tft.setCursor(12, 6);
      tft.print(get_str(STR_CHANNELS_2));


      
      for (int i = 0; i < 3; i++) {
        tft.drawBitmap(bullets_x, bullets_y[i], 
                      conditions[i] ? image_choice_bullet_on_bits : image_choice_bullet_off_bits, 
                      15, 15, 0xffff, background_color);
        tft.setCursor(strings_x, string_y[i]);
        tft.print(labels[i]);
        if (i > 0) tft.print(get_str(STR_GGH));
      }
      
      channelsIndex = 0;
      drawMenuRect(channelsIndex, 0xFFFF);

      break;

    case 1: // down button
      if (channelsIndex < 2) {
        drawMenuRect(channelsIndex, background_color);
        channelsIndex++;
        drawMenuRect(channelsIndex, 0xFFFF);
      }

      break;
    case 2: // up button
      if (channelsIndex > 0) {
        drawMenuRect(channelsIndex, background_color);
        channelsIndex--;
        drawMenuRect(channelsIndex, 0xFFFF);
      }
      break;
    case 3: // gr button
      
      channel_24 = ch24_states[channelsIndex];
      channel_58 = ch58_states[channelsIndex];
      
      for (int i = 0; i < 3; i++) {
        tft.drawBitmap(bullets_x, bullets_y[i], 
                      (i == channelsIndex) ? image_choice_bullet_on_bits : image_choice_bullet_off_bits,
                      15, 15, 0xffff, background_color);
      }

      break;

    case 4:
      handle_menu_mode(1);
      break;
  }
}

StringID timeStrings[TIME_STRINGS_Q] = {
  STR_TIME_OFF,
  STR_TIME_5,
  STR_TIME_10,
  STR_TIME_15,
  STR_TIME_30
};

void blockMenu(int action) {
  switch (action) {
    case 0:
      clearMenu();
      tft.setCursor(12, 6);
      tft.print(get_str(STR_AUTOBLOCK_2));

      tft.drawBitmap(68, 38, image_locked_icon_bits, 24, 24, ST7735_ORANGE);
      tft.drawRect(28, 67, 104, 20, 0xFFFF);
      tft.drawBitmap(20, 73, arrow_left, 4, 8, 0xFFFF);
      tft.drawBitmap(136, 73, arrow_right, 4, 8, 0xFFFF);

      if (displayConfig.blockChoice == 0) {
        tft.setCursor(textStrX[6], 70);
      } else {
        tft.setCursor(45, 70);
      }
      tft.print(get_str(timeStrings[displayConfig.blockChoice]));

      break;

    case 1: // down button
      if (displayConfig.blockChoice > 0) {
        displayConfig.blockChoice--;
        
        if (displayConfig.blockChoice == 0) {
          tft.setCursor(textStrX[6], 70);
        } else {
          tft.setCursor(45, 70);
        }
        tft.print(get_str(timeStrings[displayConfig.blockChoice]));
      }

      break;
    case 2: // up button
      if (displayConfig.blockChoice < TIME_STRINGS_Q - 1) {
        displayConfig.blockChoice++;

        tft.setCursor(45, 70);
        tft.print(get_str(timeStrings[displayConfig.blockChoice]));
      }

      break;
    case 3: // gr button
      break;   

    case 4: // back button
      ConfigMessage_t change = { .type = CONFIG_SET, .field = CFG_BLOCK_CHOICE };
      change.value.blockChoice = displayConfig.blockChoice;
      xQueueSend(configQueue, &change, portMAX_DELAY);

      handle_menu_mode(1);
      break;
  }
}

void sleepMenu(int action) {
  switch (action) {
    case 0:
      clearMenu();
      tft.setCursor(12, 6);
      tft.print(get_str(STR_SLEEP_2));

      tft.drawBitmap(68, 38, image_sleep_mode_icon_bits, 24, 24, 0xb47f);
      tft.drawRect(28, 67, 104, 20, 0xFFFF);
      tft.drawBitmap(20, 73, arrow_left, 4, 8, 0xFFFF);
      tft.drawBitmap(136, 73, arrow_right, 4, 8, 0xFFFF);

      if (displayConfig.sleepChoice == 0) {
        tft.setCursor(textStrX[6], 70);
      } else {
        tft.setCursor(45, 70);
      }

      tft.print(get_str(timeStrings[displayConfig.sleepChoice]));

      break;

    case 1: // down button
      if (displayConfig.sleepChoice > 0) {
        displayConfig.sleepChoice--;

        if (displayConfig.sleepChoice == 0) {
          tft.setCursor(textStrX[6], 70);
        } else {
          tft.setCursor(45, 70);
        }
        tft.print(get_str(timeStrings[displayConfig.sleepChoice]));
      }

      break;

    case 2: // up button
      if (displayConfig.sleepChoice < TIME_STRINGS_Q - 1) {
        displayConfig.sleepChoice++;

        tft.setCursor(45, 70);
        tft.print(get_str(timeStrings[displayConfig.sleepChoice]));
      }

      break;

    case 3: // gr button
      break;

    case 4: // back button
      ConfigMessage_t change = { .type = CONFIG_SET, .field = CFG_SLEEP_CHOICE };
      change.value.sleepChoice = displayConfig.sleepChoice;
      xQueueSend(configQueue, &change, portMAX_DELAY);

      handle_menu_mode(1);
      break;
  }
}

void reset_settings() {
  clearMenu();
  tft.setCursor(6, 6); 
  tft.print(get_str(STR_RESET_2));

  displayConfig.brightness = 10;
  analogWriteFreq(1000);
  analogWriteRange(256);
  digitalWrite(DISPLAY_LED, HIGH);

  channel_24 = true;
  channel_58 = true;
  displayConfig.blockChoice = 0;
  displayConfig.sleepChoice = 0;

  for (int i = 0; i < dronsNumber; i++) {
    displayConfig.calibrationCoeff24[i] = 100;
    displayConfig.calibrationCoeff58[i] = 100;
  }
  calibrFlag = false;

  ConfigMessage_t change;
  change = { .type = CONFIG_SET, .field = CFG_BRIGHTNESS };
  change.value.brightness = displayConfig.brightness;
  xQueueSend(configQueue, &change, portMAX_DELAY);
  
  change = { .type = CONFIG_SET, .field = CFG_BLOCK_CHOICE };
  change.value.blockChoice = displayConfig.blockChoice;
  xQueueSend(configQueue, &change, portMAX_DELAY);

  change = { .type = CONFIG_SET, .field = CFG_SLEEP_CHOICE };
  change.value.sleepChoice = displayConfig.sleepChoice;
  xQueueSend(configQueue, &change, portMAX_DELAY);

  change = { .type = CONFIG_SET, .field = CFG_CALIBR_COEFF24 };
  memcpy(change.value.calibrationCoeff24, displayConfig.calibrationCoeff24, dronsNumber);
  xQueueSend(configQueue, &change, portMAX_DELAY);

  change = { .type = CONFIG_SET, .field = CFG_CALIBR_COEFF58 };
  memcpy(change.value.calibrationCoeff58, displayConfig.calibrationCoeff58, dronsNumber);
  xQueueSend(configQueue, &change, portMAX_DELAY);

  tft.setTextSize(1);
  tft.setCursor(12, 34); 
  tft.print(get_str(STR_LIGHT));
  tft.setCursor(12, 43); 
  tft.print(get_str(STR_CALIBRATION));
  tft.setCursor(12, 52); 
  tft.print(get_str(STR_CHANNELS));
  tft.setCursor(12, 61); 
  tft.print(get_str(STR_AUTOBLOCK));
  tft.setCursor(12, 70); 
  tft.print(get_str(STR_SLEEP));

  tft.setCursor(102, 34); 
  tft.print("10");
  tft.setCursor(102, 43); 
  tft.print("--");
  tft.setCursor(102, 52); 
  tft.print("2.4 + 5.8");
  tft.setCursor(102, 61); 
  tft.print(get_str(STR_OFF));
  tft.setCursor(102, 70); 
  tft.print(get_str(STR_OFF));

  delay(3000);
  
  tft.setTextSize(2);
  tft.setTextColor(0xFFFF, background_color);
}

void resetMenu(int action) {
  static bool resetChoise = false;

  switch (action) {
    case 0:
      clearMenu();
      tft.setCursor(12, 6);
      tft.print(get_str(STR_RESET_2));

      tft.setTextColor(0xfd14);
      tft.setCursor(textStrX[4], 37);
      tft.print(get_str(STR_RESET_3));
      tft.setCursor(textStrX[5], 59);
      tft.print(get_str(STR_SETTINGS_Q));

      resetChoise = false;
      drawYesNoButtons(resetChoise);

      break;

    case 1: // down button
      resetChoise = !resetChoise;
      drawYesNoButtons(resetChoise);

      break;
    case 2: // up button
      resetChoise = !resetChoise;
      drawYesNoButtons(resetChoise);

      break;
    case 3: // gr button
      if (resetChoise) reset_settings();
      handle_menu_mode(1);

      break;

    case 4:
      handle_menu_mode(1);
      break;
  }
}

void infoMenu(int action) {
  switch (action) {
    case 0:
      clearMenu();
      tft.setCursor(12, 6);
      tft.print(get_str(STR_INFO_2));

      tft.drawBitmap(68, 34, image_info, 24, 24, 0x54bf);
      tft.setCursor(45, 70); 
      tft.print(get_str(STR_ZVONOK));
      tft.setCursor(57, 94); 

      tft.print(FIRMWARE_VERSION);

      break;

    case 1: // down button

      break;
    case 2: // up button

      break;
    case 3: // gr button

      break;
    case 4:
      handle_menu_mode(1);
      break;
  }
}

void oscillographMenu(int action) {
  int sampleHeigh_1 = 0;
  int sampleHeigh_2 = 0;
  int sampleWidth_1 = 0;
  int sampleWidth_2 = 0;

  switch(action) {
    case 0:
      menu_mode = MENU_OSCILLOGRAPH;

      tft.fillScreen(background_color);
      tft.fillRect(0, 114, 160, 14, 0x5aeb);
      tft.fillRect(0, 0, 160, 14, 0x5aeb);
      break;

    case 1: // down button

      break;
    case 2: // up button

      break;
    case 3: // gr button
      break;

    case 4: // red button
      menu_mode = MENU_DEFAULT;

      handle_menu_mode(1);
      break;

    case 5: // new buffer
      // for (int k = 0; k < 16; k++) {
        tft.fillRect(0, 14, 160, 100, background_color);
        tft.fillRect(0, 114, 160, 14, 0x5aeb);
        tft.fillRect(0, 0, 160, 14, 0x5aeb);

        for (int i = 1;  i < 128; i++) {
          // tft.drawLine(16+i-1, 14, 16+i, 100, background_color);

          // sampleHeigh_1 = 113 - oscill_buf->ch24[(i - 1) + k*128] * 100 / 4096;
          // sampleHeigh_2 = 113 - oscill_buf->ch24[i + k*128] * 100 / 4096;
          sampleHeigh_1 = 113 - oscill_buf->ch24[(i - 1)] * 100 / 4096;
          sampleHeigh_2 = 113 - oscill_buf->ch24[i ] * 100 / 4096;

          sampleWidth_1 = 16 + i - 1;
          sampleWidth_2 = 16 + i;

          tft.drawLine(sampleWidth_1, sampleHeigh_1, sampleWidth_2, sampleHeigh_2, 0x07f2);

          // sampleHeigh_1 = 113 - oscill_buf->ch58[(i - 1) + k*128] * 100 / 4096;
          // sampleHeigh_2 = 113 - oscill_buf->ch58[i + k*128] * 100 / 4096;
          sampleHeigh_1 = 113 - oscill_buf->ch58[(i - 1)] * 100 / 4096;
          sampleHeigh_2 = 113 - oscill_buf->ch58[i] * 100 / 4096;

          sampleWidth_1 = 16 + i - 1;
          sampleWidth_2 = 16 + i;

          tft.drawLine(sampleWidth_1, sampleHeigh_1, sampleWidth_2, sampleHeigh_2, 0xffff);
          // delay(1);
        }
      // }
      
      break;
  }
}

// void drawIndicator()
#define VISIBLE_LINES 8

void displayIndicatorMenu(int selected, int offset) {
  tft.setTextSize(2);
  // tft.setFreeFont(&FreeMono9pt7b);
  tft.setFreeFont(&Org_01);
  const int y_bias = 16;
  tft.fillRect(0, 24, 159, 128, background_color);
  // if (xSemaphoreTake(xMutexIndicators, portMAX_DELAY)) {
  //   for (size_t i = 0; i < indicator_count; i++) {
  //     tft.setCursor(12, 32 + (i*9));
  //     if (i == selected) tft.print(">");
  //     tft.print(indicators[i].name);
  //     tft.setCursor(80, 32 + (i*9));
  //     tft.print(indicators[i].isActive ? "[X]" : "[ ]");
  //   }
  //   xSemaphoreGive(xMutexIndicators);
  // }

  // Отображаем видимую часть списка
  if (xSemaphoreTake(xMutexIndicators, portMAX_DELAY)) {
    for (int i = 0; i < VISIBLE_LINES && (i + offset) < indicator_count; i++) {
      int idx = i + offset;
        // char line[48];
        // sprintf(line, "%c %-8s 24[%c] 58[%c]",
        //         (idx == selected) ? '>' : ' ',
        //         indicators[idx].name,
        //         indicators[idx].active_24 ? 'X' : ' ',
        //         indicators[idx].active_58 ? 'X' : ' ');
        // display_print_xy(0, 16 * (i + 1), line);

      tft.setCursor(12, 36 + (i*y_bias));
      if (idx == selected) tft.print(">");
      tft.print(indicators[idx].name);
      tft.setCursor(75, 36 + (i*y_bias));
      tft.print(indicators[i].isActive ? "[X]" : "[ ]");
      tft.setCursor(125, 36 + (i*y_bias));
      tft.print(indicators[i].isActive ? "[X]" : "[ ]");
    }
    xSemaphoreGive(xMutexIndicators);
  }

  tft.setFreeFont();
  tft.setTextSize(2);
    // for (int i = 0; i < MAX_INDICATORS; i++) {
    //     char line[64];
    //     sprintf(line, "%c %s  24[%c] 58[%c]\n",
    //             (i == selected) ? '>' : ' ',
    //             indicators[i].name,
    //             indicators[i].active_24 ? 'X' : ' ',
    //             indicators[i].active_58 ? 'X' : ' ');
    //     display_print(line);
    // }
}

void indicatorsMenu(int action) {
  static int selected = 0;           // какой индикатор выделен
  static int offset = 0;
  static int currentBand = 0;        // 0 = диапазон 24, 1 = диапазон 58
  static bool menuActive = false;

  switch(action) {
    case 0:
      menu_mode = MENU_INDICATORS;
      // clearMenu();
      // tft.setCursor(12, 6);
      // tft.print("INDICATORS");

      tft.fillScreen(background_color);
      tft.drawLine(0, 23, 159, 23, 0xAD55);

      tft.drawString(get_str(STR_DRONS), 12, 6);
      tft.drawString("2.4", 70, 6);
      tft.drawString("5.8", 120, 6);

      // menuActive = true;
      selected = 0;
      offset = 0;
      currentBand = 0;

      displayIndicatorMenu(selected, offset);

      break;

    case 1: // down button
      // if (!menuActive) break;
      // if (selected < MAX_INDICATORS - 1) {
      if (selected < indicator_count - 1) {
          selected++;
          if (selected >= offset + VISIBLE_LINES) offset++;
      }
      displayIndicatorMenu(selected, offset);
      break;
      break;
    case 2: // up button
      if (selected > 0) {
          selected--;
          if (selected < offset) offset--;
      }
      displayIndicatorMenu(selected, offset);
      break;
    case 3: // gr button
      if (xSemaphoreTake(xMutexIndicators, portMAX_DELAY)) {
        indicators[selected].isActive = !indicators[selected].isActive;
        xSemaphoreGive(xMutexIndicators);
      }

      displayIndicatorMenu(selected, offset);

      break;
    case 4:
      menu_mode = MENU_DEFAULT;
      handle_menu_mode(1);
      break;
  }

}


// char *adminStrings[admin_menu_number] = {
//   textStrings[STR_LANGUAGE],  
//   textStrings[STR_AVER_COEFF],      
//   textStrings[STR_LOGOTYPE],     
//   textStrings[STR_CALIBRATION_Q_Q] 
// };

StringID adminIndices[admin_menu_number] = {
  STR_LANGUAGE,
  STR_AVER_COEFF,
  STR_LOGOTYPE,
  STR_CALIBRATION_Q_Q
};

String options[admin_menu_number];

void print_admin_menu(int active_index) {
  tft.fillRect(0, 25, 160, 103, background_color);
  tft.setTextSize(1);
  for (int i = 0; i < admin_menu_number; i++) {
    tft.setTextColor((i == active_index) ? background_color : 0xFFFF,
                     (i == active_index) ? 0xFFFF : background_color);
    tft.setCursor(12, 34 + (i*9)); 
    tft.print(get_str(adminIndices[i]));
    tft.setCursor(102, 34 + (i*9)); 
    tft.print(options[i]);
  }
  tft.setTextSize(2);
  tft.setTextColor(0xffff, background_color);
}

void adminMenu(int action) {
  static int admIndex = 0;
  static SystemConfig_t tempConfig;

  switch (action) {
    case 0:
      clearMenu();
      tft.setCursor(12, 6);
      tft.print(get_str(STR_EXT_SET));

      options[0] = String(get_str((StringID)(STR_LANG_RUS + displayConfig.language)));
      options[1] = String(displayConfig.averCoeff);
      options[2] = String(get_str((StringID)(STR_NO + (int)(displayConfig.logoFlag))));
      options[3] = String(displayConfig.calibrNumber);

      admIndex = 0;
      print_admin_menu(admIndex);

      tempConfig = displayConfig;

      break;

    case 1: // down button
      if (admIndex < admin_menu_number - 1) {
        admIndex++;
        print_admin_menu(admIndex);
      }
      break;
    case 2: // up button
      if (admIndex > 0) {
        admIndex--;
        print_admin_menu(admIndex);
      }
      break;
    case 3: // gr button
      
      switch(admIndex) {
        case 0:
          displayConfig.language = (displayConfig.language + 1) % LANGUAGES_Q;
          set_language(displayConfig.language);
          
          clearMenu();
          tft.setCursor(12, 6);
          tft.print(get_str(STR_EXT_SET));

          options[0] = String(get_str((StringID)(STR_LANG_RUS + displayConfig.language)));
          print_admin_menu(admIndex);
          break;

        case 1:
          displayConfig.averCoeff = (displayConfig.averCoeff + 10) % 100;
          options[1] = String(displayConfig.averCoeff);
          print_admin_menu(admIndex);
          break;

        case 2:
          displayConfig.logoFlag = !displayConfig.logoFlag;
          options[2] = String(get_str((StringID)(STR_NO + (int)(displayConfig.logoFlag))));
          print_admin_menu(admIndex);
          break;

        case 3:
          displayConfig.calibrNumber = displayConfig.calibrNumber % 60 + 10;
          options[3] = String(displayConfig.calibrNumber);
          print_admin_menu(admIndex);
          break;
      }

      break;

    case 4: // back button
      if (memcmp(&tempConfig, &displayConfig, sizeof(SystemConfig_t)) != 0) {
      // if (tempConfig != displayConfig) {
        ConfigMessage_t change;
        if (tempConfig.language != displayConfig.language) {
          change = { .type = CONFIG_SET, .field = CFG_LANGUAGE };
          change.value.language = displayConfig.language;
          xQueueSend(configQueue, &change, portMAX_DELAY);
        }
        if (tempConfig.averCoeff != displayConfig.averCoeff) {
          change = { .type = CONFIG_SET, .field = CFG_AVER_COEFF };
          change.value.averCoeff = displayConfig.averCoeff;
          xQueueSend(configQueue, &change, portMAX_DELAY);
        }
        if (tempConfig.logoFlag != displayConfig.logoFlag) {
          change = { .type = CONFIG_SET, .field = CFG_LOGO_FLAG };
          change.value.logoFlag = displayConfig.logoFlag;
          xQueueSend(configQueue, &change, portMAX_DELAY);
        }
        if (tempConfig.calibrNumber != displayConfig.calibrNumber) {
          change = { .type = CONFIG_SET, .field = CFG_CALIBR_NUMBER };
          change.value.calibrNumber = displayConfig.calibrNumber;
          xQueueSend(configQueue, &change, portMAX_DELAY);
        }
      }
      handle_menu_mode(1);
      break;
  }
}

void timers_reset() {
  static TimerMessage_t msg;

  msg.type = TIMER_RESET;
  msg.timerId = TIMER_KEYLOCK;
  xQueueSend(timerQueue, &msg, portMAX_DELAY);

  msg.type = TIMER_RESET;
  msg.timerId = TIMER_SLEEP;
  xQueueSend(timerQueue, &msg, portMAX_DELAY);
}

void timers_stop() {
  static TimerMessage_t msg;

  msg.type = TIMER_STOP;
  msg.timerId = TIMER_KEYLOCK;
  xQueueSend(timerQueue, &msg, portMAX_DELAY);

  msg.type = TIMER_STOP;
  msg.timerId = TIMER_SLEEP;
  xQueueSend(timerQueue, &msg, portMAX_DELAY);
}

void handle_timers_update(int) {
  timers_reset();
}

void handle_levels_mode(int) {
  timers_reset();
  simpleDimpleFlag = !simpleDimpleFlag;
  mainDraw();
}

void handle_lock_mode(int lockmode) {
  static TimerMessage_t msg;
  switch (lockmode) {
    case 0:
      tft.drawBitmap(114, 4, image_device_lock_bits, 13, 16, background_color);
      LOCK_FLAG = false;
      currentMode = MODE_NORMAL;

      msg.type = TIMER_RESET;
      msg.timerId = TIMER_KEYLOCK;
      xQueueSend(timerQueue, &msg, portMAX_DELAY);

      break;

    case 1:
      tft.drawBitmap(114, 4, image_device_lock_bits, 13, 16, 0xFFFF);
      LOCK_FLAG = true;
      if (currentMode == MODE_NORMAL) currentMode = MODE_LOCKED;

      break;
  }
  // mainDraw();
}

void handle_sleep_mode(int sleepmode) {
  if (xSemaphoreTake(xMutexPWM, portMAX_DELAY)) {
    analogWriteFreq(1000);
    analogWriteRange(256);
    static TimerMessage_t msg;

    switch (sleepmode) {
      case 0:
        for (int i = 0; i <= displayConfig.brightness * briStep + briMin; i++){
          analogWrite(DISPLAY_LED, i);
          delay(2);
        }
        if (displayConfig.brightness == 10) {
          digitalWrite(DISPLAY_LED, HIGH);
        }

        if (LOCK_FLAG) currentMode = MODE_LOCKED;
        else currentMode = MODE_NORMAL;

        msg.type = TIMER_RESET;
        msg.timerId = TIMER_SLEEP;
        xQueueSend(timerQueue, &msg, portMAX_DELAY);
        
        break;
      case 1:
        for (int i = displayConfig.brightness * briStep + briMin; i >= 0; i--){
          analogWrite(DISPLAY_LED, i);
          delay(2);
        }

        currentMode = MODE_SLEEP;
        break;
    }
    xSemaphoreGive(xMutexPWM);
  }
}

void handle_sound_toggle(int) {
  displayConfig.soundOffFlag = !displayConfig.soundOffFlag;
  tft.drawBitmap(110, 5, image_volume_muted_bits, 18, 16, displayConfig.soundOffFlag * 0xFFFF);


  ConfigMessage_t change = { .type = CONFIG_SET, .field = CFG_SOUND_OFF_FLAG };
  change.value.soundOffFlag = displayConfig.soundOffFlag;
  xQueueSend(configQueue, &change, portMAX_DELAY);
}

void handle_threshold_mode(int mode) {
  editingThreshold = false;
  ConfigMessage_t change;

  switch (mode) {
    case 0:
      //exit menu
      change = { .type = CONFIG_SET, .field = CFG_LIMIT24 };
      change.value.limit24 = displayConfig.limit24;
      xQueueSend(configQueue, &change, portMAX_DELAY);

      change = { .type = CONFIG_SET, .field = CFG_LIMIT58 };
      change.value.limit58 = displayConfig.limit58;
      xQueueSend(configQueue, &change, portMAX_DELAY);

      timers_reset();

      currentMode = MODE_NORMAL;
      mainDraw();
      break;

    case 1:
      currentMode = MODE_THRESHOLD;
      menuThresholds();
      break;
  }
}

void handle_threshold_24_58(int mode_24_58) {
  switch (mode_24_58) {
    case 0:
      if (thresholds_24_58 == false) return;
      thresholds_24_58 = false;
      break;
    case 1:
      if (thresholds_24_58 == true) return;
      thresholds_24_58 = true;
      break;
  }

  editingThreshold = false;
  menuThresholds();
}

void handle_threshold_input(int input) {
  uint16_t *currentLimit = thresholds_24_58 ? &displayConfig.limit58 : &displayConfig.limit24;

  if ((input >= 0) && (input <= 9)) {
    // цифра
    if (!editingThreshold) {
      editingThreshold = true;
      editPos = 0;
      editDigits[0] = editDigits[1] = editDigits[2] = -1;
      originalValue = *currentLimit;
    }

    if (editPos < 3) {
      editDigits[editPos++] = input;
    }

    // если набрали все 3 цифры → сохранить
    if (editPos == 3) {
      int value = editDigits[0]*100 + editDigits[1]*10 + editDigits[2];
      if (value > 300) value = 300;
      *currentLimit = value;
      editingThreshold = false;
    }
  } else if (input == 10) {
    // down_button
    if (editingThreshold) {
      // отмена редактирования
      *currentLimit = originalValue - 1;
      if (*currentLimit < 0) *currentLimit = 0;
      editingThreshold = false;
    } else {
      (*currentLimit)--;
      if (*currentLimit < 0) *currentLimit = 0;
    }
  } else if (input == 11) {
    // up_button
    if (editingThreshold) {
      // отмена редактирования
      *currentLimit = originalValue + 1;
      if (*currentLimit > 300) *currentLimit = 300;
      editingThreshold = false;
    } else {
      (*currentLimit)++;
      if (*currentLimit > 300) *currentLimit = 300;
    }
  }

  menuThresholds();
}

void handle_menu_mode(int mode) {
  switch(mode) {
    case 0:
      timers_reset();
      currentMode = MODE_NORMAL;
      indexxx = 1;  // reset menu index
      mainDraw();
      break;
    case 1:
      currentMode = MODE_SETTINGS;
      menu();
      stopAlarm();
      break;
    case 2:
      currentMode = MODE_SUBSETTINGS;
      menu_pages[indexxx](0);
      break;
    case 3:
      currentMode = MODE_SETTINGS_ADMIN;
      adminMenu(0);
      break;
  }
}

void handle_menu_down_up(int dirrection) {
  switch(dirrection) {
    case 0:
      //down
      indexxx++;
      if (indexxx > menu_number - 1) indexxx = 0;
      break;
    case 1:
      //up
      indexxx--;
      if (indexxx < 0) indexxx = menu_number - 1;
      break;
  }

  indexxx0 = (indexxx - 1) == -1 ? (menu_number - 1) : (indexxx - 1);
  indexxx2 = (indexxx + 1) == menu_number ? (0) : (indexxx + 1);

  drawMenuSettings();
}

void handle_sub_menu(int input) {
  menu_pages[indexxx](input);
}

void handle_menu_admin(int input) {
  adminMenu(input);
}

void stopAlarm() {
  AlarmMessage m{ALARM_NONE, 1, false};
  xQueueSend(xAlarmQueue, &m, 10);
}

