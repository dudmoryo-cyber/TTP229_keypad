/*
   TTP229 8-Key Mode Example
   For modules with TP2 jumper OPEN
*/

#include <TTP229.h>

// 8-key mode
TTP229 keypad(false);  // false = 8-key mode

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  keypad.begin(true);
  
  Serial.println("\n8-Key Mode Example");
  Serial.println("Keys 1-8 are supported");
}

void loop() {
  uint8_t key = keypad.read();
  
  if (keypad.wasPressed()) {
    Serial.print("8-Key Pressed: ");
    Serial.println(key);
    
    // In 8-key mode, row 0 = keys 1-4, row 1 = keys 5-8
    uint8_t row, col;
    keypad.getPosition(row, col);
    Serial.print("Row: ");
    Serial.print(row);
    Serial.print(", Col: ");
    Serial.println(col);
  }
  
  delay(10);
}