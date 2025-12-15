/*
   TTP229 Volume Control / Menu Navigation
   Use keypad as a media controller or menu navigator
*/

#include <TTP229.h>

TTP229 keypad(2, 3, true);

// Menu structure
const char* menuItems[] = {
  "Play/Pause",
  "Next Track",
  "Previous Track",
  "Volume Up",
  "Volume Down",
  "Mute",
  "Settings",
  "Power"
};
int currentMenuItem = 0;
int volumeLevel = 50;  // 0-100
bool isMuted = false;
bool isPlaying = false;

void setup() {
  Serial.begin(115200);
  keypad.begin();
  
  Serial.println("\n=== Media Controller ===");
  Serial.println("1-9: Menu Items");
  Serial.println("*: Back/Exit");
  Serial.println("0: Toggle Play/Pause");
  Serial.println("#: Confirm/Select");
  Serial.println("A-D: Direct Functions");
  Serial.println("=======================\n");
  
  displayStatus();
}

void loop() {
  uint8_t key = keypad.read();
  
  if (keypad.wasPressed()) {
    switch(key) {
      case 1:  // Key 1 - Play/Pause
        isPlaying = !isPlaying;
        Serial.print("Play/Pause: ");
        Serial.println(isPlaying ? "Playing" : "Paused");
        break;
        
      case 2:  // Key 2 - Next
        Serial.println(">> Next Track");
        break;
        
      case 3:  // Key 3 - Previous
        Serial.println("<< Previous Track");
        break;
        
      case 4:  // Key 4 - A (Menu Up)
        currentMenuItem = (currentMenuItem - 1 + 8) % 8;
        displayMenu();
        break;
        
      case 5:  // Key 5 - Volume Up
        if (!isMuted && volumeLevel < 100) {
          volumeLevel += 5;
          if (volumeLevel > 100) volumeLevel = 100;
          Serial.print("Volume: ");
          Serial.println(volumeLevel);
        }
        break;
        
      case 6:  // Key 6 - Volume Down
        if (!isMuted && volumeLevel > 0) {
          volumeLevel -= 5;
          if (volumeLevel < 0) volumeLevel = 0;
          Serial.print("Volume: ");
          Serial.println(volumeLevel);
        }
        break;
        
      case 7:  // Key 7 - C (Menu Down)
        currentMenuItem = (currentMenuItem + 1) % 8;
        displayMenu();
        break;
        
      case 8:  // Key 8 - Mute Toggle
        isMuted = !isMuted;
        Serial.print("Mute: ");
        Serial.println(isMuted ? "ON" : "OFF");
        break;
        
      case 9:  // Key 9 - Enter Settings
        Serial.println("Entering Settings...");
        break;
        
      case 10: // Key A
        Serial.println("Function A");
        break;
        
      case 11: // Key B
        Serial.println("Function B");
        break;
        
      case 12: // Key C
        Serial.println("Function C");
        break;
        
      case 13: // Key * - Back
        Serial.println("Back/Exit");
        break;
        
      case 14: // Key 0 - Special
        Serial.println("Special Function");
        break;
        
      case 15: // Key # - Select/Confirm
        Serial.print("Selected: ");
        Serial.println(menuItems[currentMenuItem]);
        break;
        
      case 16: // Key D
        Serial.println("Function D");
        break;
    }
    
    // Update status display
    displayStatus();
  }
  
  delay(10);
}

void displayStatus() {
  Serial.println("\n--- Current Status ---");
  Serial.print("State: ");
  Serial.println(isPlaying ? "Playing" : "Paused");
  Serial.print("Volume: ");
  Serial.print(volumeLevel);
  Serial.print("% ");
  Serial.println(isMuted ? "[MUTED]" : "");
  Serial.print("Selected Menu: ");
  Serial.println(menuItems[currentMenuItem]);
  Serial.println("----------------------");
}

void displayMenu() {
  Serial.println("\n=== Menu ===");
  for (int i = 0; i < 8; i++) {
    if (i == currentMenuItem) {
      Serial.print("> ");
    } else {
      Serial.print("  ");
    }
    Serial.print(i + 1);
    Serial.print(". ");
    Serial.println(menuItems[i]);
  }
  Serial.println("============");
}