/*
   TTP229 RTOS Performance Test
   Measures RTOS vs non-RTOS performance
*/

#include <TTP229.h>
#include <freertos/task.h>
#include <freertos/queue.h>

TTP229 keypadRTOS(18, 19, true, 3, 4096);  // RTOS version
TTP229 keypadNormal(18, 19, true);         // Non-RTOS version

// Test modes
enum TestMode {
    MODE_NON_RTOS,
    MODE_RTOS_SINGLE,
    MODE_RTOS_MULTI,
    MODE_RTOS_QUEUE
};

TestMode currentMode = MODE_NON_RTOS;
uint32_t testStartTime = 0;
uint32_t readCount = 0;
uint32_t eventCount = 0;

// Performance task
void performanceTask(void* parameter) {
    uint32_t localReads = 0;
    
    while (1) {
        // Read as fast as possible
        uint8_t key = keypadRTOS.read();
        localReads++;
        
        // Update global counter every 1000 reads
        if (localReads >= 1000) {
            portENTER_CRITICAL(&keypadRTOS._statsMutex);
            readCount += localReads;
            portEXIT_CRITICAL(&keypadRTOS._statsMutex);
            localReads = 0;
        }
        
        taskYIELD();  // Yield to other tasks
    }
}

void runNonRTOSPerfTest() {
    Serial.println("\n=== Non-RTOS Performance Test ===");
    Serial.println("Reading keypad for 5 seconds...");
    
    keypadNormal.begin();
    readCount = 0;
    testStartTime = millis();
    
    while (millis() - testStartTime < 5000) {
        keypadNormal.read();
        readCount++;
    }
    
    float readsPerSec = readCount / 5.0;
    Serial.print("Reads per second: ");
    Serial.println(readsPerSec, 1);
    Serial.print("CPU Usage: High (blocking)");
}

void runRTOSSinglePerfTest() {
    Serial.println("\n=== RTOS Single Task Test ===");
    
    keypadRTOS.begin();
    keypadRTOS.beginRTOS(true);
    readCount = 0;
    testStartTime = millis();
    
    // Let RTOS task run for 5 seconds
    delay(5000);
    
    #if TTP229_RTOS_AVAILABLE
    TTP229::RTOSStats stats = keypadRTOS.getRTOSStats();
    Serial.print("Reads per second: ");
    Serial.println(stats.readsPerSecond);
    Serial.print("Task priority: ");
    Serial.println(keypadRTOS.getTaskPriority());
    Serial.print("Queue usage: ");
    Serial.println(stats.maxQueueUsage);
    #endif
}

void runRTOSMultiPerfTest() {
    Serial.println("\n=== RTOS Multi-Task Stress Test ===");
    
    // Create multiple reading tasks
    const int numTasks = 3;
    TaskHandle_t tasks[numTasks];
    
    readCount = 0;
    testStartTime = millis();
    
    for (int i = 0; i < numTasks; i++) {
        xTaskCreate(
            performanceTask,
            "PerfTask",
            2048,
            NULL,
            2 + i,  // Different priorities
            &tasks[i]
        );
    }
    
    // Run for 5 seconds
    delay(5000);
    
    // Delete tasks
    for (int i = 0; i < numTasks; i++) {
        if (tasks[i] != NULL) {
            vTaskDelete(tasks[i]);
        }
    }
    
    float readsPerSec = readCount / 5.0;
    Serial.print("Total reads per second: ");
    Serial.println(readsPerSec, 1);
    Serial.print("Reads per task: ");
    Serial.println(readCount / numTasks);
}

void runRTOSQueuePerfTest() {
    Serial.println("\n=== RTOS Queue Performance Test ===");
    
    keypadRTOS.begin();
    keypadRTOS.beginRTOS(true);
    keypadRTOS.enableEventQueue(true);
    
    eventCount = 0;
    testStartTime = millis();
    
    // Process events for 5 seconds
    while (millis() - testStartTime < 5000) {
        TTP229::KeyEvent event;
        if (keypadRTOS.getKeyEvents(event)) {
            eventCount++;
        }
        delay(1);
    }
    
    float eventsPerSec = eventCount / 5.0;
    Serial.print("Events processed per second: ");
    Serial.println(eventsPerSec, 1);
    
    #if TTP229_RTOS_AVAILABLE
    TTP229::RTOSStats stats = keypadRTOS.getRTOSStats();
    Serial.print("Queue overflows: ");
    Serial.println(stats.queueOverflows);
    #endif
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("=== TTP229 RTOS Performance Test ===");
    Serial.println("This test compares RTOS vs non-RTOS performance");
    Serial.println("Connect keypad to pins 18(SCL) and 19(SDO)");
    Serial.println();
    
    // Run all tests
    runNonRTOSPerfTest();
    delay(2000);
    
    runRTOSSinglePerfTest();
    delay(2000);
    
    runRTOSMultiPerfTest();
    delay(2000);
    
    runRTOSQueuePerfTest();
    
    Serial.println("\n=== Test Complete ===");
    Serial.println("Summary:");
    Serial.println("- Non-RTOS: Simple but blocking");
    Serial.println("- RTOS Single: Good for most applications");
    Serial.println("- RTOS Multi: Better CPU utilization");
    Serial.println("- RTOS Queue: Best for event-driven apps");
}

void loop() {
    // Show current status
    static uint32_t lastDisplay = 0;
    
    if (millis() - lastDisplay > 10000) {
        lastDisplay = millis();
        
        #if TTP229_RTOS_AVAILABLE
        if (keypadRTOS.isRTOSEnabled()) {
            TTP229::RTOSStats stats = keypadRTOS.getRTOSStats();
            Serial.print("[Status] Reads/sec: ");
            Serial.print(stats.readsPerSecond);
            Serial.print(" Queue: ");
            Serial.print(keypadRTOS.getQueueCount());
            Serial.print("/");
            Serial.println(keypadRTOS._queueSize);
        }
        #endif
    }
    
    delay(1000);
}