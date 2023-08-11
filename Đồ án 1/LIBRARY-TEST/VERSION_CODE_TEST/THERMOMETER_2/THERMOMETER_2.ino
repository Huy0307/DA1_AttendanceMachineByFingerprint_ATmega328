#include <SPI.h>
#include <Wire.h>
#include <Time.h>
#include <DS1307RTC.h>
#include <Fingerprint.h>
#include <MLX_sensor.h>
#include <SimpleTimer.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);
uint8_t id;
int enPin = 4;
int sensorPin = 5;
byte buttonPin = 8;  // Sử dụng chân số 13 để kết nối với nút bấm
uint8_t pass;
uint8_t finger_id;
byte mode = 2;
float object_temp, ambient_temp;
int sensorValue;
byte buzzer = 6;
unsigned long lastButtonTime = millis();
void delay_millis(unsigned long ms) {
  unsigned long current_time = millis();  // Lấy thời điểm hiện tại
  while (millis() - current_time < ms) {  // Kiểm tra nếu thời gian chạy đã vượt quá khoảng thời gian cần delay
    // Chờ
  }
}
void setup() {
  Serial.begin(230400);
  // Wire.begin();
  fingerprintSetup();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  MLXsetup();
  pinMode(buttonPin, INPUT_PULLUP);  // Đặt chân số 2 là INPUT_PULLUP để kết nối nút bấm
  pinMode(sensorPin, INPUT);
  pinMode(enPin, OUTPUT);
  // Kích hoạt cảm biến bằng cách đưa chân EN lên mức HIGH
  digitalWrite(enPin, HIGH);
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, LOW);
}

void loop() {
  // checkTime();
  int buttonValue = digitalRead(buttonPin);
  // kiểm tra xem nút nhấn đã được nhấn trong vòng 500ms trước đó hay chưa
  if (buttonValue == LOW && millis() - lastButtonTime > 1500) {
    // nếu nút nhấn được nhấn, và đã đủ 500ms kể từ lần nhấn trước đó
    // thì lưu lại thời điểm nhấn nút nhấn để sử dụng cho lần nhấn tiếp theo
    lastButtonTime = millis();
    mode = 1;
    // thực hiện các thao tác khi nút nhấn được nhấn ở đây
    CheckAdd();
  }
  CheckID();
  sensorValue = digitalRead(sensorPin);
  if (sensorValue == LOW) {
    digitalWrite(buzzer, HIGH);
    delay_millis(250);
    digitalWrite(buzzer, LOW);
    readTemperature(object_temp, ambient_temp);
    Serial.print("Object temp: ");
    Serial.print(object_temp);
    Serial.print(", Ambient temp: ");
    Serial.println(ambient_temp);
    display.clearDisplay();
    display.setCursor(25, 10);
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.print(object_temp);
    display.println(" C");
    display.display();
    delay_millis(1500);
    if (object_temp >= 37.5) {
      digitalWrite(buzzer, HIGH);
      Serial.print("ALERT!!!!\n ");
      delay_millis(1500);
    }
    // digitalWrite(led_pin, LOW);     //LED tắt
    digitalWrite(buzzer, LOW);  //Buzzer tắt
    // digitalWrite(laser, LOW);       //laser tắt
  }
  // CheckAdd();
  // CheckDelete();
  // delay_millis(300);
}
void CheckAdd() {
  if (mode == 1) {
    id = readnumber();
    if (id == 0) {  // ID #0 not allowed, try again!
      return;
    }
    getFingerprintEnroll(id);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("ENROLL SUCCESS!!\n");
    display.println("Enroll ID #");
    display.println(id);
    display.display();
    delay_millis(1000);
  }
}
void CheckDelete() {
  if (mode == 3) {
    id = readnumber();
    if (id == 0) {  // ID #0 not allowed, try again!
      return;
    }
    deleteFingerprint(id);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("DELETE SUCCESS!!");
    display.display();
    delay_millis(1000);
  }
}
void CheckID() {
  if (mode == 2) {
    getFingerprintID(finger_id);
    if (finger_id != 0) {
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0, 0);
      display.println("Found a print match!");
      display.println("Match found ID ");
      display.print(finger_id);
      display.display();
      delay_millis(1000);
    }
  }
}
void checkTime() {
  tmElements_t tm;
  if (RTC.read(tm)) {
    Serial.print(tm.Hour);
    Serial.print(':');
    Serial.print(tm.Minute);
    Serial.print(':');
    Serial.print(tm.Second);
    Serial.print(' ');
    Serial.print(tm.Day);
    Serial.print('/');
    Serial.print(tm.Month);
    Serial.print('/');
    Serial.print(tmYearToCalendar(tm.Year));
    Serial.print('\n');
  } else {
    if (RTC.chipPresent()) {
      Serial.print("DS1307 stopped,run set time");
    } else {
      Serial.print("DS1307 read error,check circuit");
    }
  }
}