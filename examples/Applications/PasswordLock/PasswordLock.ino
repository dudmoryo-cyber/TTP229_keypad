/*
   TTP229 Password/Code Lock Example
   Enter a 4-digit code to unlock
*/

#include <TTP229.h>

TTP229 keypad(2, 3, true);  // Adjust pins for your board

// Keypad mapping for typical 4x4 keypad
const char keyMap[16] = {
  '1', '2', '3', 'A',
  '4', '5', '6', 'B',
  '7', '8', '9', 'C',
  '*', '0', '#', 'D'
};

// Password (change this to your desired code)
const char password[5] = "1234";  // 4-digit password + null terminator
char enteredCode[5] = "";        // Store entered code
int codeIndex = 0;               // Current position in code
bool unlocked = false;           // Lock state
unsigned long lastKeyTime = 0;   // For timeout
const unsigned long timeout = 10000;  // 10 second timeout

void setup() {
  Serial.begin(115200);
  keypad.begin(true);  // Debug mode
  
  Serial.println("\n=== TTP229 Password Lock ===");
  Serial.println("Enter 4-digit code (e.g., 1234)");
  Serial.println("* = Clear, # = Enter, A-D = Special functions");
  Serial.println("Timeout after 10 seconds of inactivity");
  Serial.println("=============================\n");
  
  showLockStatus();
}

void loop() {
  uint8_t key = keypad.read();
  
  // Check for key press
  if (keypad.wasPressed()) {
    lastKeyTime = millis();
    
    // Handle special keys
    if (key == 13) {  // '*' - Clear
      clearCode();
      Serial.println("Code cleared!");
    } 
    else if (key == 15) {  // '#' - Enter/Submit
      checkCode();
    }
    else if (key >= 1 && key <= 12) {  // Number keys 0-9, A, B, C
      addToCode(key);
    }
    else if (key == 14) {  // '0'
      addToCode(key);
    }
    else if (key == 16) {  // 'D' - Reset lock
      unlocked = false;
      clearCode();
      Serial.println("Lock reset to LOCKED");
      showLockStatus();
    }
  }
  
  // Timeout check
  if (!unlocked && millis() - lastKeyTime > timeout && codeIndex > 0) {
    Serial.println("\nTimeout! Code cleared.");
    clearCode();
  }
  
  delay(10);
}

void addToCode(uint8_t key) {
  if (codeIndex < 4) {
    char keyChar = keyMap[key - 1];
    enteredCode[codeIndex] = keyChar;
    codeIndex++;
    enteredCode[codeIndex] = '\0';  // Keep null-terminated
    
    Serial.print("Code: ");
    for (int i = 0; i < codeIndex; i++) {
      Serial.print('*');
    }
    Serial.println();
    
    // Auto-submit when 4 digits entered
    if (codeIndex == 4) {
      delay(300);  // Small delay
      checkCode();
    }
  } else {
    Serial.println("Code full! Press * to clear or # to submit");
  }
}

void checkCode() {
  if (codeIndex == 4) {
    bool correct = true;
    for (int i = 0; i < 4; i++) {
      if (enteredCode[i] != password[i]) {
        correct = false;
        break;
      }
    }
    
    if (correct) {
      unlocked = true;
      Serial.println("\n✓ CORRECT CODE! UNLOCKED!");
      Serial.println("Press D to re-lock");
    } else {
      Serial.println("\n✗ WRONG CODE! Try again");
    }
    
    showLockStatus();
    clearCode();
  } else {
    Serial.println("Code must be 4 digits!");
  }
}

void clearCode() {
  codeIndex = 0;
  enteredCode[0] = '\0';
}

void showLockStatus() {
  Serial.print("\nStatus: ");
  if (unlocked) {
    Serial.println("UNLOCKED");
    Serial.println("████████████████");
    Serial.println("██          ██");
    Serial.println("██  OPEN    ██");
    Serial.println("██          ██");
    Serial.println("████████████████");
  } else {
    Serial.println("LOCKED");
    Serial.println("████████████████");
    Serial.println("██ ########## ██");
    Serial.println("██ # LOCKED # ██");
    Serial.println("██ ########## ██");
    Serial.println("████████████████");
  }
  Serial.println();
}