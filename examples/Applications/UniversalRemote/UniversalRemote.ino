/*
   TTP229 Multi-Function Remote Control
   Control multiple devices with one keypad
*/

#include <TTP229.h>

TTP229 keypad(2, 3, true);

// Modes
enum Mode {
  MODE_TV,
  MODE_LIGHTS,
  MODE_AUDIO,
  MODE_AC,
  MODE_SECURITY
};
Mode currentMode = MODE_TV;

// Device states
bool tvPower = false;
int tvVolume = 50;
int tvChannel = 1;

bool lightsPower = false;
int lightsBrightness = 100;

bool audioPower = false;
int audioVolume = 50;
String audioSource = "Radio";

bool acPower = false;
int acTemp = 22;  // Celsius
String acMode = "Cool";

bool securityArmed = false;

void setup() {
  Serial.begin(115200);
  keypad.begin();
  
  Serial.println("\n=== Universal Remote Control ===");
  Serial.println("A: Mode Select");
  Serial.println("B: Device Power");
  Serial.println("C: Settings");
  Serial.println("D: Info/Status");
  Serial.println("1-9: Mode-specific functions");
  Serial.println("*: Back/Exit");
  Serial.println("#: Confirm");
  Serial.println("===============================\n");
  
  displayMode();
}

void loop() {
  uint8_t key = keypad.read();
  
  if (keypad.wasPressed()) {
    // Mode selection
    if (key >= 1 && key <= 5) {
      currentMode = (Mode)(key - 1);
      displayMode();
    }
    // Common functions
    else if (key == 4) {  // A - Mode select
      showModeMenu();
    }
    else if (key == 8) {  // B - Power toggle
      togglePower();
    }
    else if (key == 12) { // C - Settings
      showSettings();
    }
    else if (key == 16) { // D - Info
      showStatus();
    }
    // Mode-specific functions
    else {
      handleModeSpecific(key);
    }
  }
  
  delay(10);
}

void displayMode() {
  Serial.print("\n=== Current Mode: ");
  switch(currentMode) {
    case MODE_TV:
      Serial.println("TV ===");
      Serial.println("1: Ch+, 2: Ch-, 3: Vol+, 4: Vol-");
      Serial.println("5: Mute, 6: Input, 7: Guide, 8: Menu");
      break;
    case MODE_LIGHTS:
      Serial.println("LIGHTS ===");
      Serial.println("1: Brightness+, 2: Brightness-");
      Serial.println("3: Color Temp, 4: Scene 1");
      Serial.println("5: Scene 2, 6: Scene 3, 7: Schedule");
      break;
    case MODE_AUDIO:
      Serial.println("AUDIO ===");
      Serial.println("1: Source, 2: Play/Pause");
      Serial.println("3: Next, 4: Previous, 5: Vol+");
      Serial.println("6: Vol-, 7: EQ, 8: Presets");
      break;
    case MODE_AC:
      Serial.println("AIR CONDITIONER ===");
      Serial.println("1: Temp+, 2: Temp-, 3: Fan Speed");
      Serial.println("4: Mode, 5: Swing, 6: Timer");
      Serial.println("7: Sleep, 8: Turbo");
      break;
    case MODE_SECURITY:
      Serial.println("SECURITY ===");
      Serial.println("1: Arm/Disarm, 2: Status");
      Serial.println("3: Camera, 4: Lights");
      Serial.println("5: Siren Test, 6: History");
      break;
  }
  Serial.println("====================\n");
}

void handleModeSpecific(uint8_t key) {
  switch(currentMode) {
    case MODE_TV:
      handleTV(key);
      break;
    case MODE_LIGHTS:
      handleLights(key);
      break;
    case MODE_AUDIO:
      handleAudio(key);
      break;
    case MODE_AC:
      handleAC(key);
      break;
    case MODE_SECURITY:
      handleSecurity(key);
      break;
  }
}

void handleTV(uint8_t key) {
  switch(key) {
    case 1:  // Ch+
      tvChannel++;
      Serial.print("Channel: ");
      Serial.println(tvChannel);
      break;
    case 2:  // Ch-
      tvChannel = max(1, tvChannel - 1);
      Serial.print("Channel: ");
      Serial.println(tvChannel);
      break;
    case 3:  // Vol+
      tvVolume = min(100, tvVolume + 5);
      Serial.print("Volume: ");
      Serial.println(tvVolume);
      break;
    case 4:  // Vol-
      tvVolume = max(0, tvVolume - 5);
      Serial.print("Volume: ");
      Serial.println(tvVolume);
      break;
    case 5:  // Mute
      Serial.println("TV Muted");
      break;
    case 6:  // Input
      Serial.println("Input changed");
      break;
  }
}

void handleLights(uint8_t key) {
  switch(key) {
    case 1:  // Brightness+
      lightsBrightness = min(100, lightsBrightness + 10);
      Serial.print("Brightness: ");
      Serial.println(lightsBrightness);
      break;
    case 2:  // Brightness-
      lightsBrightness = max(0, lightsBrightness - 10);
      Serial.print("Brightness: ");
      Serial.println(lightsBrightness);
      break;
    case 3:  // Color Temp
      Serial.println("Color temperature changed");
      break;
    case 4:  // Scene 1
      Serial.println("Scene 1 activated");
      break;
  }
}

void handleAudio(uint8_t key) {
  switch(key) {
    case 1:  // Source
      audioSource = (audioSource == "Radio") ? "Bluetooth" : "Radio";
      Serial.print("Source: ");
      Serial.println(audioSource);
      break;
    case 2:  // Play/Pause
      Serial.println(audioPower ? "Playing" : "Paused");
      break;
    case 3:  // Next
      Serial.println("Next track");
      break;
    case 4:  // Previous
      Serial.println("Previous track");
      break;
  }
}

void handleAC(uint8_t key) {
  switch(key) {
    case 1:  // Temp+
      acTemp++;
      Serial.print("Temperature: ");
      Serial.print(acTemp);
      Serial.println("°C");
      break;
    case 2:  // Temp-
      acTemp--;
      Serial.print("Temperature: ");
      Serial.print(acTemp);
      Serial.println("°C");
      break;
    case 3:  // Fan Speed
      Serial.println("Fan speed changed");
      break;
    case 4:  // Mode
      acMode = (acMode == "Cool") ? "Heat" : "Cool";
      Serial.print("Mode: ");
      Serial.println(acMode);
      break;
  }
}

void handleSecurity(uint8_t key) {
  switch(key) {
    case 1:  // Arm/Disarm
      securityArmed = !securityArmed;
      Serial.print("Security ");
      Serial.println(securityArmed ? "ARMED" : "DISARMED");
      break;
    case 2:  // Status
      Serial.println("All zones secure");
      break;
    case 3:  // Camera
      Serial.println("Camera view");
      break;
  }
}

void togglePower() {
  switch(currentMode) {
    case MODE_TV:
      tvPower = !tvPower;
      Serial.print("TV Power: ");
      Serial.println(tvPower ? "ON" : "OFF");
      break;
    case MODE_LIGHTS:
      lightsPower = !lightsPower;
      Serial.print("Lights: ");
      Serial.println(lightsPower ? "ON" : "OFF");
      break;
    case MODE_AUDIO:
      audioPower = !audioPower;
      Serial.print("Audio: ");
      Serial.println(audioPower ? "ON" : "OFF");
      break;
    case MODE_AC:
      acPower = !acPower;
      Serial.print("AC: ");
      Serial.println(acPower ? "ON" : "OFF");
      break;
    case MODE_SECURITY:
      // Security doesn't have simple power toggle
      Serial.println("Use Arm/Disarm for security");
      break;
  }
}

void showModeMenu() {
  Serial.println("\n=== SELECT MODE ===");
  Serial.println("1: TV Control");
  Serial.println("2: Lights Control");
  Serial.println("3: Audio System");
  Serial.println("4: Air Conditioner");
  Serial.println("5: Security System");
  Serial.println("*: Back");
  Serial.println("===================");
}

void showSettings() {
  Serial.println("\n=== SETTINGS ===");
  Serial.println("1: Remote Settings");
  Serial.println("2: Device Pairing");
  Serial.println("3: Timers/Schedules");
  Serial.println("4: Macros");
  Serial.println("5: System Info");
  Serial.println("*: Back");
  Serial.println("================");
}

void showStatus() {
  Serial.println("\n=== SYSTEM STATUS ===");
  Serial.print("TV: ");
  Serial.print(tvPower ? "ON" : "OFF");
  Serial.print(" Ch:");
  Serial.print(tvChannel);
  Serial.print(" Vol:");
  Serial.println(tvVolume);
  
  Serial.print("Lights: ");
  Serial.print(lightsPower ? "ON" : "OFF");
  Serial.print(" Bright:");
  Serial.print(lightsBrightness);
  Serial.println("%");
  
  Serial.print("Audio: ");
  Serial.print(audioPower ? "ON" : "OFF");
  Serial.print(" ");
  Serial.print(audioSource);
  Serial.print(" Vol:");
  Serial.println(audioVolume);
  
  Serial.print("AC: ");
  Serial.print(acPower ? "ON" : "OFF");
  Serial.print(" ");
  Serial.print(acTemp);
  Serial.print("°C ");
  Serial.println(acMode);
  
  Serial.print("Security: ");
  Serial.println(securityArmed ? "ARMED" : "DISARMED");
  Serial.println("=====================\n");
}