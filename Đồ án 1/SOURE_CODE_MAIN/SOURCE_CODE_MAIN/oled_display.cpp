#include "oled_display.h"
oled_display::oled_display() {}
#define I2C_ADDRESS 0x3C
void oled_display::begin() {
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.setFont(Adafruit5x7);
}

void oled_display::print_text_1x(const char *str, int x, int y) {
  oled.set1X();
  oled.setCursor(x, y);
  oled.print(str);
}

void oled_display::print_int_1x(int num, int x, int y) {
  oled.set1X();
  oled.setCursor(x, y);
  oled.print(num);
}

void oled_display::print_float_1x(float num, int x, int y) {
  oled.set1X();
  oled.setCursor(x, y);
  oled.print(num);
}

void oled_display::print_uint8t_1x(uint8_t num, int x, int y) {
  oled.set1X();
  oled.setCursor(x, y);
  oled.print(num);
}
void oled_display::print_text_2x(const char *str, int x, int y) {
  oled.set2X();
  oled.setCursor(x, y);
  oled.print(str);
}


void oled_display::print_int_2x(int num, int x, int y) {
  oled.set2X();
  oled.setCursor(x, y);
  oled.print(num);
}

void oled_display::print_float_2x(float num, int x, int y) {
  oled.set2X();
  oled.setCursor(x, y);
  oled.print(num);
}

void oled_display::print_uint8t_2x(uint8_t num, int x, int y) {
  oled.set2X();
  oled.setCursor(x, y);
  oled.print(num);
}
void oled_display::print_text1x(const char *str) {
  oled.set1X();
  oled.print(str);
}

void oled_display::print_int1x(int num) {
  oled.set1X();
  oled.print(num);
}

void oled_display::print_float1x(float num) {
  oled.set1X();
  oled.print(num);
}

void oled_display::print_uint8t1x(uint8_t num) {
  oled.set1X();
  oled.print(num);
}
void oled_display::print_text2x(const char *str) {
  oled.set2X();
  oled.print(str);
}

void oled_display::print_int2x(int num) {
  oled.set2X();
  oled.print(num);
}

void oled_display::print_float2x(float num) {
  oled.set2X();
  oled.print(num);
}

void oled_display::print_uint8t2x(uint8_t num) {
  oled.set2X();
  oled.print(num);
}
void oled_display::clear() {
  oled.clear();
}
