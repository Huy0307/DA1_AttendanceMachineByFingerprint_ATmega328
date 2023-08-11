#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <DS1307RTC.h>
#include "Fingerprint.h"
#include "ssd1306.h"
#include <Keypad.h>
#include <Keypad_I2C.h>
#include <SD.h>
#include <SPI.h>
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
const byte numRows = 4;  // số lượng hàng trên keypad
const byte numCols = 4;  // số lượng cột trên keypad
// sử dụng keypad để điều khiển menu
char customKey;
#define OLED_ADDRESS 0x3C
#define OLED_RESET 4
#define I2CADDR 0x38
char keys[numRows][numCols] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
// phím được gán trên các hàng và cột
byte rowPins[numRows] = { 7, 6, 5, 4 };  // các hàng từ 0 đến 3
byte colPins[numCols] = { 3, 2, 1, 0 };  // các cột từ 0 đến 3
// khởi tạo Keypad
Keypad_I2C customKeypad(makeKeymap(keys), rowPins, colPins, numRows, numCols, I2CADDR);
const int chipSelect = 10;  // chọn chân kết nối với SD card reader
const byte buttonPin1 = 2;  // Sử dụng chân số 13 để kết nối với nút bấm
const byte buttonPin2 = 3;  // Sử dụng chân số 13 để kết nối với nút bấm
const byte enPin = 4;
const byte sensorPin = 5;
const byte buzzer = 8;
struct Data {
  float tempC = 0;
  uint8_t id = 0;
  uint8_t finger_id = 0;
  tmElements_t tm;
  byte mode = 2;
  int late_minutes;
  byte s;
};
Data data;
volatile bool buttonPressed1 = false;
volatile bool buttonPressed2 = false;
void delay_millis(unsigned long ms) {
  unsigned long current_time = millis();  // Lấy thời điểm hiện tại
  while (millis() - current_time < ms) {  // Kiểm tra nếu thời gian chạy đã vượt quá khoảng thời gian cần delay
    // Chờ
  }
}
uint8_t readnumber() {
  unsigned long startTime = millis();  // thời gian bắt đầu
  String inputString;
  uint8_t num = 0;
  while (num == 0 && millis() - startTime < 7000) {  // vòng lặp cho đến khi nhận được dữ liệu hoặc đã trôi qua 10 giây
    char key = customKeypad.getKey();
    if (key) {
      Serial.println(key);
      if (key >= '0' && key <= '9') {  // chỉ xử lý các phím số
        inputString += key;            // thêm giá trị mới vào chuỗi
      } else if (key == '#') {
        if (inputString.length() > 0) {
          num = inputString.toInt();  // chuyển đổi chuỗi thành số
        }
      } else if (key == '*') {
        inputString = "";  // xóa dữ liệu nhập vào
      }
    }
  }

  if (num == 0) {  // nếu không nhận được dữ liệu
    return 255;    // trả về một giá trị khác để biểu thị không nhận được dữ liệu
  } else {
    return num;
  }
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  customKeypad.begin(makeKeymap(keys));
  SPI.begin();                  // Bắt đầu SPI
  pinMode(chipSelect, OUTPUT);  // Chọn thẻ nhớ SD bằng chân kết nối
  pinMode(OLED_RESET, OUTPUT);
  digitalWrite(OLED_RESET, LOW);
  digitalWrite(OLED_RESET, HIGH);
  ssd1306_init();
  ssd1306_clearScreen();
  fingerprintSetup();
  pinMode(sensorPin, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(buttonPin1, INPUT_PULLUP);  // Đặt chân số 2 là INPUT_PULLUP để kết nối nút bấm
  pinMode(buttonPin2, INPUT_PULLUP);  // Đặt chân số 3 là INPUT_PULLUP để kết nối nút bấm
  attachInterrupt(digitalPinToInterrupt(buttonPin1), buttonInterrupt1, FALLING);
  attachInterrupt(digitalPinToInterrupt(buttonPin2), buttonInterrupt2, FALLING);
  digitalWrite(buzzer, LOW);
  pinMode(enPin, OUTPUT);
  digitalWrite(enPin, HIGH);  // Kích hoạt cảm biến bằng cách đưa chân EN lên mức HIGH
  if (!SD.begin(chipSelect)) {
    Serial.println(F("Không tìm thấy thẻ nhớ SD."));
    return;
  }
  mlx.begin();
}
void loop() {
  ssd1306_setFixedFont(ssd1306xled_font6x8);
  readstate();
  readFinger();
  Time();  // oled current time
  attendance();
  readTemp();  // Check if object detected by YS-29 sensor
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
    data.mode = 3;
    Serial.print(data.mode);
    delay_millis(1000);
  }
  byte buttonState2 = digitalRead(buttonPin2);
  if (buttonPressed2 == true && buttonState2 == LOW) {
    buttonPressed2 = false;
    data.mode = 1;
    Serial.print(data.mode);
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
    ssd1306_clearScreen();
    ssd1306_printFixed2x(30, 15, "Temp", STYLE_NORMAL);
    ssd1306_printFixed2x(30, 30, String(data.tempC).c_str(), STYLE_NORMAL);
    // Update OLED oled
    delay_millis(1000);
    ssd1306_clearScreen();
  }
}
void readFinger() {
  if (data.mode == 1) {
    ssd1306_clearScreen();
    ssd1306_printFixed(10, 10, "1. PLACE FINGER", STYLE_NORMAL);
    ssd1306_printFixed(10, 15, "2. REMOVE FINGER", STYLE_NORMAL);
    ssd1306_printFixed(10, 20, "3. PLACE AGAIN", STYLE_NORMAL);
    delay_millis(2000);
    ssd1306_clearScreen();
    ssd1306_printFixed(10, 10, "Waiting enroll ID #", STYLE_NORMAL);
    delay_millis(1000);
    data.id = readnumber();
    if (data.id != 255) {
      ssd1306_clearScreen();
      ssd1306_printFixed(10, 20, String(data.id).c_str(), STYLE_NORMAL);
      delay_millis(1000);
      getFingerprintEnroll(data.id, data.s);
      if (data.s == 1) {
        ssd1306_clearScreen();
        ssd1306_printFixed(10, 10, "Enroll!!", STYLE_NORMAL);
        delay_millis(1000);
        ssd1306_clearScreen();
        data.mode = 2;
      } else if (data.s == 0) {
        ssd1306_clearScreen();
        ssd1306_printFixed(10, 10, "FAILED!!", STYLE_NORMAL);
        delay_millis(1000);
        data.mode = 1;
      }
    } else {
      data.mode = 2;
    }
    ssd1306_clearScreen();
  }
  if (data.mode == 2) {
    getFingerprintID(data.finger_id);
    if (data.finger_id != 0) {
      ssd1306_clearScreen();
      ssd1306_printFixed(5, 20, "Found a print match!", STYLE_NORMAL);
      ssd1306_printFixed(5, 30, "Match found ID", STYLE_NORMAL);
      ssd1306_printFixed2x(5, 40, "ID", STYLE_NORMAL);
      ssd1306_printFixed2x(30, 40, String(data.finger_id).c_str(), STYLE_NORMAL);
      delay_millis(2000);
      ssd1306_clearScreen();
    }
  }
  if (data.mode == 3) {
    data.id = readnumber();
    if (data.id != 0) {  // ID #0 not allowed, try again!
      deleteFingerprint(data.id);
    } else {
      data.mode = 2;
    }
    ssd1306_clearScreen();
    ssd1306_printFixed(10, 0, "DELETED!", STYLE_NORMAL);
    delay_millis(1000);
    data.mode = 2;
  }
}
void Time() {
  if (RTC.read(data.tm)) {
    ssd1306_printFixed2x(0, 0, "Time: ", STYLE_NORMAL);
    if (data.tm.Hour < 10) {
      ssd1306_printFixed2x(0, 20, "0 ", STYLE_BOLD);  // Add leading zero for single digit hour values
    }
    ssd1306_printFixed2x(0, 20, String(data.tm.Hour).c_str(), STYLE_NORMAL);
    ssd1306_printFixed2x(25, 20, ":", STYLE_BOLD);
    if (data.tm.Minute < 10) {
      ssd1306_printFixed2x(35, 20, "0 ", STYLE_BOLD);  // Add leading zero for single digit minute values
    }
    ssd1306_printFixed2x(35, 20, String(data.tm.Minute).c_str(), STYLE_NORMAL);
    ssd1306_printFixed2x(60, 20, ":", STYLE_BOLD);
    if (data.tm.Second < 10) {
      ssd1306_printFixed2x(70, 20, "0 ", STYLE_BOLD);  // Add leading zero for single digit second values
    }
    ssd1306_printFixed2x(70, 20, String(data.tm.Second).c_str(), STYLE_BOLD);
  }
  delay_millis(1000);
}
void SDcard() {
  File dataFile = SD.open("data.csv", FILE_WRITE);  // Mở file data.txt và chế độ ghi
  if (dataFile) {                                   // Nếu file đã được mở
    if (data.finger_id != 0 || data.tempC != 0) {
      Serial.println(F("Opening file"));
      if (data.finger_id != 0) {
        dataFile.print(F("ID: "));
        dataFile.print(data.finger_id);  // Lưu id vào file
      }
      dataFile.print(F(","));  // Phân cách giữa id và tempC
      if (data.tempC != 0) {
        dataFile.print(F("TEMP: "));
        dataFile.print(data.tempC);  // Lưu tempC vào file
      }
      dataFile.print(F(","));  // Phân cách
      dataFile.print(data.tm.Hour);
      dataFile.print(F(":"));
      dataFile.print(data.tm.Minute);
      dataFile.print(F(":"));
      dataFile.print(data.tm.Second);
      dataFile.println();  // Xuống dòng để lưu thông tin tiếp theo
    }
    dataFile.close();  // Đóng file
  } else {
    Serial.println(F("Error opening file"));  // In ra thông báo lỗi nếu không mở được file
  }
}
void attendance() {
  if (RTC.read(data.tm)) {
    if (data.finger_id != 0) {
      if (data.tm.Hour >= 7 && data.tm.Hour < 9) {
        // tính thời gian điểm danh so với 21h15p
        data.late_minutes = (data.tm.Hour - 8) * 60 + data.tm.Minute;
        if (data.late_minutes <= 0) {
          Serial.print(F("E"));
          ssd1306_clearScreen();
          ssd1306_printFixed2x(10, 10, "Arrived", STYLE_NORMAL);  // đến đúng giờ
          ssd1306_printFixed2x(10, 20, "early", STYLE_NORMAL);    // đến đúng giờ
          delay_millis(2000);
          ssd1306_clearScreen();
        } else if (data.late_minutes <= 15) {
          Serial.print(F("O"));
          ssd1306_clearScreen();
          ssd1306_printFixed2x(10, 10, "On Time", STYLE_NORMAL);  // đến sớm
          delay_millis(2000);
          ssd1306_clearScreen();
        } else {
          Serial.print(F("L\n"));
          ssd1306_clearScreen();
          ssd1306_printFixed2x(0, 15, "Late:", STYLE_NORMAL);
          ssd1306_printFixed2x(55, 15, String(data.late_minutes - 15).c_str(), STYLE_NORMAL);
          ssd1306_printFixed2x(85, 15, "min", STYLE_NORMAL);  // đến trễ
          delay_millis(2000);
          ssd1306_clearScreen();
        }
      } else if (data.tm.Hour < 7 || data.tm.Hour > 9) {
        Serial.print(F("Time out"));
        ssd1306_clearScreen();
        ssd1306_printFixed2x(10, 10, "Time out", STYLE_NORMAL);  // thời gian điểm danh đã kết thúc
        delay_millis(2000);
        ssd1306_clearScreen();
      }
    }
  }
}