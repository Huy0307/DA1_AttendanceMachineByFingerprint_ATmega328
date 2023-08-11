#include "Fingerprint.h"
const int buttonPin = 8;     // Pin của nút bấm
int buttonState = HIGH;      // Trạng thái ban đầu của nút bấm (nút chưa được nhấn)
int lastButtonState = HIGH;  // Trạng thái trước đó của nút bấm
uint8_t id;
uint8_t finger_id;
byte mode = 2;
byte r;
byte again;
byte s;
void setup() {
  Serial.begin(9600);
  fingerprintSetup();
  pinMode(buttonPin, INPUT_PULLUP);  // Cấu hình chân nút bấm là INPUT_PULLUP để tránh nhiễu
}
/**
*@brief (Mô tả chức năng)
-	Hàm thiết lập chế độ hoạt động của cảm biến.
*@param (tham số truyền vào dạng tham chiếu)
-	int mode: Chế độ hoạt động của cảm biến. 
*@retval (kết quả trả về)
*/
uint8_t readnumber() {
  unsigned long startTime = millis();  // thời gian bắt đầu
  uint8_t num = 0;
  while (num == 0 && millis() - startTime < 7000) {  // vòng lặp cho đến khi nhận được dữ liệu hoặc đã trôi qua 10 giây
    num = Serial.parseInt();
  }
  if (num == 0) {  // nếu không nhận được dữ liệu
    return 255;    // trả về một giá trị khác để biểu thị không nhận được dữ liệu
  } else {
    return num;
  }
}
void loop() {
  buttonState = digitalRead(buttonPin);  // Đọc trạng thái hiện tại của nút bấm
  // Nếu nút bấm được nhấn
  if (buttonState == LOW && lastButtonState == HIGH) {
    // Tăng mode lên 1 đơn vị
    mode++;

    // Nếu mode vượt quá giá trị 3, đặt lại về 1
    if (mode > 3) {
      mode = 1;
    }
  }
  // Lưu trạng thái hiện tại của nút bấm
  lastButtonState = buttonState;
  if (mode == 1) {
    id = readnumber();
    if (id == 0) {
      return;
    }
    getFingerprintEnroll(id,s);
    delay(2000);
  } else if (mode == 2) {
    getFingerprintID(finger_id);
    Serial.print(finger_id);
    delay(2000);
  } else if (mode == 3) {
    id = readnumber();
    if (id == 0) {
      return;
    }
    deleteFingerprint(id);
    delay(2000);
  }
  delay(50);
}
