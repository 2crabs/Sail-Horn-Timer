

#include "Adafruit_LEDBackpack.h"
  
int ThreeOrFiveMinuteSwitch = 2;
const int GoButton = 1;
const int BuzzerPin = 3;
const int HornButton = 2;

const int FiveMinuteLongBuzzes[] = {315000, 300000, 255000, 240000 };
const int FiveMinuteShortBuzzes[] = {305000, 304000, 303000, 302000, 301000 };
int buzzerStartWindow = 90;
int longBuzzerLength = 500;
int shortBuzzerLength = 200;
unsigned int buzzerStarted = 0;
unsigned int turnOffBuzzer; // millis() after which buzzer should be stopped.


bool running = false;
bool goButtonPressed = false;
bool isFiveMinute;

int const longBuzzMillis = 500;
int const shortBuzzMillis = 100;



int brightness = 1;

Adafruit_7segment display = Adafruit_7segment();

unsigned long millisToZero;
unsigned long millisLastChecked;
unsigned long millisCurrent;



void setup() {
  // put your setup code here, to run once:
  pinMode(ThreeOrFiveMinuteSwitch, INPUT);
  pinMode(GoButton, INPUT_PULLUP);
  pinMode(HornButton, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BuzzerPin, OUTPUT);
  Serial.begin(9600);
  isFiveMinute = digitalRead(ThreeOrFiveMinuteSwitch);
  if(isFiveMinute) {
    millisToZero = 330 * 1000 ; // five and a half minutes
  } else {
    millisToZero = 210 * 1000; // three and a half minutes
  }    

  display.begin(0x70);
  display.setBrightness(brightness);
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
        digitalWrite(BuzzerPin, LOW);
        buzzerStarted = 0;
      }
    }
  } else { //stopped, so stop buzzer.
    digitalWrite(BuzzerPin, LOW);
    buzzerStarted = 0;
  }
}

bool checkIfBuzzerShouldStart() {
  if(isFiveMinute){
    for(int i=0; i < (sizeof(FiveMinuteLongBuzzes) / sizeof(FiveMinuteLongBuzzes[0])); i++) {
      if(millisToZero < FiveMinuteLongBuzzes[i] &&
        millisToZero > (FiveMinuteLongBuzzes[i] - buzzerStartWindow) ) {
          digitalWrite(BuzzerPin, HIGH);
          buzzerStarted = millis();
          turnOffBuzzer = buzzerStarted + longBuzzerLength;
      }
    }
    for(int i=0; i < (sizeof(FiveMinuteShortBuzzes) / sizeof(FiveMinuteShortBuzzes[0])); i++) {
      if(millisToZero < FiveMinuteShortBuzzes[i] &&
        millisToZero > FiveMinuteShortBuzzes[i] - buzzerStartWindow ) {
          digitalWrite(BuzzerPin, HIGH);
          buzzerStarted = millis();
          turnOffBuzzer = buzzerStarted + shortBuzzerLength;
      }
    }
  } else { //three minute
  }
}

bool shouldBuzzerStop() {
  return turnOffBuzzer < millis();
}

void checkGoButton(){
  if(digitalRead(GoButton) == LOW){
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
  
  if(digitalRead(ThreeOrFiveMinuteSwitch) != isFiveMinute){
    isFiveMinute = digitalRead(ThreeOrFiveMinuteSwitch);
    if(isFiveMinute) {
      millisToZero = 330 * 1000 ; // five and a half minutes
    } else {
      millisToZero = 210 * 1000; // three and a half minutes
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
            millisToZero += 300 * 1000 ; // five and a half minutes
          } else {
            millisToZero += 180 * 1000; // three and a half minutes
          }
      }
      millisLastChecked = millisCurrent;
    }
}
void writeTime(){
    display.writeDigitNum(1, getMinutes(), false);
    display.drawColon(true);
    display.writeDigitNum(3, (getSeconds() / 10) % 10, false);
    display.writeDigitNum(4, getSeconds() % 10, false);
    display.writeDisplay();
}

int getMinutes(){
  return (millisToZero / 1000 / 60);
}

int getSeconds(){
  return (millisToZero / 1000 ) % 60;
}

