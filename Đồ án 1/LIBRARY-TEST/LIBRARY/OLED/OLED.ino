#include <Arduino.h>
#include "oled_display.h"

oled_display oled;

void setup() {
  oled.begin();
}

void loop() {
  oled.print_text_1x("\n",0,0);
  delay(1000);
  oled.print_text_2x("Hello",10,10);
  delay(1000);
  oled.print_int_2x(13, 15,10);
  delay(1000);
}
