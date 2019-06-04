#include "SevenSegmentClock.h"

static const uint16_t PixelCount = 4*7*3+3;

#define colorSaturation 31

// Seven Segment Layout: 3 LEDs per segment
// order of segments:
//     b
//    ---
//  a|   |c
//    --- d
//  e|   |g
//    ---
//     f
#define SegmentsPerDigit 7
#define LedsPerSegment 3
#define LedsPerDigit (SegmentsPerDigit * LedsPerSegment)
#define SeperatorLeds 3 /* num of leds as seperation between hours/mins */
#define SegOffset_a 0
#define SegOffset_b LedsPerSegment
#define SegOffset_c LedsPerSegment*2
#define SegOffset_d LedsPerSegment*3
#define SegOffset_e LedsPerSegment*4
#define SegOffset_f LedsPerSegment*5
#define SegOffset_g LedsPerSegment*6
static const uint8_t digitOffset[] = { 0, LedsPerDigit, 2*LedsPerDigit+SeperatorLeds, 3*LedsPerDigit+SeperatorLeds };

#define Seg_a 0x01
#define Seg_b 0x02
#define Seg_c 0x04
#define Seg_d 0x08
#define Seg_e 0x10
#define Seg_f 0x20
#define Seg_g 0x40

#define clockSeperatorLed1 (2*LedsPerDigit)
#define clockSeperatorLed2 (2*LedsPerDigit+1)
#define decimalPointLed (2*LedsPerDigit+2)

#define firstCharacterMapped 0x20 /* first char to be mapped is "space" */
#define lastCharacterMapped (sizeof(charMapping) + firstCharacterMapped)

static const unsigned char PROGMEM charMapping[] = {
  /* 0x20, space */ 0,
  /* ! */ 0,
  /* " */ 0,
  /* # */ 0,
  /* $ */ 0,
  /* % */ 0,
  /* & */ 0,
  /* ' */ 0,
  /* ( */ Seg_a + Seg_b + Seg_e + Seg_f,
  /* ) */ Seg_b + Seg_c + Seg_f + Seg_g,
  /* * */ 0,
  /* + */ 0,
  /* , */ 0,
  /* - */ Seg_d,
  /* . */ 0,
  /* / */ Seg_e,
  /* 0 */ Seg_a + Seg_b + Seg_c + Seg_e + Seg_f + Seg_g,
  /* 1 */ Seg_c + Seg_g,
  /* 2 */ Seg_b + Seg_c + Seg_d + Seg_e + Seg_f,
  /* 3 */ Seg_b + Seg_c + Seg_d + Seg_f + Seg_g,
  /* 4 */ Seg_a + Seg_c + Seg_d + Seg_g,
  /* 5 */ Seg_a + Seg_b + Seg_d + Seg_f + Seg_g,
  /* 6 */ Seg_a + Seg_b + Seg_d + Seg_e  + Seg_f + Seg_g,
  /* 7 */ Seg_b + Seg_c + Seg_g,
  /* 8 */ Seg_a + Seg_b + Seg_c + Seg_d + Seg_e + Seg_f + Seg_g,
  /* 9 */ Seg_a + Seg_b + Seg_c + Seg_d + Seg_g,
  /* : */ 0,
  /* ; */ 0,
  /* < */ 0,
  /* = */ Seg_d + Seg_e,
  /* > */ 0,
  /* ? */ 0,
  /* @ */ 0,
  /* A */ Seg_a + Seg_b + Seg_c + Seg_d + Seg_e + Seg_g,
  /* B */ Seg_a + Seg_b + Seg_c + Seg_d + Seg_e + Seg_f + Seg_g,
  /* C */ Seg_a + Seg_b + Seg_e + Seg_f,
  /* D */ Seg_a + Seg_b + Seg_c + Seg_e + Seg_f + Seg_g,
  /* E */ Seg_a + Seg_b + Seg_d + Seg_e + Seg_f,
  /* F */ Seg_a + Seg_b + Seg_d + Seg_e,
  /* G */ Seg_a + Seg_b + Seg_d + Seg_e  + Seg_f + Seg_g,
  /* h */ Seg_a + Seg_d + Seg_e + Seg_g,
  /* I */ Seg_a + Seg_e,
  /* J */ Seg_b + Seg_c + Seg_f + Seg_g,
  /* K */ Seg_a + Seg_c + Seg_d + Seg_e + Seg_g,
  /* L */ Seg_a + Seg_e + Seg_f,
  /* m */ Seg_d + Seg_e + Seg_g,
  /* n */ Seg_d + Seg_e + Seg_g,
  /* o */ Seg_d + Seg_e + Seg_f + Seg_g,
  /* P */ Seg_a + Seg_b + Seg_c + Seg_d + Seg_e,
  /* q */ Seg_a + Seg_b + Seg_c + Seg_d + Seg_g,
  /* r */ Seg_d + Seg_e,
  /* S */ Seg_a + Seg_b + Seg_d + Seg_f + Seg_g,
  /* t */ Seg_a + Seg_d + Seg_e + Seg_f,
  /* U */ Seg_a + Seg_c + Seg_e + Seg_f + Seg_g,
  /* v */ Seg_e + Seg_f + Seg_g,
  /* w */ Seg_e + Seg_f + Seg_g,
  /* X */ Seg_a + Seg_c + Seg_d + Seg_e + Seg_g,
  /* Y */ Seg_a + Seg_c + Seg_d + Seg_g,
  /* Z */ Seg_b + Seg_c + Seg_d + Seg_e + Seg_f,
  /* [ */ Seg_a + Seg_b + Seg_e + Seg_f,
  /* \ */ Seg_a + Seg_d + Seg_g,
  /* ] */ Seg_b + Seg_c + Seg_f + Seg_g,
  /* ^ */ Seg_a + Seg_b + Seg_c,
  /* _ */ Seg_e,
  /* 3 hor. bars */ Seg_b + Seg_d + Seg_e,
  /* 2 hor. bars, top */ Seg_b + Seg_d,
  /* 1 hor. bar, top */ Seg_b,
  /* || */ Seg_a + Seg_c + Seg_e + Seg_g
};

uint8_t SevenSegmentClock::brightness;

void SevenSegmentClock::displaySegment(unsigned int ledAddress, uint32_t color) {
  //Serial.print("displaySegment led="); Serial.print(ledAddress); Serial.print(" color=0x"); Serial.println(color, HEX);
  for (int i=0; i<LedsPerSegment; i++) {
    strip->setPixelColor(ledAddress + i, color);
  }
}

void SevenSegmentClock::displayDigit(unsigned int digitNum, char charToDisplay) {
  unsigned int c = charToDisplay;
  uint32_t color;

  //Serial.print("displayDigit: digitNum="); Serial.print(digitNum); Serial.print(" char=0x"); Serial.println(charToDisplay, HEX);
  if (digitNum < 0 || digitNum > 3) {
    Serial.print("SevenSegmentClock::displayDigit: Invalid digit num ");
    Serial.println(digitNum);
    return;
  }
  int offset = digitOffset[digitNum];
  //Serial.print("1st LED address="); Serial.println(offset);
  if (c < firstCharacterMapped || c > lastCharacterMapped) {
    Serial.print("ERROR: SevenSegmentClock::displayDigit - Cannot display character 0x");
    Serial.print(c, HEX);
    Serial.print(" at digit position ");
    Serial.println(digitNum);
    return;
  }
  c -= firstCharacterMapped;
  //Serial.print("Check char mapping at index="); Serial.println(c);
  unsigned char mapping = pgm_read_byte(charMapping + c);
  //Serial.print("Char mapping="); Serial.println(mapping, HEX);
  color = (mapping & Seg_a) ? currentColor : black;
  displaySegment(offset + SegOffset_a, color);
  color = (mapping & Seg_b) ? currentColor : black;
  displaySegment(offset + SegOffset_b, color);
  color = (mapping & Seg_c) ? currentColor : black;
  displaySegment(offset + SegOffset_c, color);
  color = (mapping & Seg_d) ? currentColor : black;
  displaySegment(offset + SegOffset_d, color);
  color = (mapping & Seg_e) ? currentColor : black;
  displaySegment(offset + SegOffset_e, color);
  color = (mapping & Seg_f) ? currentColor : black;
  displaySegment(offset + SegOffset_f, color);
  color = (mapping & Seg_g) ? currentColor : black;
  displaySegment(offset + SegOffset_g, color);
}

void SevenSegmentClock::displaySeperator(char seperatorCharacter) {
  //Serial.print("displaySeperator: seperator="); Serial.println(seperatorCharacter);
  switch (seperatorCharacter) {
    case '.':
    case ',':
      strip->setPixelColor(decimalPointLed, currentColor);
      strip->setPixelColor(clockSeperatorLed1, black);
      strip->setPixelColor(clockSeperatorLed2, black);
      break;
    case ':':
      strip->setPixelColor(decimalPointLed, black);
      strip->setPixelColor(clockSeperatorLed1, currentColor);
      strip->setPixelColor(clockSeperatorLed2, currentColor);
      break;
      case '|':
        strip->setPixelColor(decimalPointLed, currentColor);
        strip->setPixelColor(clockSeperatorLed1, currentColor);
        strip->setPixelColor(clockSeperatorLed2, currentColor);
        break;
    default:
      Serial.print("SevenSegmentClock::displaySeperator: Unknown character to be displayed: ");
      Serial.println(seperatorCharacter);
    case ' ':
    case 0:
      strip->setPixelColor(decimalPointLed, black);
      strip->setPixelColor(clockSeperatorLed1, black);
      strip->setPixelColor(clockSeperatorLed2, black);
      break;
  }
}


void SevenSegmentClock::displayTime(int hour, int minute)  {
  clockHour = hour;
  clockMinute = minute;
  Serial.print("SevenSegmentClock: new time ");
  Serial.print(clockHour); Serial.print(":"); Serial.println(clockMinute);
  displayUpdate();
};

SevenSegmentClock::BlinkMode SevenSegmentClock::blinkMode;

void SevenSegmentClock::displayUpdate(void) {
  char displayText[4];
  static int lastHour=0, lastMinute=0;
  static uint32_t lastUpdate_ms = 0;
  static uint32_t nextBlinkSwitch_ms = 0;
  static boolean currentlyBlinkOn = false;

  if (clockHour != lastHour || clockMinute != lastMinute || millis()-lastUpdate_ms > TIME_BETWEEN_DISPLAY_UPDATES_ms) {
    lastHour = clockHour;
    lastMinute = clockMinute;
    displayText[0] = (clockHour > 9) ? '0' + (clockHour/10) : ' ';
    displayText[1] = '0' + clockHour % 10;
    displayText[2] = '0' + clockMinute / 10;
    displayText[3] = '0' + clockMinute % 10;
    switch (blinkMode) {
      case NoBlinking:
        displayDigit(0, displayText[0]);
        displayDigit(1, displayText[1]);
        displayDigit(2, displayText[2]);
        displayDigit(3, displayText[3]);
        displaySeperator(':');
        break;
      case ClockBlinking:
        if (currentlyBlinkOn) {
          displayDigit(0, displayText[0]);
          displayDigit(1, displayText[1]);
          displayDigit(2, displayText[2]);
          displayDigit(3, displayText[3]);
          displaySeperator(':');
        } else {
          displayDigit(0, ' ');
          displayDigit(1, ' ');
          displayDigit(2, ' ');
          displayDigit(3, ' ');
          displaySeperator(' ');
        }
        if (millis() > nextBlinkSwitch_ms) {
          currentlyBlinkOn = !currentlyBlinkOn;
          nextBlinkSwitch_ms = millis() + (currentlyBlinkOn ? BLINK_ON_TIME_ms : BLINK_OFF_TIME_ms);
        }
        break;
      case SeperatorBlinking:
        displayDigit(0, displayText[0]);
        displayDigit(1, displayText[1]);
        displayDigit(2, displayText[2]);
        displayDigit(3, displayText[3]);
        if (currentlyBlinkOn) {
          displaySeperator('|');
        } else {
          displaySeperator(' ');
        }
        if (millis() > nextBlinkSwitch_ms) {
          currentlyBlinkOn = !currentlyBlinkOn;
          nextBlinkSwitch_ms = millis() + (currentlyBlinkOn ? BLINK_ON_TIME_ms : BLINK_OFF_TIME_ms);
        }
        break;
      case DecimalPointBlinking:
        displayDigit(0, displayText[0]);
        displayDigit(1, displayText[1]);
        displayDigit(2, displayText[2]);
        displayDigit(3, displayText[3]);
        if (currentlyBlinkOn) {
          displaySeperator('.');
        } else {
          displaySeperator(' ');
        }
        if (millis() > nextBlinkSwitch_ms) {
          currentlyBlinkOn = !currentlyBlinkOn;
          nextBlinkSwitch_ms = millis() + (currentlyBlinkOn ? BLINK_ON_TIME_ms : BLINK_OFF_TIME_ms);
        }
        break;
    }
    strip->show();
    //Serial.print("Shown: "); Serial.print(displayText[0]); Serial.print(displayText[1]);
    //Serial.print(':'); Serial.print(displayText[2]); Serial.println(displayText[3]);
    lastUpdate_ms = millis();
  }
}

uint32_t SevenSegmentClock::red, SevenSegmentClock::green, SevenSegmentClock::blue, SevenSegmentClock::white, SevenSegmentClock::black;
uint8_t SevenSegmentClock::LedDataPin;
Adafruit_NeoPixel *SevenSegmentClock::strip;

void SevenSegmentClock::initColors(uint8_t _brightness) {
  SevenSegmentClock::red = strip->Color(_brightness, 0, 0);
  SevenSegmentClock::green = strip->Color(0, _brightness, 0);
  SevenSegmentClock::blue = strip->Color(0, 0, _brightness);
  SevenSegmentClock::white = strip->Color(_brightness, _brightness, _brightness);
  SevenSegmentClock::black = strip->Color(0, 0, 0);
  SevenSegmentClock::setColor(SevenSegmentClock::getColor()); // reset color to enforce reclaculation
}

void SevenSegmentClock::setColor(Color color) {
  currentColorHandle = color;
  switch (currentColorHandle) {
    case Black: currentColor = SevenSegmentClock::black; break;
    case Blue: currentColor = SevenSegmentClock::blue; break;
    case Red: currentColor = SevenSegmentClock::red; break;
    case Green: currentColor = SevenSegmentClock::green; break;
    case White: currentColor = SevenSegmentClock::white; break;
  }
}

void SevenSegmentClock::begin(void) {
  Serial.println("Init Neopixels ...");
  Serial.print("LED pin="); Serial.println(LedDataPin);
  Serial.print("Pixels="); Serial.println(PixelCount);
  SevenSegmentClock::strip = new Adafruit_NeoPixel(PixelCount, LedDataPin, NEO_GRB + NEO_KHZ800);
  strip->begin();
  strip->clear();
  strip->show();
  initColors(colorSaturation);
  SevenSegmentClock::currentColor = SevenSegmentClock::blue;
  SevenSegmentClock::currentColorHandle = SevenSegmentClock::Blue;
}
