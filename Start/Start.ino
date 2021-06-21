
#include <TM1637TinyDisplay.h>
  
#define DISP_CLK 2
#define DISP_DIO 3
#define RMT_CLK 4
#define RMT_DIO 5
#define FIVEMIN_PIN 18
#define GOBTN_PIN 19
#define BUZZER_PIN 7
#define HORN_PIN 8

const int FiveMinuteLongBuzzes[] =
  {
    315000, 300000, 255000, 240000,
    75000, 60000, 15000, 0
  };
const int FiveMinuteShortBuzzes[] =
  {
    305000, 304000, 303000, 302000, 301000 ,
    245000, 244000, 243000, 242000, 241000 ,
    65000, 64000, 63000, 62000, 61000 ,
    5000, 4000, 3000, 2000, 1000    
  };

const int ThreeMinuteLongBuzzes[] =
  {
    195000, 180000, 179000, 178000,
    135000, 120000, 119000,
    75000,60000,
    15000, 0
  };
const int ThreeMinuteShortBuzzes[] =
  {
    185000, 184000, 183000, 182000, 181000 ,
    125000, 124000, 123000, 122000, 121000 ,
    65000, 64000, 63000, 62000, 61000 ,
    5000, 4000, 3000, 2000, 1000    
  };

int buzzerStartWindow = 90;
int longBuzzerLength = 500;
int shortBuzzerLength = 150;
unsigned int buzzerStarted = 0;
unsigned int turnOffBuzzer; // millis() after which buzzer should be stopped.


bool running = false;
bool goButtonPressed = false;
bool isFiveMinute;

int const longBuzzMillis = 500;
int const shortBuzzMillis = 100;

int brightness = 1;

TM1637TinyDisplay mainDisplay(DISP_CLK, DISP_DIO);
TM1637TinyDisplay remoteDisplay(RMT_CLK, RMT_DIO);

unsigned long millisToZero;
unsigned long millisLastChecked;
unsigned long millisCurrent;



void setup() {
  pinMode(FIVEMIN_PIN, INPUT);
  pinMode(GOBTN_PIN, INPUT_PULLUP);
  pinMode(HORN_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  isFiveMinute = digitalRead(FIVEMIN_PIN);
  if(isFiveMinute) {
    millisToZero = 330 * 1000 -1 ; // five and a half minutes
  } else {
    millisToZero = 210 * 1000 -1; // three and a half minutes
  }    

  mainDisplay.clear();
  mainDisplay.setBrightness(brightness);
  remoteDisplay.clear();
  remoteDisplay.setBrightness(brightness);
}

void loop() {
  readInput();

  if(running) {
    updateTime();
  }
  writeTime();
  checkBuzzer();

  delay(20);
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
          buzz(longBuzzerLength);
      }
    }
    for(int i=0; i < (sizeof(FiveMinuteShortBuzzes) / sizeof(FiveMinuteShortBuzzes[0])); i++) {
      if(millisToZero < FiveMinuteShortBuzzes[i] &&
        millisToZero > FiveMinuteShortBuzzes[i] - buzzerStartWindow ) {
          buzz(shortBuzzerLength);
      }
    }
  } else { //three minute
    for(int i=0; i < (sizeof(ThreeMinuteLongBuzzes) / sizeof(ThreeMinuteLongBuzzes[0])); i++) {
      if(millisToZero < ThreeMinuteLongBuzzes[i] &&
        millisToZero > (ThreeMinuteLongBuzzes[i] - buzzerStartWindow) ) {
          buzz(longBuzzerLength);
      }
    }
    for(int i=0; i < (sizeof(ThreeMinuteShortBuzzes) / sizeof(ThreeMinuteShortBuzzes[0])); i++) {
      if(millisToZero < ThreeMinuteShortBuzzes[i] &&
        millisToZero > ThreeMinuteShortBuzzes[i] - buzzerStartWindow ) {
          buzz(shortBuzzerLength);
      }
    }
  }
}

void buzz(int length) {
  digitalWrite(BUZZER_PIN, HIGH);
  buzzerStarted = millis();
  turnOffBuzzer = buzzerStarted + length;
}

bool shouldBuzzerStop() {
  return turnOffBuzzer < millis();
}

void checkGoButton(){
  if(digitalRead(GOBTN_PIN) == LOW){
    if( !goButtonPressed ) {
      goButtonPressed = true;
      toggleState();
    }
  } else {
    goButtonPressed = false;
  }
}

void toggleState(){
    if(running) {
      //stop
      running = false;
    } else {
      running = true;
      millisLastChecked = millis();
  }
}

void checkTimeMode(){
  if(digitalRead(FIVEMIN_PIN) != isFiveMinute){
    isFiveMinute = digitalRead(FIVEMIN_PIN);
    if(isFiveMinute) {
      millisToZero = 330 * 1000 -1; // five and a half minutes
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
            running = false; // three minute countdown, stop, reset to 3:30.
            millisToZero = 210 * 1000 -1; // three and a half minutes
          }
      }
      millisLastChecked = millisCurrent;
    }
}
void writeTime(){
    mainDisplay.showNumberDec( getMinutes(),0b11100000, false, 1, 1);
    mainDisplay.showNumber( getSeconds(), true, 2, 2);
    
    remoteDisplay.showNumberDec( getMinutes(),0b11100000, false, 1, 1);
    remoteDisplay.showNumber( getSeconds(), true, 2, 2);
}

int getMinutes(){
  return getTotalSeconds() / 60;
}

int getSeconds(){
  return getTotalSeconds() % 60;
}

int getTotalSeconds(){
  // so this is weird: we expect the timer to sound when zero is first display, not at the end of zero.
  return (millisToZero / 1000) + 1;
}
