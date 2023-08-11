#include <Adafruit_SSD1306.h>
#include <Adafruit_MLX90614.h>
#include <DS1307RTC.h>
#include <Fingerprint.h>
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
const byte buttonPin1 = 2;  // Sử dụng chân số 13 để kết nối với nút bấm
const byte buttonPin2 = 3;  // Sử dụng chân số 13 để kết nối với nút bấm
const byte enPin = 4;
const byte sensorPin = 5;
const byte buzzer = 8;
byte mode = 2;
uint8_t id;
uint8_t finger_id;
volatile bool buttonPressed1 = false;
volatile bool buttonPressed2 = false;
void delay_millis(unsigned long ms) {
  unsigned long current_time = millis();  // Lấy thời điểm hiện tại
  while (millis() - current_time < ms) {  // Kiểm tra nếu thời gian chạy đã vượt quá khoảng thời gian cần delay
    // Chờ
  }
}

void setup() {
  Serial.begin(9600);
  fingerprintSetup();
  pinMode(sensorPin, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(buttonPin1, INPUT_PULLUP);  // Đặt chân số 2 là INPUT_PULLUP để kết nối nút bấm
  pinMode(buttonPin2, INPUT_PULLUP);  // Đặt chân số 3 là INPUT_PULLUP để kết nối nút bấm
  attachInterrupt(digitalPinToInterrupt(buttonPin1), buttonInterrupt1, FALLING);
  attachInterrupt(digitalPinToInterrupt(buttonPin2), buttonInterrupt2, FALLING);
  digitalWrite(buzzer, LOW);
  pinMode(enPin, OUTPUT);
  // Kích hoạt cảm biến bằng cách đưa chân EN lên mức HIGH
  digitalWrite(enPin, HIGH);
  // Initialize display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  mlx.begin();
}
void buttonInterrupt1() {
  buttonPressed1 = true;
}
void buttonInterrupt2() {
  buttonPressed2 = true;
}
void loop() {
  byte buttonState1 = digitalRead(buttonPin1);
  if (buttonPressed1 == true && buttonState1 == LOW) {
    buttonPressed1 = false;
    mode = 3;
    Serial.print(mode);
    delay_millis(1000);
  }
  byte buttonState2 = digitalRead(buttonPin2);
  if (buttonPressed2 == true && buttonState2 == LOW) {
    buttonPressed2 = false;
    mode = 1;
    Serial.print(mode);
    delay_millis(1000);
  }
  // Check if object detected by YS-29 sensor
  // Display current time
  tmElements_t tm;
  if (RTC.read(tm)) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.print(F("Time: "));
    display.print(tm.Hour);
    display.print(F(":"));
    display.print(tm.Minute);
    display.print(F(":"));
    display.println(tm.Second);
    display.display();
    delay_millis(1000);
  }
  byte sensorValue = digitalRead(sensorPin);
  if (sensorValue == LOW) {
    float tempC = mlx.readObjectTempC();
    digitalWrite(buzzer, HIGH);
    delay_millis(250);
    digitalWrite(buzzer, LOW);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.print(F("Temp: "));
    display.println(tempC);
    // Update OLED display
    display.display();
    delay_millis(1000);
  }
  if (mode == 1) {
    id = readnumber();
    if (id == 0) {  // ID #0 not allowed, try again!
      return;
    }
    getFingerprintEnroll(id);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.println(F("ENROLLED!!"));
    display.display();
    delay_millis(1000);
    mode = 2;
  }
  if (mode == 2) {
    getFingerprintID(finger_id);
    if (finger_id != 0) {
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0, 10);
      display.println(F("Found a print match!"));
      display.println(F("Match found ID "));
      display.print(finger_id);
      display.display();
      delay_millis(1000);
    }
    finger_id = 0;
  }
  if (mode == 3) {
    id = readnumber();
    if (id == 0) {  // ID #0 not allowed, try again!
      return;
    }
    deleteFingerprint(id);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.println(F("DELETED!"));
    display.display();
    delay_millis(1000);
    mode = 2;
  }
}