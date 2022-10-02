#include <Reactduino.h>
#include <Adafruit_NeoPixel.h>

#define REPEAT_DELAY_MS 50
#define STEPS_UNTIL_CHANGE 175
#define BUTTON_DEBOUNCE_DELAY_MS 150

#define WHITE_MOOD_BUTTON_LED_PIN 10
#define WHITE_MOOD_BUTTON_SWITCH_PIN 11
#define WHITE_MOOD_BUTTON_SELECTOR_VALUE 4

#define RED_MOOD_BUTTON_LED_PIN 3
#define RED_MOOD_BUTTON_SWITCH_PIN 2
#define RED_MOOD_BUTTON_SELECTOR_VALUE 3

#define GREEN_MOOD_BUTTON_LED_PIN 6
#define GREEN_MOOD_BUTTON_SWITCH_PIN 9
#define GREEN_MOOD_BUTTON_SELECTOR_VALUE 2

#define BLUE_MOOD_BUTTON_LED_PIN 7
#define BLUE_MOOD_BUTTON_SWITCH_PIN 8
#define BLUE_MOOD_BUTTON_SELECTOR_VALUE 1

#define YELLOW_MOOD_BUTTON_LED_PIN 5
#define YELLOW_MOOD_BUTTON_SWITCH_PIN 4
#define YELLOW_MOOD_BUTTON_SELECTOR_VALUE 0

#define EYES_PIN 13
#define NUM_NEOPIXEL_PIXELS 12

#define COLOR_RED Adafruit_NeoPixel::Color(255, 0, 0)
#define COLOR_GREEN Adafruit_NeoPixel::Color(0, 255, 0)
#define COLOR_BLUE Adafruit_NeoPixel::Color(0, 0, 255)
#define COLOR_BLACK Adafruit_NeoPixel::Color(0, 0, 0)
#define COLOR_CYAN Adafruit_NeoPixel::Color(0, 150, 150)
#define COLOR_LIGHT_PURPLE Adafruit_NeoPixel::Color(200, 0, 100)
#define COLOR_WHITE Adafruit_NeoPixel::Color(255, 255, 255)


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

uint8_t buttonToPinMap[5] = {
  /* button0: */ YELLOW_MOOD_BUTTON_LED_PIN,
  /* button1: */ BLUE_MOOD_BUTTON_LED_PIN,
  /* button2: */ GREEN_MOOD_BUTTON_LED_PIN,
  /* button3: */ RED_MOOD_BUTTON_LED_PIN,
  /* button4: */ WHITE_MOOD_BUTTON_LED_PIN,
};

unsigned long lastDebounceTime = 0;

reaction animationReaction = INVALID_REACTION;


// Class to step through an animation
class Stepper {
  protected:
  bool setupDone;

  // Animate setup. Returns true when setup is done.
  virtual bool setupStep() = 0;

  // Basic animation step.
  virtual void animationStep() = 0;

  void setAllPixelsToColor(uint32_t color) {
    for (uint8_t i = 0; i < NUM_NEOPIXEL_PIXELS; i++) {
      eyes.setPixelColor(i, color);
    }
  }

  public:
  Stepper() : setupDone(false) { }

  virtual bool resetToSetup() {
    setupDone = false;
  }

  virtual void step() {
    if (setupDone) {
      animationStep();
    } else {
      setupDone = setupStep();
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
    // Color half the pixels color1, the other half color2
    eyes.setPixelColor(setupPixel,
        setupPixel < color2Start ? color1 : color2);
    eyes.show();
    setupPixel++;
    return setupPixel >= NUM_NEOPIXEL_PIXELS;
  }

  virtual void animationStep() {
    color1Start++;
    color1Start %= NUM_NEOPIXEL_PIXELS; // Have we walked off the end?
    eyes.setPixelColor(color1Start, color1);

    color2Start++;
    color2Start %= NUM_NEOPIXEL_PIXELS;
    eyes.setPixelColor(color2Start, color2);

    eyes.show();
  }
};

// Note: currently, only one illuminated pixel doesn't draw enough power to
// keep that USB battery on.
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
    eyes.show();
    curPixel++;
    return curPixel >= NUM_NEOPIXEL_PIXELS;
  }

  virtual void animationStep() {
    uint8_t prevPixel = curPixel % NUM_NEOPIXEL_PIXELS;
    ++curPixel %= NUM_NEOPIXEL_PIXELS;
    eyes.setPixelColor(prevPixel, COLOR_BLACK);
    eyes.setPixelColor(curPixel, color);
    eyes.show();
  }
};

class TwoPixelSpinner : public Stepper {
  uint32_t color;
  uint8_t curPixel;
  public:
  TwoPixelSpinner(uint32_t color) : color(color), curPixel(0) {}

  virtual bool setupStep() {
    if (curPixel == 0) {
      eyes.setPixelColor(curPixel, color);
    } else {
      eyes.setPixelColor(curPixel, COLOR_BLACK);
    }
    eyes.show();
    curPixel++;
    return curPixel >= NUM_NEOPIXEL_PIXELS;
  }

  virtual void animationStep() {
    uint8_t prevPixel = curPixel % NUM_NEOPIXEL_PIXELS;
    ++curPixel %= NUM_NEOPIXEL_PIXELS;
    eyes.setPixelColor(prevPixel, COLOR_BLACK);
    eyes.setPixelColor(curPixel, color);

    // do again for the opposite side
    uint8_t prevPixelOpp =
      (prevPixel + NUM_NEOPIXEL_PIXELS/2) % NUM_NEOPIXEL_PIXELS;
    uint8_t curPixelOpp =
      (curPixel + NUM_NEOPIXEL_PIXELS/2) % NUM_NEOPIXEL_PIXELS;
    eyes.setPixelColor(prevPixelOpp, COLOR_BLACK);
    eyes.setPixelColor(curPixelOpp, color);
    eyes.show();
  }
};

class TwoPixelTwoColorSpinner : public Stepper {
  uint32_t color1, color2;
  uint8_t curPixel;
  public:
  TwoPixelTwoColorSpinner(uint32_t color1, uint32_t color2) : color1(color1), color2(color2), curPixel(0) {}

  virtual bool setupStep() {
    if (curPixel == 0) {
      uint8_t curPixelOpp = (curPixel + NUM_NEOPIXEL_PIXELS/2) % NUM_NEOPIXEL_PIXELS;
      eyes.setPixelColor(curPixel, color1);
      eyes.setPixelColor(curPixelOpp, color2);
    } else {
      eyes.setPixelColor(curPixel, COLOR_BLACK);
    }
    eyes.show();
    curPixel++;
    return curPixel >= NUM_NEOPIXEL_PIXELS;
  }

  virtual void animationStep() {
    uint8_t prevPixel = curPixel % NUM_NEOPIXEL_PIXELS;
    ++curPixel %= NUM_NEOPIXEL_PIXELS;
    eyes.setPixelColor(prevPixel, COLOR_BLACK);
    eyes.setPixelColor(curPixel, color1);

    // do again for the opposite side
    uint8_t prevPixelOpp =
      (prevPixel + NUM_NEOPIXEL_PIXELS/2) % NUM_NEOPIXEL_PIXELS;
    uint8_t curPixelOpp =
      (curPixel + NUM_NEOPIXEL_PIXELS/2) % NUM_NEOPIXEL_PIXELS;
    eyes.setPixelColor(prevPixelOpp, COLOR_BLACK);
    eyes.setPixelColor(curPixelOpp, color2);
    eyes.show();
  }
};

class FadeInOut : public Stepper {
  uint8_t r, g, b, gamma; 

  public:
  FadeInOut(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b), gamma(0) {}

  virtual bool setupStep() {
    eyes.clear();
    eyes.show();
    return true;
  }

  virtual void animationStep() {
    uint8_t brightness = eyes.sine8(gamma += 10);
    setAllPixelsToColor(
        eyes.Color(r * (brightness / 255.0),
          g * (brightness / 255.0),
          b * (brightness / 255.0)));
    eyes.show();
  }
};

class Sparkle : public Stepper {
  uint8_t numSparkles;
  uint32_t color;

  public:
  Sparkle(uint8_t numSparkles, uint32_t color) : numSparkles(numSparkles), color(color) {
  }

  virtual bool setupStep() {
    eyes.clear();
    eyes.show();
    return true;
  }

  virtual void animationStep() {
    // Turn all pixels on again.
    eyes.clear();
    // Pick some pixels to turn back on.
    for (uint8_t i=0; i < numSparkles; i++) {
      uint8_t pixel = (uint8_t) random(NUM_NEOPIXEL_PIXELS);
      eyes.setPixelColor(pixel, color);
    }
    eyes.show();
  }
};


class Rainbow : public Stepper {
  int16_t speed, hue;
  public:
  Rainbow(int16_t speed) : speed(speed), hue(0) {}

  virtual bool setupStep() {
    eyes.rainbow(hue);
    eyes.show();
  }

  virtual void animationStep() {
    hue += speed;
    eyes.rainbow(hue);
    eyes.show();
  }
};

class TwoCounterRotating : public Stepper {
  uint32_t color1, color2; // best if primary colors: or'd together
  uint8_t loc1 /*cw*/, loc2 /* ccw*/;
  public:
  TwoCounterRotating(uint32_t color1, uint32_t color2) : color1(color1), color2(color2) { }

  virtual bool setupStep() {
    eyes.clear();
    eyes.show();
    return true;
  }

  virtual void animationStep() {
    eyes.clear();
    ++loc1 %= NUM_NEOPIXEL_PIXELS;
    if (loc2 <= 0) {
      loc2 = NUM_NEOPIXEL_PIXELS - 1;
    } else {
      loc2--;
    }

    if (loc1 == loc2) {
      // add the colors together assuming they're primary colors
      eyes.setPixelColor(loc1, color1 | color2);
    } else {
      eyes.setPixelColor(loc1, color1);
      eyes.setPixelColor(loc2, color2);
    }
    eyes.show();
  }
};


// Sweeps 3 active pixels, 3 through 9
class PendulumLeftRight : public Stepper {
  uint32_t color1, color2, color3;
  float pos;
  uint8_t ctr;
  public:
  PendulumLeftRight(uint32_t color1, uint32_t color2, uint32_t color3) :
    color1(color1), color2(color2), color3(color3), pos(0), ctr(0) {}

  void colorActivePixels() {
    eyes.clear();
    eyes.setPixelColor(3 + pos - 1, color1);
    eyes.setPixelColor(3 + pos, color2);
    eyes.setPixelColor(3 + pos + 1, color3);
    eyes.show();
  }

  virtual bool setupStep() {
    colorActivePixels();
    return true;
  }

  virtual void animationStep() {
    eyes.clear();
    uint8_t s = eyes.sine8(ctr+=10);
    pos = s/48.0;
    colorActivePixels();
  }
};

class SpinningFadeInOut : public Stepper {
  uint8_t r, g, b, gamma, mod, offset; 
  uint32_t spinnerColor;

  public:
  SpinningFadeInOut(uint8_t r, uint8_t g, uint8_t b, uint8_t mod, uint32_t spinnerColor) 
    : r(r), g(g), b(b), mod(mod), spinnerColor(spinnerColor), gamma(0), offset(0) {}

  virtual bool setupStep() {
    eyes.clear();
    eyes.show();
    return true;
  }

  virtual void animationStep() {
    uint8_t brightness = eyes.sine8(gamma += 10);
    for (uint8_t i = 0; i < NUM_NEOPIXEL_PIXELS; i++) {
      uint32_t bgColor = eyes.Color(r * (brightness / 255.0),
          g * (brightness / 255.0),
          b * (brightness / 255.0));
      eyes.setPixelColor(i, (i + offset) % mod ? bgColor : spinnerColor);
    }
    ++offset %= NUM_NEOPIXEL_PIXELS;
    eyes.show();
  }
};

// Background fades in/out, one pixel spins
class FadeInOutSpinner : public Stepper {
  uint8_t r, g, b, gamma, curPixel; 
  uint32_t pixelColor;

  public:
  FadeInOutSpinner(uint8_t r, uint8_t g, uint8_t b, uint32_t pixelColor) 
    : r(r), g(g), b(b), pixelColor(pixelColor), gamma(0), curPixel(0) {}

  virtual bool setupStep() {
    eyes.clear();
    eyes.show();
    return true;
  }
  virtual void animationStep() {
    uint8_t brightness = eyes.sine8(gamma += 10);
    uint32_t bgColor = eyes.Color(r * (brightness / 255.0),
        g * (brightness / 255.0),
        b * (brightness / 255.0));
    // Set the background
    setAllPixelsToColor(bgColor);
    // Advance the one pixel
    ++curPixel %= NUM_NEOPIXEL_PIXELS;
    eyes.setPixelColor(curPixel, pixelColor);
    eyes.show();
  }
};

class Blink : public Stepper {
  bool blinked;
  uint32_t bgColor, fgColor;
  public:
  Blink(uint32_t bgColor, uint32_t fgColor) : bgColor(bgColor), fgColor(fgColor), blinked(false) {}
  virtual bool setupStep() {
    setAllPixelsToColor(bgColor);
    eyes.show();
    return true;
  }
  virtual void animationStep() {
    if (!blinked && 95 < random(100)) {
      setAllPixelsToColor(fgColor);
      blinked = true;
    } else {
      setAllPixelsToColor(bgColor);
      blinked = false;
    }
    eyes.show();
  }
};

Stepper* steppers[] = {
  new Rainbow(2000),
  new PendulumLeftRight(COLOR_RED, COLOR_BLUE, COLOR_RED),
  new PendulumLeftRight(COLOR_CYAN, COLOR_LIGHT_PURPLE, COLOR_CYAN),
  new PendulumLeftRight(COLOR_LIGHT_PURPLE, COLOR_GREEN, COLOR_LIGHT_PURPLE),
  new Blink(COLOR_RED, COLOR_WHITE),
  new Blink(COLOR_BLUE, COLOR_GREEN),
  new SpinningFadeInOut(170, 0, 200, 3, COLOR_CYAN),
  new SpinningFadeInOut(0, 255, 170, 4, COLOR_RED),
  new SpinningFadeInOut(100, 0, 150, 3, COLOR_GREEN),
  new TwoColorRotatingStepper(COLOR_RED, COLOR_BLUE),
  new TwoColorRotatingStepper(COLOR_BLUE, COLOR_GREEN),
  new TwoColorRotatingStepper(COLOR_RED, COLOR_GREEN),
  new OnePixelSpinner(COLOR_RED),
  new OnePixelSpinner(COLOR_GREEN),
  new OnePixelSpinner(eyes.Color(0, 150, 150)),
  new FadeInOut(255, 0, 200),
  new FadeInOut(0, 0, 255),
  new Sparkle(3, eyes.Color(255, 0, 0)),
  new Sparkle(1, eyes.Color(255, 255, 255)),
  new TwoPixelSpinner(COLOR_CYAN),
  new TwoPixelSpinner(COLOR_LIGHT_PURPLE),
  new TwoPixelSpinner(COLOR_GREEN),
  new TwoPixelTwoColorSpinner(COLOR_RED, COLOR_BLUE),
  new TwoPixelTwoColorSpinner(COLOR_GREEN, COLOR_BLUE),
  new TwoPixelTwoColorSpinner(COLOR_CYAN, COLOR_LIGHT_PURPLE),
  new TwoCounterRotating(COLOR_RED, COLOR_BLUE),
  new TwoCounterRotating(COLOR_GREEN, COLOR_RED),
  new TwoCounterRotating(eyes.Color(255, 0, 255), eyes.Color(165, 170, 0)),
  new FadeInOutSpinner(0, 255, 0, eyes.Color(255, 0, 255)),
  new FadeInOutSpinner(255, 0, 0, COLOR_BLUE),
  new FadeInOutSpinner(0, 255, 0, COLOR_CYAN),
  new Rainbow(-5000)
};

void step() {
  stepsSinceChange++;
  if (stepsSinceChange >= STEPS_UNTIL_CHANGE) {
    stepsSinceChange = 0;
    curStepper++;
    uint8_t numSteppers = sizeof(steppers) / sizeof(steppers[0]);
    curStepper = curStepper >= numSteppers
      ? curStepper % numSteppers
      : curStepper;

    // Flip the button lights based on the bit pattern of curStepper
    digitalWrite(buttonToPinMap[0], curStepper & 1 ? HIGH : LOW);
    digitalWrite(buttonToPinMap[1], curStepper & (1 <<1) ? HIGH : LOW);
    digitalWrite(buttonToPinMap[2], curStepper & (1 <<2) ? HIGH : LOW);
    digitalWrite(buttonToPinMap[3], curStepper & (1 <<3) ? HIGH : LOW);
    digitalWrite(buttonToPinMap[4], curStepper & (1 <<4) ? HIGH : LOW);
  }

  Stepper* stepper = steppers[curStepper];
  stepper->step();
}

// buttonPressed: expects button 0 through 4
bool moodSelector(uint8_t buttonPressed) {
  uint8_t pin = buttonToPinMap[buttonPressed];
  stepsSinceChange = 0;
  // XOR button toggle
  curStepper ^= (1 << buttonPressed);
  Serial.print("buttonPressed=");
  Serial.print(buttonPressed);
  Serial.print(" pin=");
  Serial.print(pin);
  Serial.print(" stepper=");
  Serial.println(curStepper);
  bool val = curStepper & (1 << buttonPressed);

  // turn on/off the light
	digitalWrite(pin, val ? HIGH : LOW);

  // Start the animation
  steppers[curStepper]->resetToSetup();
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
    long now = millis();
    if (now - lastDebounceTime < BUTTON_DEBOUNCE_DELAY_MS) {
      return;
    }
    lastDebounceTime = now;
    moodSelector(WHITE_MOOD_BUTTON_SELECTOR_VALUE);
  });

  // question: why NoInt? neopixel?
  app.onPinFallingNoInt(RED_MOOD_BUTTON_SWITCH_PIN, [] () {
    long now = millis();
    if (now - lastDebounceTime < BUTTON_DEBOUNCE_DELAY_MS) {
      return;
    }
    lastDebounceTime = now;
    moodSelector(RED_MOOD_BUTTON_SELECTOR_VALUE);
  });

  app.onPinFallingNoInt(GREEN_MOOD_BUTTON_SWITCH_PIN, [] () {
    long now = millis();
    if (now - lastDebounceTime < BUTTON_DEBOUNCE_DELAY_MS) {
      return;
    }
    lastDebounceTime = now;
    moodSelector(GREEN_MOOD_BUTTON_SELECTOR_VALUE);
  });

  app.onPinFallingNoInt(BLUE_MOOD_BUTTON_SWITCH_PIN, [] () {
    long now = millis();
    if (now - lastDebounceTime < BUTTON_DEBOUNCE_DELAY_MS) {
      return;
    }
    lastDebounceTime = now;
    moodSelector(BLUE_MOOD_BUTTON_SELECTOR_VALUE);
  });

  app.onPinFallingNoInt(YELLOW_MOOD_BUTTON_SWITCH_PIN, [] () {
    long now = millis();
    if (now - lastDebounceTime < BUTTON_DEBOUNCE_DELAY_MS) {
      return;
    }
    lastDebounceTime = now;
    moodSelector(YELLOW_MOOD_BUTTON_SELECTOR_VALUE);
  });

  animationReaction = app.repeat(REPEAT_DELAY_MS, [] () {
    step();
  });
  Serial.println("started");
});
