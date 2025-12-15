/*
   TTP229 Simple Piano
   Play musical notes with the keypad
*/

#include <TTP229.h>

TTP229 keypad(2, 3, true);

// Musical notes (frequencies in Hz)
const int notes[] = {
  262,  // C4
  294,  // D4
  330,  // E4
  349,  // F4
  392,  // G4
  440,  // A4
  494,  // B4
  523,  // C5
  587,  // D5
  659,  // E5
  698,  // F5
  784,  // G5
  880,  // A5
  988,  // B5
  1047, // C6
  1175  // D6
};

const char* noteNames[] = {
  "C4", "D4", "E4", "F4",
  "G4", "A4", "B4", "C5",
  "D5", "E5", "F5", "G5",
  "A5", "B5", "C6", "D6"
};

int currentOctave = 0;  // 0 = base octave
int volume = 100;       // 0-100
bool sustain = false;
unsigned long noteStartTime[16] = {0};
bool notePlaying[16] = {false};

#ifdef ESP32
  // ESP32 has built-in tone support
  #include "esp32-hal-ledc.h"
  #define TONE_PIN 25
#endif

void setup() {
  Serial.begin(115200);
  keypad.begin();
  
  #ifdef ESP32
    ledcSetup(0, 2000, 8);  // Channel 0, 2KHz, 8-bit resolution
    ledcAttachPin(TONE_PIN, 0);
  #else
    pinMode(8, OUTPUT);  // Use pin 8 for tone on Arduino
  #endif
  
  Serial.println("\n=== TTP229 Piano ===");
  Serial.println("Keys 1-16: Play notes");
  Serial.println("*: Octave down");
  Serial.println("#: Octave up");
  Serial.println("A: Volume down");
  Serial.println("B: Volume up");
  Serial.println("C: Toggle sustain");
  Serial.println("D: All notes off");
  Serial.println("====================\n");
  
  Serial.print("Octave: ");
  Serial.println(currentOctave);
  Serial.print("Volume: ");
  Serial.println(volume);
  Serial.print("Sustain: ");
  Serial.println(sustain ? "ON" : "OFF");
  Serial.println();
}

void loop() {
  uint8_t key = keypad.read();
  
  // Check for note presses
  for (int i = 1; i <= 16; i++) {
    if (keypad.isKeyPressed(i)) {
      if (!notePlaying[i-1]) {
        playNote(i);
        notePlaying[i-1] = true;
        noteStartTime[i-1] = millis();
      }
    } else {
      if (notePlaying[i-1] && !sustain) {
        stopNote(i);
        notePlaying[i-1] = false;
      }
    }
  }
  
  // Handle control keys
  if (keypad.wasPressed()) {
    switch(key) {
      case 13:  // * - Octave down
        currentOctave = max(-2, currentOctave - 1);
        Serial.print("Octave: ");
        Serial.println(currentOctave);
        break;
        
      case 15:  // # - Octave up
        currentOctave = min(2, currentOctave + 1);
        Serial.print("Octave: ");
        Serial.println(currentOctave);
        break;
        
      case 4:   // A - Volume down
        volume = max(0, volume - 10);
        Serial.print("Volume: ");
        Serial.println(volume);
        break;
        
      case 8:   // B - Volume up
        volume = min(100, volume + 10);
        Serial.print("Volume: ");
        Serial.println(volume);
        break;
        
      case 12:  // C - Toggle sustain
        sustain = !sustain;
        Serial.print("Sustain: ");
        Serial.println(sustain ? "ON" : "OFF");
        if (!sustain) {
          stopAllNotes();
        }
        break;
        
      case 16:  // D - All notes off
        stopAllNotes();
        Serial.println("All notes stopped");
        break;
    }
  }
  
  // Auto-release for sustain mode
  if (sustain) {
    for (int i = 0; i < 16; i++) {
      if (notePlaying[i] && millis() - noteStartTime[i] > 2000) {
        stopNote(i + 1);
        notePlaying[i] = false;
      }
    }
  }
  
  delay(10);
}

void playNote(uint8_t key) {
  if (key < 1 || key > 16) return;
  
  int noteIndex = key - 1;
  int frequency = notes[noteIndex];
  
  // Apply octave shift
  if (currentOctave > 0) {
    frequency <<= currentOctave;  // Multiply by 2^octave
  } else if (currentOctave < 0) {
    frequency >>= -currentOctave;  // Divide by 2^octave
  }
  
  Serial.print("Playing: ");
  Serial.print(noteNames[noteIndex]);
  Serial.print(" (");
  Serial.print(frequency);
  Serial.println(" Hz)");
  
  #ifdef ESP32
    ledcWriteTone(0, frequency);
    ledcWrite(0, volume * 2.55);  // Convert 0-100 to 0-255
  #else
    tone(8, frequency, 100);  // Non-ESP32 boards
  #endif
}

void stopNote(uint8_t key) {
  if (key < 1 || key > 16) return;
  
  Serial.print("Stopping: ");
  Serial.println(noteNames[key-1]);
  
  #ifdef ESP32
    ledcWriteTone(0, 0);
  #else
    noTone(8);
  #endif
}

void stopAllNotes() {
  for (int i = 1; i <= 16; i++) {
    notePlaying[i-1] = false;
  }
  
  #ifdef ESP32
    ledcWriteTone(0, 0);
  #else
    noTone(8);
  #endif
  
  Serial.println("All notes stopped");
}