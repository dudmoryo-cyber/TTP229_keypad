#include "TTP229.h"

// ==============================================
// CONSTRUCTORS
// ==============================================

// Constructor 1: Default - auto-detect everything
TTP229::TTP229() {
    _sclPin = 0;
    _sdoPin = 0;
    _is16KeyMode = true;
    _debug = false;
    _initialized = false;
    
    detectBoard();
    setBoardDefaults();
    initializeState();
}

// Constructor 2: Auto-detect board with specified mode
TTP229::TTP229(bool is16KeyMode) : TTP229() {
    _is16KeyMode = is16KeyMode;
}

// Constructor 3: Custom pins with optional mode
TTP229::TTP229(uint8_t sclPin, uint8_t sdoPin, bool is16KeyMode) {
    _sclPin = sclPin;
    _sdoPin = sdoPin;
    _is16KeyMode = is16KeyMode;
    _debug = false;
    _initialized = false;
    
    detectBoard();
    setBoardDefaults();
    initializeState();
}

// Constructor 4: RTOS-specific constructor
#if TTP229_RTOS_SUPPORT
TTP229::TTP229(uint8_t sclPin, uint8_t sdoPin, bool is16KeyMode,
               uint8_t taskPriority, uint32_t stackDepth) {
    _sclPin = sclPin;
    _sdoPin = sdoPin;
    _is16KeyMode = is16KeyMode;
    _debug = false;
    _initialized = false;
    
    // Initialize board detection and defaults FIRST
    detectBoard();
    setBoardDefaults();
    initializeState();  // This must come BEFORE RTOS setup
    
    #if defined(ESP32)
    _taskHandle = NULL;
    _eventQueue = NULL;
    _mutex = NULL;
    _readSemaphore = NULL;
    _statsMutex = portMUX_INITIALIZER_UNLOCKED;
    #endif
    
    _rtosEnabled = false;
    _taskRunning = false;
    _taskPriority = taskPriority;
    _taskStackDepth = stackDepth;
    _queueSize = 10;
    _eventQueueEnabled = true;
    _holdThreshold = DEFAULT_HOLD_THRESHOLD_MS;	
    _lastHoldKey = 0;
    _holdStartTime = 0;
	_holdEventSent = false;        // NEW
    _longPressEventSent = false;   // NEW
    _lastKeyFromISR = 0;
    
    memset(&_stats, 0, sizeof(_stats));
    _lastStatsReset = millis();
}
#endif

// Destructor
TTP229::~TTP229() {
    #if TTP229_RTOS_SUPPORT
    // Signal task to stop if running
    if (_rtosEnabled && _taskRunning) {
        #if defined(ESP32)
        if (_taskHandle != NULL) {
            _taskRunning = false;
            // Give task time to exit gracefully
            vTaskDelay(pdMS_TO_TICKS(50));
            
            // Force delete if still running
            if (eTaskGetState(_taskHandle) != eDeleted) {
                vTaskDelete(_taskHandle);
            }
            _taskHandle = NULL;
        }
        #endif
    }
    endRTOS();
    #endif
}

// ==============================================
// TIMING HELPER (MILLIS OVERFLOW SAFE)
// ==============================================

bool TTP229::timeElapsed(uint32_t startTime, uint32_t interval) {
    uint32_t currentTime = millis();
    
    // Handle millis() overflow (wraps around every ~50 days)
    if (currentTime >= startTime) {
        return (currentTime - startTime) >= interval;
    } else {
        // Overflow has occurred
        return ((0xFFFFFFFF - startTime + currentTime) >= interval);
    }
}

// ==============================================
// INITIALIZATION
// ==============================================

void TTP229::initializeState() {
    _currentKey = 0;
    _lastKey = 0;
    _lastValidKey = 0;
    _lastDebounceTime = 0;
    _lastReadTime = 0;
    
    // Initialize debounce state
    _lastRawKey = 0;
    _stableKey = 0;
    _lastChangeTime = 0;
    
    #if TTP229_RTOS_SUPPORT
    _lastKeyFromISR = 0;
	_holdEventSent = false;        // NEW
    _longPressEventSent = false;   // NEW
    #endif
}

void TTP229::detectBoard() {
    // Board detection with automatic pin assignment
    #if defined(ESP32)
        _boardName = "ESP32";
        if (_sclPin == 0) _sclPin = 18;  // Default ESP32 pins
        if (_sdoPin == 0) _sdoPin = 19;
    #elif defined(ESP8266)
        _boardName = "ESP8266";
        if (_sclPin == 0) _sclPin = D5;  // GPIO14
        if (_sdoPin == 0) _sdoPin = D6;  // GPIO12
    #elif defined(ARDUINO_AVR_UNO)
        _boardName = "Arduino Uno";
        if (_sclPin == 0) _sclPin = 2;   // Avoid pins 0,1 (Serial)
        if (_sdoPin == 0) _sdoPin = 3;
    #elif defined(ARDUINO_AVR_NANO)
        _boardName = "Arduino Nano";
        if (_sclPin == 0) _sclPin = 2;
        if (_sdoPin == 0) _sdoPin = 3;
    #elif defined(ARDUINO_AVR_MEGA2560)
        _boardName = "Arduino Mega";
        if (_sclPin == 0) _sclPin = 22;  // Use higher pins
        if (_sdoPin == 0) _sdoPin = 23;
    #elif defined(ARDUINO_SAMD_ZERO)
        _boardName = "Arduino Zero";
        if (_sclPin == 0) _sclPin = 2;
        if (_sdoPin == 0) _sdoPin = 3;
    #elif defined(ARDUINO_RASPBERRY_PI_PICO)
        _boardName = "Raspberry Pi Pico";
        if (_sclPin == 0) _sclPin = 2;
        if (_sdoPin == 0) _sdoPin = 3;
    #else
        _boardName = "Unknown Arduino";
        if (_sclPin == 0) _sclPin = 2;
        if (_sdoPin == 0) _sdoPin = 3;
    #endif
}

void TTP229::setBoardDefaults() {
    // Set timing defaults based on board speed
    #if defined(ESP32) || defined(ESP8266)
        // Fast boards need shorter delays
        _clkDelay = 2;      // 2 microseconds
        _readDelay = 2;     // 2 microseconds
        _debounceDelay = 20;  // 20 milliseconds
        _scanInterval = 10;   // 10 milliseconds
    #elif defined(ARDUINO_SAMD_ZERO) || defined(ARDUINO_RASPBERRY_PI_PICO)
        // ARM-based boards
        _clkDelay = 10;     // 10 microseconds
        _readDelay = 10;    // 10 microseconds
        _debounceDelay = 30;  // 30 milliseconds
        _scanInterval = 20;   // 20 milliseconds
    #else
        // AVR boards (Uno, Nano, Mega) are slower
        _clkDelay = 100;    // 100 microseconds
        _readDelay = 5;     // 5 milliseconds
        _debounceDelay = 50;  // 50 milliseconds
        _scanInterval = 50;   // 50 milliseconds
    #endif
}

bool TTP229::isValidPin(uint8_t pin) {
    // Platform-specific pin validation
    #if defined(ESP32)
        // ESP32 valid GPIO pins (excluding input-only and strapping pins)
        return (pin < 40 && pin != 6 && pin != 7 && pin != 8 && 
                pin != 9 && pin != 10 && pin != 11);
    #elif defined(ESP8266)
        // ESP8266 valid GPIO pins
        return (pin <= 16);
    #elif defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO)
        // Uno/Nano digital pins (avoid 0,1 for Serial)
        return (pin >= 2 && pin <= 13);
    #elif defined(ARDUINO_AVR_MEGA2560)
        // Mega digital pins
        return (pin <= 53);
    #elif defined(ARDUINO_RASPBERRY_PI_PICO)
        // Pico GPIO pins
        return (pin <= 28);
    #else
        // Generic validation - assume reasonable range
        return (pin <= 50);
    #endif
}

bool TTP229::validateTiming(uint16_t clkDelay, uint16_t readDelay) {
    // Ensure timing values are reasonable
    // Clock delay should be at least 1µs, read delay at least 1µs
    // Maximum reasonable values to prevent overflow issues
    if (clkDelay < 1 || clkDelay > 10000) return false;
    if (readDelay < 1 || readDelay > 10000) return false;
    return true;
}

bool TTP229::begin() {
    return begin(false);
}

bool TTP229::begin(bool debugMode) {
    _debug = debugMode;
    
    // Validate pins
    if (!isValidPin(_sclPin) || !isValidPin(_sdoPin)) {
        if (_debug) {
            Serial.begin(115200);
            delay(100);
            Serial.println("ERROR: Invalid pin configuration");
        }
        return false;
    }
    
    // Configure pins
    pinMode(_sclPin, OUTPUT);
    
    // Set appropriate input mode based on board
    #if defined(ESP32) || defined(ESP8266) || defined(ARDUINO_RASPBERRY_PI_PICO)
        // These boards need pull-up resistors
        pinMode(_sdoPin, INPUT_PULLUP);
    #else
        // AVR boards typically work without pull-up
        pinMode(_sdoPin, INPUT);
    #endif
    
    // Initialize clock line
    digitalWrite(_sclPin, HIGH);
    delay(10);  // Let module stabilize
    
    _initialized = true;
    
    // Debug output if enabled
    if (_debug) {
        Serial.begin(115200);
        delay(100);  // Wait for serial
        printDebugInfo();
    }
    
    return true;
}

// ==============================================
// RTOS INITIALIZATION
// ==============================================

#if TTP229_RTOS_SUPPORT

bool TTP229::beginRTOS(bool createTask) {
    #if defined(ESP32)
    // Create mutex for thread safety
    _mutex = xSemaphoreCreateMutex();
    if (_mutex == NULL) {
        if (_debug) Serial.println("ERROR: Failed to create mutex");
        return false;
    }
    
    // Create binary semaphore for blocking reads
    _readSemaphore = xSemaphoreCreateBinary();
    if (_readSemaphore == NULL) {
        if (_debug) Serial.println("ERROR: Failed to create semaphore");
        vSemaphoreDelete(_mutex);
        _mutex = NULL;
        return false;
    }
    
    // Create event queue if enabled
    if (_eventQueueEnabled) {
        _eventQueue = xQueueCreate(_queueSize, sizeof(KeyEvent));
        if (_eventQueue == NULL) {
            if (_debug) Serial.println("ERROR: Failed to create event queue");
            vSemaphoreDelete(_mutex);
            vSemaphoreDelete(_readSemaphore);
            _mutex = NULL;
            _readSemaphore = NULL;
            return false;
        }
    }
    
    _rtosEnabled = true;
    _taskRunning = true;
    
    // Create RTOS task if requested
    if (createTask) {
        BaseType_t result = xTaskCreate(
            rtosTask,           // Task function
            "TTP229_Task",      // Task name (max 16 chars)
            _taskStackDepth,    // Stack size in words
            this,               // Parameter passed to task
            _taskPriority,      // Priority (0-24, higher = more priority)
            &_taskHandle        // Task handle
        );
        
        if (result != pdPASS) {
            if (_debug) Serial.println("ERROR: Failed to create RTOS task");
            endRTOS();
            return false;
        } else if (_debug) {
            Serial.println("RTOS task created successfully");
        }
    }
    
    if (_debug) {
        Serial.println("RTOS mode initialized");
        Serial.print("  Queue size: ");
        Serial.println(_queueSize);
        Serial.print("  Task priority: ");
        Serial.println(_taskPriority);
        Serial.print("  Stack depth: ");
        Serial.println(_taskStackDepth);
        Serial.print("  Event queue: ");
        Serial.println(_eventQueueEnabled ? "Enabled" : "Disabled");
    }
    
    return true;
    
    #else
    // For other RTOS platforms, mark as enabled but don't create tasks
    _rtosEnabled = true;
    _taskRunning = true;
    if (_debug) Serial.println("RTOS enabled (limited support on this platform)");
    return true;
    #endif
}

void TTP229::endRTOS() {
    #if defined(ESP32)
    // Signal task to stop
    _taskRunning = false;
    
    // Wait for task to exit if it's still running
    if (_taskHandle != NULL) {
        // Give task time to exit gracefully
        vTaskDelay(pdMS_TO_TICKS(50));
        
        // Force delete if still running
        if (eTaskGetState(_taskHandle) != eDeleted) {
            vTaskDelete(_taskHandle);
        }
        _taskHandle = NULL;
    }
    
    if (_eventQueue != NULL) {
        vQueueDelete(_eventQueue);
        _eventQueue = NULL;
    }
    
    if (_mutex != NULL) {
        vSemaphoreDelete(_mutex);
        _mutex = NULL;
    }
    
    if (_readSemaphore != NULL) {
        vSemaphoreDelete(_readSemaphore);
        _readSemaphore = NULL;
    }
    #endif
    
    _rtosEnabled = false;
    
    if (_debug) {
        Serial.println("RTOS resources cleaned up");
    }
}

void TTP229::stopRTOS() {
    if (_rtosEnabled && _taskRunning) {
        _taskRunning = false;
        
        // Wait a moment for task to exit
        #if defined(ESP32)
        vTaskDelay(pdMS_TO_TICKS(100));
        #endif
        
        endRTOS();
    }
}

#endif // TTP229_RTOS_SUPPORT

// ==============================================
// KEY READING METHODS
// ==============================================

uint8_t TTP229::read() {
    #if TTP229_RTOS_SUPPORT
    if (_rtosEnabled) {
        #if defined(ESP32)
        // Thread-safe access using mutex
        if (takeMutex(10)) {  // 10ms timeout
            uint8_t key = _lastValidKey;
            giveMutex();
            return key;
        }
        return KEY_NONE;
        #else
        return _lastValidKey;
        #endif
    }
    #endif
    
    // Non-RTOS reading logic
    unsigned long now = millis();
    
    // Only read at the specified interval
    if (timeElapsed(_lastReadTime, _scanInterval)) {
        _lastReadTime = now;
        
        // Save previous state for edge detection
        _lastKey = _lastValidKey;
        
        // Read the keypad
        _currentKey = readDebounced();
        
        // Update valid key if debounce period has passed
        if (timeElapsed(_lastDebounceTime, _debounceDelay)) {
            _lastValidKey = _currentKey;
        }
    }
    
    // Allow other tasks to run (important for cooperative multitasking)
    #if defined(ESP8266) || defined(ARDUINO_ARCH_AVR)
    delay(0);  // Calls yield() on these platforms
    #endif
    
    return _lastValidKey;
}

uint8_t TTP229::getKey() {
    return _lastValidKey;
}

uint8_t TTP229::getKeyNumber() {
    if (_lastValidKey == KEY_NONE) return KEY_INVALID;
    
    uint8_t row, col;
    getPosition(row, col);
    
    if (row == POSITION_INVALID || col == POSITION_INVALID) return KEY_INVALID;
    
    // Convert to 0-based key number
    return (row * 4) + col;  // 0-15 for 16-key, 0-7 for 8-key
}

void TTP229::getPosition(uint8_t &row, uint8_t &col) {
    getPositionInternal(_lastValidKey, &row, &col);
}

void TTP229::getPositionInternal(uint8_t key, uint8_t *row, uint8_t *col) {
    // Return POSITION_INVALID for invalid/no key
    if (key == KEY_NONE) {
        *row = POSITION_INVALID;
        *col = POSITION_INVALID;
        return;
    }
    
    uint8_t maxKeys = _is16KeyMode ? 16 : 8;
    
    // Check bounds
    if (key > maxKeys) {
        *row = POSITION_INVALID;
        *col = POSITION_INVALID;
        return;
    }
    
    // Convert key number to row/column (0-based)
    // Keys 1-4: Row 0, Columns 0-3
    // Keys 5-8: Row 1, Columns 0-3
    // Keys 9-12: Row 2, Columns 0-3
    // Keys 13-16: Row 3, Columns 0-3
    *row = (key - 1) / 4;
    *col = (key - 1) % 4;
}

// ==============================================
// STATE CHECKING METHODS
// ==============================================

bool TTP229::isPressed() {
    return (_lastValidKey != KEY_NONE);
}

bool TTP229::isKeyPressed(uint8_t keyNum) {
    return (_lastValidKey == keyNum);
}

bool TTP229::wasPressed() {
    // Key was just pressed if current is non-zero and last was zero
    return (_lastValidKey != KEY_NONE && _lastKey == KEY_NONE);
}

bool TTP229::wasReleased() {
    // Key was just released if current is zero and last was non-zero
    return (_lastValidKey == KEY_NONE && _lastKey != KEY_NONE);
}

// ==============================================
// CONFIGURATION METHODS
// ==============================================

bool TTP229::setMode(bool is16KeyMode) {
    _is16KeyMode = is16KeyMode;
    return true;
}

bool TTP229::setDebounce(uint16_t ms) {
    // Validate reasonable debounce range (1-500ms)
    if (ms < 1 || ms > 500) {
        if (_debug) Serial.println("ERROR: Invalid debounce value (1-500ms)");
        return false;
    }
    _debounceDelay = ms;
    return true;
}

bool TTP229::setScanInterval(uint16_t ms) {
    // Validate reasonable scan interval (1-1000ms)
    if (ms < 1 || ms > 1000) {
        if (_debug) Serial.println("ERROR: Invalid scan interval (1-1000ms)");
        return false;
    }
    _scanInterval = ms;
    return true;
}

bool TTP229::setTiming(uint16_t clkDelay, uint16_t readDelay) {
    if (!validateTiming(clkDelay, readDelay)) {
        if (_debug) Serial.println("ERROR: Invalid timing values");
        return false;
    }
    _clkDelay = clkDelay;
    _readDelay = readDelay;
    return true;
}

bool TTP229::setHoldThreshold(uint16_t holdMs, uint16_t longPressMs) {
    if (holdMs >= longPressMs) {
        if (_debug) Serial.println("ERROR: Hold threshold must be less than long press threshold");
        return false;
    }
    
    if (holdMs < 100 || longPressMs > 10000) {
        if (_debug) Serial.println("ERROR: Invalid threshold values");
        return false;
    }
    
    #if TTP229_RTOS_SUPPORT
    if (takeMutex(10)) {
        _holdThreshold = holdMs;
        giveMutex();
    }
    #else
    _holdThreshold = holdMs;
    #endif
    
    return true;
}

// ==============================================
// INFORMATION METHODS
// ==============================================

const char* TTP229::getBoardName() {
    return _boardName;
}

uint8_t TTP229::getSCLPin() {
    return _sclPin;
}

uint8_t TTP229::getSDOPin() {
    return _sdoPin;
}

bool TTP229::is16KeyMode() {
    return _is16KeyMode;
}

bool TTP229::isInitialized() {
    return _initialized;
}

// ==============================================
// DEBUG METHODS
// ==============================================

void TTP229::printDebugInfo() {
    if (!_debug) return;
    
    Serial.println("========================================");
    Serial.print("TTP229 Library - ");
    Serial.println(_boardName);
    Serial.println("========================================");
    Serial.print("Initialized: ");
    Serial.println(_initialized ? "Yes" : "No");
    Serial.print("Mode: ");
    Serial.println(_is16KeyMode ? "16-key" : "8-key");
    Serial.print("SCL Pin: ");
    Serial.println(_sclPin);
    Serial.print("SDO Pin: ");
    Serial.println(_sdoPin);
    Serial.print("Clock Delay: ");
    Serial.print(_clkDelay);
    Serial.println(" µs");
    Serial.print("Read Delay: ");
    Serial.print(_readDelay);
    Serial.println(" µs");
    Serial.print("Debounce: ");
    Serial.print(_debounceDelay);
    Serial.println(" ms");
    Serial.print("Scan Interval: ");
    Serial.print(_scanInterval);
    Serial.println(" ms");
    
    #if TTP229_RTOS_SUPPORT
    Serial.print("RTOS Support: Available");
    if (_rtosEnabled) {
        Serial.print(" (Enabled)");
        Serial.print(" Task Priority: ");
        Serial.println(_taskPriority);
    } else {
        Serial.println(" (Disabled)");
    }
    #else
    Serial.println("RTOS Support: Not available on this board");
    #endif
    
    Serial.println("========================================");
}

void TTP229::printRawReadings() {
    static uint8_t lastPrintedKey = KEY_NONE;
    uint8_t currentKey = read();
    
    if (currentKey != lastPrintedKey) {
        if (currentKey > KEY_NONE) {
            Serial.print("Key: ");
            Serial.print(currentKey);
            
            uint8_t row, col;
            getPosition(row, col);
            if (row != POSITION_INVALID && col != POSITION_INVALID) {
                Serial.print(" (Row ");
                Serial.print(row);
                Serial.print(", Col ");
                Serial.print(col);
                Serial.print(")");
            }
            Serial.println();
        } else {
            Serial.println("No key");
        }
        lastPrintedKey = currentKey;
    }
}

// ==============================================
// LOW-LEVEL READING METHODS
// ==============================================

uint8_t TTP229::readRaw() {
    uint8_t keyValue = KEY_NONE;
    uint8_t maxKeys = _is16KeyMode ? 16 : 8;
    
    // Start with clock high
    digitalWrite(_sclPin, HIGH);
    delayMicroseconds(_readDelay);
    
    // Read each key position
    for (uint8_t i = 1; i <= maxKeys; i++) {
        // Clock pulse low
        digitalWrite(_sclPin, LOW);
        delayMicroseconds(_clkDelay);
        
        // Read data (active LOW means key is pressed)
        if (digitalRead(_sdoPin) == LOW) {
            keyValue = i;  // Store which key is pressed
        }
        
        // Clock pulse high
        digitalWrite(_sclPin, HIGH);
        delayMicroseconds(_clkDelay);
    }
    
    // Ensure clock is high at end
    digitalWrite(_sclPin, HIGH);
    
    return keyValue;
}

uint8_t TTP229::readDebounced() {
    uint8_t rawKey = readRaw();
    unsigned long now = millis();
    
    // If key state changed, reset debounce timer
    if (rawKey != _lastRawKey) {
        _lastChangeTime = now;
        _lastRawKey = rawKey;
    }
    
    // Only return stable key if debounce time has passed
    if (timeElapsed(_lastChangeTime, _debounceDelay)) {
        _stableKey = rawKey;
    }
    
    return _stableKey;
}

// ==============================================
// RTOS-SPECIFIC METHODS IMPLEMENTATION
// ==============================================

#if TTP229_RTOS_SUPPORT

// RTOS task function (static method)
void TTP229::rtosTask(void* parameter) {
    TTP229* keypad = (TTP229*)parameter;
    if (keypad->_debug) {
        Serial.println("**************************");
    }   
    
    TickType_t lastWakeTime = xTaskGetTickCount();
    uint8_t lastProcessedKey = TTP229::KEY_NONE;
    uint32_t totalReadTime = 0;
    uint32_t readCount = 0;
    
    keypad->_taskRunning = true;
    
    while (keypad->_taskRunning) {
        // Measure read time
        uint32_t startTime = micros();
        uint8_t currentKey = keypad->readDebounced();
        uint32_t readTime = micros() - startTime;
        
        // Accumulate for statistics
        totalReadTime += readTime;
        readCount++;
        
        // Store in member variable
        keypad->_currentKey = currentKey;
        
        // *********** FIXED: Always process events, not just on change ***********
        // This ensures hold/long press detection works
        keypad->processKeyEvents();
        
        // Only update lastProcessedKey if the key actually changed
        if (currentKey != lastProcessedKey) {
            lastProcessedKey = currentKey;
            
            if (keypad->_debug) {
                Serial.println(currentKey);
                Serial.print("Queue count = ");
                Serial.println(keypad->getQueueCount());
            }
        }
        // ***********************************************************************
        
        // Update statistics every 100 reads
        if (readCount >= 100) {
            uint32_t avgReadTime = totalReadTime / readCount;
            keypad->updateStats(readCount, false);
            totalReadTime = 0;
            readCount = 0;
        }
        
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(keypad->_scanInterval));
    }
    
    #if defined(ESP32)
    vTaskDelete(NULL);
    if (keypad->_debug) {
        Serial.println("###########################");
    }
    #endif
}

void TTP229::processKeyEvents() {
    #if defined(ESP32)
    if (!_rtosEnabled || !takeMutex(5)) {  // 5ms timeout
        if (_debug) Serial.println("processKeyEvents: RTOS not enabled or mutex timeout");
        return;
    }
    
    uint32_t currentTime = millis();
    
    // Check if debounce period has elapsed since last change
    if (timeElapsed(_lastDebounceTime, _debounceDelay)) {
        // Debounce period has passed, we can process changes
        if (_currentKey != _lastValidKey) {
            uint8_t oldKey = _lastValidKey;
            _lastValidKey = _currentKey;
            
            if (_debug) {
                Serial.print("Key change ACCEPTED: old=");
                Serial.print(oldKey);
                Serial.print(", new=");
                Serial.println(_lastValidKey);
            }
            
            // Handle key press event
            if (_lastValidKey != KEY_NONE) {
                // Key was pressed
                if (_debug) Serial.println("Adding PRESS event to queue");
                addEventToQueue(_lastValidKey, EVENT_PRESS);
                _holdStartTime = currentTime;
                _lastHoldKey = _lastValidKey;
                _holdEventSent = false;      // Reset hold flag
                _longPressEventSent = false; // Reset long press flag
                
                // Signal waiting tasks
                if (_readSemaphore != NULL) {
                    xSemaphoreGive(_readSemaphore);
                }
            } else if (oldKey != KEY_NONE) {
                // Key was released
                if (_debug) Serial.println("Adding RELEASE event to queue");
                addEventToQueue(oldKey, EVENT_RELEASE);
                _holdStartTime = 0;
                _lastHoldKey = 0;
                _holdEventSent = false;
                _longPressEventSent = false;
            }
            
            _lastKey = _lastValidKey;
            
            // Reset debounce timer for next change
            _lastDebounceTime = currentTime;
        }
    }
    
    // ******************** HOLD & LONG PRESS DETECTION ********************
    // Check for hold events continuously
    if (_lastHoldKey != KEY_NONE && _holdStartTime != 0) {
        uint32_t holdDuration = currentTime - _holdStartTime;
        
        // Debug hold timing (less verbose)
        if (_debug && holdDuration > 900 && holdDuration < 2100) {
            Serial.print("Hold duration: ");
            Serial.print(holdDuration);
            Serial.print("ms (HOLD@");
            Serial.print(_holdThreshold);
            Serial.print("ms, LONG@");
            Serial.print(DEFAULT_LONG_PRESS_THRESHOLD_MS);
            Serial.println("ms)");
        }
        
        // Check for long press (2000ms by default) - trigger only once
        if (!_longPressEventSent && holdDuration >= DEFAULT_LONG_PRESS_THRESHOLD_MS) {
            if (_debug) Serial.println("*** Adding LONG_PRESS event to queue ***");
            addEventToQueue(_lastHoldKey, EVENT_LONG_PRESS);
            _longPressEventSent = true;  // Mark as sent
        } 
        // Check for hold (1000ms by default) - trigger only once
        else if (!_holdEventSent && holdDuration >= _holdThreshold) {
            if (_debug) Serial.println("*** Adding HOLD event to queue ***");
            addEventToQueue(_lastHoldKey, EVENT_HOLD);
            _holdEventSent = true;  // Mark as sent
        }
    }
    // ********************************************************************
    
    giveMutex();
    #endif
}

void TTP229::addEventToQueue(uint8_t key, uint8_t eventType) {
    #if defined(ESP32)
    if (_eventQueue == NULL) {
        if (_debug) Serial.println("ERROR: Event queue is NULL!");
        return;
    }
    
    KeyEvent event;
    event.key = key;
    event.eventType = eventType;
    event.timestamp = millis();
    
    uint8_t row, col;
    getPositionInternal(key, &row, &col);
    event.row = row;
    event.col = col;
    
    if (_debug) {
        Serial.print("addEventToQueue: key=");
        Serial.print(key);
        Serial.print(", type=");
        Serial.print(eventType);
        Serial.print(", queueFree=");
        Serial.println(uxQueueSpacesAvailable(_eventQueue));
    }
    
    // Try to add to queue (non-blocking)
    if (xQueueSend(_eventQueue, &event, 0) != pdTRUE) {
        // Queue is full
        if (_debug) Serial.println("ERROR: Queue is full!");
        portENTER_CRITICAL(&_statsMutex);
        _stats.queueOverflows++;
        portEXIT_CRITICAL(&_statsMutex);
    } else {
        // Update max queue usage
        uint32_t queueCount = uxQueueMessagesWaiting(_eventQueue);
        portENTER_CRITICAL(&_statsMutex);
        if (queueCount > _stats.maxQueueUsage) {
            _stats.maxQueueUsage = queueCount;
        }
        portEXIT_CRITICAL(&_statsMutex);
        
        if (_debug) {
            Serial.print("Event added. Queue now has ");
            Serial.print(queueCount);
            Serial.println(" events");
        }
    }
    #endif
}

bool TTP229::takeMutex(uint32_t timeout) {
    #if defined(ESP32)
    if (_mutex == NULL) return true;
    
    TickType_t timeoutTicks = (timeout == portMAX_DELAY) ? 
                              portMAX_DELAY : 
                              pdMS_TO_TICKS(timeout);
    
    return (xSemaphoreTake(_mutex, timeoutTicks) == pdTRUE);
    #else
    return true;  // No mutex on non-ESP32 platforms
    #endif
}

void TTP229::giveMutex() {
    #if defined(ESP32)
    if (_mutex != NULL) {
        xSemaphoreGive(_mutex);
    }
    #endif
}

uint8_t TTP229::readFromISR() {
    #if defined(ESP32)
    // Use volatile read to ensure we get the latest value
    uint8_t key = _lastValidKey;
    
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    // Check if queue has events (from ISR context)
    if (_eventQueue != NULL && uxQueueMessagesWaitingFromISR(_eventQueue) > 0) {
        // Signal waiting tasks
        if (_readSemaphore != NULL) {
            xSemaphoreGiveFromISR(_readSemaphore, &xHigherPriorityTaskWoken);
        }
    }
    
    // Yield if a higher priority task was woken
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
    
    return key;
    #else
    return _lastValidKey;  // Not safe for ISR on non-ESP32 platforms
    #endif
}

uint8_t TTP229::readWithTimeout(uint32_t timeoutMs) {
    #if defined(ESP32)
    if (!_rtosEnabled) return read();
    
    TickType_t timeoutTicks = pdMS_TO_TICKS(timeoutMs);
    
    // Wait for semaphore (blocking)
    if (xSemaphoreTake(_readSemaphore, timeoutTicks) == pdTRUE) {
        return _lastValidKey;
    }
    
    return KEY_NONE;  // Timeout
    #else
    // Non-RTOS fallback
    unsigned long startTime = millis();
    while (timeElapsed(startTime, timeoutMs) == false) {
        uint8_t key = read();
        if (key != KEY_NONE) return key;
        delay(1);
    }
    return KEY_NONE;
    #endif
}

bool TTP229::getKeyEvents(KeyEvent &event) {
    #if defined(ESP32)
    if (!_rtosEnabled || _eventQueue == NULL) return false;
    return (xQueueReceive(_eventQueue, &event, 0) == pdTRUE);
    #else
    return false;
    #endif
}

bool TTP229::isPressedFromISR() {
    return (_lastValidKey != KEY_NONE);
}

bool TTP229::wasPressedFromISR() {
    #if TTP229_RTOS_SUPPORT
    // Use atomic operations for ISR safety
    uint8_t currentKey = _lastValidKey;
    bool pressed = (currentKey != KEY_NONE && _lastKeyFromISR == KEY_NONE);
    _lastKeyFromISR = currentKey;
    return pressed;
    #else
    return false;
    #endif
}

void TTP229::setTaskPriority(uint8_t priority) {
    _taskPriority = priority;
    #if defined(ESP32)
    if (_taskHandle != NULL) {
        vTaskPrioritySet(_taskHandle, priority);
    }
    #endif
}

void TTP229::setStackDepth(uint32_t depth) {
    _taskStackDepth = depth;
    // Note: Cannot change stack depth of running task
}

void TTP229::setQueueSize(uint8_t size) {
    _queueSize = size;
    // Note: Queue would need to be recreated to change size
}

void TTP229::enableEventQueue(bool enable) {
    _eventQueueEnabled = enable;
}

bool TTP229::isRTOSEnabled() {
    return _rtosEnabled;
}

uint32_t TTP229::getQueueCount() {
    #if defined(ESP32)
    if (_eventQueue == NULL) return 0;
    return uxQueueMessagesWaiting(_eventQueue);
    #else
    return 0;
    #endif
}

TTP229::RTOSStats TTP229::getRTOSStats() {
    RTOSStats stats;
    #if defined(ESP32)
    portENTER_CRITICAL(&_statsMutex);
    memcpy(&stats, &_stats, sizeof(RTOSStats));
    portEXIT_CRITICAL(&_statsMutex);
    #else
    memset(&stats, 0, sizeof(RTOSStats));
    #endif
    return stats;
}

void TTP229::resetRTOSStats() {
    #if defined(ESP32)
    portENTER_CRITICAL(&_statsMutex);
    memset(&_stats, 0, sizeof(_stats));
    _lastStatsReset = millis();
    portEXIT_CRITICAL(&_statsMutex);
    #endif
}

void TTP229::updateStats(uint32_t reads, bool queueFull) {
    #if defined(ESP32)
    static uint32_t lastStatUpdate = 0;
    static uint32_t readCount = 0;
    
    uint32_t currentTime = millis();
    readCount += reads;
    
    // Update statistics every second
    if (timeElapsed(lastStatUpdate, 1000)) {
        portENTER_CRITICAL(&_statsMutex);
        _stats.readsPerSecond = readCount;
        if (queueFull) _stats.queueOverflows++;
        _stats.taskRunTime = currentTime - _lastStatsReset;
        portEXIT_CRITICAL(&_statsMutex);
        
        readCount = 0;
        lastStatUpdate = currentTime;
    }
    #endif
}

#endif // TTP229_RTOS_SUPPORT