int enPin = 4;
int sensorPin = 5;
int sensorValue;
void setup() {
  pinMode(sensorPin, INPUT);
  pinMode(enPin, OUTPUT);
  // Kích hoạt cảm biến bằng cách đưa chân EN lên mức HIGH
  digitalWrite(enPin, HIGH);
  Serial.begin(9600);
}
// Hàm đọc giá trị từ cảm biến YS-29
// Tham số: sensorPin - chân digital kết nối với Sensor trên Arduino
//          sensorValue - biến tham chiếu để lưu giá trị đọc được từ cảm biến
void readSensor(int &sensorValue) {
  // Đọc giá trị từ cảm biến
  sensorValue = digitalRead(sensorPin);
}
void loop() {
  readSensor(sensorValue);
  if (sensorValue == LOW) {
    Serial.print("Detected\n");
    delay(1000);
  }
}