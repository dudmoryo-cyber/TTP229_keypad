# TTP229 Capacitive Touch Keypad Library - Complete Documentation

## 📋 Table of Contents
1. [Library Overview](#library-overview)
2. [Features](#features)
3. [Installation](#installation)
4. [Hardware Setup](#hardware-setup)
5. [API Reference](#api-reference)
6. [Examples Guide](#examples-guide)
7. [RTOS Support](#rtos-support)
8. [Performance Tuning](#performance-tuning)
9. [Troubleshooting](#troubleshooting)
10. [Changelog](#changelog)

---

## 📚 Library Overview

The **TTP229 Library** provides comprehensive support for TTP229-based capacitive touch keypads on various Arduino platforms. This library supports both 8-key and 16-key modules, automatic board detection, debouncing, RTOS integration, and multiple usage patterns.

### Supported Platforms
- ✅ ESP32 (Full RTOS support)
- ✅ ESP8266
- ✅ Arduino Uno/Nano
- ✅ Arduino Mega
- ✅ Arduino Zero
- ✅ Raspberry Pi Pico
- ✅ Other Arduino-compatible boards

---

## ✨ Features

### Core Features
- **Auto-detection** of board type and optimal timing
- **Debouncing** with configurable timing
- **Row/Column position** conversion
- **8-key and 16-key mode** support
- **Non-blocking** operation
- **Debug mode** with serial output

### Advanced Features
- **RTOS Support** for ESP32 (FreeRTOS)
- **Event queue system** with press, release, hold, and long-press events
- **Thread-safe** operation with mutex protection
- **ISR-safe** reading methods
- **Performance statistics** tracking
- **Multiple keypad instances** support

---

## 🔧 Installation

### Method 1: Arduino Library Manager
1. Open Arduino IDE
2. Go to **Tools → Manage Libraries**
3. Search for "TTP229"
4. Click Install

### Method 2: Manual Installation
1. Download the library ZIP
2. Extract to `Arduino/libraries/TTP229/`
3. Restart Arduino IDE

---

## 🔌 Hardware Setup

### Pin Connections
| TTP229 Pin | Arduino Pin | Description |
|------------|-------------|-------------|
| VCC        | 3.3V/5V     | Power (3.3V recommended) |
| GND        | GND         | Ground |
| SCL        | Digital Pin | Clock signal |
| SDO        | Digital Pin | Data output |

### Module Types
- **16-key mode**: TP2 jumper CLOSED (factory default)
- **8-key mode**: TP2 jumper OPEN

### Default Pin Mappings by Board
| Board | Default SCL | Default SDO |
|-------|-------------|-------------|
| ESP32 | GPIO18 | GPIO19 |
| ESP8266 | D5 (GPIO14) | D6 (GPIO12) |
| Arduino Uno | 2 | 3 |
| Arduino Mega | 22 | 23 |

---

## 📖 API Reference

### Constructors

```cpp
// Auto-detect board, 16-key mode
TTP229();

// Auto-detect board with specific mode
TTP229(bool is16KeyMode);

// Custom pins with mode
TTP229(uint8_t sclPin, uint8_t sdoPin, bool is16KeyMode = true);

// RTOS constructor (ESP32 only)
TTP229(uint8_t sclPin, uint8_t sdoPin, bool is16KeyMode, 
       uint8_t taskPriority, uint32_t stackDepth);
```

### Initialization Methods

```cpp
// Basic initialization
bool begin();
bool begin(bool debugMode);  // Enable debug output

// RTOS initialization (ESP32 only)
bool beginRTOS(bool createTask = true);
void endRTOS();
```

### Key Reading Methods

```cpp
// Read current key (with debouncing)
uint8_t read();

// Get last valid key
uint8_t getKey();

// Get key number (0-15 or 0-7)
uint8_t getKeyNumber();

// Get row/column position (0-based)
void getPosition(uint8_t &row, uint8_t &col);
```

### State Checking Methods

```cpp
// Check key states
bool isPressed();                    // Any key pressed?
bool isKeyPressed(uint8_t keyNum);   // Specific key pressed?
bool wasPressed();                   // Key just pressed?
bool wasReleased();                  // Key just released?
```

### Configuration Methods

```cpp
// Configuration
bool setMode(bool is16KeyMode);
bool setDebounce(uint16_t ms);       // 1-500ms
bool setScanInterval(uint16_t ms);   // 1-1000ms
bool setTiming(uint16_t clkDelay, uint16_t readDelay);  // microseconds

// Hold detection (RTOS only)
bool setHoldThreshold(uint16_t holdMs, uint16_t longPressMs = 2000);
```

### Information Methods

```cpp
// Get information
const char* getBoardName();
uint8_t getSCLPin();
uint8_t getSDOPin();
bool is16KeyMode();
bool isInitialized();
```

### RTOS-Specific Methods (ESP32)

```cpp
// Event structure
struct KeyEvent {
    uint8_t key;           // Key number
    uint8_t eventType;     // EVENT_PRESS, EVENT_RELEASE, EVENT_HOLD, EVENT_LONG_PRESS
    uint32_t timestamp;    // Event time
    uint8_t row;           // Row (0-based)
    uint8_t col;           // Column (0-based)
};

// RTOS methods
uint8_t readFromISR();
uint8_t readWithTimeout(uint32_t timeoutMs);
bool getKeyEvents(KeyEvent &event);
bool isPressedFromISR();
bool wasPressedFromISR();

// RTOS configuration
void setTaskPriority(uint8_t priority);
void enableEventQueue(bool enable = true);

// Statistics
RTOSStats getRTOSStats();
void resetRTOSStats();
uint32_t getQueueCount();
```

---

## 🎮 Examples Guide

### 1. **UniversalBasic.ino** - Basic Usage
```cpp
#include <TTP229.h>
TTP229 keypad;  // Auto-detect

void setup() {
    keypad.begin(true);  // Enable debug
}

void loop() {
    uint8_t key = keypad.read();
    if (keypad.wasPressed()) {
        Serial.print("Key pressed: ");
        Serial.println(key);
    }
}
```

### 2. **PasswordLock.ino** - Password Entry System
Creates a 4-digit password lock with timeout and visual feedback.

**Key Features:**
- 4-digit password protection
- 10-second timeout
- Visual lock/unlock status
- Serial debug output

### 3. **Calculator.ino** - 4-Function Calculator
Implements a full calculator with:
- Basic arithmetic (+, -, ×, ÷)
- Decimal support
- Clear and backspace functions
- Result formatting

### 4. **GameController.ino** - Simple Grid Game
Creates a 5×5 grid game where you move a player to catch targets.

**Controls:**
- 2,4,6,8: Movement (like numpad)
- #: Shoot/Action
- *: Exit
- 60-second timer

### 5. **UniversalRemote.ino** - Multi-Device Remote
Control multiple devices with one keypad:

**Supported Modes:**
- TV Control (channels, volume)
- Lights Control (brightness, scenes)
- Audio System (source, playback)
- Air Conditioner (temperature, mode)
- Security System (arm/disarm)

### 6. **Piano.ino** - Musical Keyboard
Play musical notes with the keypad:

**Features:**
- 16-note range (C4-D6)
- Octave shifting
- Volume control
- Sustain mode
- ESP32 tone support

### 7. **MediaController.ino** - Media & Menu Control
Menu navigation system for media players:

**Functions:**
- Play/Pause control
- Volume adjustment
- Menu navigation
- Direct function keys

---

## 🔄 RTOS Support

### RTOS Examples

#### 1. **RTOS_Basic.ino** - Simple RTOS Integration
```cpp
#include <TTP229.h>

// RTOS constructor: pins, mode, priority, stack
TTP229 keypad(18, 19, true, 1, 4096);

void setup() {
    keypad.begin();
    keypad.beginRTOS();  // Start RTOS task
    keypad.enableEventQueue(true);
}

void loop() {
    TTP229::KeyEvent event;
    if (keypad.getKeyEvents(event)) {
        // Handle different event types
        switch(event.eventType) {
            case TTP229::EVENT_PRESS:
                Serial.print("Press: Key ");
                Serial.println(event.key);
                break;
            case TTP229::EVENT_HOLD:
                Serial.println("Hold detected!");
                break;
            case TTP229::EVENT_LONG_PRESS:
                Serial.println("Long press detected!");
                break;
        }
    }
    delay(10);
}
```

#### 2. **RTOS_MultiTask.ino** - Multi-Tasking Example
Demonstrates multiple tasks using the same keypad:
- Sound task (plays tones)
- LED task (visual feedback)
- Serial task (command processing)

#### 3. **RTOS_Performance.ino** - Performance Testing
Compares RTOS vs non-RTOS performance with metrics:
- Reads per second
- Queue usage
- CPU utilization
- Event processing rate

#### 4. **RTOS_QueueTest.ino** - Event Queue Testing
Tests the RTOS event queue system with hold/long-press detection.

### RTOS Event Types
```cpp
EVENT_PRESS      // Key pressed
EVENT_RELEASE    // Key released
EVENT_HOLD       // Key held for 1 second (configurable)
EVENT_LONG_PRESS // Key held for 2 seconds (configurable)
```

### RTOS Configuration
```cpp
// Set hold thresholds (optional)
keypad.setHoldThreshold(1000, 2000);  // Hold at 1s, Long press at 2s

// Configure RTOS task
keypad.setTaskPriority(2);    // Higher priority = more CPU time
keypad.setStackDepth(4096);   // Task stack size in bytes
keypad.setQueueSize(20);      // Event queue size
```

---

## ⚡ Performance Tuning

### Timing Configuration
Different boards require different timing:

```cpp
// For ESP32/ESP8266 (fast boards)
keypad.setTiming(2, 2);       // 2µs clock, 2µs read delay

// For Arduino Uno/Nano (slower)
keypad.setTiming(100, 5);     // 100µs clock, 5ms read delay

// Debounce timing
keypad.setDebounce(20);       // 20ms debounce (ESP32)
keypad.setDebounce(50);       // 50ms debounce (Arduino)

// Scan interval
keypad.setScanInterval(10);   // 10ms between reads
```

### RTOS Performance Tips
1. **Task Priority**: Keypad task should have medium priority (1-3)
2. **Stack Size**: Minimum 2048 bytes for ESP32
3. **Queue Size**: 10-20 events for most applications
4. **Scan Interval**: 10-50ms for balance of responsiveness and CPU usage

### Memory Usage
- **Non-RTOS**: ~1.5KB RAM
- **RTOS (ESP32)**: ~4-6KB RAM (including task stacks)
- **Flash**: ~8-12KB

---

## 🔍 Troubleshooting

### Common Issues

#### 1. **No Key Detection**
- Check power (3.3V recommended)
- Verify SCL/SDO connections
- Ensure correct pin mode (INPUT_PULLUP for SDO on ESP32)
- Check TP2 jumper position (8 vs 16 key mode)

#### 2. **Erratic Key Presses**
- Increase debounce time: `keypad.setDebounce(50)`
- Add delay between reads: `keypad.setScanInterval(50)`
- Check for power supply noise
- Add 0.1µF capacitor between VCC and GND

#### 3. **RTOS Issues (ESP32)**
- Ensure FreeRTOS is properly installed
- Check task priority doesn't cause starvation
- Increase stack size if experiencing crashes
- Verify mutex acquisition timeouts

#### 4. **Serial Debug Output**
```cpp
keypad.begin(true);  // Enable debug
keypad.printDebugInfo();  // Print configuration
keypad.printRawReadings();  // Show raw key presses
```

### Error Codes
- `KEY_NONE` (0): No key pressed
- `KEY_INVALID` (255): Invalid key or error
- `POSITION_INVALID` (255): Invalid row/column

### Diagnostic Commands
```cpp
// Get all information
Serial.print("Board: ");
Serial.println(keypad.getBoardName());
Serial.print("Mode: ");
Serial.println(keypad.is16KeyMode() ? "16-key" : "8-key");
Serial.print("Pins: SCL=");
Serial.print(keypad.getSCLPin());
Serial.print(", SDO=");
Serial.println(keypad.getSDOPin());
```

---

## 📊 Changelog

### Version 1.0.0
- Initial release with full feature set
- Auto-detection for all major boards
- RTOS support for ESP32
- Comprehensive examples
- Thread-safe operation
- Hold/long-press detection

### Version Highlights
- **Multi-platform support**: Works on 10+ Arduino boards
- **RTOS integration**: Full FreeRTOS support on ESP32
- **Event system**: Press, release, hold, long-press events
- **Performance**: Optimized timing for each board type
- **Examples**: 12+ practical examples for different use cases

---

## 📝 License

MIT License - See LICENSE file for details.

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Commit your changes
4. Push to the branch
5. Create a Pull Request

## 📧 Support

For issues, questions, or feature requests:
1. Check the Troubleshooting section
2. Review the examples
3. Create a GitHub issue
4. Provide your board type and code example

---

## 🎯 Quick Start Cheat Sheet

```cpp
// Minimal code to get started
#include <TTP229.h>

TTP229 keypad;  // Auto-detect everything

void setup() {
    Serial.begin(115200);
    keypad.begin();
}

void loop() {
    if (keypad.wasPressed()) {
        Serial.print("Key: ");
        Serial.println(keypad.getKey());
    }
    delay(10);
}
```

```cpp
// RTOS minimal example (ESP32)
#include <TTP229.h>

TTP229 keypad(18, 19, true, 1, 2048);

void setup() {
    keypad.begin();
    keypad.beginRTOS();
}

void loop() {
    TTP229::KeyEvent event;
    if (keypad.getKeyEvents(event)) {
        Serial.print("Event: Key ");
        Serial.println(event.key);
    }
    delay(10);
}
```

---

**Happy Coding!** 🚀