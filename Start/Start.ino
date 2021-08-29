
#include <TM1637TinyDisplay.h>
  
#define DISP_CLK 2
#define DISP_DIO 3
#define RMT_CLK 4
#define RMT_DIO 5
#define FIVEMIN_PIN 11
#define GOBTN_PIN 10
#define BUZZER_PIN 7
#define HORN_PIN 8

const int FiveMinuteLongBuzzes[] =
  {
    315000, 300000, 255000, 240000,
    75000, 60000, 15000, 0
  };
int FiveMinLongBuzzTotalCount;
int FiveMinLongBuzzNextBuzzIndex;
const int FiveMinuteShortBuzzes[] =
  {
    305000, 304000, 303000, 302000, 301000 ,
    245000, 244000, 243000, 242000, 241000 ,
    65000, 64000, 63000, 62000, 61000 ,
    5000, 4000, 3000, 2000, 1000    
  };
int FiveMinShortBuzzTotalCount;
int FiveMinShortBuzzNextBuzzIndex;

const int ThreeMinuteLongBuzzes[] =
  {
    195000, 180000, 179000, 178000,
    135000, 120000, 119000,
    75000,60000,
    15000, 0
  };
int ThreeMinLongBuzzTotalCount;
int ThreeMinLongBuzzNextBuzzIndex;

const int ThreeMinuteShortBuzzes[] =
  {
    185000, 184000, 183000, 182000, 181000 ,
    125000, 124000, 123000, 122000, 121000 ,
    65000, 64000, 63000, 62000, 61000 ,
    5000, 4000, 3000, 2000, 1000    
  };
int ThreeMinShortBuzzTotalCount;
int ThreeMinShortBuzzNextBuzzIndex;


const int FiveMinuteExtraLongHorns[] =
  {
    60000
  };
int FiveMinExtraLongHornTotalCount;
int FiveMinExtraLongHornNextHornIndex;

const int FiveMinuteLongHorns[] =
  {
    300000 ,
    240000 ,
    0
  };
int FiveMinLongBuzzTotalCount;
int FiveMinLongBuzzNextBuzzIndex;

const int ThreeMinuteLongHorns[] =
  {
    180000, 179000, 178000,
    120000, 119000,
    60000,
    0 // not really used. special case is handled below.
  };
int ThreeMinShortBuzzTotalCount;
int ThreeMinShortBuzzNextBuzzIndex;


const int ThreeMinuteShortHorns[] =
  {
    190000, 189500, 189000, 188500, 188000
  };
int ThreeMinShortBuzzTotalCount;
int ThreeMinShortBuzzNextBuzzIndex;

// buzzerStartWindow should be shorter than any buzzer length.
const int buzzerStartWindow = 90;
const int longBuzzerLength = 500;
const int shortBuzzerLength = 150;
unsigned int buzzerStarted = 0;
unsigned int turnOffBuzzer; // millis() after which buzzer should be stopped.

const int hornStartWindow = 90;
const int longHornLength = 800;
const int extraLongHornLength = 2000;
const int shortHornLength = 250;
unsigned int hornStarted = 0;
unsigned int turnOffHorn; // millis() after which horn should be stopped.

bool running = false;
bool isGoButtonPressed = false;
bool isFiveMinute;

unsigned long millisToZero;
unsigned long millisLastChecked;
unsigned long millisCurrent;
int seconds;

const int brightness = BRIGHT_HIGH;
//int brightness = BRIGHT_7;
//int brightness = BRIGHT_0;

TM1637TinyDisplay mainDisplay(DISP_CLK, DISP_DIO);
TM1637TinyDisplay remoteDisplay(RMT_CLK, RMT_DIO, 200);

void setup() {
  pinMode(FIVEMIN_PIN, INPUT_PULLUP);
  pinMode(GOBTN_PIN, INPUT_PULLUP);
  pinMode(HORN_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(HORN_PIN, OUTPUT);

  isFiveMinute = digitalRead(FIVEMIN_PIN);

  mainDisplay.clear();
  mainDisplay.setBrightness(brightness);
  remoteDisplay.clear();
  remoteDisplay.setBrightness(brightness);

  resetTime();
}

void loop() {
  readInput();

  if(running) {
    updateTime();
  }
  writeTime(false);
  checkHorn();
  checkBuzzer();

  delay(2);
}

void readInput(){
  checkGoButton();
  if(!running){
    checkTimeMode();
  }
}

void checkBuzzer(){
  if(running) {
    if(buzzerStarted == 0) {
      checkIfBuzzerShouldStart();
    } else {
      if(shouldBuzzerStop()) {      
        digitalWrite(BUZZER_PIN, LOW);
        buzzerStarted = 0;
      }
    }
  } else { //stopped, so stop buzzer.
    digitalWrite(BUZZER_PIN, LOW);
    buzzerStarted = 0;
  }
}

bool checkIfBuzzerShouldStart() {
  if(isFiveMinute){
    for(int i=0; i < (sizeof(FiveMinuteLongBuzzes) / sizeof(FiveMinuteLongBuzzes[0])); i++) {
      if(millisToZero < FiveMinuteLongBuzzes[i] &&
        millisToZero > (FiveMinuteLongBuzzes[i] - buzzerStartWindow) ) {
          startBuzzer(longBuzzerLength);
      }
    }
    for(int i=0; i < (sizeof(FiveMinuteShortBuzzes) / sizeof(FazazaiveMinuteShortBuzzes[0])); i++) {
      if(millisToZero < FiveMinuteShortBuzzes[i] &&
        millisToZero > FiveMinuteShortBuzzes[i] - buzzerStartWindow ) {
          startBuzzer(shortBuzzerLength);
      }
    }
  } else { //three minute
    for(int i=0; i < (sizeof(ThreeMinuteLongBuzzes) / sizeof(ThreeMinuteLongBuzzes[0])); i++) {
      if(millisToZero < ThreeMinuteLongBuzzes[i] &&
        millisToZero > (ThreeMinuteLongBuzzes[i] - buzzerStartWindow) ) {
          startBuzzer(longBuzzerLength);
      }
    }
    for(int i=0; i < (sizeof(ThreeMinuteShortBuzzes) / sizeof(ThreeMinuteShortBuzzes[0])); i++) {
      if(millisToZero < ThreeMinuteShortBuzzes[i] &&
        millisToZero > ThreeMinuteShortBuzzes[i] - buzzerStartWindow ) {
          startBuzzer(shortBuzzerLength);
      }
    }
  }
}

void startBuzzer(int length) {
  digitalWrite(BUZZER_PIN, HIGH);
  buzzerStarted = millis();
  turnOffBuzzer = buzzerStarted + length;
}

bool shouldBuzzerStop() {
  return turnOffBuzzer < millis();
}

void checkHorn(){
  if(hornStarted == 0) {
    checkIfHornShouldStart();
  } else {
    if(shouldHornStop()) {      
      digitalWrite(HORN_PIN, LOW);
      hornStarted = 0;
    }
  }
}

bool checkIfHornShouldStart() {
  if(isFiveMinute){
    for(int i=0; i < (sizeof(FiveMinuteLongHorns) / sizeof(FiveMinuteLongHorns[0])); i++) {
      if(millisToZero < FiveMinuteLongHorns[i] &&
        millisToZero > (FiveMinuteLongHorns[i] - hornStartWindow) ) {
          startHorn(longHornLength);
      }
    }
    for(int i=0; i < (sizeof(FiveMinuteExtraLongHorns) / sizeof(FiveMinuteExtraLongHorns[0])); i++) {
      if(millisToZero < FiveMinuteExtraLongHorns[i] &&
        millisToZero > FiveMinuteExtraLongHorns[i] - hornStartWindow ) {
          startHorn(extraLongHornLength);
      }
    }
  } else { //three minute
    for(int i=0; i < (sizeof(ThreeMinuteLongHorns) / sizeof(ThreeMinuteLongHorns[0])); i++) {
      if(millisToZero < ThreeMinuteLongHorns[i] &&
        millisToZero > (ThreeMinuteLongHorns[i] - hornStartWindow) ) {
          startHorn(longHornLength);
      }
    }
    for(int i=0; i < (sizeof(ThreeMinuteShortHorns) / sizeof(ThreeMinuteShortHorns[0])); i++) {
      if(millisToZero < ThreeMinuteShortHorns[i] &&
        millisToZero > ThreeMinuteShortHorns[i] - hornStartWindow ) {
          startHorn(shortHornLength);
      }
    }
  }
}

void startHorn(int length) {
  digitalWrite(HORN_PIN, HIGH);
  hornStarted = millis();
  turnOffHorn = hornStarted + length;
}

bool shouldHornStop() {
  return turnOffHorn < millis();
}

void checkGoButton(){
  if(digitalRead(GOBTN_PIN) == LOW){
    if( !isGoButtonPressed ) {
      isGoButtonPressed = true;
      goButtonPress();
    }
  } else {
    isGoButtonPressed = false;
  }
}

void goButtonPress(){
    if(running) {
      //stop
      running = false;
      startBuzzer(shortBuzzerLength);
      resetTime();
    } else {
      // Go
      running = true;
      startBuzzer(shortBuzzerLength);
      millisLastChecked = millis();
  }
}

void resetTime() {
  if(isFiveMinute) {
    millisToZero = 360 * 1000 -1 ; // six minutes
  } else {
    millisToZero = 210 * 1000 -1; // three and a half minutes
  }
  writeTime(true);
}

void checkTimeMode(){
  if(digitalRead(FIVEMIN_PIN) != isFiveMinute){
    isFiveMinute = digitalRead(FIVEMIN_PIN);
    if(isFiveMinute) {
      millisToZero = 360 * 1000 -1; // six minutes
    } else {
      millisToZero = 210 * 1000 -1; // three and a half minutes
    }    
  }
}

void updateTime(){
  millisCurrent = millis();
  if(millisCurrent > millisLastChecked) {
    if(millisToZero > (millisCurrent - millisLastChecked)) {
      millisToZero = millisToZero - (millisCurrent - millisLastChecked);
    } else {
      millisToZero = millisToZero - (millisCurrent - millisLastChecked);
        if(isFiveMinute) {
          millisToZero += 300 * 1000 ; // five minutes - keep running
        } else {
          // special case here: timer is stopped, but need to blow horn last time:
          running = false; // three minute countdown, stop, reset to 3:30.
          startHorn(longHornLength);
          resetTime();
        }
    }
    millisLastChecked = millisCurrent;
  }
}

void writeTime(bool force){
  if(force || seconds != getSeconds()){   
    seconds = getSeconds();
    mainDisplay.showString(" ",1,0);
    mainDisplay.showNumberDec( getMinutes(),0b11100000, false, 1, 1);
    mainDisplay.showNumber( seconds, true, 2, 2);

    remoteDisplay.showString(" ",1,0);
    remoteDisplay.showNumberDec( getMinutes(),0b11100000, false, 1, 1);
    remoteDisplay.showNumber( seconds, true, 2, 2);
  }
}

int getMinutes(){
  return getTotalSeconds() / 60;
}

int getSeconds(){
  return getTotalSeconds() % 60;
}

int getTotalSeconds(){
  // we expect the timer to sound when zero is first display, not at the end of zero.
  return (millisToZero / 1000) + 1;
}
