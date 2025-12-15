/*
   TTP229 Game Controller
   Use keypad for simple games
*/

#include <TTP229.h>

TTP229 keypad(2, 3, true);

// Game variables
int playerX = 2;  // 0-4 position
int playerY = 2;
int score = 0;
int targetX = 0;
int targetY = 0;
int lives = 3;
unsigned long gameTime = 0;
const int gameDuration = 60000;  // 60 seconds
bool gameRunning = false;

// Game grid (5x5)
char grid[5][5];

void setup() {
  Serial.begin(115200);
  keypad.begin();
  
  Serial.println("\n=== TTP229 Game Controller ===");
  Serial.println("2,4,6,8: Move (like numpad)");
  Serial.println("5: Start/Restart");
  Serial.println("*: Exit");
  Serial.println("#: Action/Shoot");
  Serial.println("=============================\n");
  
  showMenu();
}

void loop() {
  uint8_t key = keypad.read();
  
  if (keypad.wasPressed()) {
    if (!gameRunning) {
      // Menu navigation
      switch(key) {
        case 5:  // Start game
          startGame();
          break;
        case 2:  // Instructions
          showInstructions();
          break;
        case 8:  // High scores (placeholder)
          Serial.println("High Scores: 1500, 1200, 1000");
          break;
      }
    } else {
      // In-game controls
      handleGameInput(key);
    }
  }
  
  // Game loop
  if (gameRunning) {
    gameLoop();
  }
  
  delay(50);
}

void startGame() {
  playerX = 2;
  playerY = 2;
  score = 0;
  lives = 3;
  gameTime = millis();
  gameRunning = true;
  placeTarget();
  
  Serial.println("\n=== GAME STARTED ===");
  Serial.println("Move with 2,4,6,8");
  Serial.println("Shoot with #");
  Serial.println("Exit with *");
  Serial.println("===================\n");
  
  drawGame();
}

void gameLoop() {
  static unsigned long lastUpdate = 0;
  
  // Move target occasionally
  if (millis() - lastUpdate > 2000) {
    lastUpdate = millis();
    if (random(100) < 30) {  // 30% chance to move
      placeTarget();
      drawGame();
    }
  }
  
  // Check game over
  if (millis() - gameTime > gameDuration) {
    endGame();
  }
}

void handleGameInput(uint8_t key) {
  int oldX = playerX;
  int oldY = playerY;
  
  switch(key) {
    case 2:  // Down
      playerY = min(4, playerY + 1);
      break;
    case 4:  // Left
      playerX = max(0, playerX - 1);
      break;
    case 6:  // Right
      playerX = min(4, playerX + 1);
      break;
    case 8:  // Up
      playerY = max(0, playerY - 1);
      break;
    case 5:  // Center/Wait
      // Do nothing
      break;
    case 15: // # - Shoot/Action
      if (playerX == targetX && playerY == targetY) {
        score += 100;
        Serial.print("HIT! +100 Points! Score: ");
        Serial.println(score);
        placeTarget();
      }
      break;
    case 13: // * - Exit
      endGame();
      return;
  }
  
  // Draw if position changed
  if (oldX != playerX || oldY != playerY) {
    drawGame();
  }
}

void placeTarget() {
  do {
    targetX = random(5);
    targetY = random(5);
  } while (targetX == playerX && targetY == playerY);
}

void drawGame() {
  // Clear grid
  for (int y = 0; y < 5; y++) {
    for (int x = 0; x < 5; x++) {
      grid[y][x] = '.';
    }
  }
  
  // Draw player
  grid[playerY][playerX] = 'P';
  
  // Draw target
  grid[targetY][targetX] = 'T';
  
  // Display game
  Serial.println("\n╔═════════════════════╗");
  Serial.print("║ Score: ");
  Serial.print(score);
  Serial.print(" Lives: ");
  Serial.print(lives);
  Serial.print(" ║\n");
  Serial.print("║ Time: ");
  Serial.print((gameDuration - (millis() - gameTime)) / 1000);
  Serial.println("s        ║");
  Serial.println("╠═════════════════════╣");
  
  for (int y = 0; y < 5; y++) {
    Serial.print("║ ");
    for (int x = 0; x < 5; x++) {
      Serial.print(grid[y][x]);
      Serial.print(" ");
    }
    Serial.println("║");
  }
  
  Serial.println("╚═════════════════════╝");
  Serial.println("P=You, T=Target");
}

void endGame() {
  gameRunning = false;
  Serial.println("\n=== GAME OVER ===");
  Serial.print("Final Score: ");
  Serial.println(score);
  Serial.print("Time: ");
  Serial.print((millis() - gameTime) / 1000);
  Serial.println(" seconds");
  Serial.println("=================\n");
  
  showMenu();
}

void showMenu() {
  Serial.println("\n=== MAIN MENU ===");
  Serial.println("5: Start Game");
  Serial.println("2: Instructions");
  Serial.println("8: High Scores");
  Serial.println("*: Exit");
  Serial.println("================");
}

void showInstructions() {
  Serial.println("\n=== INSTRUCTIONS ===");
  Serial.println("Move your player (P) to catch");
  Serial.println("the target (T) and shoot with #");
  Serial.println("2=Down, 8=Up, 4=Left, 6=Right");
  Serial.println("You have 60 seconds!");
  Serial.println("Press * to return to menu");
  Serial.println("=====================");
}