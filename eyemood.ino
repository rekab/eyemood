#include <Reactduino.h>
#include <Adafruit_NeoPixel.h>

#define REPEAT_DELAY_MS 50

#define WHITE_MOOD_BUTTON_LED_PIN 10
#define WHITE_MOOD_BUTTON_SWITCH_PIN 11

#define RED_MOOD_BUTTON_LED_PIN 3
#define RED_MOOD_BUTTON_SWITCH_PIN 2

#define GREEN_MOOD_BUTTON_LED_PIN 6
#define GREEN_MOOD_BUTTON_SWITCH_PIN 9

#define BLUE_MOOD_BUTTON_LED_PIN 7
#define BLUE_MOOD_BUTTON_SWITCH_PIN 8

#define YELLOW_MOOD_BUTTON_LED_PIN 5
#define YELLOW_MOOD_BUTTON_SWITCH_PIN 4

#define EYES_PIN 13
#define NUM_NEOPIXEL_PIXELS 12

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel eyes = Adafruit_NeoPixel(NUM_NEOPIXEL_PIXELS, EYES_PIN, NEO_GRB + NEO_KHZ800);
// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

enum Mood {
  UNKNOWN_MOOD, // Starting state.
  WHITE_MOOD,
  RED_MOOD,
  GREEN_MOOD,
  BLUE_MOOD,
  YELLOW_MOOD
};

Mood curMood = UNKNOWN_MOOD;

uint32_t stepsSinceChange = 0;
uint16_t curPixelNum = 0;
uint32_t curColor = 0;
bool whiteToggle = false;
bool redToggle = false;
bool greenToggle = false;
bool blueToggle = false;
bool yellowToggle = false;

reaction animationReaction = INVALID_REACTION;

// Returns the color of the current pixel based on the mood.
uint32_t getColor() {
  switch (curMood) {
    case WHITE_MOOD:
      return eyes.Color(255, 255, 255);
    case RED_MOOD:
      return eyes.Color(255, 0, 0);
    case GREEN_MOOD:
      return eyes.Color(0, 255, 0);
    case BLUE_MOOD:
      return eyes.Color(0, 0, 255);
    case YELLOW_MOOD:
      return eyes.Color(255, 255, 0);
  }
  return eyes.Color(0, 0, 0);
}

void step() {
  Serial.println("step()ing");
  stepsSinceChange++;
  curPixelNum++;
  if (curPixelNum >= NUM_NEOPIXEL_PIXELS) {
    curPixelNum = 0;
  }
  uint32_t color = getColor();

  Serial.print("setting pixel ");
  Serial.print(curPixelNum);
  Serial.print(" to color ");
  Serial.println(color, HEX);
  eyes.setPixelColor(curPixelNum, color);
  eyes.show();
}

void goWhiteMood() {
  whiteToggle = !whiteToggle;
  Serial.println("white");
  stepsSinceChange = 0;
	digitalWrite(WHITE_MOOD_BUTTON_LED_PIN, whiteToggle ? HIGH : LOW);
  curMood = WHITE_MOOD;
}

void goRedMood() {
  redToggle = !redToggle;
  Serial.println("red");
  stepsSinceChange = 0;
	digitalWrite(RED_MOOD_BUTTON_LED_PIN, redToggle ? HIGH : LOW);
  curMood = RED_MOOD;
}

void goGreenMood() {
  greenToggle = !greenToggle;
  Serial.println("green");
  stepsSinceChange = 0;
	digitalWrite(GREEN_MOOD_BUTTON_LED_PIN, greenToggle ? HIGH : LOW);
  curMood = GREEN_MOOD;
}

void goBlueMood() {
  blueToggle = !blueToggle;
  Serial.println("blue");
  stepsSinceChange = 0;
	digitalWrite(BLUE_MOOD_BUTTON_LED_PIN, blueToggle ? HIGH : LOW);
  curMood = BLUE_MOOD;
}

void goYellowMood() {
  yellowToggle = !yellowToggle;
  Serial.println("yellow");
  stepsSinceChange = 0;
	digitalWrite(YELLOW_MOOD_BUTTON_LED_PIN, yellowToggle ? HIGH : LOW);
  curMood = YELLOW_MOOD;
}

Reactduino app([] () {
  Serial.begin(115200);
  Serial.println("setup()");

  pinMode(WHITE_MOOD_BUTTON_LED_PIN, OUTPUT);
  pinMode(WHITE_MOOD_BUTTON_SWITCH_PIN, INPUT_PULLUP);

  pinMode(RED_MOOD_BUTTON_LED_PIN, OUTPUT);
  pinMode(RED_MOOD_BUTTON_SWITCH_PIN, INPUT_PULLUP);

  pinMode(GREEN_MOOD_BUTTON_LED_PIN, OUTPUT);
  pinMode(GREEN_MOOD_BUTTON_SWITCH_PIN, INPUT_PULLUP);

  pinMode(BLUE_MOOD_BUTTON_LED_PIN, OUTPUT);
  pinMode(BLUE_MOOD_BUTTON_SWITCH_PIN, INPUT_PULLUP);

  pinMode(YELLOW_MOOD_BUTTON_LED_PIN, OUTPUT);
  pinMode(YELLOW_MOOD_BUTTON_SWITCH_PIN, INPUT_PULLUP);

  // Initialize neopixels, all pixels to "off".
  eyes.begin();
  eyes.show();
  
  delay(200); // wait for voltage to stabilize

  app.onPinFallingNoInt(WHITE_MOOD_BUTTON_SWITCH_PIN, [] () {
    Serial.println("white mood button pressed");
    goWhiteMood();
    // Small delay to debounce button presses.
    delay(5);
  });

  // question: why NoInt? neopixel?
  app.onPinFallingNoInt(RED_MOOD_BUTTON_SWITCH_PIN, [] () {
    Serial.println("red mood button pressed");
    goRedMood();
    delay(5);
  });

  app.onPinFallingNoInt(GREEN_MOOD_BUTTON_SWITCH_PIN, [] () {
    Serial.println("green mood button pressed");
    goGreenMood();
    delay(5);
  });

  app.onPinFallingNoInt(BLUE_MOOD_BUTTON_SWITCH_PIN, [] () {
    Serial.println("blue mood button pressed");
    goBlueMood();
    delay(5);
  });

  app.onPinFallingNoInt(YELLOW_MOOD_BUTTON_SWITCH_PIN, [] () {
    Serial.println("yellow mood button pressed");
    goYellowMood();
    delay(5);
  });

  animationReaction = app.repeat(REPEAT_DELAY_MS, [] () {
    step();
  });
  Serial.println("started");
});
