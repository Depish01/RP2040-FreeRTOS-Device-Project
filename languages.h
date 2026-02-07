#pragma once

#include <Arduino.h>

#define LANGUAGES_Q 2

typedef enum {
  STR_LIGHT,
  STR_CALIBRATION,
  STR_CHANNELS,
  STR_DRONS,
  STR_AUTOBLOCK,
  STR_SLEEP,
  STR_RESET,
  STR_OSCILLOGRAM,
  STR_INFO,
  STR_SEARCH,
  STR_SEARCH_PLUS,
  STR_LIMIT,
  STR_SETTINGS,
  STR_LIGHT_2,
  STR_PERFORM,
  STR_CALIBRATION_Q,
  STR_NO,
  STR_YES,
  STR_CHANNELS_2,
  STR_GGH,
  STR_AUTOBLOCK_2,
  STR_SLEEP_2,
  STR_RESET_2,
  STR_RESET_3,
  STR_SETTINGS_Q,
  STR_INFO_2,
  STR_ZVONOK,
  STR_OFF,
  STR_EXT_SET,
  STR_LANGUAGE,
  STR_AVER_COEFF,
  STR_LOGOTYPE,
  STR_CALIBRATION_Q_Q,

  STR_LANG_RUS,
  STR_LANG_ENG,

  STR_TIME_OFF,
  STR_TIME_5,
  STR_TIME_10,
  STR_TIME_15,
  STR_TIME_30
} StringID;


void set_language(uint8_t lang_code);

char *get_str(StringID id);

extern char **textStrings;
extern byte *textStrX;
