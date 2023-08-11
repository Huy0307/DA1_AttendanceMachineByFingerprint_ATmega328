#include <SPI.h>
#include <Wire.h>
#include <Time.h>
#include <DS1307RTC.h>

// #if (SSD1306_LCDHEIGHT != 64)
// #error("Height incorrect, please fix Adafruit_SSD1306.h!");
// #endif

void setup() {
  Serial.begin(9600);
}

void loop() {
  tmElements_t tm;
  if (RTC.read(tm)) {
    print2digits(tm.Hour);
    Serial.write(':');
    print2digits(tm.Minute);
    Serial.write(':');
    print2digits(tm.Second);
    Serial.print(" ");
    Serial.print(tm.Day);
    Serial.write('/');
    Serial.print(tm.Month);
    Serial.write('/');
    Serial.print(tmYearToCalendar(tm.Year));
    Serial.print("\n");
  } else {
    if (RTC.chipPresent()) {
      Serial.print("DS1307 stopped,run set time");
    } else {
      Serial.print("DS1307 read error,check circuit");
    }
    delay(9000);
  }
  delay(1000);
}

void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}
