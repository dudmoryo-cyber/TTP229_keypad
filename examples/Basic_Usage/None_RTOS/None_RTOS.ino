#include <TTP229.h>

TTP229 keypad;  // Auto-detect
// or: TTP229 keypad(2, 3, true);  // Custom pins

void setup() {
  keypad.begin();
}

void loop() {
  uint8_t key = keypad.read();
  if (keypad.wasPressed()) {
    Serial.println(key);
  }
}