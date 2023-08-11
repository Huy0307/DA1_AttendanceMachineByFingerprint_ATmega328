#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"

class oled_display {
  public:
    oled_display();
    void begin();
    void print_text_1x(const char *str, int x, int y);
    void print_int_1x(int num, int x, int y);
    void print_float_1x(float num, int x, int y);
    void print_uint8t_1x(uint8_t num, int x, int y);
    void print_text_2x(const char *str, int x, int y);
    void print_int_2x(int num, int x, int y);
    void print_float_2x(float num, int x, int y);
    void print_uint8t_2x(uint8_t num, int x, int y);
    void print_text1x(const char *str);
    void print_int1x(int num);
    void print_float1x(float num);
    void print_uint8t1x(uint8_t num);
    void print_text2x(const char *str);
    void print_int2x(int num);
    void print_float2x(float num);
    void print_uint8t2x(uint8_t num);
    void clear();
  private:
    SSD1306AsciiAvrI2c oled;
};
#endif
