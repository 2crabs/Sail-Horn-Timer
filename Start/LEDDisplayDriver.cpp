
#include <Arduino.h>
#include <inttypes.h>
#include <stdio.h>
#include "LEDDisplayDriver.h"

// Manual for library: http://lygte-info.dk/project/DisplayDriver%20UK.html
// This is version 1.18 from 2021-1-21
// There is nothing to adjust in this module, everything is controlled from the .h file.
// By HKJ from lygte-info.dk


const DIGIT_TYPE segmentPatterns[] = {
  digit0, digit1, digit2, digit3, digit4, digit5, digit6, digit7,
  digit8, digit9, digitA, digitB, digitC, digitD, digitE, digitF
};



const PROGMEM DIGIT_TYPE letterPatterns[] = {
  // Symbols
  digitSpace, digitEmpty, digitQuote2, digitEmpty, digitEmpty, digitEmpty, digitEmpty, digitQuote1,
  digitOpen, digitClose, digitAsterix, digitPlus, digitDp, digitMinus, digitDp, digitSlash,
  // Numbers
  digit0, digit1, digit2, digit3, digit4, digit5, digit6, digit7,
  digit8, digit9, digitEmpty, digitEmpty, digitLT, digitEqual, digitGT, digitEmpty,
  // Upper case alphabet
  digitEmpty, digitA, digitB, digitC, digitD, digitE, digitF, digitG,
  digitH, digitI, digitJ, digitK, digitL, digitM, digitN, digit0,
  digitP, digitQ, digitR, digitS, digitT, digitU, digitV, digitW,
  digitX, digitY, digitZ, digitOpen, digitBackslash, digitClose, digitEmpty, digitUnderscore,
  // Lower case alphabet
  digitEmpty, digita, digitb, digitc, digitd, digite, digitf, digitg,
  digith, digiti, digitj, digitk, digitl, digitm, digitn, digito,
  digitp, digitq, digitr, digits, digitt, digitu, digitv, digitw,
  digitx, digity, digitz, digitOpen, digitVerticalslash, digitClose, digitEmpty, digitEmpty
};


// This function has two different headers, depending on what is enabled
#ifdef _EXTRA_PINS_
LEDDisplayDriver::LEDDisplayDriver(byte data, byte clock, byte load, byte *extraPins, boolean autoUpdate, byte digits) {
  this->extraPins = extraPins;
#else
LEDDisplayDriver::LEDDisplayDriver(byte data, byte clock, __attribute__((unused)) byte load, boolean autoUpdate, byte digits) {
#endif
  this->autoUpdate = autoUpdate;
  this->digits = digits;
  ack = true;
  on = true;
  dpDigit = 0xff;
  brightness = MAX_BRIGHTNESS / 2;
  clear();
  pinMode(clock, OUTPUT);
  pinMode(data, OUTPUT);
#ifdef _USE_LOAD_
  pinMode(load, OUTPUT);
#endif
#ifdef _FAST_CLOCK_
  // With fast clock IO needs a bitmask and a pointer to the ports, that is at 3 bytes for clock and load, but 5 bytes for data
  this->clockPort   = portOutputRegister(digitalPinToPort(clock));
  this->clockBitMask = digitalPinToBitMask(clock);
  this->dataPort   = portOutputRegister(digitalPinToPort(data));
  this->dataBitMask = digitalPinToBitMask(data);
#ifdef _DATA_BIDIRECTIONAL_
  this->dataInPort = portInputRegister(digitalPinToPort(data));
  this->dataDDPort = portModeRegister(digitalPinToPort(data));
#endif
#ifdef _USE_LOAD_
  this->loadPort   = portOutputRegister(digitalPinToPort(load));
  this->loadBitMask = digitalPinToBitMask(load);
#endif
#else
  // When using Arduino IO only the pin number is needed
  this->clockPin = clock;
  this->dataPin = data;
#ifdef _USE_LOAD_
  this->loadPin = load;
#endif
#endif

  init();
}


void LEDDisplayDriver::showDigits(const DIGIT_TYPE data[], byte first, byte count) {
  if (first >= digits) return;
  if (first + count > digits) count = digits - first;
  for (byte i = 0; i < count; i++) {
    this->data[i + first] = data[i];
  }
  if (autoUpdate) update();
}

void LEDDisplayDriver::showDigits(const DIGIT_TYPE data, byte first, byte count) {
  if (first >= digits) return;
  if (first + count > digits) count = digits - first;
  for (byte i = 0; i < count; i++) {
    this->data[i + first] = data;
  }
  if (autoUpdate) update();
}


#ifdef _INCLUDE_BYTE_
// Byte sized routines are not really needed, but will be slightly faster than int versions.

void LEDDisplayDriver::showNum(byte num, byte first, byte count, boolean showLeadingZeros) {
  byte n = 0;
  if (first >= digits) return;
  if (first + count > digits) count = digits - first;
  first = first + count - 1;
  while (n < count && num != 0) {
    data[first--] = segmentPatterns[num % 10];
    num /= 10;
    n++;
  }
  if (n == 0) {
    data[first--] = digit0;
    n++;
  }
  byte prefix = showLeadingZeros ? digit0 : digitEmpty;
  while (n < count) {
    data[first--] = prefix;
    n++;
  }
  if (autoUpdate) update();
}

void LEDDisplayDriver::showHex(byte num, byte first, byte count) {
  if (first >= digits) return;
  if (first + count > digits) count = digits - first;
  first = digits - first - 1;
  for (byte i = 0; i < count; i++) {
    data[first--] = segmentPatterns[num & 0x0f];
    num >>= 4;
  }
  if (autoUpdate) update();
}
#endif



void LEDDisplayDriver::showNum(int num, byte first, byte count, boolean showLeadingZeros) {
  byte n = 0;
  boolean s = false;
  if (num < 0) {
    s = true;
    num = -num;
  }
  if (first >= digits) return;
  if (first + count > digits) count = digits - first;
  first = first + count - 1;
  while (n < count && num != 0) {
    data[first--] = segmentPatterns[num % 10];
    num /= 10;
    n++;
  }
  if (n == 0) {
    data[first--] = digit0;
    n++;
  }
  if (s && n < count) {
    data[first--] = digitMinus;
    n++;
    showLeadingZeros = false;
  }
  byte prefix = showLeadingZeros ? digit0 : digitEmpty;
  while (n < count) {
    data[first--] = prefix;
    n++;
  }
  if (autoUpdate) update();
}

void LEDDisplayDriver::showNumWithPoint(int num, int8_t dpPos, byte first, byte count) {
  byte n = 0;
  boolean s = false;
  if (num < 0) {
    s = true;
    num = -num;
  }
  if (first >= digits) return;
  if (first + count > digits) count = digits - first;
  first = first + count - 1;
  while (n < count && num != 0) {
    data[first--] = segmentPatterns[num % 10] | (dpPos == 0 ? digitDp : 0);
    num /= 10;
    dpPos--;
    n++;
  }
  // When the leading position ought to show "-0." and only one digit is left, it will show "-." on that digit.
  while ((n == 0 || dpPos >= 0) && !(s && dpPos == 0 && n == count - 1) && n < count) {
    data[first--] = digit0  | (dpPos == 0 ? digitDp : 0);
    n++;
    dpPos--;
  }
  if (s && n < count) {
    data[first--] = digitMinus | (dpPos == 0 ? digitDp : 0);
    n++;
  }
  while (n < count) {
    data[first--] = digitEmpty;
    n++;
  }
  if (autoUpdate) update();
}

void LEDDisplayDriver::showNumWithPrefix(DIGIT_TYPE prefix, int num, int8_t dpPos, byte first, byte count) {
  if (num < 0) {  // With negative numbers use the minus segment in prefix
    num = -num;
    prefix |= digitMinus;
  }
  if (dpPos == count - 1) {
    prefix |= digitDp;
  }
  data[first] = prefix;
  showNumWithPoint(num, dpPos, first + 1, count - 1);
}


#ifdef _ENABLE_6_DIGITS_
// Support for long, this makes it possible to display numbers above 32767, this is only needed for displays larger than 4 digits

void LEDDisplayDriver::showNumWithPoint(long num, int8_t dpPos, byte first, byte count) {
  byte n = 0;
  boolean s = false;
  if (num < 0) {
    s = true;
    num = -num;
  }
  if (first >= digits) return;
  if (first + count > digits) count = digits - first;
  first = first + count - 1;
  while (n < count && num != 0) {
    data[first--] = segmentPatterns[num % 10] | (dpPos == 0 ? digitDp : 0);
    num /= 10;
    dpPos--;
    n++;
  }
  // When the leading position ought to show "-0." and only one digit is left, it will show "-." on that digit.
  while ((n == 0 || dpPos >= 0) && !(s && dpPos == 0 && n == count - 1) && n < count) {
    data[first--] = digit0  | (dpPos == 0 ? digitDp : 0);
    n++;
    dpPos--;
  }
  if (s && n < count) {
    data[first--] = digitMinus | (dpPos == 0 ? digitDp : 0);
    n++;
  }
  while (n < count) {
    data[first--] = digitEmpty;
    n++;
  }
  if (autoUpdate) update();
}

void LEDDisplayDriver::showNumWithPrefix(DIGIT_TYPE prefix, long num, int8_t dpPos, byte first, byte count) {
  if (num < 0) {  // With negative numbers use the minus segment in prefix
    num = -num;
    prefix |= digitMinus;
  }
  if (dpPos == count - 1) {
    prefix |= digitDp;
  }
  data[first] = prefix;
  showNumWithPoint(num, dpPos, first + 1, count - 1);
}

void LEDDisplayDriver::showNum(long num, byte first, byte count, boolean showLeadingZeros) {
  if (first >= digits) return;
  if (first + count > digits) count = digits - first;
  first = first + count - 1;
  byte n = 0;
  boolean s = false;
  if (num < 0) {
    s = true;
    num = -num;
  }
  while (n < count && num != 0) {
    data[first--] = segmentPatterns[num % 10];
    num /= 10;
    n++;
  }
  if (n == 0) {
    data[first--] = digit0;
    n++;
  }
  if (s && n < count) {
    data[first--] = digitMinus;
    n++;
    showLeadingZeros = false;
  }
  DIGIT_TYPE prefix = showLeadingZeros ? digit0 : digitEmpty;
  while (n < count) {
    data[first--] = prefix;
    n++;
  }
  if (autoUpdate) update();
}
#endif

// Maximum value to show in a number of digits, the .49 is due to rounding
float maxFloat[] = {9.49, 99.49, 999.49, 9999.49, 99999.49, 999999.4, 9999995.0, 99999950.0};

void LEDDisplayDriver::showNum(double num, byte maxDecimals, byte first, byte count) {
  if (num < 0) {  // With negative numbers reserve the first digit for sign
    num = -num;
    data[first] = digitMinus;
    first++;
    count--;
  }
  float mx = maxFloat[count - 1];
  if (num > mx) {   // Overflow
    showMinus(first, count);
    return;
  }
  int8_t dp = 0;
  mx /= 10;
  maxDecimals = min(maxDecimals, count - 1);
  while (dp < maxDecimals  && num < mx) {
    dp++;
    num *= 10;
  }
  // Select display routine depending on available formats, int is limited to show 4 digits correctly
#ifdef _ENABLE_6_DIGITS_
  showNumWithPoint((long) (num + roundValue(num)), dp, first, count);
#else
  showNumWithPoint((int) (num + roundValue(num)), dp, first, count);
#endif
}

void LEDDisplayDriver::showNumWithPrefix(DIGIT_TYPE prefix, double num, byte maxDecimals, byte first, byte count) {
  data[first] = prefix;
  if (num < 0) {  // With negative numbers use the minus segment is in prefix
    num = -num;
    data[first] |= digitMinus;
  }
  float mx = maxFloat[count - 2];
  if (num > mx) {   // Overflow
    showMinus(first, count);
    return;
  }
  int8_t dp = 0;
  mx /= 10;
  maxDecimals = min(maxDecimals, count - 2);
  while (dp <  maxDecimals && num < mx) {
    dp++;
    num *= 10;
  }

  if (num < mx / 10) { // A small number, place the point in the prefix field
    data[first] |= digitDp;
    num *= 10;
    dp++;
  }
  // Select display routine depending on available formats, int is limited to show 4 digits correctly
#ifdef _ENABLE_6_DIGITS_
  showNumWithPoint((long) (num + roundValue(num)), dp, first + 1, count - 1);
#else
  showNumWithPoint((int) (num + roundValue(num)), dp, first + 1, count - 1);
#endif
}

void LEDDisplayDriver::showHex(int num, byte first, byte count) {
  if (first >= digits) return;
  if (first + count > digits) count = digits - first;
  first = digits - first - 1;
  for (byte i = 0; i < count; i++) {
    data[first--] = segmentPatterns[num & 0x0f];
    num >>= 4;
  }
  if (autoUpdate) update();
}

void LEDDisplayDriver::showChar(byte digit, byte c) {
  showCharInternal(digit, c);
  if (autoUpdate) update(digit);
}

void LEDDisplayDriver::showCharInternal(byte digit, byte c) {
  c -= 0x20;
  if (c > 0x5f) c = 0;
#ifdef  SEGMENTS_7
  data[digit] = pgm_read_byte_near(letterPatterns + c);
#else
  data[digit] = pgm_read_word_near(letterPatterns + c);
#endif
}

void LEDDisplayDriver::showText(const String msg, byte first, byte count) {
  if (first >= digits) return;
  if (first + count > digits) count = digits - first;
  for (byte i = 0; i < count; i++) {
    showCharInternal(first++, i < msg.length() ? msg[i] : ' ');
    if (msg[i + 1] == '.' || msg[i + 1] == ',') {
      data[first - 1] |= digitDp;
      i++;
      count++;
    }
  }
  if (autoUpdate) update();
}

#ifdef _INCLUDE_SCROLLING_TEXT_
void LEDDisplayDriver::showTextScroll(const String msg, byte first, byte count) {
  byte end;
  if (count >= msg.length()) {
    end = 0;
  } else {
    end = msg.length() - count;
  }

  for (byte i = 0; i <= end; i++) {
    showText(msg.substring(i, i + count), first, count);
    if (!autoUpdate) update();
    delay(scrollDelay);
  }
}
#endif

void LEDDisplayDriver::showText(const char *msg, byte first, byte count) {
  if (first >= digits) return;
  if (first + count > digits) count = digits - first;
  for (byte i = 0; i < count; i++) {
    showCharInternal(first++, *msg != 0 ? *msg++ : ' ');
    if (*msg == '.' || *msg == ',') {
      data[first - 1] |= digitDp;
      msg++;
      count++;
    }
  }
  if (autoUpdate) update();
}

#ifdef _INCLUDE_SCROLLING_TEXT_
void LEDDisplayDriver::showTextScroll(const char *msg, byte first, byte count) {
  byte end;
  if (count >= strlen(msg)) {
    end = 0;
  } else {
    end = strlen(msg) - count;
  }

  for (byte i = 0; i <= end; i++) {
    showText(msg + i, first, count);
    if (!autoUpdate) update();
    delay(scrollDelay);
  }
}
#endif

void LEDDisplayDriver::showBin(int num, byte format, byte first, byte count) {
  if (first >= digits) return;
  if (first + count > digits) count = digits - first;
  first = digits - first - 1;
  if (format <= 2) {
    // Formats with one digit for each digit
    DIGIT_TYPE d0;
    DIGIT_TYPE d1;
    switch (format) {
      default:  // Using default to avoid warning about uninitialized variable
        d0 = digit0;
        d1 = digit1;
        break;
      case 1:
        d0 = digitSmall0;
        d1 = digitSmall1;
        break;
      case 2:
        d0 = digitSmallTop0;
        d1 = digitSmallTop1;
        break;
    }
    for (byte i = 0; i < count; i++) {
      data[first--] = num & 1 ? d1 : d0;
      num >>= 1;
    }
  } else {
    // Formats with two digits for each digit
    DIGIT_TYPE p1;
    DIGIT_TYPE p2;
    switch (format) {
      default:
        p1 = digitRightBottom1;
        p2 = digitLeftBottom1;
        break;
      case 4:
        p1 = digitRightTop1;
        p2 = digitLeftTop1;
        break;
      case 5:
        p1 = digitRight1;
        p2 = digitLeft1;
        break;
    }
    for (byte i = 0; i < count; i++) {
      data[first--] = (num & 1 ? p1 : 0) | (num & 2 ? p2 : 0);
      num >>= 2;
    }
  }
  if (autoUpdate) update();
}

#ifdef _ENABLE_6_DIGITS_
void LEDDisplayDriver::showHex(long num, byte first, byte count) {
  if (first >= digits) return;
  if (first + count > digits) count = digits - first;
  first = digits - first - 1;
  for (byte i = 0; i < count; i++) {
    data[first--] = segmentPatterns[num & 0x0f];
    num >>= 4;
  }
  if (autoUpdate) update();
}
#endif



//******************************************************************************************************
//******************************************************************************************************
// The LCD API code
//******************************************************************************************************
//******************************************************************************************************


size_t LEDLCDAPIDriver::write(uint8_t c) {
  if (col > 0 && (c == '.' || c == ',')) {
    data[col - 1] |= digitDp;
    if (autoUpdate) LEDDisplayDriver::update((byte) (col - 1));
    return 1;
  }
  if (c == '\n') {
    col = 0;
    return 0;
  }
  if (col == digits) {
    for (byte i = 1; i < digits; i++) {
      data[i - 1] = data[i];
    }
#ifdef _INCLUDE_SCROLLING_TEXT_
    delay(scrollDelay);
#endif
    col--;
    update();
  }
  showChar(col, c);
  if (col < digits) col++;
  return 1;
}

void LEDLCDAPIDriver::clear() {
  showDigits(digitEmpty, 0, digits);
  col = 0;
}


//******************************************************************************************************
//******************************************************************************************************
// Init timer interrupt
// All routines setup a 1kHz interrupt
//******************************************************************************************************
//******************************************************************************************************

#if defined(_USE_TIMER0_)
#if defined(_mega0_)
void LEDDisplayDriver::initInterrupt() {
  noInterrupts();
  TCB0.CCMP = F_CPU / 1000; // 1000 intr/sec
  TCB0.CTRLA = 1; // Enable timer
  TCB0.CTRLB = 0; // Periodic interrupt mode
  TCB0.INTCTRL = 1; // Enable interrupt
  interrupts();
}
#else
#error "Interrupt 1 not supported on this processor"
#endif
#elif defined(_USE_TIMER1_)
#if defined(_uno_) || defined(_mega_) || defined(_U4_)
void LEDDisplayDriver::initInterrupt() {
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  OCR1A = (F_CPU / 1000) - 1; // set compare match register for 1khz
  TCCR1B = (1 << CS10) | (1 << WGM12); // Set for /1 prescaler and CTC mode
  TIMSK1 = (1 << OCIE1A); // enable timer compare interrupt
  interrupts();
}
#elif defined(_mega0_)
void LEDDisplayDriver::initInterrupt() {
  noInterrupts();
  TCB1.CCMP = F_CPU / 1000; // 1000 intr/sec
  TCB1.CTRLA = 1; // Enable timer
  TCB1.CTRLB = 0; // Periodic interrupt mode
  TCB1.INTCTRL = 1; // Enable interrupt
  interrupts();
}
#else
#error "Interrupt 1 not supported on this processor"
#endif
#elif defined(_USE_TIMER2_)
#if defined(_uno_) || defined(_mega_)
void LEDDisplayDriver::initInterrupt() {
  noInterrupts();
  TCCR2A = 0;
  TCCR2B = 0;
  OCR2A = (F_CPU / 64000) - 1; // set compare match register for 1khz
  TCCR2A = (1 << WGM21);  // turn on CTC mode
  TCCR2B = (1 << CS22); // Set for /64 prescaler
  TIMSK2 = (1 << OCIE2A); // enable timer compare interrupt
  interrupts();
}
#elif defined(_mega0_)
void LEDDisplayDriver::initInterrupt() {
  noInterrupts();
  TCB2.CCMP = F_CPU / 1000; // 1000 intr/sec
  TCB2.CTRLA = 1; // Enable timer
  TCB2.CTRLB = 0; // Periodic interrupt mode
  TCB2.INTCTRL = 1; // Enable interrupt
  interrupts();
}
#else
#error "Interrupt 2 not supported on this processor"
#endif
#elif defined(_USE_TIMER3_)
#if defined(_mega_) || defined(_U4_)
void LEDDisplayDriver::initInterrupt() {
  noInterrupts();
  TCCR3A = 0;
  TCCR3B = 0;
  OCR3A = (F_CPU / 1000) - 1; // set compare match register for 1khz
  TCCR3B = (1 << CS30) | (1 << WGM32); // Set for /1 prescaler and CTC mode
  TIMSK3 = (1 << OCIE3A); // enable timer compare interrupt
  interrupts();
}
#elif defined(_mega0_)
void LEDDisplayDriver::initInterrupt() {
  noInterrupts();
  TCB3.CCMP = F_CPU / 1000; // 1000 intr/sec
  TCB3.CTRLA = 1; // Enable timer
  TCB3.CTRLB = 0; // Periodic interrupt mode
  TCB3.INTCTRL = 1; // Enable interrupt
  interrupts();
}
#else
#error "Interrupt 3 not supported on this processor"
#endif
#elif defined(_USE_TIMER4_)
#if defined(_U4_)
void LEDDisplayDriver::initInterrupt() {
  noInterrupts();
  OCR4C = (F_CPU / 64000); // set compare match register for 1khz
  TCCR4A = 0;
  TCCR4B = (1 << CS42) | (1 << CS41); // Prescaler is /32
  TIMSK4 = (1 << TOIE4);
  interrupts();
}
#elif defined(_mega_)
void LEDDisplayDriver::initInterrupt() {
  noInterrupts();
  TCCR4A = 0;
  TCCR4B = 0;
  OCR4A = (F_CPU / 1000) - 1; // set compare match register for 1khz
  TCCR4B = (1 << CS40) | (1 << WGM42); // Set for /1 prescaler and CTC mode
  TIMSK4 = (1 << OCIE4A); // enable timer compare interrupt
  interrupts();
}
#else
#error "Interrupt 4 not supported on this processor"
#endif
#elif defined(_USE_TIMER5_)
#if defined(_mega_)
void LEDDisplayDriver::initInterrupt() {
  noInterrupts();
  TCCR5A = 0;
  TCCR5B = 0;
  OCR5A = (F_CPU / 1000) - 1; // set compare match register for 1khz
  TCCR5B = (1 << CS50) | (1 << WGM52); // Set for /1 prescaler and CTC mode
  TIMSK5 = (1 << OCIE5A); // enable timer compare interrupt
  interrupts();
}
#elif defined(_mega0_)
void LEDDisplayDriver::initInterrupt() {
  noInterrupts();
  TCA0.SINGLE.PER = F_CPU / 1000; // 1000 intr/sec
  TCA0.SINGLE.CTRLA = 1; // Enable timer at clock freq
  TCA0.SINGLE.CTRLB = 0; // Normal mode
  TCA0.SINGLE.INTCTRL = 1; // Enable interrupt
  interrupts();
}
#else
#error "Interrupt 5 not supported on this processor"
#endif

#else
void LEDDisplayDriver::initInterrupt() {
}
#endif
//******************************************************************************************************
//******************************************************************************************************
// The driver code for each display type
//******************************************************************************************************
//******************************************************************************************************
/*

  A driver must implement:
  void LEDDisplayDriver::update();      Called to update all digits on the display, this also includes indicators
  void LEDDisplayDriver::update(byte digit);  Called to update a single digit on the display
  void LEDDisplayDriver::UpdateIntr();      Only needed/used for interrupt based display, then it is called 1000 times each second
  void LEDDisplayDriver::updateSetup(); Called to update brightness and on/off functions
  void LEDDisplayDriver::init();      Called once during initialization

  Optionelly:
  KEY_TYPE LEDDisplayDriver::readKeys();  Called when the user want to read keys, it must return a bitmask of pressed keys

  Support routines can be added to the class header in a #ifdef

  Bytes can optionally be transmitted with:
  shiftOutMsbFirst(byte);   8 bits at once
  shiftOutLsbFirst(byte);   8 bits at once

  Manipulation of IO must be done with:
  clockHigh();clockLow();
  dataHigh();dataLow();
  loadHigh();loadLow();
  clockDelay();
  dataInput();dataOutput();dataRead();    Switch direction and read input

  These will map to digitalWrite or direct port IO depending on #defines

*/


//*****************************************************************************************************
//*****************************************************************************************************
// TM1637
//*****************************************************************************************************
//*****************************************************************************************************

#ifdef _TM1637_

const byte TM1637_COMM_DATA = 0x40;
const byte TM1637_COMM_ADDRESS = 0xC0;
const byte TM1637_COMM_CONTROL = 0x80;

void LEDDisplayDriver::init() {
  clockLow();
  dataLow();
}

void LEDDisplayDriver::updateSetup() {
  if (autoUpdate) update(0);
}


void LEDDisplayDriver::update() {
  delayMicroseconds(3000);
  start();
  writeByte(TM1637_COMM_DATA);
  stop();
  start();
  writeByte(TM1637_COMM_ADDRESS + 0);
  for (byte i = 0; i < digits; i++) {
    if (dpDigit == i) writeByte(data[i] | digitDp);
    else writeByte(data[i]);
  }
  stop();
  start();
  writeByte(TM1637_COMM_CONTROL | brightness | (on ? 0x08 : 0));
  stop();
}

void LEDDisplayDriver::update(byte digit) {
  start();
  writeByte(TM1637_COMM_DATA);
  stop();
  start();
  writeByte(TM1637_COMM_ADDRESS + digit);
  if (dpDigit == digit) writeByte(data[digit] | digitDp);
  else writeByte(data[digit]);
  stop();
  start();
  writeByte(TM1637_COMM_CONTROL | brightness | (on ? 0x08 : 0));
  stop();
}

KEY_TYPE LEDDisplayDriver::readKeys() {
  byte w = 0;
  start();
  writeByte(TM1637_COMM_DATA + 2); // Read keys
  dataInput();
  for (byte i = 0; i < 8; i++) {
    clockLow();
    clockDelay();
    w = (w >> 1) | (dataRead() ? 0 : 0x80);
    clockHigh();
    clockDelay();
  }
  clockLow();    // After this clock the DIO will be pulled low from the chip.
  dataInput();
  dataHigh();   // Pullup to detect missing display
  clockDelay();
  clockHigh();
  clockDelay();
  ack = dataRead() == 0;  // Detect if display is present
  clockLow();  // After this clock the DIO will be released from the chip
  dataOutput();
  clockDelay();
  stop();
  // Convert keys to bitmap
  byte ww = 1 << (w & 0x07);
  if (w & 0x08) return ww;
  if (w & 0x10) return ((unsigned int) ww) << 8;
  return 0;
}


void LEDDisplayDriver::stop() {
  dataOutput();
  dataLow();
  clockDelay();
  clockHigh();
  clockDelay();
  dataHigh();
  clockDelay();
}

void LEDDisplayDriver::start() {
  dataOutput();
  dataLow();
  clockDelay();
}

void LEDDisplayDriver::writeByte(byte b)
{
  dataOutput();
  for (byte i = 0; i < 8; i++) {
    clockLow();
    clockDelay();
    if (b & 1) dataHigh();
    else dataLow();
    clockDelay();
    clockHigh();
    b >>= 1;
    clockDelay();
  }
  clockLow();    // After this clock the DIO will be pulled low from the chip.
  dataInput();
  dataHigh();   // Pullup to detect missing display
  clockDelay();
  clockHigh();
  clockDelay();
  ack = dataRead() == 0;  // Detect if display is present
  clockLow();  // After this clock the DIO will be released from the chip
}
#endif



//*****************************************************************************************************
//*****************************************************************************************************
// TM1637 with colon
//*****************************************************************************************************
//*****************************************************************************************************

#ifdef _TM1637_COLON_

const byte TM1637_COMM_DATA = 0x40;
const byte TM1637_COMM_ADDRESS = 0xC0;
const byte TM1637_COMM_CONTROL = 0x80;

void LEDDisplayDriver::init() {
  clockLow();
  dataLow();
}

void LEDDisplayDriver::updateSetup() {
  if (autoUpdate) update(0);
}


void LEDDisplayDriver::update() {
  delayMicroseconds(3000);
  start();
  writeByte(TM1637_COMM_DATA);
  stop();
  start();
  writeByte(TM1637_COMM_ADDRESS + 0);
  for (byte i = 0; i < digits; i++) {
    if (i == 1) writeByte((data[i] & ~digitDp) | (indicator & COLON_DISP_0 ? digitDp : 0));
    else writeByte(data[i] & ~digitDp);  // Remove any decimal point
  }
  stop();
  start();
  writeByte(TM1637_COMM_CONTROL | brightness | (on ? 0x08 : 0));
  stop();
}

void LEDDisplayDriver::update(byte digit) {
  start();
  writeByte(TM1637_COMM_DATA);
  stop();
  start();
  writeByte(TM1637_COMM_ADDRESS + digit);
  if (dpDigit == digit) writeByte(data[digit] | digitDp);
  else writeByte(data[digit]);
  stop();
  start();
  writeByte(TM1637_COMM_CONTROL | brightness | (on ? 0x08 : 0));
  stop();
}


void LEDDisplayDriver::stop() {
  dataOutput();
  dataLow();
  clockDelay();
  clockHigh();
  clockDelay();
  dataHigh();
  clockDelay();
}

void LEDDisplayDriver::start() {
  dataOutput();
  dataLow();
  clockDelay();
}

void LEDDisplayDriver::writeByte(byte b)
{
  dataOutput();
  for (byte i = 0; i < 8; i++) {
    clockLow();
    clockDelay();
    if (b & 1) dataHigh();
    else dataLow();
    clockDelay();
    clockHigh();
    b >>= 1;
    clockDelay();
  }
  clockLow();    // After this clock the DIO will be pulled low from the chip.
  dataInput();
  dataHigh();   // Pullup to detect missing display
  clockDelay();
  clockHigh();
  clockDelay();
  ack = dataRead() == 0;  // Detect if display is present
  clockLow();  // After this clock the DIO will be released from the chip
}
#endif


//*****************************************************************************************************
//*****************************************************************************************************
// TM1638 board with a STC15W204S for serial communication, this driver uses the TM1638 directly.
//*****************************************************************************************************
//*****************************************************************************************************

#ifdef _TM1638_HWA11_

const byte TM1638_COMM_DATA = 0x40;
const byte TM1638_COMM_ADDRESS = 0xC0;
const byte TM1638_COMM_CONTROL = 0x80;


void LEDDisplayDriver::init() {
  updateSetup();
}

void LEDDisplayDriver::updateSetup() {
  loadLow();
  writeByte(TM1638_COMM_CONTROL | (on ? 0x08 : 0) | brightness);
  loadHigh();
}

void LEDDisplayDriver::update() {
  loadLow();
  writeByte(TM1638_COMM_ADDRESS);  // Address instruction setting
  loadHigh();
  loadLow();
  writeByte(TM1638_COMM_DATA);
  for (byte i = 0; i < 8; i++) {
    writeByte(data[8 - 1 - i] | (dpDigit == i ? digitDp : 0));
    writeByte((indicator >> (i * 2)) & 0x03);
  }
  loadHigh();
}

void LEDDisplayDriver::update(byte digit) {
  loadLow();
  writeByte(TM1638_COMM_ADDRESS + digit * 2);
  loadHigh();
  loadLow();
  writeByte(TM1638_COMM_DATA);
  writeByte(data[digit] | (dpDigit == digit ? digitDp : 0));
  writeByte((indicator >> (digit * 2)) & 0x03);
  loadHigh();
}


KEY_TYPE LEDDisplayDriver::readKeys() {
  KEY_TYPE w = 0;
  loadLow();
  writeByte(TM1638_COMM_ADDRESS);  // Address instruction setting
  loadHigh();
  loadLow();
  writeByte(TM1638_COMM_DATA + 2);  // Input mode
  dataInput();
  for (byte i = 0; i < 4; i++) {
    w = (w << 8) | readByte();
  }
  loadHigh();
  dataOutput();
  return w;
}

byte LEDDisplayDriver::readByte() {
  byte b = 0;
  for (byte i = 0; i < 8; i++) {
    clockLow();
    clockDelay();
    b = b << 1 | dataRead();
    clockHigh();
    clockDelay();
  }
  return b;
}

void LEDDisplayDriver::writeByte(byte b) {
  for (byte i = 0; i < 8; i++) {
    clockLow();
    clockDelay();
    if (b & 1) dataHigh();
    else dataLow();
    clockDelay();
    clockHigh();
    b >>= 1;
    clockDelay();
  }
}
#endif




//*****************************************************************************************************
//*****************************************************************************************************
// TM1638 module QYF-TM1638 from QIFEI
//*****************************************************************************************************
//*****************************************************************************************************

#ifdef _TM1638_QYF_

const byte TM1638_COMM_DATA = 0x40;
const byte TM1638_COMM_ADDRESS = 0xC0;
const byte TM1638_COMM_CONTROL = 0x80;


void LEDDisplayDriver::init() {
  updateSetup();
}

void LEDDisplayDriver::updateSetup() {
  loadLow();
  writeByte(TM1638_COMM_CONTROL | (on ? 0x08 : 0) | brightness);
  loadHigh();
}

void LEDDisplayDriver::update() {
  loadLow();
  writeByte(TM1638_COMM_ADDRESS);  // Address instruction setting
  loadHigh();
  loadLow();
  writeByte(TM1638_COMM_DATA);
  byte mask = 1;
  for (byte i = 0; i < 8; i++) {
    byte d = 0;
    for (byte j = 0; j < 8; j++) {
      d = d >> 1 | ((data[digits - j - 1] & mask) != 0 ? 0x80 : 0);
      if (i == 7 && digits - j - 1 == dpDigit) d |= 0x80;
    }
    mask <<= 1;
    writeByte(d);
    writeByte(0x0);
  }
  loadHigh();
}

void LEDDisplayDriver::update(__attribute__((unused))byte digit) {
  update();
}

KEY_TYPE LEDDisplayDriver::readKeys() {
  KEY_TYPE w = 0;
  loadLow();
  writeByte(TM1638_COMM_ADDRESS);  // Address instruction setting
  loadHigh();
  loadLow();
  writeByte(TM1638_COMM_DATA + 2);  // Input mode
  dataInput();
  for (byte i = 0; i < 4; i++) {
    w = (w << 8) | readByte();
  }
  loadHigh();
  dataOutput();
  return w;
}

byte LEDDisplayDriver::readByte() {
  byte b = 0;
  for (byte i = 0; i < 8; i++) {
    clockLow();
    clockDelay();
    b = b << 1 | dataRead();
    clockHigh();
    clockDelay();
  }
  return b;
}

void LEDDisplayDriver::writeByte(byte b) {
  for (byte i = 0; i < 8; i++) {
    clockLow();
    clockDelay();
    if (b & 1) dataHigh();
    else dataLow();
    clockDelay();
    clockHigh();
    b >>= 1;
    clockDelay();
  }
}
#endif



//*****************************************************************************************************
//*****************************************************************************************************
// TM1638
//*****************************************************************************************************
//*****************************************************************************************************

#ifdef _TM1638_

const byte TM1638_COMM_DATA = 0x40;
const byte TM1638_COMM_ADDRESS = 0xC0;
const byte TM1638_COMM_CONTROL = 0x80;


void LEDDisplayDriver::init() {
  updateSetup();
}

void LEDDisplayDriver::updateSetup() {
  loadLow();
  writeByte(TM1638_COMM_CONTROL | (on ? 0x08 : 0) | brightness);
  loadHigh();
}

void LEDDisplayDriver::update() {
  loadLow();
  writeByte(TM1638_COMM_ADDRESS);  // Address instruction setting
  loadHigh();
  loadLow();
  writeByte(TM1638_COMM_DATA);
  for (byte i = 0; i < 8; i++) {
    writeByte(data[i] | (dpDigit == i ? digitDp : 0));
    writeByte((indicator >> (i * 2)) & 0x03);
  }
  loadHigh();
}

void LEDDisplayDriver::update(byte digit) {
  loadLow();
  writeByte(TM1638_COMM_ADDRESS + digit * 2);
  loadHigh();
  loadLow();
  writeByte(TM1638_COMM_DATA);
  writeByte(data[digit] | (dpDigit == digit ? digitDp : 0));
  writeByte((indicator >> (digit * 2)) & 0x03);
  loadHigh();
}

KEY_TYPE LEDDisplayDriver::readKeys() {
  KEY_TYPE w = 0;
  loadLow();
  writeByte(TM1638_COMM_ADDRESS);  // Address instruction setting
  loadHigh();
  loadLow();
  writeByte(TM1638_COMM_DATA + 2);  // Input mode
  dataInput();
  for (byte i = 0; i < 4; i++) {
    w = (w << 8) | readByte();
  }
  loadHigh();
  dataOutput();
  return w;
}

byte LEDDisplayDriver::readByte() {
  byte b = 0;
  for (byte i = 0; i < 8; i++) {
    clockLow();
    clockDelay();
    b = b << 1 | dataRead();
    clockHigh();
    clockDelay();
  }
  return b;
}

void LEDDisplayDriver::writeByte(byte b) {
  for (byte i = 0; i < 8; i++) {
    clockLow();
    clockDelay();
    if (b & 1) dataHigh();
    else dataLow();
    clockDelay();
    clockHigh();
    b >>= 1;
    clockDelay();
  }
}
#endif


//*****************************************************************************************************
//*****************************************************************************************************
// MAX7219
//*****************************************************************************************************
//*****************************************************************************************************


#ifdef _MAX7219_

const byte MAX7219_DECODEMODE = 9;
const byte MAX7219_BRIGHTNESS  = 10;
const byte MAX7219_SCANLIMIT  = 11;
const byte MAX7219_SHUTDOWN = 12;
const byte MAX7219_DISPLAYTEST = 15;

void LEDDisplayDriver::init() {
  clockLow();
  loadHigh();
  update();
  updateSetup();
}

void LEDDisplayDriver::update() {
  for (byte i = 0; i < 8; i++) update(i);
}

void LEDDisplayDriver::update(byte digit) {
  digit = digit % 8;    // Map digit to 0-7 range, we need to update same position on all chained displays
  loadLow();
  for (int8_t j = (digits - 1) / 8 + 1; j >= 0 ; j--) {
    shiftOutMsbFirst(8 - digit);
    byte n = digit + (j << 3);
    shiftOutMsbFirst(n < digits ? (data[n] | (n == dpDigit ? digitDp : 0)) : 0);
  }
  loadHigh();
}

void LEDDisplayDriver::updateSetup() {
  write(MAX7219_BRIGHTNESS, brightness);
  write(MAX7219_SCANLIMIT, 7);
  write(MAX7219_DECODEMODE, 0);
  write(MAX7219_SHUTDOWN, on ? 1 : 0);
  write(MAX7219_DISPLAYTEST, 0);
}

void LEDDisplayDriver::write(byte commandAndAddress, byte data) {
  loadLow();
  for (byte i = 0; i < (digits - 1) / 8 + 1; i++) {
    shiftOutMsbFirst(commandAndAddress);
    shiftOutMsbFirst(data);
  }
  loadHigh();
}


#endif

//*****************************************************************************************************
//*****************************************************************************************************
// HC595 including INTR routines
//*****************************************************************************************************
//*****************************************************************************************************

#ifdef _HC595_

void LEDDisplayDriver::update() {
}

void LEDDisplayDriver::updateSetup() {
}

void LEDDisplayDriver::update(__attribute__((unused))byte digit) {
}

void LEDDisplayDriver::init() {
  loadLow();
  initInterrupt();
}

void LEDDisplayDriver::updateIntr() {
  shiftOutMsbFirst(on ? ~data[currentDigit] : 0xff);
  shiftOutMsbFirst(0x01 << (digits - currentDigit - 1));
  // 4 digit need 0x08, 0x04, 0x02, 0x01 for selecting the correct digit for data[]
  loadHigh();
  currentDigit = (currentDigit + 1) % digits;
  loadLow();
}

#endif

//*****************************************************************************************************
//*****************************************************************************************************
// HC595R including INTR routines
//*****************************************************************************************************
//*****************************************************************************************************

#ifdef _HC595R_

void LEDDisplayDriver::update() {
}

void LEDDisplayDriver::updateSetup() {
}

void LEDDisplayDriver::update(__attribute__((unused))byte digit) {
}

void LEDDisplayDriver::init() {
  loadLow();
  initInterrupt();
}

void LEDDisplayDriver::updateIntr() {
  shiftOutMsbFirst(on ? ~data[currentDigit] : 0xff);
  shiftOutMsbFirst(0x01 << currentDigit);
  // 4 digit need 0x01, 0x02, 0x04, 0x08 for selecting the correct digit for data[]
  loadHigh();
  currentDigit = (currentDigit + 1) % digits;
  loadLow();
}

#endif


//*****************************************************************************************************
//*****************************************************************************************************
// HC595A including INTR routines
//*****************************************************************************************************
//*****************************************************************************************************

#ifdef _HC595A_

void LEDDisplayDriver::updateSetup() {
}

void LEDDisplayDriver::update() {
}

void LEDDisplayDriver::update(__attribute__((unused))byte digit) {
}

void LEDDisplayDriver::init() {
  loadLow();
  initInterrupt();
}

void LEDDisplayDriver::updateIntr() {
  shiftOutMsbFirst(0x01 << currentDigit);
  shiftOutMsbFirst(on ? ~data[currentDigit] : 0xff);
  loadHigh();
  currentDigit = (currentDigit + 1) % digits;
  loadLow();
}

#endif



//*****************************************************************************************************
//*****************************************************************************************************
// HC164 including INTR routines
//*****************************************************************************************************
//*****************************************************************************************************

#ifdef _HC164_

void LEDDisplayDriver::updateSetup() {
}

void LEDDisplayDriver::update() {
}

void LEDDisplayDriver::update(__attribute__((unused))byte digit) {
}

void LEDDisplayDriver::init() {
  initInterrupt();
}

void LEDDisplayDriver::updateIntr() {
  shiftOutMsbFirst(on ? ~data[currentDigit] : 0xff);
  shiftOutMsbFirst(0x01 << currentDigit);
  currentDigit = (currentDigit + 1) % digits;
}

#endif

//*****************************************************************************************************
//*****************************************************************************************************
// HC595 static
//*****************************************************************************************************
//*****************************************************************************************************


#if defined(_HC595_STATIC_)

void LEDDisplayDriver::update() {

  for (byte i = 0; i < digits; i++) {
    byte j = digits - i - 1;
    shiftOutMsbFirst(on ? ~(data[j] | (j == dpDigit ? digitDp : 0)) : 0xff);
  }
  loadHigh();
  clockDelay();
  loadLow();
}

void LEDDisplayDriver::update(__attribute__((unused))byte digit) {
  update();
}


void LEDDisplayDriver::updateSetup() {
}


void LEDDisplayDriver::init() {

}
#endif

//*****************************************************************************************************
//*****************************************************************************************************
// HT1621 with 6 digit display
//*****************************************************************************************************
//*****************************************************************************************************
#ifdef _HT1621_6D_

// The point is one digit offset on this display, compensate for that and place battery symbol in indicators.
byte LEDDisplayDriver::getDigitData(byte digit) {
  byte d = data[digit] & ~digitDp;
  if ((digit == 3 && data[2]&digitDp) || (digit == 4 && data[3]&digitDp) || (digit == 5 && data[4]&digitDp) || indicator & (1 << digit)) d |= digitDp;
  return d;
}

void LEDDisplayDriver::update() {
  loadLow();
  clockLow();
  write(0b101000000, 9); // Write data from address 0
  for (byte i = 0; i < 6; i++) {
    write(getDigitData(digits - i - 1), 8);
  }
  loadHigh();
}

void LEDDisplayDriver::update(byte digit) {
  loadLow();
  clockLow();
  write(0b101000000 | (digit * 2), 9); // Write data from address digit*2
  write(getDigitData(digits - digit - 1), 8);
  loadHigh();
}

void LEDDisplayDriver::init() {
  writeCmd(0b0101001); // Bias 1/3 4 commons
  updateSetup();
}

void LEDDisplayDriver::write(unsigned int data, byte bits) {
  unsigned int mask = 1 << (bits - 1);
  for (byte i = 0; i < bits; i++) {
    clockLow();
    if ((data & mask) != 0) dataHigh(); else dataLow();
    clockHigh();
    data <<= 1;
  }
}

void LEDDisplayDriver::writeCmd(byte cmd) {
  loadLow();
  clockLow();
  write((0b100 << 9) | (cmd << 1), 12);
  loadHigh();
}

void LEDDisplayDriver::updateSetup() {
  if (on) {
    writeCmd(1); // SYS EN
    writeCmd(3); // LCD ON
  } else {
    writeCmd(0); // SYS DIS
  }
}
#endif


//*****************************************************************************************************
//*****************************************************************************************************
// HT1621 with 12 digit display
//*****************************************************************************************************
//*****************************************************************************************************
#ifdef _HT1621_12D_

void LEDDisplayDriver::update() {
  loadLow();
  clockLow();
  write(0b101000000, 9); // Write data from address 0
  for (byte i = 0; i < 12; i++) {
    write(data[i] | (i == dpDigit ? digitDp : 0), 8);
  }
  write(indicator, 8);
  loadHigh();
}

void LEDDisplayDriver::update(byte digit) {
  loadLow();
  clockLow();
  write(0b101000000 | (digit * 2), 9); // Write data from address digit*2
  write(data[digit] | (digit == dpDigit ? digitDp : 0), 8);
  loadHigh();
}

void LEDDisplayDriver::init() {
  writeCmd(0b0101001); // Bias 1/3 4 commons
  updateSetup();
}

void LEDDisplayDriver::write(unsigned int data, byte bits) {
  unsigned int mask = 1 << (bits - 1);
  for (byte i = 0; i < bits; i++) {
    clockLow();
    if ((data & mask) != 0) dataHigh(); else dataLow();
    clockHigh();
    data <<= 1;
  }
}

void LEDDisplayDriver::writeCmd(byte cmd) {
  loadLow();
  clockLow();
  write((0b100 << 9) | (cmd << 1), 12);
  loadHigh();
}

void LEDDisplayDriver::updateSetup() {
  if (on) {
    writeCmd(1); // SYS EN
    writeCmd(3); // LCD ON
  } else {
    writeCmd(0); // SYS DIS
  }
}
#endif


//*****************************************************************************************************
//*****************************************************************************************************
// HT16K33 with 8 digit display
//*****************************************************************************************************
//*****************************************************************************************************
#ifdef _HT16K33_8D_
#define ADDRESS 0x70

void LEDDisplayDriver::update() {
  for (byte a = 0; a < (digits + 7) / 8; a++) {
    dataLow();      // Start condition
    clockLow();
    writeByte((ADDRESS + a) << 1);
    writeByte(0x00);
    for (byte i = 0; i < 8; i++) {
      writeByte(data[i + (a << 3)] | (dpDigit == (i + (a << 3)) ? digitDp : 0));
      writeByte(0);
    }
    clockHigh();
    dataHigh();   // Stop condition
  }
}

void LEDDisplayDriver::update(byte digit) {
  dataLow();      // Start condition
  clockDelay();
  clockLow();
  writeByte((ADDRESS + digit / 8) << 1);
  writeByte((digit % 8) << 1);
  writeByte(data[digit] | (dpDigit == digit ? digitDp : 0));
  writeByte(0);
  clockHigh();
  dataHigh();   // Stop condition
}

void LEDDisplayDriver::init() {
  dataHigh();
  clockDelay();
  clockHigh();
  for (byte a = 0; a < (digits + 7) / 8; a++) {
    writeCmd( (ADDRESS + a) << 1, 0x21); // System oscillator on
    writeCmd((ADDRESS + a) << 1, 0xa0); // INT/ROW pin is ROW
  }
  updateSetup();
}

void LEDDisplayDriver::updateSetup() {
  for (byte a = 0; a < (digits + 7) / 8; a++) {
    writeCmd((ADDRESS + a) << 1, on ? 0x81 : 0x80); // Display on/off
    writeCmd((ADDRESS + a) << 1, 0xe0 | brightness); // Display brightness
  }
}

void LEDDisplayDriver::writeCmd(byte addr, byte cmd) {
  dataLow();      // Start condition
  clockDelay();
  clockLow();
  writeByte(addr);
  writeByte(cmd);
  clockHigh();
  dataHigh();   // Stop condition
}

void LEDDisplayDriver::writeByte(byte b) {
  shiftOutMsbFirst(b);
  dataInput();
  clockHigh();
  clockDelay();
  ack = dataRead() == 0;
  clockLow();
  clockDelay();
  dataOutput();
}
#endif

//*****************************************************************************************************
//*****************************************************************************************************
// HT16K33 with 4 digit display with colon and points
//*****************************************************************************************************
//*****************************************************************************************************
#ifdef _HT16K33_4D_
#define ADDRESS 0x70

void LEDDisplayDriver::update() {
  for (byte a = 0; a < digits >> 2; a++) {
    dataLow();      // Start condition
    clockLow();
    writeByte((ADDRESS + a) << 1);
    writeByte(0x00);
    for (byte i = 0; i < 5; i++) {
      if (i == 2) {
        // Colon is on bit 1 in 3 digit
        writeByte(indicator & 1 << a ? 0x02 : 0);
      } else {
        writeByte(data[(i < 2 ? i : i - 1) + (a << 2)] | (dpDigit == (i < 2 ? i : i - 1) + (a << 2) ? digitDp : 0));
      }
      writeByte(0);
    }
    clockHigh();
    dataHigh();   // Stop condition
  }
}

void LEDDisplayDriver::update(byte digit) {
  dataLow();      // Start condition
  clockDelay();
  clockLow();
  writeByte((ADDRESS + (digit >> 2)) << 1);
  byte d = (digit % 4);
  writeByte(((d < 2 ? d : d + 1)) << 1);
  writeByte(data[digit] | (dpDigit == digit ? digitDp : 0));
  writeByte(0);
  clockHigh();
  dataHigh();   // Stop condition
}

void LEDDisplayDriver::init() {
  dataHigh();
  clockDelay();
  clockHigh();
  for (int a = 0; a < digits >> 2; a++) {
    writeCmd( (ADDRESS + a) << 1, 0x21); // System oscillator on
    writeCmd((ADDRESS + a) << 1, 0xa0); // INT/ROW pin is ROW
  }
  updateSetup();
}

void LEDDisplayDriver::updateSetup() {
  for (int a = 0; a < digits >> 2 ; a++) {
    writeCmd((ADDRESS + a) << 1, on ? 0x81 : 0x80); // Display on/off
    writeCmd((ADDRESS + a) << 1, 0xe0 | brightness); // Display brightness
  }
}

void LEDDisplayDriver::writeCmd(byte addr, byte cmd) {
  dataLow();      // Start condition
  clockDelay();
  clockLow();
  writeByte(addr);
  writeByte(cmd);
  clockHigh();
  dataHigh();   // Stop condition
}

void LEDDisplayDriver::writeByte(byte b) {
  shiftOutMsbFirst(b);
  dataInput();
  clockHigh();
  clockDelay();
  ack = dataRead() == 0;
  clockLow();
  clockDelay();
  dataOutput();
}
#endif


//*****************************************************************************************************
//*****************************************************************************************************
// HT16K33 with 4 digit 1.2" display with colon and some dots
//*****************************************************************************************************
//*****************************************************************************************************
#ifdef _HT16K33_4DL_
#define ADDRESS 0x70

void LEDDisplayDriver::update() {
  for (byte a = 0; a < digits >> 2; a++) {
    dataLow();      // Start condition
    clockLow();
    writeByte((ADDRESS + a) << 1);
    writeByte(0x00);
    for (byte i = 0; i < 5; i++) {
      if (i == 2) {
        // Colon is on bit 1 in 3 digit
        writeByte(((byte)((indicator >> (a * 4)) & 0x0f)) << 1);
      } else {
        writeByte(data[(i < 2 ? i : i - 1) + (a << 2)] | (dpDigit == (i < 2 ? i : i - 1) + (a << 2) ? digitDp : 0));
      }
      writeByte(0);
    }
    clockHigh();
    dataHigh();   // Stop condition
  }
}

void LEDDisplayDriver::update(byte digit) {
  dataLow();      // Start condition
  clockDelay();
  clockLow();
  writeByte((ADDRESS + (digit >> 2)) << 1);
  byte d = (digit % 4);
  writeByte(((d < 2 ? d : d + 1)) << 1);
  writeByte(data[digit] | (dpDigit == digit ? digitDp : 0));
  writeByte(0);
  clockHigh();
  dataHigh();   // Stop condition
}

void LEDDisplayDriver::init() {
  dataHigh();
  clockDelay();
  clockHigh();
  for (int a = 0; a < digits >> 2; a++) {
    writeCmd( (ADDRESS + a) << 1, 0x21); // System oscillator on
    writeCmd((ADDRESS + a) << 1, 0xa0); // INT/ROW pin is ROW
  }
  updateSetup();
}

void LEDDisplayDriver::updateSetup() {
  for (int a = 0; a < digits >> 2 ; a++) {
    writeCmd((ADDRESS + a) << 1, on ? 0x81 : 0x80); // Display on/off
    writeCmd((ADDRESS + a) << 1, 0xe0 | brightness); // Display brightness
  }
}

void LEDDisplayDriver::writeCmd(byte addr, byte cmd) {
  dataLow();      // Start condition
  clockDelay();
  clockLow();
  writeByte(addr);
  writeByte(cmd);
  clockHigh();
  dataHigh();   // Stop condition
}

void LEDDisplayDriver::writeByte(byte b) {
  shiftOutMsbFirst(b);
  dataInput();
  clockHigh();
  clockDelay();
  ack = dataRead() == 0;
  clockLow();
  clockDelay();
  dataOutput();
}
#endif

//*****************************************************************************************************
//*****************************************************************************************************
// HC595 static
//*****************************************************************************************************
//*****************************************************************************************************


#ifdef _HC595A_STATIC_

void LEDDisplayDriver::update() {
  for (byte i = 0; i < digits; i++) {
    byte j = digits - i - 1;
    shiftOutMsbFirst(on ? (data[j] | (j == dpDigit ? digitDp : 0)) : 0x00);
  }
  loadHigh();
  clockDelay();
  loadLow();
}

void LEDDisplayDriver::update(__attribute__((unused))byte digit) {
  update();
}


void LEDDisplayDriver::updateSetup() {
}


void LEDDisplayDriver::init() {

}
#endif


//*****************************************************************************************************
//*****************************************************************************************************
// HT16K33 with 8 digit 14 segment displays
//*****************************************************************************************************
//*****************************************************************************************************
#ifdef _HT16K33_14SEG_8D_
#define ADDRESS 0x70

void LEDDisplayDriver::update() {
  for (byte a = 0; a < (digits + 7) / 8; a++) {
    dataLow();      // Start condition
    clockLow();
    writeByte((ADDRESS + a) << 1);
    writeByte(0x00);
    for (byte i = 0; i < 8; i++) {
      byte j = i + (a << 3);
      uint16_t w = data[j] | (dpDigit == j ? digitDp : 0);
      writeByte(w);
      writeByte(w >> 8);
    }
    clockHigh();
    dataHigh();   // Stop condition
  }
}

void LEDDisplayDriver::update(byte digit) {
  dataLow();      // Start condition
  clockDelay();
  clockLow();
  writeByte((ADDRESS + digit / 8) << 1);
  writeByte((digit % 8) << 1);
  uint16_t w = data[digit] | (dpDigit == digit ? digitDp : 0);
  writeByte(w);
  writeByte(w >> 8);
  clockHigh();
  dataHigh();   // Stop condition
}

void LEDDisplayDriver::init() {
  dataHigh();
  clockDelay();
  clockHigh();
  for (byte a = 0; a < (digits + 7) / 8; a++) {
    writeCmd((ADDRESS + a) << 1, 0x21); // System oscillator on
    writeCmd((ADDRESS + a) << 1, 0xa0); // INT/ROW pin is ROW
  }
  updateSetup();
}

void LEDDisplayDriver::updateSetup() {
  for (byte a = 0; a < (digits + 7) / 8; a++) {
    writeCmd((ADDRESS + a) << 1, on ? 0x81 : 0x80); // Display on/off
    writeCmd((ADDRESS + a) << 1, 0xe0 | brightness); // Display brightness
  }
}

void LEDDisplayDriver::writeCmd(byte addr, byte cmd) {
  dataLow();      // Start condition
  clockDelay();
  clockLow();
  writeByte(addr);
  writeByte(cmd);
  clockHigh();
  dataHigh();   // Stop condition
}

void LEDDisplayDriver::writeByte(byte b) {
  shiftOutMsbFirst(b);
  dataInput();
  clockHigh();
  clockDelay();
  ack = dataRead() == 0;
  clockLow();
  clockDelay();
  dataOutput();
}
#endif


//*****************************************************************************************************
//*****************************************************************************************************
// HT16K33 with 4 digit 14 segment displays
//*****************************************************************************************************
//*****************************************************************************************************
#ifdef _HT16K33_14SEG_4D_
#define ADDRESS 0x70

void LEDDisplayDriver::update() {
  for (byte a = 0; a < (digits + 3) / 4; a++) {
    dataLow();      // Start condition
    clockLow();
    writeByte((ADDRESS + a) << 1);
    writeByte(0x00);
    for (byte i = 0; i < 8; i++) {
      byte j = i + (a << 2);
      uint16_t w = data[j] | (dpDigit == j ? digitDp : 0);
      writeByte(w);
      writeByte(w >> 8);
    }
    clockHigh();
    dataHigh();   // Stop condition
  }
}

void LEDDisplayDriver::update(byte digit) {
  dataLow();      // Start condition
  clockDelay();
  clockLow();
  writeByte((ADDRESS + digit / 4) << 1);
  writeByte((digit % 4) << 1);
  uint16_t w = data[digit] | (dpDigit == digit ? digitDp : 0);
  writeByte(w);
  writeByte(w >> 8);
  clockHigh();
  dataHigh();   // Stop condition
}

void LEDDisplayDriver::init() {
  dataHigh();
  clockDelay();
  clockHigh();
  for (byte a = 0; a < (digits + 3) / 4; a++) {
    writeCmd((ADDRESS + a) << 1, 0x21); // System oscillator on
    writeCmd((ADDRESS + a) << 1, 0xa0); // INT/ROW pin is ROW
  }
  updateSetup();
}

void LEDDisplayDriver::updateSetup() {
  for (byte a = 0; a < (digits + 3) / 4; a++) {
    writeCmd((ADDRESS + a) << 1, on ? 0x81 : 0x80); // Display on/off
    writeCmd((ADDRESS + a) << 1, 0xe0 | brightness); // Display brightness
  }
}

void LEDDisplayDriver::writeCmd(byte addr, byte cmd) {
  dataLow();      // Start condition
  clockDelay();
  clockLow();
  writeByte(addr);
  writeByte(cmd);
  clockHigh();
  dataHigh();   // Stop condition
}

void LEDDisplayDriver::writeByte(byte b) {
  shiftOutMsbFirst(b);
  dataInput();
  clockHigh();
  clockDelay();
  ack = dataRead() == 0;
  clockLow();
  clockDelay();
  dataOutput();
}
#endif


//*****************************************************************************************************
//*****************************************************************************************************
// TM1637 6 digit display with swapped digits
//*****************************************************************************************************
//*****************************************************************************************************

#ifdef _TM1637_6DX_

const byte TM1637_COMM_DATA = 0x40;
const byte TM1637_COMM_ADDRESS = 0xC0;
const byte TM1637_COMM_CONTROL = 0x80;

const byte digitSequence[] = {2, 1, 0, 5, 4, 3};

void LEDDisplayDriver::init() {
  clockLow();
  dataLow();
}

void LEDDisplayDriver::updateSetup() {
  if (autoUpdate) update(0);
}

void LEDDisplayDriver::update() {
  delayMicroseconds(3000);
  start();
  writeByte(TM1637_COMM_DATA);
  stop();
  start();
  writeByte(TM1637_COMM_ADDRESS + 0);
  for (byte i = 0; i < digits; i++) {
    byte j = digitSequence[i];
    if (dpDigit == j) writeByte(data[j] | digitDp);
    else writeByte(data[j]);
  }
  stop();
  start();
  writeByte(TM1637_COMM_CONTROL | brightness | (on ? 0x08 : 0));
  stop();
}

void LEDDisplayDriver::update(byte digit) {
  byte addr;
  for (addr = 0; addr < 6; addr++) if (addr == digitSequence[digit]) break; // Locate correct address
  start();
  writeByte(TM1637_COMM_DATA);
  stop();
  start();
  writeByte(TM1637_COMM_ADDRESS + addr);
  if (dpDigit == digit) writeByte(data[digit] | digitDp);
  else writeByte(data[digit]);
  stop();
  start();
  writeByte(TM1637_COMM_CONTROL | brightness | (on ? 0x08 : 0));
  stop();
}

void LEDDisplayDriver::stop() {
  dataOutput();
  dataLow();
  clockDelay();
  clockHigh();
  clockDelay();
  dataHigh();
  clockDelay();
}

void LEDDisplayDriver::start() {
  dataOutput();
  dataLow();
  clockDelay();
}

void LEDDisplayDriver::writeByte(byte b)
{
  dataOutput();
  for (byte i = 0; i < 8; i++) {
    clockLow();
    clockDelay();
    if (b & 1) dataHigh();
    else dataLow();
    clockDelay();
    clockHigh();
    b >>= 1;
    clockDelay();
  }
  clockLow();    // After this clock the DIO will be pulled low from the chip.
  dataInput();
  dataHigh();   // Pullup to detect missing display
  clockDelay();
  clockHigh();
  clockDelay();
  ack = dataRead() == 0;  // Detect if display is present
  clockLow();  // After this clock the DIO will be released from the chip
}
#endif

//*****************************************************************************************************
//*****************************************************************************************************
// HT16K33 with 6 digit 14 segment displays, HKJ design
//*****************************************************************************************************
//*****************************************************************************************************
#ifdef _HT16K33_14SEG_HKJ_6D_
#define ADDRESS 0x70

void LEDDisplayDriver::update() {
  for (byte a = 0; a < (digits + 5) / 6; a++) {
    dataLow();      // Start condition
    clockLow();
    writeByte((ADDRESS + a) << 1);
    writeByte(0x00);
    for (byte i = 0; i < 6; i++) {
      byte j = i + (a * 6);
      uint16_t w = data[j] | (dpDigit == j ? digitDp : 0);
      writeByte(w);
      writeByte(w >> 8);
    }
    // Indicators
    if (a == 0) {
      writeByte(indicator);
      writeByte(indicator >> 8);
      writeByte(indicator >> 16);
    } else {
      for (byte i = 0; i < 4; i++) writeByte(0);
    }
    clockHigh();
    dataHigh();   // Stop condition
  }
}

void LEDDisplayDriver::update(byte digit) {
  dataLow();      // Start condition
  clockDelay();
  clockLow();
  writeByte((ADDRESS + digit / 6) << 1);
  writeByte((digit % 6) << 1);
  uint16_t w = data[digit] | (dpDigit == digit ? digitDp : 0);
  writeByte(w);
  writeByte(w >> 8);
  clockHigh();
  dataHigh();   // Stop condition
}

KEY_TYPE LEDDisplayDriver::readKeys() {
  dataLow();      // Start condition
  clockLow();
  writeByte((ADDRESS) << 1);
  writeByte(0x40);    // First key address

  clockHigh();
  dataHigh();   // Stop condition
  clockDelay();
  dataLow();      // Start condition
  clockLow();

  byte b[8];
  writeByte((ADDRESS) << 1 | 1); // Read mode
  for (byte i = 0; i < 8; i++) {
    b[i] = readByte(i == 7);
  }
  clockHigh();
  dataHigh();   // Stop condition
  return b[0];
}

byte LEDDisplayDriver::readByte(boolean last) {
  dataInput();
  byte b = 0;
  for (byte i = 0; i < 8; i++) {
    clockHigh();
    clockDelay();
    b = b << 1 | dataRead();
    clockLow();
    clockDelay();
  }
  dataOutput();
  if (last) {
    clockHigh();
    clockDelay();
    clockLow();
  } else {
    dataLow();
    clockHigh();
    clockDelay();
    clockLow();
  }
  return b;
}


void LEDDisplayDriver::init() {
  dataHigh();
  clockDelay();
  clockHigh();
  for (byte a = 0; a < (digits + 5) / 6; a++) {
    writeCmd((ADDRESS + a) << 1, 0x21); // System oscillator on
    writeCmd((ADDRESS + a) << 1, 0xa0); // INT/ROW pin is ROW
  }
  updateSetup();
}

void LEDDisplayDriver::updateSetup() {
  for (byte a = 0; a < (digits + 5) / 6; a++) {
    writeCmd((ADDRESS + a) << 1, on ? 0x81 : 0x80); // Display on/off
    writeCmd((ADDRESS + a) << 1, 0xe0 | brightness); // Display brightness
  }
}

void LEDDisplayDriver::writeCmd(byte addr, byte cmd) {
  dataLow();      // Start condition
  clockDelay();
  clockLow();
  writeByte(addr);
  writeByte(cmd);
  clockHigh();
  dataHigh();   // Stop condition
}

void LEDDisplayDriver::writeByte(byte b) {
  shiftOutMsbFirst(b);
  dataInput();
  clockHigh();
  clockDelay();
  ack = dataRead() == 0;
  clockLow();
  clockDelay();
  dataOutput();
}

#endif



//*****************************************************************************************************
//*****************************************************************************************************
// HT16K33 with 6 digit 14 segment 8 bit drive displays, HKJ design
//*****************************************************************************************************
//*****************************************************************************************************
#ifdef _HT16K33_14SEG_HKJ_6D8_
#define ADDRESS 0x70

void LEDDisplayDriver::update() {
  for (byte a = 0; a < (digits + 5) / 6; a++) {
    byte aa = a * 6;
    dataLow();      // Start condition
    clockLow();
    writeByte((ADDRESS + a) << 1);
    writeByte(0x00);
    writeByte(data[aa + 0]);
    writeByte(data[aa + 2]);
    writeByte((data[aa + 0] | (dpDigit == aa + 0 ? digitDp : 0)) >> 8);
    writeByte((data[aa + 2] | (dpDigit == aa + 2 ? digitDp : 0)) >> 8);
    writeByte(data[aa + 1]);
    writeByte(data[aa + 3]);
    writeByte((data[aa + 1] | (dpDigit == aa + 1 ? digitDp : 0)) >> 8);
    writeByte((data[aa + 3] | (dpDigit == aa + 3 ? digitDp : 0)) >> 8);
    writeByte(data[aa + 4]);
    writeByte(a == 0 ? indicator : 0);
    writeByte((data[aa + 4] | (dpDigit == aa + 4 ? digitDp : 0)) >> 8);
    writeByte(a == 0 ? indicator >> 8 : 0);
    writeByte(data[aa + 5]);
    writeByte(a == 0 ? indicator >> 16 : 0);
    writeByte((data[aa + 5] | (dpDigit == aa + 5 ? digitDp : 0)) >> 8);
    clockHigh();
    dataHigh();   // Stop condition
  }
}

void LEDDisplayDriver::update(__attribute__((unused))byte digit) {
  update();
}

KEY_TYPE LEDDisplayDriver::readKeys() {
  dataLow();      // Start condition
  clockLow();
  writeByte((ADDRESS) << 1);
  writeByte(0x40);    // First key address

  clockHigh();
  dataHigh();   // Stop condition
  clockDelay();
  dataLow();      // Start condition
  clockLow();

  byte b[8];
  writeByte((ADDRESS) << 1 | 1); // Read mode
  for (byte i = 0; i < 8; i++) {
    b[i] = readByte(i == 7);
  }
  clockHigh();
  dataHigh();   // Stop condition
  return b[0];
}

byte LEDDisplayDriver::readByte(boolean last) {
  dataInput();
  byte b = 0;
  for (byte i = 0; i < 8; i++) {
    clockHigh();
    clockDelay();
    b = b << 1 | dataRead();
    clockLow();
    clockDelay();
  }
  dataOutput();
  if (last) {
    clockHigh();
    clockDelay();
    clockLow();
  } else {
    dataLow();
    clockHigh();
    clockDelay();
    clockLow();
  }
  return b;
}


void LEDDisplayDriver::init() {
  dataHigh();
  clockDelay();
  clockHigh();
  for (byte a = 0; a < (digits + 5) / 6; a++) {
    writeCmd((ADDRESS + a) << 1, 0x21); // System oscillator on
    writeCmd((ADDRESS + a) << 1, 0xa0); // INT/ROW pin is ROW
  }
  updateSetup();
}

void LEDDisplayDriver::updateSetup() {
  for (byte a = 0; a < (digits + 5) / 6; a++) {
    writeCmd((ADDRESS + a) << 1, on ? 0x81 : 0x80); // Display on/off
    writeCmd((ADDRESS + a) << 1, 0xe0 | brightness); // Display brightness
  }
}

void LEDDisplayDriver::writeCmd(byte addr, byte cmd) {
  dataLow();      // Start condition
  clockDelay();
  clockLow();
  writeByte(addr);
  writeByte(cmd);
  clockHigh();
  dataHigh();   // Stop condition
}

void LEDDisplayDriver::writeByte(byte b) {
  shiftOutMsbFirst(b);
  dataInput();
  clockHigh();
  clockDelay();
  ack = dataRead() == 0;
  clockLow();
  clockDelay();
  dataOutput();
}

#endif


//*****************************************************************************************************
//*****************************************************************************************************
// HT16K33 with 4 digit 16 segment displays, HKJ design
//*****************************************************************************************************
//*****************************************************************************************************
#ifdef _HT16K33_16SEG_HKJ_4D_
#define ADDRESS 0x70

void LEDDisplayDriver::update() {
  for (byte a = 0; a < (digits + 3) / 4; a++) {
    dataLow();      // Start condition
    clockLow();
    writeByte((ADDRESS + a) << 1);
    writeByte(0x00);
    for (byte i = 0; i < 4; i++) {
      byte j = i + (a * 4);
      uint16_t w = data[j];
      writeByte(w);
      writeByte(w >> 8);
    }
    // Indicators
    if (a == 0) {
      writeByte(indicator);
      writeByte(indicator >> 8);
    } else {
      writeByte(0);
      writeByte(0);
    }
    clockHigh();
    dataHigh();   // Stop condition
  }
}

void LEDDisplayDriver::update(byte digit) {
  dataLow();      // Start condition
  clockDelay();
  clockLow();
  writeByte((ADDRESS + digit / 4) << 1);
  writeByte((digit % 4) << 1);
  uint16_t w = data[digit];
  writeByte(w);
  writeByte(w >> 8);
  clockHigh();
  dataHigh();   // Stop condition
}

KEY_TYPE LEDDisplayDriver::readKeys() {
  dataLow();      // Start condition
  clockLow();
  writeByte((ADDRESS) << 1);
  writeByte(0x40);    // First key address

  clockHigh();
  dataHigh();   // Stop condition
  clockDelay();
  dataLow();      // Start condition
  clockLow();

  byte b[8];
  writeByte((ADDRESS) << 1 | 1); // Read mode
  for (byte i = 0; i < 8; i++) {
    b[i] = readByte(i == 7);
  }
  clockHigh();
  dataHigh();   // Stop condition
  return b[0];
}

byte LEDDisplayDriver::readByte(boolean last) {
  dataInput();
  byte b = 0;
  for (byte i = 0; i < 8; i++) {
    clockHigh();
    clockDelay();
    b = b << 1 | dataRead();
    clockLow();
    clockDelay();
  }
  dataOutput();
  if (last) {
    clockHigh();
    clockDelay();
    clockLow();
  } else {
    dataLow();
    clockHigh();
    clockDelay();
    clockLow();
  }
  return b;
}


void LEDDisplayDriver::init() {
  dataHigh();
  clockDelay();
  clockHigh();
  for (byte a = 0; a < (digits + 3) / 4; a++) {
    writeCmd((ADDRESS + a) << 1, 0x21); // System oscillator on
    writeCmd((ADDRESS + a) << 1, 0xa0); // INT/ROW pin is ROW
  }
  updateSetup();
}

void LEDDisplayDriver::updateSetup() {
  for (byte a = 0; a < (digits + 3) / 4; a++) {
    writeCmd((ADDRESS + a) << 1, on ? 0x81 : 0x80); // Display on/off
    writeCmd((ADDRESS + a) << 1, 0xe0 | brightness); // Display brightness
  }
}

void LEDDisplayDriver::writeCmd(byte addr, byte cmd) {
  dataLow();      // Start condition
  clockDelay();
  clockLow();
  writeByte(addr);
  writeByte(cmd);
  clockHigh();
  dataHigh();   // Stop condition
}

void LEDDisplayDriver::writeByte(byte b) {
  shiftOutMsbFirst(b);
  dataInput();
  clockHigh();
  clockDelay();
  ack = dataRead() == 0;
  clockLow();
  clockDelay();
  dataOutput();
}

#endif

//*****************************************************************************************************
//*****************************************************************************************************
// HT16K33 with 4 digit 16 segment with extern DP control displays, HKJ design
//*****************************************************************************************************
//*****************************************************************************************************
#ifdef _HT16K33_16SEG_HKJ_4DW_
#define ADDRESS 0x70

void LEDDisplayDriver::update() {
  byte *p = extraPins;
  for (byte a = 0; a < (digits + 3) / 4; a++) {
    dataLow();      // Start condition
    clockLow();
    writeByte((ADDRESS + a) << 1);
    writeByte(0x00);
    for (byte i = 0; i < 4; i++) {
      byte j = i + (a * 4);
      uint32_t w = data[j];
      writeByte(w);
      writeByte(w >> 8);

      if (p != NULL) {
        if (*p != 255) {
          digitalWrite(*p, (w & digitDp) || (dpDigit == j)  ? HIGH : LOW);
        }
        p++;
      }
    }
    // Indicators
    if (a == 0) {
      writeByte(indicator);
      writeByte(indicator >> 8);
    } else {
      writeByte(0);
      writeByte(0);
    }
    clockHigh();
    dataHigh();   // Stop condition
  }
}

void LEDDisplayDriver::update(byte digit) {
  dataLow();      // Start condition
  clockDelay();
  clockLow();
  writeByte((ADDRESS + digit / 4) << 1);
  writeByte((digit % 4) << 1);
  uint16_t w = data[digit];
  writeByte(w);
  writeByte(w >> 8);
  clockHigh();
  dataHigh();   // Stop condition
}

KEY_TYPE LEDDisplayDriver::readKeys() {
  dataLow();      // Start condition
  clockLow();
  writeByte((ADDRESS) << 1);
  writeByte(0x40);    // First key address

  clockHigh();
  dataHigh();   // Stop condition
  clockDelay();
  dataLow();      // Start condition
  clockLow();

  byte b[8];
  writeByte((ADDRESS) << 1 | 1); // Read mode
  for (byte i = 0; i < 8; i++) {
    b[i] = readByte(i == 7);
  }
  clockHigh();
  dataHigh();   // Stop condition
  return b[0];
}

byte LEDDisplayDriver::readByte(boolean last) {
  dataInput();
  byte b = 0;
  for (byte i = 0; i < 8; i++) {
    clockHigh();
    clockDelay();
    b = b << 1 | dataRead();
    clockLow();
    clockDelay();
  }
  dataOutput();
  if (last) {
    clockHigh();
    clockDelay();
    clockLow();
  } else {
    dataLow();
    clockHigh();
    clockDelay();
    clockLow();
  }
  return b;
}


void LEDDisplayDriver::init() {
  dataHigh();
  clockDelay();
  clockHigh();
  for (byte a = 0; a < (digits + 3) / 4; a++) {
    writeCmd((ADDRESS + a) << 1, 0x21); // System oscillator on
    writeCmd((ADDRESS + a) << 1, 0xa0); // INT/ROW pin is ROW
  }
  byte *p = extraPins;
  if (p != NULL) {
    for (byte i = 0; i < digits; i++) {
      if (*p != 255) {
        pinMode(*p, OUTPUT);
        digitalWrite(*p, LOW);
      }
      p++;
    }
  }
  updateSetup();
}

void LEDDisplayDriver::updateSetup() {
  for (byte a = 0; a < (digits + 3) / 4; a++) {
    writeCmd((ADDRESS + a) << 1, on ? 0x81 : 0x80); // Display on/off
    writeCmd((ADDRESS + a) << 1, 0xe0 | brightness); // Display brightness
  }
}

void LEDDisplayDriver::writeCmd(byte addr, byte cmd) {
  dataLow();      // Start condition
  clockDelay();
  clockLow();
  writeByte(addr);
  writeByte(cmd);
  clockHigh();
  dataHigh();   // Stop condition
}

void LEDDisplayDriver::writeByte(byte b) {
  shiftOutMsbFirst(b);
  dataInput();
  clockHigh();
  clockDelay();
  ack = dataRead() == 0;
  clockLow();
  clockDelay();
  dataOutput();
}

#endif


//*****************************************************************************************************
//*****************************************************************************************************
// HT16K33 with 4 digit, indicators and button support, HKJ design
//*****************************************************************************************************
//*****************************************************************************************************
#ifdef _HT16K33_HKJ_4D_
#define ADDRESS 0x70

void LEDDisplayDriver::update() {
  for (byte a = 0; a < (digits + 3) / 4; a++) {
    dataLow();      // Start condition
    clockLow();
    writeByte((ADDRESS + a) << 1);
    writeByte(0x00);
    for (byte i = 0; i < 4; i++) {
      byte j = i + (a * 4);
      writeByte(data[j] | (dpDigit == j ? digitDp : 0));
      writeByte(a == 0 ? indicator >> (i * 4) : 0);
    }
    clockHigh();
    dataHigh();   // Stop condition
  }
}

void LEDDisplayDriver::update(byte digit) {
  dataLow();      // Start condition
  clockDelay();
  clockLow();
  writeByte((ADDRESS + digit / 4) << 1);
  writeByte((digit % 4) << 1);
  writeByte(data[digit] | (dpDigit == digit ? digitDp : 0));
  writeByte(digit < 4 ? indicator >> ((digit % 4) * 4) : 0);
  clockHigh();
  dataHigh();   // Stop condition
}

KEY_TYPE LEDDisplayDriver::readKeys() {
  dataLow();      // Start condition
  clockLow();
  writeByte((ADDRESS) << 1);
  writeByte(0x40);    // First key address

  clockHigh();
  dataHigh();   // Stop condition
  clockDelay();
  dataLow();      // Start condition
  clockLow();

  byte b[8];
  writeByte((ADDRESS) << 1 | 1); // Read mode
  for (byte i = 0; i < 8; i++) {
    b[i] = readByte(i == 7);
  }
  clockHigh();
  dataHigh();   // Stop condition
  return b[0];
}

byte LEDDisplayDriver::readByte(boolean last) {
  dataInput();
  byte b = 0;
  for (byte i = 0; i < 8; i++) {
    clockHigh();
    clockDelay();
    b = b << 1 | dataRead();
    clockLow();
    clockDelay();
  }
  dataOutput();
  if (last) {
    clockHigh();
    clockDelay();
    clockLow();
  } else {
    dataLow();
    clockHigh();
    clockDelay();
    clockLow();
  }
  return b;
}


void LEDDisplayDriver::init() {
  dataHigh();
  clockDelay();
  clockHigh();
  for (byte a = 0; a < (digits + 3) / 4; a++) {
    writeCmd((ADDRESS + a) << 1, 0x21); // System oscillator on
    writeCmd((ADDRESS + a) << 1, 0xa0); // INT/ROW pin is ROW
  }
  updateSetup();
}

void LEDDisplayDriver::updateSetup() {
  for (byte a = 0; a < (digits + 3) / 4; a++) {
    writeCmd((ADDRESS + a) << 1, on ? 0x81 : 0x80); // Display on/off
    writeCmd((ADDRESS + a) << 1, 0xe0 | brightness); // Display brightness
  }
}

void LEDDisplayDriver::writeCmd(byte addr, byte cmd) {
  dataLow();      // Start condition
  clockDelay();
  clockLow();
  writeByte(addr);
  writeByte(cmd);
  clockHigh();
  dataHigh();   // Stop condition
}

void LEDDisplayDriver::writeByte(byte b) {
  shiftOutMsbFirst(b);
  dataInput();
  clockHigh();
  clockDelay();
  ack = dataRead() == 0;
  clockLow();
  clockDelay();
  dataOutput();
}

#endif


//*****************************************************************************************************
//*****************************************************************************************************
// HT16K33 with 6 digit, indicators and button support, HKJ design
//*****************************************************************************************************
//*****************************************************************************************************
#ifdef _HT16K33_HKJ_6D_
#define ADDRESS 0x70

void LEDDisplayDriver::update() {
  for (byte a = 0; a < (digits + 5) / 6; a++) {
    dataLow();      // Start condition
    clockLow();
    writeByte((ADDRESS + a) << 1);
    writeByte(0x00);
    for (byte i = 0; i < 6; i++) {
      byte j = i + (a * 6);
      writeByte(data[j] | (dpDigit == j ? digitDp : 0));
      writeByte(a == 0 ? indicator >> (i * 4) : 0);
    }
    clockHigh();
    dataHigh();   // Stop condition
  }
}

void LEDDisplayDriver::update(byte digit) {
  dataLow();      // Start condition
  clockDelay();
  clockLow();
  writeByte((ADDRESS + digit / 6) << 1);
  writeByte((digit % 6) << 1);
  writeByte(data[digit] | (dpDigit == digit ? digitDp : 0));
  writeByte(digit < 4 ? indicator >> ((digit % 4) * 4) : 0);
  clockHigh();
  dataHigh();   // Stop condition
}

KEY_TYPE LEDDisplayDriver::readKeys() {
  dataLow();      // Start condition
  clockLow();
  writeByte((ADDRESS) << 1);
  writeByte(0x40);    // First key address

  clockHigh();
  dataHigh();   // Stop condition
  clockDelay();
  dataLow();      // Start condition
  clockLow();

  byte b[8];
  writeByte((ADDRESS) << 1 | 1); // Read mode
  for (byte i = 0; i < 8; i++) {
    b[i] = readByte(i == 7);
  }
  clockHigh();
  dataHigh();   // Stop condition
  return b[0];
}

byte LEDDisplayDriver::readByte(boolean last) {
  dataInput();
  byte b = 0;
  for (byte i = 0; i < 8; i++) {
    clockHigh();
    clockDelay();
    b = b << 1 | dataRead();
    clockLow();
    clockDelay();
  }
  dataOutput();
  if (last) {
    clockHigh();
    clockDelay();
    clockLow();
  } else {
    dataLow();
    clockHigh();
    clockDelay();
    clockLow();
  }
  return b;
}


void LEDDisplayDriver::init() {
  dataHigh();
  clockDelay();
  clockHigh();
  for (byte a = 0; a < (digits + 5) / 6; a++) {
    writeCmd((ADDRESS + a) << 1, 0x21); // System oscillator on
    writeCmd((ADDRESS + a) << 1, 0xa0); // INT/ROW pin is ROW
  }
  updateSetup();
}

void LEDDisplayDriver::updateSetup() {
  for (byte a = 0; a < (digits + 5) / 6; a++) {
    writeCmd((ADDRESS + a) << 1, on ? 0x81 : 0x80); // Display on/off
    writeCmd((ADDRESS + a) << 1, 0xe0 | brightness); // Display brightness
  }
}

void LEDDisplayDriver::writeCmd(byte addr, byte cmd) {
  dataLow();      // Start condition
  clockDelay();
  clockLow();
  writeByte(addr);
  writeByte(cmd);
  clockHigh();
  dataHigh();   // Stop condition
}

void LEDDisplayDriver::writeByte(byte b) {
  shiftOutMsbFirst(b);
  dataInput();
  clockHigh();
  clockDelay();
  ack = dataRead() == 0;
  clockLow();
  clockDelay();
  dataOutput();
}

#endif



//*****************************************************************************************************
//*****************************************************************************************************
// HT16K33 with 4 digit 14 segment displays row/column swapped, only 8x8 matrix supported
//*****************************************************************************************************
//*****************************************************************************************************
#ifdef _HT16K33_14SEG_4DS_
#define ADDRESS 0x70

void LEDDisplayDriver::update() {
  for (byte a = 0; a < (digits + 3) / 4; a++) {
    dataLow();      // Start condition
    clockLow();
    writeByte((ADDRESS + a) << 1);
    writeByte(0x00);
    byte a0 = (a << 2);
    for (byte i = 0; i < 7; i++) {
      uint16_t m1 = 1 << i;
      uint16_t m2 = 1 << (i + 8);
      byte w = 0;
      for (byte j = 0; j < 4; j++) {
        uint16_t d = data[j + a0];
        w |= ( d & m1) ? 0x001 << j : 0;
        w |= ( d & m2) ? 0x010 << j : 0;
      }
      writeByte(w);
      writeByte(((i == 0 && (data[a0 + 1]&SEG_DP) != 0) || (i == 1 && (data[a0 + 2]&SEG_DP) != 0)) ? 0x01 : 0); // Handle . and :
    }
    clockHigh();
    dataHigh();   // Stop condition
  }
}

void LEDDisplayDriver::update(__attribute__((unused))byte digit) {
  update();
}

void LEDDisplayDriver::init() {
  dataHigh();
  clockDelay();
  clockHigh();
  for (byte a = 0; a < (digits + 3) / 4; a++) {
    writeCmd((ADDRESS + a) << 1, 0x21); // System oscillator on
    writeCmd((ADDRESS + a) << 1, 0xa0); // INT/ROW pin is ROW
  }
  updateSetup();
}

void LEDDisplayDriver::updateSetup() {
  for (byte a = 0; a < (digits + 3) / 4; a++) {
    writeCmd((ADDRESS + a) << 1, on ? 0x81 : 0x80); // Display on/off
    writeCmd((ADDRESS + a) << 1, 0xe0 | brightness); // Display brightness
  }
}

void LEDDisplayDriver::writeCmd(byte addr, byte cmd) {
  dataLow();      // Start condition
  clockDelay();
  clockLow();
  writeByte(addr);
  writeByte(cmd);
  clockHigh();
  dataHigh();   // Stop condition
}

void LEDDisplayDriver::writeByte(byte b) {
  shiftOutMsbFirst(b);
  dataInput();
  clockHigh();
  clockDelay();
  ack = dataRead() == 0;
  clockLow();
  clockDelay();
  dataOutput();
}
#endif

