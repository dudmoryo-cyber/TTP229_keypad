# TTP229 Keypad Library

Advanced Arduino library for TTP229 capacitive touch keypad with RTOS support.

## Features

- **Multi-platform Support**: Works on ESP32, ESP8266, Arduino Uno/Nano/Mega, Raspberry Pi Pico, and more
- **RTOS Integration**: Full FreeRTOS support for ESP32 with event queues and mutex protection
- **Dual Mode**: Supports both 8-key and 16-key TTP229 modules
- **Advanced Features**:
  - Debouncing with configurable timing
  - Hold and long-press detection
  - Row/column position tracking
  - Automatic board detection
  - Thread-safe RTOS operations
  - ISR-safe reading methods

## Installation

1. Download this library as a ZIP file
2. In Arduino IDE: Sketch → Include Library → Add .ZIP Library
3. Select the downloaded ZIP file

## Quick Start

```cpp
#include <TTP229.h>

TTP229 keypad;  // Auto-detect everything

void setup() {
  Serial.begin(115200);
  keypad.begin(true);  // Enable debug
}

void loop() {
  uint8_t key = keypad.read();
  
  if (keypad.wasPressed()) {
    Serial.print("Key pressed: ");
    Serial.println(key);
  }
}