

#include "Adafruit_LEDBackpack.h"
  
int ThreeOrFiveMinuteSwitch = 2;
const int GoButton = 1;
int HornButton = 2;
bool running = false;
bool goButtonPressed = false;
bool isFiveMinute;

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

  delay(100);
}

void readInput(){
  checkGoButton();
  if(!running){
    checkTimeMode();
  }
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
      millisToZero = millisToZero - (millisCurrent - millisLastChecked);
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
