#include "TTP229.h"

TTP229 keypad(18, 19, true, 1, 2048);

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("=== TTP229 RTOS Queue Test ===");
    
    keypad.begin(false);  // Enable debug
    keypad.beginRTOS();  // Initialize RTOS with default settings
    
    Serial.println("\nPress keys to test queue...");
    Serial.println("Waiting 5 seconds for key presses...");
    
    // Wait for some key presses
    delay(5000);
    
    // Now check what's in the queue
    Serial.println("\n=== Checking Queue ===");
    Serial.print("Queue count: ");
    Serial.println(keypad.getQueueCount());
    
    TTP229::KeyEvent event;
    int eventCount = 0;
    
    while (keypad.getKeyEvents(event)) {
        eventCount++;
        Serial.print("\nEvent ");
        Serial.print(eventCount);
        Serial.print(": Key=");
        Serial.print(event.key);
        Serial.print(", Type=");
        switch(event.eventType) {
            case TTP229::EVENT_PRESS: Serial.println("PRESS"); break;
            case TTP229::EVENT_RELEASE: Serial.println("RELEASE"); break;
            case TTP229::EVENT_HOLD: Serial.println("HOLD"); break;
            case TTP229::EVENT_LONG_PRESS: Serial.println("LONG_PRESS"); break;
        }
        Serial.print("Timestamp: ");
        Serial.println(event.timestamp);
        Serial.print("Position: Row=");
        Serial.print(event.row);
        Serial.print(", Col=");
        Serial.println(event.col);
    }
    
    if (eventCount == 0) {
        Serial.println("No events in queue!");
    }
    
    Serial.println("=== Test Complete ===");
}

void loop() {
    // Empty loop
    delay(1000);
}