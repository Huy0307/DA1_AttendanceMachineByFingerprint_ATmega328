#include <Wire.h>
#include <U8glib.h>
#include <Adafruit_MLX90614.h>
#include <DS1307RTC.h>
#include <Fingerprint.h>
#include <SD.h>
#include <SPI.h>

// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE | U8G_I2C_OPT_DEV_0);

Adafruit_MLX90614 mlx = Adafruit_MLX90614();
const int chipSelect = 10;
const byte buttonPin1 = 2;
const byte buttonPin2 = 3;
const byte enPin = 4;
const byte sensorPin = 5;
const byte buzzer = 8;
byte mode = 2;

struct Data {
  float tempC;
  uint8_t id;
  uint8_t finger_id;
  tmElements_t tm;
};
Data data;

volatile bool buttonPressed1 = false;
volatile bool buttonPressed2 = false;

void delay_millis(unsigned long ms) {
  unsigned long current_time = millis();
  while (millis() - current_time < ms) {
    // wait
  }
}

void setup() {
  Serial.begin(9600);

  SPI.begin();

  pinMode(chipSelect, OUTPUT);

  u8g.setFont(u8g_font_5x7);

  fingerprintSetup();

  pinMode(sensorPin, INPUT);
  pinMode(buzzer, OUTPUT);

  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(buttonPin1), buttonInterrupt1, FALLING);
  attachInterrupt(digitalPinToInterrupt(buttonPin2), buttonInterrupt2, FALLING);

  digitalWrite(buzzer, LOW);
  pinMode(enPin, OUTPUT);
  digitalWrite(enPin, HIGH);

  if (!SD.begin(chipSelect)) {
    Serial.println(F("Không tìm thấy thẻ nhớ SD."));
    return;
  }
}

void loop() {
  readstate();

  readTemp();

  readFinger();

  Time();

  SDcard();

  data.tempC = 0;
  data.finger_id = 0;
}

void buttonInterrupt1() {
  buttonPressed1 = true;
}

void buttonInterrupt2() {
  buttonPressed2 = true;
}

void readstate() {
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
}

void readTemp() {
  byte sensorValue = digitalRead(sensorPin);
  if (sensorValue == LOW) {
    data.tempC = mlx.readObjectTempC();
    digitalWrite(buzzer, HIGH);
    delay_millis(250);
    digitalWrite(buzzer, LOW);
    u8g.firstPage();
    do {
      u8g.setFont(u8g_font_unifont);
      u8g.setPrintPos(0, 20);
      u8g.print("Temp: ");
      u8g.print(data.tempC);
    } while (u8g.nextPage());
    delay_millis(1000);
  }
}

void readFinger() {
  // Code for reading fingerprints goes here
}

void Time() {
  if (RTC.read(data.tm)) {
    u8g.firstPage();
    do {
      u8g.setFont(u8g_font_unifont);
      u8g.setPrintPos(0, 20);
      u8g.print(F("Time: "));
      u8g.print(data.tm.Hour);
      u8g.print(F(":"));
      u8g.print(data.tm.Minute);
      u8g.print(F(":"));
      u8g.println(data.tm.Second);
    } while (u8g.nextPage());
    delay_millis(1000);
    if (data.finger_id != 0) {
      if (data.tm.Hour >= 21 && data.tm.Hour < 22) {
        // tính thời gian điểm danh so với 8 giờ sáng
        int minutes_late = (data.tm.Hour - 21) * 60 + data.tm.Minute;
        if (minutes_late <= 0) {
          Serial.print(F("E"));
          do {
            u8g.setPrintPos(10, 10);
            u8g.print(F("Arrived early"));  // đến đúng giờ
          } while (u8g.nextPage());
          delay_millis(2000);
        } else if (minutes_late <= 15) {
          Serial.print(F("O"));
          do {
            u8g.setPrintPos(10, 10);
            u8g.print(F("On Time"));  // đến sớm
          } while (u8g.nextPage());
          delay_millis(2000);
        } else {
          Serial.print(F("L\n"));
          do {
            u8g.setPrintPos(10, 10);
            u8g.print(F("Late"));  // đến trễ
          } while (u8g.nextPage());
          delay_millis(2000);
        }
      } else if (data.tm.Hour < 21 || data.tm.Hour > 22) {
        Serial.print(F("Time out"));
        do {
          u8g.setPrintPos(10, 10);
          u8g.print(F("Time out"));  // thời gian điểm danh đã kết thúc
        } while (u8g.nextPage());
        delay_millis(2000);
      }
    }
  }
}

void SDcard() {
  File dataFile = SD.open("data.csv", FILE_WRITE);  // Mở file data.txt và chế độ ghi
  if (dataFile) {                                   // Nếu file đã được mở
    if (data.finger_id != 0 || data.tempC != 0) {
      Serial.println(F("Opening file"));
      if (data.finger_id != 0) {
        dataFile.print(data.finger_id);  // Lưu id vào file
      }
      dataFile.print(",");  // Phân cách giữa id và tempC
      if (data.tempC != 0) {
        dataFile.print(data.tempC);  // Lưu tempC vào file
      }
      dataFile.print(",");  // Phân cách
      dataFile.print(data.tm.Hour);
      dataFile.print(":");
      dataFile.print(data.tm.Minute);
      dataFile.print(":");
      dataFile.print(data.tm.Second);
      dataFile.println();  // Xuống dòng để lưu thông tin tiếp theo
    }
    dataFile.close();  // Đóng file
  } else {
    Serial.println(F("Error opening file"));  // In ra thông báo lỗi nếu không mở được file
  }
}
