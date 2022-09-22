#include <Reactduino.h>
#include <Adafruit_NeoPixel.h>

#define REPEAT_DELAY_MS 50

#define NUM_NEOPIXEL_PIXELS 12

#define EYES_PIN 7

#define RED_MOOD_BUTTON_LED_PIN 24
#define RED_MOOD_BUTTON_SWITCH_PIN 25
#define GREEN_MOOD_BUTTON_LED_PIN 22
#define GREEN_MOOD_BUTTON_SWITCH_PIN 23
#define BLUE_MOOD_BUTTON_LED_PIN 24
#define BLUE_MOOD_BUTTON_SWITCH_PIN 25

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
  RED_MOOD,
  GREEN_MOOD,
  BLUE_MOOD
};

Mood curMood = UNKNOWN_MOOD;

/*
int redMoodLight = LOW;
int greenMoodLight = LOW;
int blueMoodLight = LOW;

int redMoodButton = HIGH;
int greenMoodButton = HIGH;
int blueMoodButton = HIGH;
*/

uint32_t stepsSinceChange = 0;
uint16_t curPixelNum = 0;

reaction animationReaction = INVALID_REACTION;

// Returns the color of the current pixel based on the mood.
uint32_t getColor() {
  switch (curMood) {
    case RED_MOOD:
      // TODO: look at the current pixel, return different animations
      return eyes.Color(255, 0, 0);
    case GREEN_MOOD:
      return eyes.Color(0, 255, 0);
    case BLUE_MOOD:
      return eyes.Color(0, 0, 255);
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

void goRedMood() {
  if (curMood != RED_MOOD) {
    Serial.println("going red");
    stepsSinceChange = 0;
  }
	digitalWrite(RED_MOOD_BUTTON_LED_PIN, HIGH);
	digitalWrite(GREEN_MOOD_BUTTON_LED_PIN, LOW);
	digitalWrite(BLUE_MOOD_BUTTON_LED_PIN, LOW);
  curMood = RED_MOOD;
}

void goGreenMood() {
  if (curMood != GREEN_MOOD) {
    Serial.println("going green");
    stepsSinceChange = 0;
  }
	digitalWrite(RED_MOOD_BUTTON_LED_PIN, LOW);
	digitalWrite(GREEN_MOOD_BUTTON_LED_PIN, HIGH);
	digitalWrite(BLUE_MOOD_BUTTON_LED_PIN, LOW);
  curMood = GREEN_MOOD;
}

void goBlueMood() {
  if (curMood != BLUE_MOOD) {
    Serial.println("going blue");
    stepsSinceChange = 0;
  }
	digitalWrite(RED_MOOD_BUTTON_LED_PIN, LOW);
	digitalWrite(GREEN_MOOD_BUTTON_LED_PIN, LOW);
	digitalWrite(BLUE_MOOD_BUTTON_LED_PIN, HIGH);
  curMood = BLUE_MOOD;
}

Reactduino app([] () {
  Serial.begin(115200);
  Serial.println("setup()");

  pinMode(RED_MOOD_BUTTON_LED_PIN, OUTPUT);
  //  TODO: verify that 1) the mood switches are open? and 2) open switches are pullups?
  pinMode(RED_MOOD_BUTTON_SWITCH_PIN, INPUT_PULLUP);
  pinMode(GREEN_MOOD_BUTTON_LED_PIN, OUTPUT);
  pinMode(GREEN_MOOD_BUTTON_SWITCH_PIN, INPUT_PULLUP);
  pinMode(BLUE_MOOD_BUTTON_LED_PIN, OUTPUT);
  pinMode(BLUE_MOOD_BUTTON_SWITCH_PIN, INPUT_PULLUP);

  // Initialize neopixels, all pixels to "off".
  eyes.begin();
  eyes.show();
  
  delay(200); // wait for voltage to stabilize

  // question: why NoInt? neopixel?
  app.onPinFallingNoInt(RED_MOOD_BUTTON_SWITCH_PIN, [] () {
    Serial.println("red mood button pressed");
    goRedMood();
  });

  app.onPinFallingNoInt(GREEN_MOOD_BUTTON_SWITCH_PIN, [] () {
    Serial.println("green mood button pressed");
    goGreenMood();
  });

  app.onPinFallingNoInt(BLUE_MOOD_BUTTON_SWITCH_PIN, [] () {
    Serial.println("blue mood button pressed");
    goBlueMood();
  });

  animationReaction = app.repeat(REPEAT_DELAY_MS, [] () {
    step();
  });
});
