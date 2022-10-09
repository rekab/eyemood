# eyemood

A thing to animate NeoPixel ring(s) with 12 LEDs.

Takes 5 input buttons (white, red, green, blue, yellow) that have LED
lights.

Together these represent the bits in a 5-bit integer, which is used
as an index of 32 different animations ("Steppers").

Each of the buttons have an LED that's lit based on if their bit is
a 1 (on) or 0 (off). This bit pattern changes by counting upwards by one
every ~8.7 seconds. It wraps back to 0 after 31.

## Requirements

### Hardware

* Tested on an Arduino UNO R3
* Two [NeoPixel 12 LED Rings](https://www.adafruit.com/product/1643)
* A 1000µF (6.3V or higher) capacitor on the power to the NeoPixel rings.
* A 470Ω resistor on the data pin to the NeoPixel rings.
* Five buttons. I used 5 diffent colors of
  [Mini LED Arcade Button - 24mm](https://www.adafruit.com/product/3432)
* [Micro USB pinboard](https://www.amazon.com/Pinboard-MELIFE-Interface-Adapter-Breakout/dp/B07W6T97HZ/)
  for powering the NeoPixel rings

### Arduino Libraries

* Reactduino (https://github.com/Reactduino/Reactduino)
* Adafruit Neopixel (1.10.5 https://github.com/adafruit/Adafruit\_NeoPixel)


### Wiring and Pins

|  Color |    Purpose    | Pin Number |
|:------:|:-------------:|:----------:|
| Black  | white switch  | 11         |
| White  | white LED     | 10         |
| Gray   | green switch  | 9          |
| Purple | blue switch   | 8          |
| Blue   | blue LED      | 7          |
| Green  | green LED     | 6          |
| Yellow | yellow LED    | 5          |
| Orange | yellow switch | 4          |
| Red    | red LED       | 3          |
| Brown  | red switch    | 2          |

External power [must be connected to common ground](https://learn.adafruit.com/adafruit-neopixel-uberguide/basic-connections).

![Fritzing wiring diagram](https://github.com/rekab/eyemood/blob/initial-setup/fritzing-wiring-screenshot.png?raw=true)
