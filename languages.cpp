#include "languages.h"

#define STRING_LEN 30
#define TEXT_STRINGS_Q 40
#define TEXT_STR_X 7



// char LANGUAGES[LANGUAGES_Q][STRING_LEN] = {
//   {"Русский"},
//   {"English"}
// };

// char RUSSIAN_LANG[TEXT_STRINGS_Q][STRING_LEN] = {
//   // Menu lines
//   /*0*/   { "Яркость   " },
//   /*1*/   { "Калибровка" },
//   /*2*/   { "Каналы    " },
//   /*3*/   { "Автоблок  " },
//   /*4*/   { "Сон       " },
//   /*5*/   { "Сброс     " },
//   /*6*/   { "Осцилограф" },
//   /*7*/   { "Инфо      " },
//   ////////////////
//   /*8*/   { "Поиск    "  },
//   /*9*/   { "Поиск+    " },
//   /*10*/   { "Порог:"     },
//   /*11*/  { "Настройки"  },
//   /*12*/  { "Яркость  " },
//   /*13*/  { "Провести" },
//   /*14*/  { "калибровку?" },
//   /*15*/  { "Нет" },
//   /*16*/  { "Да" },
//   /*17*/  { "Каналы   " },
//   /*18*/  { "ГГц" },
//   /*19*/  { "Автоблок " },
//   /*20*/  { "Сон      " },
//   /*21*/  { "Сброс    " },
//   /*22*/  { "Сбросить" },
//   /*23*/  { "настройки?" },
//   /*24*/  { "Инфо     " },
//   /*25*/  { "Звонок" },
//   /*26*/  { "выкл" },
//   /*27*/  { "Расш настр" },
//   /*28*/  { "Язык" },
//   /*29*/  { "Коэф усредн" },
//   /*30*/  { "Логотип" },
//   /*31*/  { "Калибровка" }  
// };

// char RUSSIAN_TIME_STRINGS[TIME_STRINGS_Q][STRING_LEN] = {
//   {" Выкл "},
//   {" 5 сек"},
//   {"10 сек"},
//   {"15 сек"}, 
//   {"30 сек"}
// };

byte RUSSIAN_LANG_XY[TEXT_STR_X] = {
  /*0*/ 33, ///*12*/  { "Провести" },
  /*1*/ 15, ///*13*/  { "калибровку?" },
  /*2*/ 35, ///*14*/  { "Нет" },
  /*3*/ 97, ///*15*/  { "Да" },
  /*4*/ 33, ///*21*/  { "Сбросить" },
  /*5*/ 21, ///*22*/  { "настройки?" },
  /*6*/ 45  // times {" Выкл "},
};

// char RUSSIAN_Y_N[2][STRING_LEN] = {
//   {"Нет"},
//   {"Да"}
// };
// ///////////////////////

// char ENGLISH_LANG[TEXT_STRINGS_Q][STRING_LEN] = {
//   // Menu lines
//   /*0*/   { "Light     " },
//   /*1*/   { "Calibrat  " },
//   /*2*/   { "Channels  " },
//   /*3*/   { "Autoblock " },
//   /*4*/   { "Sleep     " },
//   /*5*/   { "Reset     " },
//   /*6*/   { "Oscilogr" },
//   /*7*/   { "Info      " },
//   ////////////////
//   /*8*/   { "Search   "  },
//   /*9*/   { "Search+   " },
//   /*10*/   { "Limit:"     },
//   /*11*/  { "Settings "  },
//   /*12*/  { "Light    " },
//   /*13*/  { "Perform" },
//   /*14*/  { "calibration?" },
//   /*15*/  { "No" },
//   /*16*/  { "Yes" },
//   /*17*/  { "Channels " },
//   /*18*/  { "GGh" },
//   /*19*/  { "Autoblock" },
//   /*20*/  { "Sleep    " },
//   /*21*/  { "Reset    " },
//   /*22*/  { "Reset" },
//   /*23*/  { "settings?" },
//   /*24*/  { "Info     " },
//   /*25*/  { "Zvonok" },
//   /*26*/  { "off" },
//   /*27*/  { "Ext set" },
//   /*28*/  { "Language" },
//   /*29*/  { "Aver coeff" },
//   /*30*/  { "Logotype" },
//   /*31*/  { "Calibration" }
// };

// char ENGLISH_TIME_STRINGS[TIME_STRINGS_Q][STRING_LEN] = {
//   {" Off  "},
//   {" 5 sec"},
//   {"10 sec"},
//   {"15 sec"}, 
//   {"30 sec"}
// };

byte ENGLISH_LANG_XY[TEXT_STR_X] = {
  /*0*/ 41, ///*12*/  { "Perform" },
  /*1*/ 10, ///*13*/  { "calibration?" },
  /*2*/ 41, ///*14*/  { "No" },
  /*3*/ 91, ///*15*/  { "Yes" },
  /*4*/ 53, ///*21*/  { "Reset" },
  /*5*/ 31, ///*22*/  { "settings?" },
  /*6*/ 52  // times {" Off "},
};

// char ENGLISH_Y_N[2][STRING_LEN] = {
//   {"No"},
//   {"Yes"}
// };
////////
// byte textStrX[TEXT_STR_X];
byte *textStrX;
// char textStrings[TEXT_STRINGS_Q][STRING_LEN];
// char timeStrings[TIME_STRINGS_Q][STRING_LEN];
char y_n_strings[2][STRING_LEN];


char *lang_rus[TEXT_STRINGS_Q] = {
  [STR_LIGHT]           = "Яркость   ",
  [STR_CALIBRATION]     = "Калибровка",
  [STR_CHANNELS]        = "Каналы    ",
  [STR_DRONS]           = "Типы БЛА  ",
  [STR_AUTOBLOCK]       = "Автоблок  ",
  [STR_SLEEP]           = "Сон       ",
  [STR_RESET]           = "Сброс     ",
  // [STR_OSCILLOGRAM]     = "Осцилограф",
  [STR_OSCILLOGRAM]     = "Осциллогр",
  [STR_INFO]            = "Инфо      ",
  [STR_SEARCH]          = "Поиск    ",
  [STR_SEARCH_PLUS]     = "Поиск+    ",
  [STR_LIMIT]           = "Порог:",
  [STR_SETTINGS]        = "Настройки",
  [STR_LIGHT_2]         = "Яркость  ",
  [STR_PERFORM]         = "Провести",
  [STR_CALIBRATION_Q]   = "калибровку?",
  [STR_NO]              = "Нет",
  [STR_YES]             = "Да",
  [STR_CHANNELS_2]      = "Каналы   ",
  [STR_GGH]             = "ГГц",
  [STR_AUTOBLOCK_2]     = "Автоблок ",
  [STR_SLEEP_2]         = "Сон      ",
  [STR_RESET_2]         = "Сброс    ",
  [STR_RESET_3]         = "Сбросить",
  [STR_SETTINGS_Q]      = "настройки?",
  [STR_INFO_2]          = "Инфо     ",
  [STR_ZVONOK]          = "Звонок",
  [STR_OFF]             = "выкл",
  [STR_EXT_SET]         = "Расш настр",
  [STR_LANGUAGE]        = "Язык",
  [STR_AVER_COEFF]      = "Коэф усредн",
  [STR_LOGOTYPE]        = "Логотип",
  [STR_CALIBRATION_Q_Q] = "Калибровка",
  [STR_LANG_RUS]        = "Русский",
  [STR_LANG_ENG]        = "English",
  [STR_TIME_OFF]        = " Выкл ",
  [STR_TIME_5]          = " 5 сек",
  [STR_TIME_10]         = "10 сек",
  [STR_TIME_15]         = "15 сек",
  [STR_TIME_30]         = "30 сек"
};

char *lang_eng[TEXT_STRINGS_Q] = {
  [STR_LIGHT]           =  "Light     " ,
  [STR_CALIBRATION]     =  "Calibrat  " ,
  [STR_CHANNELS]        =  "Channels  " ,
  [STR_DRONS]           =  "UAV types " ,
  [STR_AUTOBLOCK]       =  "Autoblock " ,
  [STR_SLEEP]           =  "Sleep     " ,
  [STR_RESET]           =  "Reset     " ,
  [STR_OSCILLOGRAM]     =  "Oscillogr" ,
  [STR_INFO]            =  "Info      " ,
  [STR_SEARCH]          =  "Search   "  ,
  [STR_SEARCH_PLUS]     =  "Search+   " ,
  [STR_LIMIT]           =  "Limit:"     ,
  [STR_SETTINGS]        =  "Settings "  ,
  [STR_LIGHT_2]         =  "Light    " ,
  [STR_PERFORM]         =  "Perform" ,
  [STR_CALIBRATION_Q]   =  "calibration?" ,
  [STR_NO]              =  "No" ,
  [STR_YES]             =  "Yes" ,
  [STR_CHANNELS_2]      =  "Channels " ,
  [STR_GGH]             =  "GGh" ,
  [STR_AUTOBLOCK_2]     =  "Autoblock" ,
  [STR_SLEEP_2]         =  "Sleep    " ,
  [STR_RESET_2]         =  "Reset    " ,
  [STR_RESET_3]         =  "Reset" ,
  [STR_SETTINGS_Q]      =  "settings?" ,
  [STR_INFO_2]          =  "Info     " ,
  [STR_ZVONOK]          =  "Zvonok" ,
  [STR_OFF]             =  "off" ,
  [STR_EXT_SET]         =  "Ext set" ,
  [STR_LANGUAGE]        =  "Language" ,
  [STR_AVER_COEFF]      =  "Aver coeff" ,
  [STR_LOGOTYPE]        =  "Logotype" ,
  [STR_CALIBRATION_Q_Q] =  "Calibration",
  [STR_LANG_RUS]        =  "Русский",
  [STR_LANG_ENG]        =  "English",
  [STR_TIME_OFF]        =  " Off  ",
  [STR_TIME_5]          =  " 5 sec",
  [STR_TIME_10]         =  "10 sec",
  [STR_TIME_15]         =  "15 sec",
  [STR_TIME_30]         =  "30 sec"
};

char *get_str(StringID id) {
  return textStrings[id];
}

// const char **current_lang = lang_rus;
char **textStrings = lang_rus;
uint8_t *current_lang_xy = RUSSIAN_LANG_XY;

void set_language(uint8_t lang_code) {
  switch (lang_code) {
    case 0:
      textStrings = lang_rus;
      // current_lang_xy = RUSSIAN_LANG_XY;
      textStrX = RUSSIAN_LANG_XY;
      break;
    case 1:
      textStrings = lang_eng;
      // current_lang_xy = ENGLISH_LANG_XY;
      textStrX = ENGLISH_LANG_XY;
      break;
  }
}

// void languageInit() {
//   for (int i = 0; i < TIME_STRINGS_Q; i++) {
//     for (int j = 0; j < STRING_LEN; j++) {
//       if (language == 0) {
//         timeStrings[i][j] = RUSSIAN_TIME_STRINGS[i][j];
//       } else if (language == 1) {
//         timeStrings[i][j] = ENGLISH_TIME_STRINGS[i][j];
//       }
//     }
//   }

//   for (int i = 0; i < TEXT_STRINGS_Q; i++) {
//     for (int j = 0; j < STRING_LEN; j++) {
//       if (language == 0) {
//         textStrings[i][j] = RUSSIAN_LANG[i][j];
//       } else if (language == 1) {
//         textStrings[i][j] = ENGLISH_LANG[i][j];
//       }
//     }
//   }

//   for (int i = 0; i < TEXT_STR_X; i++) {
//     if (language == 0) {
//       textStrX[i] = RUSSIAN_LANG_XY[i];
//     } else if (language == 1) {
//       textStrX[i] = ENGLISH_LANG_XY[i];
//     }
//   }


//   for (int i = 0; i < 2; i++) {
//     for (int j = 0; j < STRING_LEN; j++) {
//       if (language == 0) {
//         y_n_strings[i][j] = RUSSIAN_Y_N[i][j];
//       } else if (language == 1) {
//         y_n_strings[i][j] = ENGLISH_Y_N[i][j];
//       }
//     }
//   }
// }