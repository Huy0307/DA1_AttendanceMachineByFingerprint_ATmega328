#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <DS1307RTC.h>
#include "Fingerprint.h"
#include "oled_display.h"
#include <Keypad.h>
#include <Keypad_I2C.h>
#include <SD.h>
#include <SPI.h>
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
const byte numRows = 4;  // số lượng hàng trên keypad
const byte numCols = 4;  // số lượng cột trên keypad
// sử dụng keypad để điều khiển menu
#define I2CADDR 0x38
char key;
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
const byte enPin = 4;
const byte sensorPin = 5;
const byte buzzer = 8;
oled_display oled;
const byte numMenuItems = 2;
char menuItems[numMenuItems][20] = {
  "ENROLL",
  "DELETE",
};
struct Data {
  int currentMenuItem = 0;
  bool menuMode = true;
  bool itemSelected = false;
  bool redrawMenu = true;
  float tempC = 0;
  uint8_t id = 0;
  uint8_t finger_id = 0;
  tmElements_t tm;
  int mode = 2;
  byte to;
  byte e;
  byte l;
  byte o;
  int pass;
  int late_minutes;
  byte s;
};
Data data;
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
  bool keyPressed = false;  // biến cờ kiểm tra có ký tự mới được nhập vào hay không

  while (num == 0 && millis() - startTime < 7000) {  // vòng lặp cho đến khi nhận được dữ liệu hoặc đã trôi qua 7 giây
    key = customKeypad.getKey();
    if (key) {
      keyPressed = true;               // đặt biến cờ khi có ký tự mới được nhập vào
      if (key >= '0' && key <= '9') {  // chỉ xử lý các phím số
        inputString += key;            // thêm giá trị mới vào chuỗi
        int x = inputString.toInt();
        oled.print_int_1x(x, 0, 40);  // hiển thị số lên màn hình
      } else if (key == '#') {
        if (inputString.length() > 0) {
          num = inputString.toInt();  // chuyển đổi chuỗi thành số
        }
      } else if (key == '*') {
        if (inputString.length() > 0) {
          inputString.remove(inputString.length() - 1);  // Xóa ký tự cuối cùng
          oled.clear();                                  // Xóa mật khẩu đã hiển thị trên màn hình
          if (data.mode == 1) {
            oled.print_text_1x("\n", 1, 1);
            oled.print_text_1x("Waiting enroll ID #\n", 10, 10);
          } else if (data.mode == 3) {
            oled.print_text_1x("\n", 1, 1);
            oled.print_text_1x("Waiting for...\n", 10, 10);
          }
          for (int i = inputString.length() - 1; i >= 0; i--) {
            int y = inputString.toInt();
            oled.print_int_1x(y, 0, 40);  // hiển thị số lên màn hình
          }
        }
      }
    }
    if (keyPressed) {
      startTime = millis();  // cập nhật thời gian bắt đầu khi có ký tự mới được nhập vào
      keyPressed = false;    // đặt lại biến cờ
    }
  }

  if (num == 0) {  // nếu không nhận được dữ liệu
    return 255;    // trả về một giá trị khác để biểu thị không nhận được dữ liệu
  } else {
    return num;
  }
}
int readPass() {
  unsigned long startTime = millis();
  String passString;
  uint8_t num = 0;
  while (num == 0 && millis() - startTime < 7000) {
    key = customKeypad.getKey();
    if (key) {
      if (key >= '0' && key <= '9') {
        passString += key;
        Serial.println(key);
        oled.print_text1x("*");  // Hiển thị dấu "*" cho mỗi ký tự nhập vào
      } else if (key == '#') {
        if (passString.length() > 0) {
          return passString.toInt();
        }
      } else if (key == '*') {
        if (passString.length() > 0) {
          passString.remove(passString.length() - 1);  // Xóa ký tự cuối cùng
          Serial.println(passString);
          oled.clear();  // Xóa mật khẩu đã hiển thị trên màn hình
          oled.print_text_1x("\n", 1, 1);
          oled.print_text_2x("Enter pass: \n", 0, 10);
          for (int i = passString.length() - 1; i >= 0; i--) {
            oled.print_text1x("*");  // Hiển thị dấu "*" cho mỗi ký tự nhập vào
          }
        }
      }
    }
  }
  if (num == 0) {  // nếu không nhận được dữ liệu
    return 255;    // trả về một giá trị khác để biểu thị không nhận được dữ liệu
    oled.clear();
  } else {
    return num;
    oled.clear();
  }
}
void readKey() {
  // đọc phím nhấn từ keypad
  key = customKeypad.getKey();
  // điều khiển menu
  if (data.menuMode) {
    if (data.redrawMenu) {
      showMenu();
    }
    if (key != NO_KEY) {
      // xử lý các giá trị nhận được từ keypad
      if (key) {
        switch (key) {
          case 'A':
            if (data.currentMenuItem > 0) {
              data.currentMenuItem--;
              data.redrawMenu = true;
            }
            break;
          case 'B':
            if (data.currentMenuItem < numMenuItems - 1) {
              data.currentMenuItem++;
              data.redrawMenu = true;
            }
            break;
          case 'C':
            selectMenuItem();
            break;
          case 'D':
            data.menuMode = true;
            data.redrawMenu = true;
            break;
        }
      }
    }
  } else {
    // quay trở lại menu nếu đã hoàn thành chức năng
    if (data.itemSelected) {
      data.menuMode = true;
      data.itemSelected = false;
      data.redrawMenu = true;
    }
  }
}
void setup() {
  Serial.begin(9600);
  Wire.begin();
  customKeypad.begin(makeKeymap(keys));
  SPI.begin();                  // Bắt đầu SPI
  pinMode(chipSelect, OUTPUT);  // Chọn thẻ nhớ SD bằng chân kết nối
  oled.begin();
  fingerprintSetup();
  pinMode(sensorPin, INPUT);
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, HIGH);
  pinMode(enPin, OUTPUT);
  digitalWrite(enPin, HIGH);  // Kích hoạt cảm biến bằng cách đưa chân EN lên mức HIGH
  if (!SD.begin(chipSelect)) {
    Serial.println(F("Không tìm thấy thẻ nhớ SD."));
    return;
  }
  mlx.begin();
}
void loop() {
  readKey();
  readFinger();
  readTemp();  // Check if object detected by YS-29 sensor
  Time();      // oled current time
  SDcard();
  data.tempC = 0;
  data.finger_id = 0;
}
void showMenu() {
  oled.print_text_1x("\n", 1, 1);
  oled.print_text2x("\n");
  oled.print_text_1x("   MENU FINGER\n", 0, 30);
  for (int i = 0; i < numMenuItems; i++) {
    if (data.currentMenuItem == i) {
      oled.print_text_1x(">", 0, 30);
    } else {
      oled.print_text_1x(" ", 0, 30);
    }
    oled.print_text_1x(menuItems[i], 5, 30);
    oled.print_text1x("\n");
  }
  data.redrawMenu = false;
}
void selectMenuItem() {
  data.itemSelected = true;
  switch (data.currentMenuItem) {
    case 0:
      data.mode = 1;
      Serial.print(data.mode);
      break;
    case 1:
      data.mode = 3;
      Serial.print(data.mode);
      break;
  }
}
void readTemp() {
  byte sensorValue = digitalRead(sensorPin);
  if (sensorValue == LOW) {
    data.tempC = mlx.readObjectTempC();
    digitalWrite(buzzer, LOW);
    delay_millis(250);
    digitalWrite(buzzer, HIGH);
    oled.clear();
    oled.print_text_2x("Temp \n", 40, 40);
    oled.print_float_2x(data.tempC, 30, 40);
    if (data.tempC >= 37.5) {
      digitalWrite(buzzer, LOW);
      delay_millis(1000);
      digitalWrite(buzzer, HIGH);
    }
    // Update OLED oled
    delay_millis(1000);
    oled.clear();
    data.redrawMenu = true;
  }
}
void readFinger() {
  if (data.mode == 1) {
    oled.clear();
    oled.print_text_1x("\n", 1, 1);
    oled.print_text_2x("Enter pass: \n", 0, 10);
    delay_millis(1000);
    data.pass = readPass();
    if (data.pass == 12) {
      oled.clear();
      oled.print_text_1x("\n", 1, 1);
      oled.print_text_1x("Waiting enroll ID #\n", 10, 10);
      data.id = readnumber();
      if (data.id != 255) {
        oled.clear();
        oled.print_text_1x("\n", 1, 1);
        oled.print_text_1x("Waiting enroll ID #\n", 10, 10);
        oled.print_uint8t_2x(data.id, 10, 25);
        getFingerprintEnroll(data.id, data.s);
        if (data.s == 1) {
          oled.clear();
          oled.print_text_2x("Enroll!!", 10, 10);
          delay_millis(1000);
          oled.clear();
          data.mode = 2;
        } else if (data.s == 0) {
          oled.clear();
          oled.print_text_2x("FAILED!!", 10, 10);
          delay_millis(1000);
          data.mode = 1;
        }
      } else {
        data.mode = 2;
      }
      data.redrawMenu = true;
      oled.clear();
    } else {
      data.mode = 2;
    }
    oled.clear();
  }
  if (data.mode == 2) {
    getFingerprintID(data.finger_id);
    if (data.finger_id != 0) {
      oled.clear();
      oled.print_text1x("\n");
      // oled.print_text_1x("Found a print match!\n", 10, 20);
      oled.print_text_2x("Found ID\n", 10, 20);
      oled.print_uint8t_2x(data.finger_id, 10, 25);
      data.redrawMenu = true;
      delay_millis(1000);
      oled.clear();
    }
  }
  if (data.mode == 3) {
    oled.clear();
    oled.print_text_1x("\n", 1, 1);
    oled.print_text_2x("Enter pass: \n", 0, 10);
    delay_millis(1000);
    data.pass = readPass();
    if (data.pass == 13) {
      oled.clear();
      oled.print_text_1x("\n", 1, 1);
      oled.print_text_1x("Waiting for...\n", 10, 10);
      data.id = readnumber();
      if (data.id != 255) {  // ID #0 not allowed, try again!
        deleteFingerprint(data.id);
        oled.clear();
        oled.print_text_1x("\n", 1, 1);
        oled.print_text_2x("DELETE ID\n", 10, 10);
        oled.print_uint8t_2x(data.id, 10, 25);
        delay_millis(2000);
        oled.clear();
        data.mode = 2;
      } else {
        data.mode = 2;
      }
      data.redrawMenu = true;
    } else {
      data.mode = 2;
    }
    oled.clear();
  }
}
void Time() {
  if (RTC.read(data.tm)) {
    data.redrawMenu = true;
    if (data.finger_id != 0) {
      if (data.tm.Hour >= 7 && data.tm.Hour < 9) {
        data.late_minutes = (data.tm.Hour - 8) * 60 + data.tm.Minute;
        if (data.late_minutes <= 0) {
          data.e = 1;
          Serial.print(F("E"));
          oled.clear();
          oled.print_text_2x("Arrived\n", 10, 10);  // đến đúng giờ
          oled.print_text_2x("early", 12, 10);      // đến đúng giờ
          delay_millis(2000);
          oled.clear();
        } else if (data.late_minutes <= 15) {
          data.o = 1;
          Serial.print(F("O"));
          oled.clear();
          oled.print_text_2x("On Time", 10, 10);  // đến sớm
          delay_millis(2000);
          oled.clear();
        } else {
          data.l = 1;
          Serial.print(F("L\n"));
          oled.clear();
          oled.print_text_2x("Late: ", 10, 10);
          oled.print_int2x(data.late_minutes - 15);
          oled.print_text2x("minutes");  // đến trễ
          delay_millis(2000);
          oled.clear();
        }
      } else if (data.tm.Hour < 7 || data.tm.Hour > 9) {
        data.to = 1;
        Serial.print(F("Time out"));
        oled.clear();
        oled.print_text_2x("Time out", 10, 10);  // thời gian điểm danh đã kết thúc
        delay_millis(2000);
        oled.clear();
      }
    }
    oled.print_text1x("\n");
    oled.print_text_2x("Time: \n", 0, 0);
    if (data.tm.Hour < 10) {
      oled.print_text_2x("0", 0, 20);  // Add leading zero for single digit hour values
    }
    oled.print_int_2x(data.tm.Hour, 0, 20);
    oled.print_text2x(":");
    if (data.tm.Minute < 10) {
      oled.print_text2x("0");  // Add leading zero for single digit minute values
    }
    oled.print_int2x(data.tm.Minute);
    oled.print_text2x(":");
    if (data.tm.Second < 10) {
      oled.print_text2x("0");  // Add leading zero for single digit second values
    }
    oled.print_int2x(data.tm.Second);
  }
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
      dataFile.print(data.tm.Day);
      dataFile.print(F("-"));
      dataFile.print(data.tm.Month);
      dataFile.print(F("-"));
      dataFile.print(tmYearToCalendar(data.tm.Year));
      dataFile.print(F(","));
      dataFile.print(F(" "));
      dataFile.print(data.tm.Hour);
      dataFile.print(F(":"));
      dataFile.print(data.tm.Minute);
      dataFile.print(F(":"));
      dataFile.print(data.tm.Second);
      dataFile.print(F(","));  // Phân cách
      if (data.finger_id != 0) {
        if (data.e == 1) {
          dataFile.print(F("EARLY"));
        } else if (data.o == 1) {
          dataFile.print(F("ON TIME"));
        } else if (data.l == 1) {
          dataFile.print(F("LATE: "));
          dataFile.print(data.late_minutes - 15);
        } else if (data.to == 1) {
          dataFile.print(F("TIME OUT"));
        }
        dataFile.println();  // Xuống dòng để lưu thông tin tiếp theo
      }
    }
    dataFile.close();  // Đóng file
  } else {
    Serial.println(F("Error opening file"));  // In ra thông báo lỗi nếu không mở được file
  }
  data.redrawMenu = true;
}