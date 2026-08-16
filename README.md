# Arduino OLED Flappy Bird

A Flappy Bird-inspired embedded game developed using Arduino, a 128×64 OLED display, a push button, and a buzzer.

## Features

- Flappy Bird-style gameplay
- 128×64 OLED graphics
- Push-button flap control
- Randomly generated obstacles
- Collision detection
- Current score and high score
- Flap, scoring, and crash sound effects
- Animated bird sprite
- Total screen-on time tracking
- Total gameplay time tracking
- Random game-over messages

## Hardware Requirements

| Component          |    Quantity |
| ------------------ | ----------: |
| Arduino board      |           1 |
| 128×64 SH1106 OLED |           1 |
| Push button        |           1 |
| Buzzer             |           1 |
| Jumper wires       | As required |

## Pin Configuration

| Component   | Arduino Pin |
| ----------- | ----------: |
| Flap Button |          D2 |
| Buzzer      |          D3 |
| OLED SDA    |     I²C SDA |
| OLED SCL    |     I²C SCL |

The button uses the Arduino internal `INPUT_PULLUP` configuration.

## Libraries

- Adafruit GFX Library
- Adafruit SH110X Library
- SPI
- Wire

## How It Works

The bird uses momentum-based movement. A button press gives the bird upward momentum, while gravity increases downward momentum. Obstacles continuously move from right to left and are repositioned with randomly generated gaps.

## Game Flow

```text
Initialize OLED
	↓
Initialize Button & Buzzer
	↓
Start Game
	↓
Read Button
	↓
Update Bird Position
	↓
Move Obstacles
	↓
Check Collision
   ↙        ↘
 No         Yes
 ↓           ↓
Update      Game Over
Score       Screen
 ↓
Continue
```

## Wiring Diagram

Simple wiring for an Arduino UNO (I2C OLED):

- OLED VCC → 5V (or 3.3V depending on your module)
- OLED GND → GND
- OLED SDA → A4 (SDA)
- OLED SCL → A5 (SCL)
- Flap Button → D2 (other side → GND). Use `INPUT_PULLUP`.
- Buzzer → D3 (other side → GND)

ASCII wiring (view from component side):

OLED 128x64 Arduino UNO

---

VCC → 5V
GND → GND
SDA → A4 (SDA)
SCL → A5 (SCL)

Button → D2 (other leg to GND)
Buzzer → D3 (other leg to GND)

## Parts / Buy List

- Arduino UNO or compatible board
- 128×64 SH1106 OLED module
- Tactile push button
- Small passive buzzer (or piezo)
- Breadboard and jumper wires

## Example Sketch

An example Arduino sketch is provided as [Flappy_OLED.ino](Flappy_OLED.ino). It demonstrates basic gameplay, drawing to the 128×64 OLED, button input handling (`INPUT_PULLUP`) and simple buzzer feedback. You may need to adjust the display constructor depending on your SH1106 library variant.

Quick usage

1. Install the listed libraries in the Arduino Library Manager: `Adafruit GFX` and `Adafruit SH110X`.
2. Open [Flappy_OLED.ino](Flappy_OLED.ino) in the Arduino IDE.
3. Select your board and upload.
