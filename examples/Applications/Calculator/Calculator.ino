/*
   TTP229 Calculator
   Simple 4-function calculator
*/

#include <TTP229.h>

TTP229 keypad(2, 3, true);

// Calculator variables
float firstNumber = 0;
float secondNumber = 0;
float result = 0;
char operation = 0;
bool newNumber = true;
String display = "0";

// Keypad layout for calculator
const char calcKeys[16] = {
  '7', '8', '9', '/',
  '4', '5', '6', '*',
  '1', '2', '3', '-',
  'C', '0', '=', '+'
};

void setup() {
  Serial.begin(115200);
  keypad.begin();
  
  Serial.println("\n=== TTP229 Calculator ===");
  Serial.println("C: Clear, =: Equals");
  Serial.println("+, -, *, /: Operations");
  Serial.println("========================\n");
  
  updateDisplay();
}

void loop() {
  uint8_t key = keypad.read();
  
  if (keypad.wasPressed() && key >= 1 && key <= 16) {
    char keyChar = calcKeys[key - 1];
    
    // Handle numeric keys (0-9)
    if (keyChar >= '0' && keyChar <= '9') {
      if (newNumber) {
        display = String(keyChar);
        newNumber = false;
      } else {
        display += keyChar;
      }
    }
    // Handle decimal point (using 'A' key from original)
    else if (key == 4) {  // A key
      if (display.indexOf('.') == -1) {
        display += '.';
        newNumber = false;
      }
    }
    // Handle operations
    else if (keyChar == '+' || keyChar == '-' || keyChar == '*' || keyChar == '/') {
      if (operation == 0) {
        firstNumber = display.toFloat();
        operation = keyChar;
        newNumber = true;
        Serial.print(firstNumber);
        Serial.print(" ");
        Serial.println(operation);
      }
    }
    // Handle equals
    else if (keyChar == '=') {
      if (operation != 0) {
        secondNumber = display.toFloat();
        calculate();
        newNumber = true;
      }
    }
    // Handle clear
    else if (keyChar == 'C') {
      clearCalculator();
    }
    // Handle backspace (using 'B' key)
    else if (key == 8) {  // B key
      if (display.length() > 1) {
        display = display.substring(0, display.length() - 1);
      } else {
        display = "0";
        newNumber = true;
      }
    }
    
    updateDisplay();
  }
  
  delay(10);
}

void calculate() {
  switch(operation) {
    case '+':
      result = firstNumber + secondNumber;
      break;
    case '-':
      result = firstNumber - secondNumber;
      break;
    case '*':
      result = firstNumber * secondNumber;
      break;
    case '/':
      if (secondNumber != 0) {
        result = firstNumber / secondNumber;
      } else {
        display = "Error";
        return;
      }
      break;
  }
  
  // Format result
  if (result == (int)result) {
    display = String((int)result);
  } else {
    display = String(result, 4);
    // Remove trailing zeros
    while (display.endsWith("0")) {
      display.remove(display.length() - 1);
    }
    if (display.endsWith(".")) {
      display.remove(display.length() - 1);
    }
  }
  
  // Reset for next calculation
  firstNumber = result;
  operation = 0;
}

void clearCalculator() {
  firstNumber = 0;
  secondNumber = 0;
  result = 0;
  operation = 0;
  newNumber = true;
  display = "0";
}

void updateDisplay() {
  Serial.println("\n═══════════════");
  Serial.print("  ");
  Serial.println(display);
  Serial.println("═══════════════");
}