/*
   TTP229 RTOS Basic Example
   Shows basic RTOS usage with keypad in separate task
*/

#include <TTP229.h>

// Create keypad with RTOS support
TTP229 keypad(18, 19, true, 1, 4096);  // Pins, 16-key, priority=1, stack=4096

// Task handles
TaskHandle_t displayTaskHandle = NULL;

void displayTask(void* parameter) {
    while (1) {
        // Get key events from queue
        TTP229::KeyEvent event;
        if (keypad.getKeyEvents(event)) {
            Serial.print("Event: Key ");
            Serial.print(event.key);
            Serial.print(" - ");
            
            switch(event.eventType) {
                case TTP229::EVENT_PRESS:
                    Serial.print("PRESS");
                    break;
                case TTP229::EVENT_RELEASE:
                    Serial.print("RELEASE");
                    break;
                case TTP229::EVENT_HOLD:
                    Serial.print("HOLD (1 sec)");
                    break;
                case TTP229::EVENT_LONG_PRESS:
                    Serial.print("LONG_PRESS (2 sec)");
                    break;
            }
            
            Serial.print(" (");
            Serial.print(event.row);
            Serial.print(",");
            Serial.print(event.col);
            Serial.print(") @ ");
            Serial.print(event.timestamp);
            Serial.println(" ms");
            
            // Optional: Reset hold timer on release
            if (event.eventType == TTP229::EVENT_RELEASE) {
                // Clear any pending hold events
                while (keypad.getKeyEvents(event)) {
                    // Empty the queue
                }
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));  // 10ms delay
    }
}
/*
void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("=== TTP229 RTOS Basic Example ===");
    
    // Initialize keypad with RTOS
    keypad.begin(true);  // Debug mode
    keypad.beginRTOS();  // Enable RTOS with default task
    
    // Configure hold and long press thresholds (optional)
    keypad.setHoldThreshold(1000, 2000);  // Hold at 1s, Long press at 2s 

    // Create display task
    xTaskCreate(
        displayTask,
        "DisplayTask",
        4096,
        NULL,
        1,
        &displayTaskHandle
    );
    
    Serial.println("Ready! Press keys...");
    Serial.println("RTOS statistics every 5 seconds:");
}
*/
void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("=== TTP229 HOLD TEST ===");
    Serial.println("Hold a key for >1 second for HOLD event");
    Serial.println("Hold a key for >2 seconds for LONG_PRESS event");
    
    keypad.begin();  // Enable debug
    keypad.beginRTOS();
    
    // Optional: Reduce scan interval for faster response
    keypad.setScanInterval(50);  // 50ms scan interval
    
    // Optional: Set custom hold thresholds
    keypad.setHoldThreshold(800, 1500);  // Hold at 0.8s, Long press at 1.5s
    
    // Create display task
    xTaskCreate(
        displayTask,
        "DisplayTask",
        4096,
        NULL,
        1,
        &displayTaskHandle
    );

    Serial.println("Ready! Press and HOLD a key...");
}

void loop() {
    // Main loop can do other things
    static unsigned long lastStats = 0;
    
    if (millis() - lastStats > 5000) {
        lastStats = millis();
        
        #if TTP229_RTOS_AVAILABLE
        TTP229::RTOSStats stats = keypad.getRTOSStats();
        Serial.println("\n--- RTOS Statistics ---");
        Serial.print("Reads/sec: ");
        Serial.println(stats.readsPerSecond);
        Serial.print("Queue overflows: ");
        Serial.println(stats.queueOverflows);
        Serial.print("Max queue usage: ");
        Serial.println(stats.maxQueueUsage);
        Serial.print("Task runtime: ");
        Serial.print(stats.taskRunTime / 1000);
        Serial.println(" seconds");
        Serial.println("----------------------");
        #endif
    }
    
    delay(1000);  // Main loop delay
}