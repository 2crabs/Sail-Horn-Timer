
#include <TM1637TinyDisplay.h>
#include <ArduinoBLE.h>
  
#define DISP_CLK 2
#define DISP_DIO 3
#define FIVEMIN_PIN 11
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
    195000, 180000, 178500, 177000,
    135000, 120000, 118500,
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

const int FiveMinuteExtraLongHorns[] =
  {
    60000
  };

const int FiveMinuteLongHorns[] =
  {
    300000 ,
    240000 ,
    0
  };

const int ThreeMinuteLongHorns[] =
  {
    180000, 178500, 177000,
    120000, 118500,
    60000,
    0 // not really used. special case is handled below.
  };

const int ThreeMinuteShortHorns[] =
  {
    190000, 189500, 189000, 188500, 188000
  };

// The totalCount variables are set in initializeArrayVariables() 
int FiveMinLongBuzzTotalCount;
int FiveMinLongBuzzNextBuzzIndex = 0;
int FiveMinShortBuzzTotalCount;
int FiveMinShortBuzzNextBuzzIndex = 0;
int ThreeMinLongBuzzTotalCount;
int ThreeMinLongBuzzNextBuzzIndex = 0;
int ThreeMinShortBuzzTotalCount;
int ThreeMinShortBuzzNextBuzzIndex = 0;
int FiveMinExtraLongHornTotalCount;
int FiveMinExtraLongHornNextHornIndex = 0;
int FiveMinLongHornTotalCount;
int FiveMinLongHornNextHornIndex = 0;
int ThreeMinLongHornTotalCount;
int ThreeMinLongHornNextHornIndex = 0;
int ThreeMinShortHornTotalCount;
int ThreeMinShortHornNextHornIndex = 0;

// buzzerStartWindow should be shorter than any buzzer length.
const int buzzerStartWindow = 90;
const int longBuzzerLength = 900;
const int shortBuzzerLength = 150;
bool buzzerStarted = false;
unsigned int turnOffBuzzer; // millis() after which buzzer should be stopped.

const int hornStartWindow = 90;
const int longHornLength = 900;
const int extraLongHornLength = 2000;
const int shortHornLength = 250;
bool hornStarted = false;
unsigned long turnOffHorn; // millis() after which horn should be stopped.

bool running = false;
bool isGoButtonPressed = false;
unsigned long lastGoButtonPress;
unsigned long lastGoButtonDown;
bool isFiveMinute;

//unsigned long millisToZero;
unsigned long endMilli;
unsigned long pausedMillisRemaining;
//unsigned long millisLastChecked;
unsigned long millisCurrent;
int seconds;

const int brightness = BRIGHT_HIGH;
//int brightness = BRIGHT_7;
//int brightness = BRIGHT_0;

TM1637TinyDisplay mainDisplay(DISP_CLK, DISP_DIO);

BLEService timerService("6dd77192-0d0c-49c4-a6ab-4d2c4eece29f"); // create service

// create switch characteristic and allow remote device to get notifications
BLEShortCharacteristic timeCharacteristic("6dd77193-0d0c-49c4-a6ab-4d2c4eece29f", BLERead | BLENotify);
// create button characteristic and allow remote device to read and write
BLEBoolCharacteristic buttonCharacteristic("6dd77194-0d0c-49c4-a6ab-4d2c4eece29f", BLERead | BLEWrite);


void setup() {
  Serial.begin(115200);
  pinMode(FIVEMIN_PIN, INPUT_PULLUP);
  pinMode(HORN_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(HORN_PIN, OUTPUT);

  initializeArrayVariables();
  isFiveMinute = digitalRead(FIVEMIN_PIN);

  BLE.begin();

  // set the local name peripheral advertises
  BLE.setLocalName("LHYChorn");
  BLE.setDeviceName("LHYC Horn");
  // set the UUID for the service this peripheral advertises:
  BLE.setAdvertisedService(timerService);

  BLEDescriptor timeName("2901", "Remaining Time");
  timeCharacteristic.addDescriptor(timeName);

  
  BLEDescriptor buttonName("2901", "Start/Reset");
  buttonCharacteristic.addDescriptor(buttonName);

  // add the characteristics to the service
  timerService.addCharacteristic(timeCharacteristic);
  timerService.addCharacteristic(buttonCharacteristic);

  // add the service
  BLE.addService(timerService);

  timeCharacteristic.writeValue(0);
  buttonCharacteristic.writeValue(0);

  // start advertising
  BLE.advertise();


  mainDisplay.clear();
  mainDisplay.setBrightness(brightness);


  resetTime();
}

void loop() {
  BLE.poll();
  readInput();

  if(running) {
    isTimerPastZero();
  }
  checkHorn();
  checkBuzzer();
  writeTime(false);

  if(Serial){
    Serial.println("In loop");
  }

}

void initializeArrayVariables(){
  FiveMinLongBuzzTotalCount = sizeof(FiveMinuteLongBuzzes) / sizeof(FiveMinuteLongBuzzes[0]);
  FiveMinShortBuzzTotalCount = sizeof(FiveMinuteShortBuzzes) / sizeof(FiveMinuteShortBuzzes[0]);
  ThreeMinLongBuzzTotalCount = sizeof(ThreeMinuteLongBuzzes) / sizeof(ThreeMinuteLongBuzzes[0]);
  ThreeMinShortBuzzTotalCount = sizeof(ThreeMinuteShortBuzzes) / sizeof(ThreeMinuteShortBuzzes[0]);
  FiveMinExtraLongHornTotalCount = sizeof(FiveMinuteExtraLongHorns) / sizeof(FiveMinuteExtraLongHorns[0]);
  FiveMinLongHornTotalCount = sizeof(FiveMinuteLongHorns) / sizeof(FiveMinuteLongHorns[0]);
  ThreeMinLongHornTotalCount = sizeof(ThreeMinuteLongHorns) / sizeof(ThreeMinuteLongHorns[0]);
  ThreeMinShortHornTotalCount = sizeof(ThreeMinuteShortHorns) / sizeof(ThreeMinuteShortHorns[0]);
}

void readInput(){
  checkGoButton();
  if(!running){
    checkTimeMode();
  }
}

void checkBuzzer(){
  if(running) {
    if(!buzzerStarted) {
      checkIfBuzzerShouldStart();
    } else {
      if(shouldBuzzerStop()) {      
        digitalWrite(BUZZER_PIN, LOW);
        buzzerStarted = false;
      }
    }
  } else { //stopped, so stop buzzer.
    digitalWrite(BUZZER_PIN, LOW);
    buzzerStarted = false;
  }
}

bool checkIfBuzzerShouldStart() {
  if(isFiveMinute){
    if( FiveMinLongBuzzNextBuzzIndex < FiveMinLongBuzzTotalCount &&
      getMillisRemaining() < FiveMinuteLongBuzzes[FiveMinLongBuzzNextBuzzIndex]) {
        FiveMinLongBuzzNextBuzzIndex++;
        startBuzzer(longBuzzerLength);
      }
    
    if( FiveMinShortBuzzNextBuzzIndex < FiveMinShortBuzzTotalCount &&
      getMillisRemaining() < FiveMinuteShortBuzzes[FiveMinShortBuzzNextBuzzIndex] ) {
        FiveMinShortBuzzNextBuzzIndex++;
        startBuzzer(shortBuzzerLength);
      }
  } else { //three minute  
    if( ThreeMinLongBuzzNextBuzzIndex < ThreeMinLongBuzzTotalCount &&
      getMillisRemaining() < ThreeMinuteLongBuzzes[ThreeMinLongBuzzNextBuzzIndex] ) {
        ThreeMinLongBuzzNextBuzzIndex++;
        startBuzzer(longBuzzerLength);
      }    
    if( ThreeMinShortBuzzNextBuzzIndex < ThreeMinShortBuzzTotalCount &&
      getMillisRemaining() < ThreeMinuteShortBuzzes[ThreeMinShortBuzzNextBuzzIndex] ) {
        ThreeMinShortBuzzNextBuzzIndex++;
        startBuzzer(shortBuzzerLength);
      }
  }
}

void startBuzzer(int length) {
  digitalWrite(BUZZER_PIN, HIGH);
  buzzerStarted = true;
  turnOffBuzzer = millis() + length;
}

bool shouldBuzzerStop() {
  return turnOffBuzzer < millis();
}

void checkHorn(){
  if(!hornStarted) {
    checkIfHornShouldStart();
  } else {
    if(shouldHornStop()) {
      digitalWrite(HORN_PIN, LOW);
      hornStarted = false;
    }
  }
}

bool checkIfHornShouldStart() {
  if(isFiveMinute){
    if( FiveMinLongHornNextHornIndex < FiveMinLongHornTotalCount &&
    getMillisRemaining() < FiveMinuteLongHorns[FiveMinLongHornNextHornIndex] ) {
      FiveMinLongHornNextHornIndex++;
      startHorn(longHornLength);
    }
    if( FiveMinExtraLongHornNextHornIndex < FiveMinExtraLongHornTotalCount &&
    getMillisRemaining() < FiveMinuteExtraLongHorns[FiveMinExtraLongHornNextHornIndex] ) {
      FiveMinExtraLongHornNextHornIndex++;
      startHorn(extraLongHornLength);
    }
  } else { //three minute
  
    if( ThreeMinLongHornNextHornIndex < ThreeMinLongHornTotalCount &&
    getMillisRemaining() < ThreeMinuteLongHorns[ThreeMinLongHornNextHornIndex] ) {
      ThreeMinLongHornNextHornIndex++;
      startHorn(longHornLength);
    }

    if( ThreeMinShortHornNextHornIndex < ThreeMinShortHornTotalCount &&
    getMillisRemaining() < ThreeMinuteShortHorns[ThreeMinShortHornNextHornIndex] ) {
      ThreeMinShortHornNextHornIndex++;
      startHorn(shortHornLength);
    }
  }
}

void startHorn(int length) {
  digitalWrite(HORN_PIN, HIGH);
  hornStarted = true;
    
  turnOffHorn = millis() + (unsigned long)length;  
}

bool shouldHornStop() {
  return turnOffHorn < millis();
}

void checkGoButton(){

  // With move to BLE, need to  remove some of the debounce logic from this.
  if(buttonCharacteristic.value()){
    if( !isGoButtonPressed) {
      isGoButtonPressed = true;
      if(lastGoButtonPress < (millis() - 20)
        && lastGoButtonDown < (millis() - 20)){
          lastGoButtonPress = millis();
          goButtonPress();
      }
    }
    lastGoButtonDown = millis();
    buttonCharacteristic.setValue(0);
  } else {
    if(lastGoButtonPress < (millis() - 20)
      && lastGoButtonDown < (millis() - 20)){
        isGoButtonPressed = false;
    }
  }
}

void goButtonPress(){
  if(running) {
    //stop
    running = false;
    startBuzzer(shortBuzzerLength);
    resetTime();
    // if time isn't reset, make sure to save endMilli minus millis() to pausedMillisRemaining
  } else {
    // Go
    running = true;
    endMilli = millis() + pausedMillisRemaining;
    startBuzzer(shortBuzzerLength);
    //millisLastChecked = millis();
  }
}

void resetTime() {
  if(isFiveMinute) {
    setMillisRemaining(360 * 1000 -1) ; // six minutes
    resetArrayIndexes();
  } else {
    setMillisRemaining( 210 * 1000 -1); // three and a half minutes
    resetArrayIndexes();
  }
  writeTime(true);
}

void resetArrayIndexes() {
  FiveMinLongBuzzNextBuzzIndex = 0;
  FiveMinShortBuzzNextBuzzIndex = 0;
  ThreeMinLongBuzzNextBuzzIndex = 0;
  ThreeMinShortBuzzNextBuzzIndex = 0;
  FiveMinExtraLongHornNextHornIndex = 0;
  FiveMinLongHornNextHornIndex = 0;
  ThreeMinLongHornNextHornIndex = 0;
  ThreeMinShortHornNextHornIndex = 0;

}

void checkTimeMode(){
  if(digitalRead(FIVEMIN_PIN) != isFiveMinute){
    isFiveMinute = digitalRead(FIVEMIN_PIN);
    resetTime();    
  }
}

void isTimerPastZero(){
  if(endMilli <= millis()){
    if(isFiveMinute) {
      resetToFiveMin();
    } else {
      // special case here: timer is stopped, but need to blow horn last time:
      running = false; // three minute countdown, stop, reset to 3:30.
      startHorn(longHornLength);
      resetTime();
    }
  }
}

unsigned long getMillisRemaining(){
  if(running) {
    return endMilli - millis();
  } else {
    return pausedMillisRemaining;
  }
}

void setMillisRemaining(unsigned long millisRemaining){
  if(running){
    endMilli = millis() + millisRemaining;
  } else {
    pausedMillisRemaining = millisRemaining;
  }
}

void resetToFiveMin(){
  endMilli += 300 * 1000 ; // five minutes - keep running
  resetArrayIndexes();
  setArrayIndexesToFiveMin();
}

void setArrayIndexesToFiveMin(){
  FiveMinLongBuzzNextBuzzIndex = 
    getFiveMinuteIndex(FiveMinuteLongBuzzes, sizeof(FiveMinuteLongBuzzes)/sizeof(FiveMinuteLongBuzzes[0]));
  FiveMinShortBuzzNextBuzzIndex =
    getFiveMinuteIndex(FiveMinuteShortBuzzes, sizeof(FiveMinuteShortBuzzes)/sizeof(FiveMinuteShortBuzzes[0]));
  FiveMinExtraLongHornNextHornIndex =
    getFiveMinuteIndex(FiveMinuteExtraLongHorns, sizeof(FiveMinuteExtraLongHorns)/sizeof(FiveMinuteExtraLongHorns[0]));
  FiveMinLongHornNextHornIndex =
    getFiveMinuteIndex(FiveMinuteLongHorns, sizeof(FiveMinuteLongHorns)/sizeof(FiveMinuteLongHorns[0]));
}

int getFiveMinuteIndex(const int arrayToCheck[], int arrayLength){
  for(int i = 0; i < arrayLength; i++ ){
    if(arrayToCheck[i] <= (360 * 1000) ){
      return i;
    }
  }
  
  return arrayLength;
}

void writeTime(bool force){
  if(force || seconds != getSeconds()){
    timeCharacteristic.writeValue(getTotalSeconds());   
    seconds = getSeconds();
    mainDisplay.showString(" ",1,0);
    mainDisplay.showNumberDec( getMinutes(),0b11100000, false, 1, 1);
    mainDisplay.showNumber( seconds, true, 2, 2);
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
  return (getMillisRemaining() / 1000) + 1;
}
