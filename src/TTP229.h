#ifndef TTP229_H
#define TTP229_H

#include <Arduino.h>

// RTOS detection - automatically detect supported platforms
#if defined(ESP32) || defined(ESP8266) || defined(ARDUINO_ARCH_MBED) || defined(ARDUINO_ARCH_RP2040)
  #define TTP229_RTOS_SUPPORT 1
  #if defined(ESP32)
    #include <freertos/FreeRTOS.h>
    #include <freertos/task.h>
    #include <freertos/semphr.h>
    #include <freertos/queue.h>
  #endif
#else
  #define TTP229_RTOS_SUPPORT 0
#endif

class TTP229 {
public:
    // ==============================================
    // CONSTANTS
    // ==============================================
    
    // Key constants
    static const uint8_t KEY_NONE = 0;
    static const uint8_t KEY_INVALID = 255;
    static const uint8_t KEY_1 = 1;
    static const uint8_t KEY_2 = 2;
    static const uint8_t KEY_3 = 3;
    static const uint8_t KEY_4 = 4;
    static const uint8_t KEY_5 = 5;
    static const uint8_t KEY_6 = 6;
    static const uint8_t KEY_7 = 7;
    static const uint8_t KEY_8 = 8;
    static const uint8_t KEY_9 = 9;
    static const uint8_t KEY_10 = 10;
    static const uint8_t KEY_11 = 11;
    static const uint8_t KEY_12 = 12;
    static const uint8_t KEY_13 = 13;
    static const uint8_t KEY_14 = 14;
    static const uint8_t KEY_15 = 15;
    static const uint8_t KEY_16 = 16;
    
    // Timing constants
    static const uint16_t DEFAULT_HOLD_THRESHOLD_MS = 1000;
    static const uint16_t DEFAULT_LONG_PRESS_THRESHOLD_MS = 2000;
    
    // Position constants
    static const uint8_t POSITION_INVALID = 255;
    
    // Constructors
    TTP229();                                       // Auto-detect, 16-key mode
    TTP229(bool is16KeyMode);                       // Auto-detect with mode
    TTP229(uint8_t sclPin, uint8_t sdoPin, bool is16KeyMode = true);  // Custom pins
    
    // RTOS-specific constructor (only available on RTOS platforms)
    #if TTP229_RTOS_SUPPORT
    TTP229(uint8_t sclPin, uint8_t sdoPin, bool is16KeyMode, uint8_t taskPriority, uint32_t stackDepth);
    #endif
    
    // Destructor
    ~TTP229();
    
    // Initialization
    bool begin();                      // Returns true if successful
    bool begin(bool debugMode);        // Returns true if successful
    
    // RTOS Initialization
    #if TTP229_RTOS_SUPPORT
    bool beginRTOS(bool createTask = true);  // Returns true if successful
    void endRTOS();
    void stopRTOS();                    // Gracefully stop RTOS task
    #endif
    
    // Key reading - available on all platforms
    uint8_t read();                    // Read key with debouncing
    uint8_t getKey();                  // Get current key (1-16/1-8)
    uint8_t getKeyNumber();            // Get key number (0-15 for 16-key, 0-7 for 8-key)
    void getPosition(uint8_t &row, uint8_t &col);  // Get row/col (0-based)
    
    // State checking - available on all platforms
    bool isPressed();                  // Any key pressed?
    bool isKeyPressed(uint8_t keyNum); // Specific key pressed?
    bool wasPressed();                 // Key just pressed?
    bool wasReleased();                // Key just released?
    
    // Configuration - available on all platforms
    bool setMode(bool is16KeyMode);    // Change mode (8/16 key)
    bool setDebounce(uint16_t ms);     // Set debounce time (default: 20-50ms)
    bool setScanInterval(uint16_t ms); // Set scan interval (default: 10-50ms)
    bool setTiming(uint16_t clkDelay, uint16_t readDelay); // Advanced timing (microseconds)
	
	bool setHoldThreshold(uint16_t holdMs, uint16_t longPressMs = DEFAULT_LONG_PRESS_THRESHOLD_MS);
    
    // Information - available on all platforms
    const char* getBoardName();
    uint8_t getSCLPin();
    uint8_t getSDOPin();
    bool is16KeyMode();
    bool isInitialized();
    
    // Debug - available on all platforms
    void printDebugInfo();
    void printRawReadings();
    
    // ==============================================
    // RTOS-SPECIFIC METHODS (only on RTOS platforms)
    // ==============================================
    #if TTP229_RTOS_SUPPORT
    
    // Event structure for RTOS queue
    typedef struct {
        uint8_t key;           // Key number (1-16 or 1-8)
        uint8_t eventType;     // Event type (see constants below)
        uint32_t timestamp;    // Time when event occurred (milliseconds)
        uint8_t row;           // Row (0-based)
        uint8_t col;           // Column (0-based)
    } KeyEvent;
    
    // Event type constants
    static const uint8_t EVENT_PRESS = 0;
    static const uint8_t EVENT_RELEASE = 1;
    static const uint8_t EVENT_HOLD = 2;
    static const uint8_t EVENT_LONG_PRESS = 3;
    
    // RTOS-specific reading methods
    uint8_t readFromISR();                    // Safe to call from interrupt context
    uint8_t readWithTimeout(uint32_t timeoutMs); // Blocking read with timeout
    bool getKeyEvents(KeyEvent &event);       // Get event from queue (non-blocking)
    
    // RTOS state checking
    bool isPressedFromISR();
    bool wasPressedFromISR();
    
    // RTOS configuration
    void setTaskPriority(uint8_t priority);
    void setStackDepth(uint32_t depth);
    void setQueueSize(uint8_t size);
    void enableEventQueue(bool enable = true);
    
    // RTOS information
    bool isRTOSEnabled();
    uint32_t getQueueCount();
    
    // RTOS statistics
    typedef struct {
        uint32_t readsPerSecond;   // Average reads per second
        uint32_t queueOverflows;   // Number of times queue was full
        uint32_t missedEvents;     // Events that couldn't be queued
        uint32_t taskRunTime;      // How long RTOS task has been running (ms)
        uint32_t maxQueueUsage;    // Maximum number of events in queue
    } RTOSStats;
    
    RTOSStats getRTOSStats();
    void resetRTOSStats();
    
    #endif // TTP229_RTOS_SUPPORT

private:
    // Pin configuration
    uint8_t _sclPin;
    uint8_t _sdoPin;
    bool _is16KeyMode;
    bool _debug;
    bool _initialized;
    
    // State tracking (instance variables for multiple keypad support)
    uint8_t _currentKey;
    uint8_t _lastKey;
    uint8_t _lastValidKey;
    unsigned long _lastDebounceTime;
    unsigned long _lastReadTime;
    
    // Debounce state (moved from static to instance)
    uint8_t _lastRawKey;
    uint8_t _stableKey;
    unsigned long _lastChangeTime;
    
    // Timing
    uint16_t _debounceDelay;
    uint16_t _scanInterval;
    uint16_t _clkDelay;     // microseconds
    uint16_t _readDelay;    // microseconds
    
    // Board information
    const char* _boardName;
    
    // ==============================================
    // RTOS-SPECIFIC PRIVATE MEMBERS
    // ==============================================
    #if TTP229_RTOS_SUPPORT
    
    // RTOS handles (ESP32/FreeRTOS specific)
    #if defined(ESP32)
    TaskHandle_t _taskHandle;
    QueueHandle_t _eventQueue;
    SemaphoreHandle_t _mutex;
    SemaphoreHandle_t _readSemaphore;
    portMUX_TYPE _statsMutex;
    #endif
    
    // RTOS configuration
    bool _rtosEnabled;
    volatile bool _taskRunning;  // Flag to control RTOS task execution
    uint8_t _taskPriority;
    uint32_t _taskStackDepth;
    uint8_t _queueSize;
    bool _eventQueueEnabled;
    uint32_t _holdThreshold;        // Hold detection threshold (ms)
    
    // Hold detection state
    volatile uint8_t _lastHoldKey;
    volatile uint32_t _holdStartTime;
	volatile bool _holdEventSent;      // NEW: Track if HOLD event was sent
	volatile bool _longPressEventSent; // NEW: Track if LONG_PRESS event was sent
    
    // ISR-safe state tracking
    volatile uint8_t _lastKeyFromISR;
    
    // Statistics
    RTOSStats _stats;
    uint32_t _lastStatsReset;
    
    // RTOS internal methods
    static void rtosTask(void* parameter);
    void processKeyEvents();
    void addEventToQueue(uint8_t key, uint8_t eventType);
    bool takeMutex(uint32_t timeout = portMAX_DELAY);
    void giveMutex();
    void updateStats(uint32_t reads, bool queueFull);
    
    #endif // TTP229_RTOS_SUPPORT
    
    // Internal methods (available on all platforms)
    uint8_t readRaw();
    uint8_t readDebounced();
    void getPositionInternal(uint8_t key, uint8_t *row, uint8_t *col);
    void detectBoard();
    void setBoardDefaults();
    void initializeState();
    bool isValidPin(uint8_t pin);
    bool validateTiming(uint16_t clkDelay, uint16_t readDelay);
    
    // Timing helper that handles millis() overflow
    bool timeElapsed(uint32_t startTime, uint32_t interval);
};

#endif // TTP229_H