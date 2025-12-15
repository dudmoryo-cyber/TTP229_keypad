/*
   TTP229 Board-Specific Example
   Works on ALL Arduino boards
*/

#include <TTP229.h>

// Choose the constructor that works for your board:

// OPTION 1: Auto-detect everything (uses board defaults)
// TTP229 keypad;

// OPTION 2: Auto-detect board, specify mode
// TTP229 keypad(true);  // true = 16-key, false = 8-key

// OPTION 3: Specify pins and mode (for any board)
TTP229 keypad(2, 3, true);  // SCL=2, SDO=3, 16-key mode

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Initialize with debug mode
  keypad.begin(true);
  
  Serial.println("\nTTP229 Keypad Test");
  Serial.println("Press any key...");
  
  // You can adjust timing if needed:
  // For ESP32 (fast board):
  // keypad.setTiming(2, 2);  // 2us clock delay, 2us read delay
  
  // For Arduino Uno/Nano (slower):
  // keypad.setTiming(100, 5);  // 100us clock delay, 5ms read delay
}

void loop() {
  // Read the keypad
  uint8_t key = keypad.read();
  
  // Check if a key was just pressed
  if (keypad.wasPressed()) {
    Serial.println("----------------------");
    Serial.print("Key Pressed: ");
    Serial.println(key);
    
    // Get key number (0-15)
    uint8_t keyNum = keypad.getKeyNumber();
    if (keyNum != 255) {
      Serial.print("Key Number: ");
      Serial.println(keyNum);
    }
    
    // Get position
    uint8_t row, col;
    keypad.getPosition(row, col);
    if (row != 255 && col != 255) {
      Serial.print("Position: Row ");
      Serial.print(row);
      Serial.print(", Column ");
      Serial.println(col);
    }
    
    // Common key labels
    if (key == TTP229::KEY_13) {
      Serial.println("(*) pressed");
    } else if (key == TTP229::KEY_14) {
      Serial.println("(0) pressed");
    } else if (key == TTP229::KEY_15) {
      Serial.println("(#) pressed");
    }
  }
  
  // Check if key was released
  if (keypad.wasReleased()) {
    Serial.println("Key released");
    Serial.println();
  }
  
  // Small delay
  delay(10);
}