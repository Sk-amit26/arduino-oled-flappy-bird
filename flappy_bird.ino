#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SH1106G display(128, 64, &Wire, -1);

// Pins
#define FLAP_BUTTON 2
#define BUZZER 3

#define SPRITE_HEIGHT 16
#define SPRITE_WIDTH 16
#define GAME_SPEED 50

int game_state = 1;
int score = 0;
int high_score = 0;

int bird_x = SCREEN_WIDTH / 4;
int bird_y;
int momentum = 0;

int wall_x[2];
int wall_y[2];

int wall_gap = 30;
int wall_width = 10;

// ⏱ TIME TRACKING
unsigned long startTime = 0;
unsigned long totalScreenTime = 0;

unsigned long gameStartTime = 0;
unsigned long totalPlayTime = 0;

// 🐦 BITMAPS
static const unsigned char PROGMEM wing_down_bmp[] = {
  B00000000,B00000000,B00000000,B00000000,B00000011,B11000000,
  B00011111,B11110000,B00111111,B00111000,B01111111,B11111110,
  B11111111,B11000001,B11011111,B01111110,B11011111,B01111000,
  B11011111,B01111000,B11001110,B01111000,B11110001,B11110000,
  B01111111,B11100000,B00111111,B11000000,B00000111,B00000000,
  B00000000,B00000000
};

static const unsigned char PROGMEM wing_up_bmp[] = {
  B00000000,B00000000,B00000000,B00000000,B00000011,B11000000,
  B00011111,B11110000,B00111111,B00111000,B01110001,B11111110,
  B11101110,B11000001,B11011111,B01111110,B11011111,B01111000,
  B11111111,B11111000,B11111111,B11111000,B11111111,B11110000,
  B01111111,B11100000,B00111111,B11000000,B00000111,B00000000,
  B00000000,B00000000
};

// 🔊 SOUND
unsigned long lastToneTime = 0;
int toneStep = 0;

void scoreSoundNonBlocking() {
  unsigned long now = millis();
  if (toneStep == 1 && now - lastToneTime > 40) {
    tone(BUZZER, 1400);
    lastToneTime = now;
    toneStep = 2;
  } else if (toneStep == 2 && now - lastToneTime > 40) {
    tone(BUZZER, 1800, 40);
    toneStep = 0;
  }
}

void startScoreSound() {
  tone(BUZZER, 1000);
  lastToneTime = millis();
  toneStep = 1;
}

void flapSound() {
  tone(BUZZER, 1500, 30);
}

void crashSound() {
  for (int f = 1000; f > 300; f -= 150) {
    tone(BUZZER, f, 40);
    delay(20);
  }
}

// 🐦 ANIMATION
unsigned long flapTimer = 0;
bool flapState = false;

// 💬 MESSAGES
const char* messages[] = {
  "Better luck!",
  "Try again!",
  "So close!",
  "Keep going!",
  "Nice try!"
};

const int msgCount = sizeof(messages) / sizeof(messages[0]);
const char* currentMessage = "";

// TEXT CENTER
void textCenter(int y, const char* txt) {
  int x = SCREEN_WIDTH / 2 - strlen(txt) * 3;
  display.setCursor(x, y);
  display.print(txt);
}

// SETUP
void setup() {
  display.begin(0x3C, true);
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);

  pinMode(FLAP_BUTTON, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);

  randomSeed(analogRead(0));

  startTime = millis();
}

// LOOP
void loop() {

  totalScreenTime = millis() - startTime;

  if (game_state == 0) {

    display.clearDisplay();

    if (digitalRead(FLAP_BUTTON) == LOW) {
      momentum = -4;
      flapSound();
    }

    momentum++;
    bird_y += momentum;

    if (bird_y < 0) bird_y = 0;
    if (bird_y > SCREEN_HEIGHT - SPRITE_HEIGHT) {
      bird_y = SCREEN_HEIGHT - SPRITE_HEIGHT;
      momentum = -2;
    }

    if (millis() - flapTimer > 120) {
      flapState = !flapState;
      flapTimer = millis();
    }

    display.drawBitmap(bird_x, bird_y,
      (momentum < 0 && flapState) ? wing_up_bmp : wing_down_bmp,
      16, 16, SH110X_WHITE);

    // WALLS
    for (int i = 0; i < 2; i++) {

      display.fillRect(wall_x[i], 0, wall_width, wall_y[i], SH110X_WHITE);
      display.fillRect(wall_x[i], wall_y[i] + wall_gap, wall_width,
                       SCREEN_HEIGHT - wall_y[i] - wall_gap, SH110X_WHITE);

      int old_x = wall_x[i];
      wall_x[i] -= 4;

      if (wall_x[i] < 0) {
        wall_x[i] = SCREEN_WIDTH;
        wall_y[i] = random(0, SCREEN_HEIGHT - wall_gap);
      }

      if (old_x >= bird_x && wall_x[i] < bird_x) {
        score++;
        if (score > high_score) high_score = score;
        startScoreSound();
      }

      // 💥 COLLISION
      if ((bird_x + SPRITE_WIDTH > wall_x[i] &&
           bird_x < wall_x[i] + wall_width) &&
          (bird_y < wall_y[i] ||
           bird_y + SPRITE_HEIGHT > wall_y[i] + wall_gap)) {

        crashSound();
        currentMessage = messages[random(msgCount)];

        totalPlayTime += millis() - gameStartTime;

        noTone(BUZZER);   // 🔥 FIX: stop buzzer
        toneStep = 0;     // 🔥 FIX: reset sound

        game_state = 1;
      }
    }

    if (toneStep != 0) scoreSoundNonBlocking();

    // SCORE TOP
    char buf[10];
    sprintf(buf, "%d", score);
    textCenter(0, buf);

    display.display();
    delay(GAME_SPEED);

  } else {

    noTone(BUZZER);  // 🔥 EXTRA SAFETY

    display.clearDisplay();

    textCenter(5, "GAME OVER");
    textCenter(18, currentMessage);

    // SCORE + HIGH (SIDE BY SIDE)
    char left[20];
    sprintf(left, "Score:%d", score);
    display.setCursor(0, 30);
    display.print(left);

    char right[20];
    sprintf(right, "High:%d", high_score);
    int rightX = SCREEN_WIDTH - (strlen(right) * 6);
    display.setCursor(rightX, 30);
    display.print(right);

    // PLAY TIME
    int playSec = totalPlayTime / 1000;
    char buf3[20];
    sprintf(buf3, "Play:%02d:%02d", playSec/60, playSec%60);
    textCenter(45, buf3);

    // SCREEN TIME
    int onSec = totalScreenTime / 1000;
    char buf4[25];
    sprintf(buf4, "Screen:%02d:%02d", onSec/60, onSec%60);
    textCenter(55, buf4);

    display.display();

    while (digitalRead(FLAP_BUTTON) == LOW);

    // RESET
    bird_y = SCREEN_HEIGHT / 2;
    momentum = -4;

    wall_x[0] = SCREEN_WIDTH;
    wall_x[1] = SCREEN_WIDTH + SCREEN_WIDTH / 2;

    score = 0;

    while (digitalRead(FLAP_BUTTON) == HIGH);

    gameStartTime = millis();
    game_state = 0;
  }
}