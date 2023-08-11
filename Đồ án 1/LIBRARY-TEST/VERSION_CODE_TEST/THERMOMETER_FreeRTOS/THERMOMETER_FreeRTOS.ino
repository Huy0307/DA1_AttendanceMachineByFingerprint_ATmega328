#include <Adafruit_SSD1306.h>
#include <Adafruit_MLX90614.h>
#include <DS1307RTC.h>
#include <Fingerprint.h>
#include <Arduino_FreeRTOS.h>
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
const byte buttonPin = 2;
byte mode = 2;
const byte enPin = 4;
const byte sensorPin = 5;
const byte buzzer = 8;
uint8_t id;
uint8_t finger_id;
TaskHandle_t Task1Handle;
TaskHandle_t Task2Handle;

void delay_millis(unsigned long ms) {
  vTaskDelay(pdMS_TO_TICKS(ms));  // delay dùng FreeRTOS
}

void setup() {
  Serial.begin(9600);
  fingerprintSetup();
  pinMode(sensorPin, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  digitalWrite(buzzer, LOW);
  pinMode(enPin, OUTPUT);
  digitalWrite(enPin, HIGH);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  mlx.begin();

  xTaskCreate(
    Task1,
    "Task1",
    1000,
    NULL,
    1,
    &Task1Handle);

  xTaskCreate(
    Task2,
    "Task2",
    1000,
    NULL,
    2,
    &Task2Handle);
}
void Task1(void *pvParameters) {
  while (1) {
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
      vTaskDelay(1000 / portTICK_PERIOD_MS);
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
        vTaskDelay(1000 / portTICK_PERIOD_MS);
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
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void Task2(void *pvParameters) {
  while (1) {
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
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    byte sensorValue = digitalRead(sensorPin);
    if (sensorValue == LOW) {
      float tempC = mlx.readObjectTempC();
      digitalWrite(buzzer, HIGH);
      vTaskDelay(250 / portTICK_PERIOD_MS);
      digitalWrite(buzzer, LOW);
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0, 10);
      display.print(F("Temp: "));
      display.println(tempC);
      // Update OLED display
      display.display();
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }
}
void loop() {
}
