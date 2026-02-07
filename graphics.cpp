#include "graphics.h"

Icon icon_light =   ICON_DEFAULT(image_light, ST7735_YELLOW);
Icon icon_calibr =  ICON_DEFAULT(image_calibration, 0x8e5f);
Icon icon_channel = ICON_DEFAULT(image_antenna_bits, 0x1ff6);
Icon icon_drons =   ICON_DEFAULT(image_drons, 0xFA28);
Icon icon_lock =    ICON_DEFAULT(image_locked_icon_bits, ST7735_ORANGE);
Icon icon_sleep =   ICON_DEFAULT(image_sleep_mode_icon_bits, 0xb47f);
Icon icon_reset =   ICON_DEFAULT(icon_resetreset_icon, 0xf145);
Icon icon_sinus =   ICON_DEFAULT(image_wave_sine_icon_bits, ST7735_GREEN);
Icon icon_info =    ICON_DEFAULT(image_info, 0x54bf);

Icon icon_arrow_left = {arrow_left, 20, 73, 4, 8, ST7735_WHITE};
Icon icon_arrow_right = {arrow_right, 136, 73, 4, 8, ST7735_WHITE};

Icon icon_arrow_left_small = {image_ButtonLeftSmall_bits, 83, 11, 3, 5, ST7735_WHITE};
Icon icon_arrow_right_small = {image_ButtonRightSmall_bits, 126, 11, 3, 5, ST7735_WHITE};

Icon icon_arrow_up = {image_SmallArrowUp_bits, 0, 0, 5, 3, ST7735_WHITE};
Icon icon_arrow_down = {image_SmallArrowDown_bits, 0, 0, 5, 3, ST7735_WHITE};

Icon icon_logotype = { logotype, 0, 0, 160, 128, 0xe6b6 };

const unsigned char *battery_icons[7] = {
  image_battery_full_bits,
  image_battery_83_bits,
  image_battery_67_bits,
  image_battery_50_bits,
  image_battery_33_bits,
  image_battery_17_bits,
  image_battery_0_bits
};

const uint16_t battery_colors[7] = {
  ST7735_GREEN,
  ST7735_GREEN,
  ST7735_YELLOW,
  ST7735_YELLOW,
  ST7735_RED,
  ST7735_RED,
  ST7735_RED
};
