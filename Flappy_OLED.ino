/*
  Flappy_OLED.ino
  Minimal Flappy Bird-like example for 128x64 SH1106 OLED.

  Notes:
  - Install Adafruit GFX and Adafruit SH110X via Library Manager.
  - You may need to adjust the display constructor for your SH1106 variant.
*/

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// Pins
const uint8_t BUTTON_PIN = 2; // D2
const uint8_t BUZZER_PIN = 3; // D3

// Display (adjust constructor if your library variant differs)
Adafruit_SH1106 display = Adafruit_SH1106(128, 64, &Wire);

// Game state
int birdY = 32;
float vel = 0;
const float GRAVITY = 0.5;
const float FLAP = -6.0;
const int BIRD_X = 28;

struct Pipe { int x; int gapY; bool scored; };
const int PIPE_COUNT = 3;
Pipe pipes[PIPE_COUNT];
const int GAP = 18;
int score = 0;

unsigned long lastFrame = 0;
const unsigned long FRAME_MS = 40; // ~25 FPS

void setupPipes() {
  int spacing = 48;
  for (int i = 0; i < PIPE_COUNT; i++) {
    pipes[i].x = 128 + i * spacing;
    pipes[i].gapY = random(12, 44);
    pipes[i].scored = false;
  }
}

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  Wire.begin();
  display.begin(0x3C); // common I2C addr; adjust if needed
  display.clearDisplay();

  randomSeed(analogRead(A0));
  setupPipes();
}

bool readButton() {
  static uint32_t lastDebounce = 0;
  static bool lastState = HIGH;
  bool state = digitalRead(BUTTON_PIN);
  if (state != lastState) lastDebounce = millis();
  if ((millis() - lastDebounce) > 30) {
    lastState = state;
  }
  return (state == LOW); // pressed (INPUT_PULLUP)
}

void playTone(uint16_t freq, uint16_t dur) {
  tone(BUZZER_PIN, freq, dur);
}

void gameOver() {
  playTone(150, 200);
  delay(200);
  playTone(100, 300);
  // Reset
  score = 0;
  birdY = 32;
  vel = 0;
  setupPipes();
}

void loop() {
  if (millis() - lastFrame < FRAME_MS) return;
  lastFrame = millis();

  // Input
  if (readButton()) {
    vel = FLAP;
    playTone(1200, 40);
  }

  // Physics
  vel += GRAVITY;
  birdY += (int)vel;
  if (birdY < 0) { birdY = 0; vel = 0; }
  if (birdY > 63) { birdY = 63; vel = 0; }

  // Move pipes
  for (int i = 0; i < PIPE_COUNT; i++) {
    pipes[i].x -= 2;
    if (pipes[i].x < -16) {
      pipes[i].x = 128;
      pipes[i].gapY = random(12, 44);
      pipes[i].scored = false;
    }
    // Score
    if (!pipes[i].scored && pipes[i].x + 8 < BIRD_X) {
      pipes[i].scored = true;
      score++;
      playTone(2000, 30);
    }
  }

  // Collision
  for (int i = 0; i < PIPE_COUNT; i++) {
    int px = pipes[i].x;
    if (px < BIRD_X + 6 && px + 12 > BIRD_X) {
      // Bird horizontally overlaps pipe
      if (birdY < pipes[i].gapY || birdY > pipes[i].gapY + GAP) {
        // collision
        gameOver();
        break;
      }
    }
  }

  // Draw
  display.clearDisplay();
  // Draw pipes
  for (int i = 0; i < PIPE_COUNT; i++) {
    int px = pipes[i].x;
    display.fillRect(px, 0, 12, pipes[i].gapY, SH110X_WHITE);
    display.fillRect(px, pipes[i].gapY + GAP, 12, 64 - (pipes[i].gapY + GAP), SH110X_WHITE);
  }
  // Draw bird (simple circle)
  display.fillCircle(BIRD_X, birdY, 3, SH110X_WHITE);

  // Score
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(96, 2);
  display.print("Score:");
  display.setCursor(116, 2);
  display.print(score);

  display.display();
}
