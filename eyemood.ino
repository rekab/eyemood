#include <Reactduino.h>
#include <Adafruit_NeoPixel.h>

#define REPEAT_DELAY_MS 50

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
      //Serial.println("calling animationStep()");
      animationStep();
    } else {
      //Serial.println("calling setupStep()");
      setupDone = setupStep();
      //Serial.print("setupDone=");
      //Serial.println(setupDone);
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
    //Serial.println("TwoColorRotationStepper::animationStep()");
    color1Start++;
    color1Start %= NUM_NEOPIXEL_PIXELS; // Have we walked off the end?
    Serial.print(color1, HEX);
    eyes.setPixelColor(color1Start, color1);

    color2Start++;
    color2Start %= NUM_NEOPIXEL_PIXELS;
    Serial.println(color2, HEX);
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
    Serial.println("OnePixelSpinner setup()");
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
    //Serial.println("OnePixelSpinner animationStep()");
    uint8_t prevPixel = curPixel % NUM_NEOPIXEL_PIXELS;
    ++curPixel %= NUM_NEOPIXEL_PIXELS;
    //Serial.print("OnePixelSpinner: pixel ");
    //Serial.print(prevPixel);
    //Serial.print(" to black and pixel ");
    //Serial.print(curPixel);
    //Serial.print(" to ");
    //Serial.println(color, HEX);
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
    Serial.println("TwoPixelSpinner setup()");
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
    //Serial.println("TwoPixelSpinner animationStep()");
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
    Serial.println("TwoPixelTwoColorSpinner setup()");
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
    //Serial.println("TwoPixelSpinner animationStep()");
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
    Serial.println("FadeInOut setup()");
    eyes.clear();
    eyes.show();
    return true;
  }

  virtual void animationStep() {
    //Serial.println("FadeInOut animationStep()");
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
    Serial.println("Sparkle setup()");
    eyes.clear();
    eyes.show();
    return true;
  }

  virtual void animationStep() {
    //Serial.println("Sparkle animationStep()");
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
    //Serial.println("Rainbow setup()");
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
    Serial.println("TwoCounterRotating setup()");
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


/*
// Sweeps 3 active pixels, 3 through 9
class PendulumLeftRight : public Stepper {
  uint32_t color;
  uint8_t activePixel;
  public:
  PendulumLeftRight(uint32_t color) : color(color), sweep(6) {}
  virtual bool setupStep() {
    setAllPixelsToColor(COLOR_BLACK);
    colorActivePixels();
    eyes.show();
    return true;
  }

  void colorActivePixels() {
    eyes.setPixelColor(activePixel - 1, color);
    eyes.setPixelColor(activePixel, color);
    eyes.setPixelColor(activePixel + 1, color);
  }

  virtual void animationStep() {
    setAllPixelsToColor(COLOR_BLACK);
    colorActivePixels();
    activePixel = 
    // how many cycles we skip is determined by sweep
  }
};

*/

// something
class SpinningFadeInOut : public Stepper {
  uint8_t r, g, b, gamma, mod, offset; 
  uint32_t spinnerColor;

  public:
  SpinningFadeInOut(uint8_t r, uint8_t g, uint8_t b, uint8_t mod, uint32_t spinnerColor) 
    : r(r), g(g), b(b), mod(mod), spinnerColor(spinnerColor), gamma(0), offset(0) {}

  virtual bool setupStep() {
    Serial.println("SpinningFadeInOut setup()");
    eyes.clear();
    eyes.show();
    return true;
  }

  virtual void animationStep() {
    //Serial.println("FadeInOut animationStep()");
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
    Serial.println("FadeInOutSpinner setup()");
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
    Serial.println("Blink setup()");
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
  new Blink(COLOR_RED, COLOR_WHITE),
  new Blink(COLOR_BLUE, COLOR_GREEN),
  new SpinningFadeInOut(170, 0, 200, 3, COLOR_CYAN),
  new SpinningFadeInOut(0, 255, 170, 4, COLOR_RED),
  new SpinningFadeInOut(100, 0, 150, 3, COLOR_GREEN),
  new Rainbow(2000),
  new Rainbow(-5000),
  new TwoColorRotatingStepper(COLOR_RED, COLOR_BLUE),
  new TwoColorRotatingStepper(COLOR_BLUE, COLOR_GREEN),
  new TwoColorRotatingStepper(COLOR_RED, COLOR_GREEN),
  new OnePixelSpinner(COLOR_RED),
  new OnePixelSpinner(COLOR_GREEN),
  new OnePixelSpinner(eyes.Color(0, 150, 150)),
  new FadeInOut(255, 0, 200),
  new FadeInOut(0, 0, 255),
  new Sparkle(3, eyes.Color(255, 0, 0)),
  new Sparkle(2, eyes.Color(200, 165, 0)),
  new Sparkle(1, eyes.Color(255, 255, 255)),
  new TwoPixelSpinner(COLOR_CYAN),
  new TwoPixelSpinner(COLOR_LIGHT_PURPLE),
  new TwoPixelSpinner(COLOR_GREEN),
  new TwoPixelTwoColorSpinner(COLOR_RED, COLOR_BLUE),
  new TwoPixelTwoColorSpinner(COLOR_GREEN, eyes.Color(200, 165, 0)),
  new TwoPixelTwoColorSpinner(COLOR_CYAN, COLOR_LIGHT_PURPLE),
  new TwoCounterRotating(COLOR_RED, COLOR_BLUE),
  new TwoCounterRotating(COLOR_GREEN, COLOR_RED),
  new TwoCounterRotating(eyes.Color(255, 0, 255), eyes.Color(165, 170, 0)),
  new FadeInOutSpinner(0, 255, 0, eyes.Color(255, 0, 255)),
  new FadeInOutSpinner(255, 0, 0, COLOR_BLUE),
  new FadeInOutSpinner(0, 255, 0, COLOR_CYAN)
};

void step() {
  stepsSinceChange++;
  curStepper = curStepper >= sizeof(steppers) 
    ? curStepper % sizeof(steppers) 
    : curStepper;

  //Serial.print("step()ing: curStepper=");
  //Serial.print(curStepper);
  //Serial.print(" stepsSinceChange=");
  //Serial.print(stepsSinceChange);
  //Serial.print(" curStepper=");
  //Serial.print(curStepper);

  Stepper* stepper = steppers[curStepper];
  while (NULL == stepper) {
    Serial.print("skipping NULL stepper ");
    Serial.println(curStepper);
    ++curStepper %= sizeof(steppers);
    stepper = steppers[curStepper];
  }

  stepper->step();
}

void goWhiteMood() {
  whiteToggle = !whiteToggle;
  //Serial.println("white");
  stepsSinceChange = 0;
	digitalWrite(WHITE_MOOD_BUTTON_LED_PIN, whiteToggle ? HIGH : LOW);
  ++curStepper %= sizeof(steppers);
  steppers[curStepper]->resetToSetup();
}

void goRedMood() {
  redToggle = !redToggle;
  //Serial.println("red");
  stepsSinceChange = 0;
	digitalWrite(RED_MOOD_BUTTON_LED_PIN, redToggle ? HIGH : LOW);
  ++curStepper %= sizeof(steppers);
  steppers[curStepper]->resetToSetup();
}

void goGreenMood() {
  greenToggle = !greenToggle;
  //Serial.println("green");
  stepsSinceChange = 0;
	digitalWrite(GREEN_MOOD_BUTTON_LED_PIN, greenToggle ? HIGH : LOW);
  ++curStepper %= sizeof(steppers);
  steppers[curStepper]->resetToSetup();
}

void goBlueMood() {
  blueToggle = !blueToggle;
  //Serial.println("blue");
  stepsSinceChange = 0;
	digitalWrite(BLUE_MOOD_BUTTON_LED_PIN, blueToggle ? HIGH : LOW);
  ++curStepper %= sizeof(steppers);
  steppers[curStepper]->resetToSetup();
}

// buttonPressed: expects button 0 through 4
bool moodSelector(uint8_t buttonPressed, uint8_t lightPin) {
  stepsSinceChange = 0;
  // XOR button toggle
  curStepper ^= (1 << buttonPressed);
  Serial.print("stepper=");
  Serial.println(curStepper);
  bool val = curStepper & (1 << buttonPressed);

  // turn on/off the light
	digitalWrite(lightPin, val ? HIGH : LOW);

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
    Serial.println("white");
    goWhiteMood();
    // Small delay to debounce button presses.
    delay(5);
  });

  // question: why NoInt? neopixel?
  app.onPinFallingNoInt(RED_MOOD_BUTTON_SWITCH_PIN, [] () {
    Serial.println("red");
    goRedMood();
    delay(5);
  });

  app.onPinFallingNoInt(GREEN_MOOD_BUTTON_SWITCH_PIN, [] () {
    Serial.println("green");
    goGreenMood();
    delay(5);
  });

  app.onPinFallingNoInt(BLUE_MOOD_BUTTON_SWITCH_PIN, [] () {
    Serial.println("blue");
    goBlueMood();
    delay(5);
  });

  app.onPinFallingNoInt(YELLOW_MOOD_BUTTON_SWITCH_PIN, [] () {
    Serial.println("yellow");
    moodSelector(YELLOW_MOOD_BUTTON_SELECTOR_VALUE, YELLOW_MOOD_BUTTON_LED_PIN);
    delay(5);
  });

  animationReaction = app.repeat(REPEAT_DELAY_MS, [] () {
    step();
  });
  Serial.println("started");
});
