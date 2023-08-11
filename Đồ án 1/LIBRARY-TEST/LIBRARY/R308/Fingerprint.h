#ifndef FINGERPRINT_H
#define FINGERPRINT_H
#include <SoftwareSerial.h>//Thư viện hỗ trợ thiết lập chân giao tiếp Serial, Link GitHub: https://github.com/PaulStoffregen/SoftwareSerial (PaulStoffregen )
#include <Adafruit_Fingerprint.h>//Thư viện hỗ trợ giao tiếp cảm biến vân tay: Link GitHub: https://github.com/adafruit/Adafruit-Fingerprint-Sensor-Library (Adafruit)
uint8_t getFingerprintEnroll(uint8_t &id, byte &s);
uint8_t getFingerprintID(uint8_t &finger_id);
uint8_t deleteFingerprint(uint8_t &id);
void fingerprintSetup();
#endif