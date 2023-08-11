#include <Wire.h>
#include <Keypad_I2C.h>
#include <Keypad.h>
#define I2CADDR 0x38
String inputString;
long inputInt;
const byte ROWS = 4;  //four rows
const byte COLS = 4;  //three columns
char keys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

// Digitran keypad, bit numbers of PCF8574 i/o port
byte rowPins[ROWS] = { 7, 6, 5, 4 };  //connect to the row pinouts of the keypad
byte colPins[COLS] = { 3, 2, 1, 0 };  //connect to the column pinouts of the keypad

Keypad_I2C kpd(makeKeymap(keys), rowPins, colPins, ROWS, COLS, I2CADDR, PCF8574);

void setup() {
  Wire.begin();
  kpd.begin(makeKeymap(keys));
  Serial.begin(9600);
  Serial.println("start");
}

void loop() {
  char key = kpd.getKey();

  if (key) {
    Serial.println(key);
    if (key >= '0' && key <= '9') {  // only act on numeric keys
      inputString += key;            // append new character to input string
    } else if (key == '#') {
      if (inputString.length() > 0) {
        inputInt = inputString.toInt();  // YOU GOT AN INTEGER NUMBER
        Serial.println(inputInt);
        inputString = "";                // clear input
        // DO YOUR WORK HERE
      }
    } else if (key == '*') {
      inputString = "";  // clear input
    }
  }
  
}