/*
   TTP229 RTOS Multi-Task Example
   Multiple tasks using the same keypad with mutex protection
*/

#include <TTP229.h>

TTP229 keypad(18, 19, true, 2, 4096);  // Higher priority for keypad task

// Task handles
TaskHandle_t soundTaskHandle = NULL;
TaskHandle_t ledTaskHandle = NULL;
TaskHandle_t serialTaskHandle = NULL;

// Shared variables
volatile uint8_t lastKey = 0;
volatile uint32_t keyPressCount = 0;

// Sound task - plays tones based on keys
void soundTask(void* parameter) {
    while (1) {
        uint8_t currentKey = keypad.read();
        
        if (keypad.wasPressed()) {
            lastKey = currentKey;
            keyPressCount++;
            
            // Play different tones for different keys
            if (currentKey >= 1 && currentKey <= 12) {
                // Play tone (simplified - actual tone code depends on board)
                Serial.print("[Sound] Playing tone for key ");
                Serial.println(currentKey);
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

// LED task - controls LEDs based on keypad
void ledTask(void* parameter) {
    const int ledPins[] = {2, 4, 5, 12, 13, 14, 15, 16};
    
    // Setup LED pins
    for (int i = 0; i < 8; i++) {
        pinMode(ledPins[i], OUTPUT);
    }
    
    while (1) {
        #if TTP229_RTOS_AVAILABLE
        // Use mutex-protected read
        if (keypad.takeMutex(10)) {
            uint8_t key = keypad.getKey();
            
            // Light LEDs based on key
            if (key != 0) {
                for (int i = 0; i < 8; i++) {
                    digitalWrite(ledPins[i], (key & (1 << i)) ? HIGH : LOW);
                }
            } else {
                // Turn off all LEDs when no key
                for (int i = 0; i < 8; i++) {
                    digitalWrite(ledPins[i], LOW);
                }
            }
            
            keypad.giveMutex();
        }
        #endif
        
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// Serial task - handles serial commands
void serialTask(void* parameter) {
    char command[32];
    uint8_t cmdIndex = 0;
    
    while (1) {
        // Check for serial input
        while (Serial.available() > 0) {
            char c = Serial.read();
            
            if (c == '\n' || c == '\r') {
                command[cmdIndex] = '\0';
                cmdIndex = 0;
                
                // Process command
                if (strcmp(command, "stats") == 0) {
                    #if TTP229_RTOS_AVAILABLE
                    TTP229::RTOSStats stats = keypad.getRTOSStats();
                    Serial.print("Key presses: ");
                    Serial.println(keyPressCount);
                    Serial.print("Current key: ");
                    Serial.println(lastKey);
                    Serial.print("Queue items: ");
                    Serial.println(keypad.getQueueCount());
                    #endif
                }
                else if (strcmp(command, "reset") == 0) {
                    keyPressCount = 0;
                    #if TTP229_RTOS_AVAILABLE
                    keypad.resetRTOSStats();
                    #endif
                    Serial.println("Statistics reset");
                }
            } else if (cmdIndex < 31) {
                command[cmdIndex++] = c;
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("=== TTP229 RTOS Multi-Task Example ===");
    
    // Initialize keypad
    keypad.begin();
    keypad.beginRTOS(true);
    keypad.enableEventQueue(true);
    
    // Create tasks
    xTaskCreate(soundTask, "SoundTask", 2048, NULL, 1, &soundTaskHandle);
    xTaskCreate(ledTask, "LEDTask", 2048, NULL, 1, &ledTaskHandle);
    xTaskCreate(serialTask, "SerialTask", 4096, NULL, 1, &serialTaskHandle);
    
    Serial.println("Tasks created. Commands: stats, reset");
    Serial.println("Press keys to see multi-task operation");
}

void loop() {
    // Empty - everything runs in RTOS tasks
    delay(1000);
}