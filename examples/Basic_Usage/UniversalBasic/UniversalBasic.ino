/*
   TTP229 Universal Basic Example
   Works on all boards with automatic detection
*/

#include <TTP229.h>

// Auto-detect board and use 16-key mode
TTP229 keypad;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Initialize with debug info
  keypad.begin(true);  // true enables debug output
  
  Serial.println("\nPress any key on the keypad...");
}

void loop() {
  // Read keypad
  uint8_t key = keypad.read();
  
  // Check for key press
  if (keypad.wasPressed()) {
    Serial.println("-------------------------");
    
    // Raw key value
    Serial.print("Raw Key: ");
    Serial.println(key);
    
    // Key number (0-15)
    uint8_t keyNum = keypad.getKeyNumber();
    if (keyNum != 255) {
      Serial.print("Key Number: ");
      Serial.println(keyNum);
    }
    
    // Row and column
    uint8_t row, col;
    keypad.getPosition(row, col);
    if (row != 255 && col != 255) {
      Serial.print("Row: ");
      Serial.print(row);
      Serial.print(", Column: ");
      Serial.println(col);
    }
  }
  
  // Check for key release
  if (keypad.wasReleased()) {
    Serial.println("Key released");
    Serial.println();
  }
  
  delay(10);
}