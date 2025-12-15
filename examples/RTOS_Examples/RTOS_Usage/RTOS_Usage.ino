#include <TTP229.h>

// RTOS constructor: pins, mode, task priority, stack depth
TTP229 keypad(18, 19, true, 1, 2048);

void setup() {
  keypad.begin();
  keypad.beginRTOS();  // Start RTOS task
  keypad.enableEventQueue(true);
}

void loop() {
  // Get events from queue
  TTP229::KeyEvent event;
  if (keypad.getKeyEvents(event)) {
    // Handle event
  }
}