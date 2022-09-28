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

#define COLOR_RED Adafruit_NeoPixel::Color(255, 0, 0)
#define COLOR_GREEN Adafruit_NeoPixel::Color(0, 255, 0)
#define COLOR_BLUE Adafruit_NeoPixel::Color(0, 0, 255)
#define COLOR_BLACK Adafruit_NeoPixel::Color(0, 0, 0)


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

// Current animation being run.
uint8_t curStepper = 0;
// Steps since the stepper changed
uint32_t stepsSinceChange = 0;

bool whiteToggle = false;
bool redToggle = false;
bool greenToggle = false;
bool blueToggle = false;
bool yellowToggle = false;

reaction animationReaction = INVALID_REACTION;


// Class to step through an animation
class Stepper {

  protected:
  bool setupDone;

  // Animate setup. Returns true when setup is done.
  virtual bool setupStep() = 0;

  // Basic animation step.
  virtual void animationStep() = 0;

  public:
  Stepper() : setupDone(false) { }

  virtual bool resetToSetup() {
    setupDone = false;
  }

  virtual void step() {
    if (setupDone) {
      Serial.println("calling animationStep()");
      animationStep();
    } else {
      Serial.println("calling setupStep()");
      setupDone = setupStep();
      Serial.print("setupDone=");
      Serial.println(setupDone);
    }
  }
};

class TwoColorRotatingStepper : public Stepper {
  uint8_t setupPixel, color1Start, color2Start;
  uint32_t color1, color2;

  public:
  TwoColorRotatingStepper(uint32_t color1, uint32_t color2) :
    setupPixel(0),
    color1Start(0),
    color2Start(NUM_NEOPIXEL_PIXELS / 2),
    color1(color1),
    color2(color2) {}

  virtual bool resetToSetup() {
    setupPixel = 0;
    Stepper::resetToSetup();
  }

  virtual bool setupStep() {
    Serial.print("TwoColorRotationStepper: setupStep pixel ");
    Serial.println(setupPixel);
    // Color half the pixels color1, the other half color2
    eyes.setPixelColor(setupPixel,
        setupPixel < color2Start ? color1 : color2);
    eyes.show();
    setupPixel++;
    return setupPixel >= NUM_NEOPIXEL_PIXELS;
  }

  virtual void animationStep() {
    Serial.println("TwoColorRotationStepper::animationStep()");
    color1Start++;
    color1Start %= NUM_NEOPIXEL_PIXELS; // Have we walked off the end?
    Serial.print("TwoColorRotationStepper: pixel ");
    Serial.print(color1Start);
    Serial.print(" to color1 ");
    Serial.print(color1, HEX);
    eyes.setPixelColor(color1Start, color1);

    color2Start++;
    color2Start %= NUM_NEOPIXEL_PIXELS;
    Serial.print(" pixel ");
    Serial.print(color2Start);
    Serial.print(" to color2 ");
    Serial.println(color2, HEX);
    eyes.setPixelColor(color2Start, color2);

    eyes.show();
  }
};

class OnePixelSpinner : public Stepper {
  uint32_t color;
  uint8_t curPixel;
  public:
  OnePixelSpinner(uint32_t color) : color(color), curPixel(0) {}

  virtual bool setupStep() {
    if (curPixel == 0) {
      eyes.setPixelColor(curPixel, color);
    } else {
      eyes.setPixelColor(curPixel, COLOR_BLACK);
    }
    curPixel++;
    return curPixel >= NUM_NEOPIXEL_PIXELS;
  }

  virtual void animationStep() {
    uint8_t prevPixel = curPixel % NUM_NEOPIXEL_PIXELS;
    ++curPixel %= NUM_NEOPIXEL_PIXELS;
    Serial.print("OnePixelSpinner: pixel ");
    Serial.print(prevPixel);
    Serial.print(" to black and pixel ");
    Serial.print(curPixel);
    Serial.print(" to ");
    Serial.println(color, HEX);
    eyes.setPixelColor(prevPixel, COLOR_BLACK);
    eyes.setPixelColor(curPixel, color);
    eyes.show();
  }
};

Stepper* steppers[] = {
  new TwoColorRotatingStepper(COLOR_RED, COLOR_BLUE),
  new TwoColorRotatingStepper(COLOR_BLUE, COLOR_GREEN),
  new TwoColorRotatingStepper(COLOR_RED, COLOR_GREEN),
  new OnePixelSpinner(COLOR_RED),
  new OnePixelSpinner(COLOR_GREEN)
};

void step() {
  Serial.print("step()ing: curStepper=");
  Serial.print(curStepper);
  stepsSinceChange++;
  Serial.print("stepsSinceChange=");
  Serial.println(stepsSinceChange);

  Stepper* stepper = steppers[curStepper];
  stepper->step();
}

void goWhiteMood() {
  whiteToggle = !whiteToggle;
  Serial.println("white");
  stepsSinceChange = 0;
	digitalWrite(WHITE_MOOD_BUTTON_LED_PIN, whiteToggle ? HIGH : LOW);
  if (curStepper != 0) {
    steppers[0]->resetToSetup();
  }
  curStepper = 0;
}

void goRedMood() {
  redToggle = !redToggle;
  Serial.println("red");
  stepsSinceChange = 0;
	digitalWrite(RED_MOOD_BUTTON_LED_PIN, redToggle ? HIGH : LOW);
  if (curStepper != 1) {
    steppers[1]->resetToSetup();
  }
  curStepper = 1;
}

void goGreenMood() {
  greenToggle = !greenToggle;
  Serial.println("green");
  stepsSinceChange = 0;
	digitalWrite(GREEN_MOOD_BUTTON_LED_PIN, greenToggle ? HIGH : LOW);
  if (curStepper != 2) {
    steppers[2]->resetToSetup();
  }
  curStepper = 2;
}

void goBlueMood() {
  blueToggle = !blueToggle;
  Serial.println("blue");
  stepsSinceChange = 0;
	digitalWrite(BLUE_MOOD_BUTTON_LED_PIN, blueToggle ? HIGH : LOW);
  if (curStepper != 3) {
    steppers[3]->resetToSetup();
  }
  curStepper = 3;
}

void goYellowMood() {
  yellowToggle = !yellowToggle;
  Serial.println("yellow");
  stepsSinceChange = 0;
	digitalWrite(YELLOW_MOOD_BUTTON_LED_PIN, yellowToggle ? HIGH : LOW);
  if (curStepper != 4) {
    steppers[4]->resetToSetup();
  }
  curStepper = 4;
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
