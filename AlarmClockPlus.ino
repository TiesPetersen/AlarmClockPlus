 // Serial.print("Raw data: "); Serial.print(dt.year); Serial.print(dt.month); Serial.print(dt.day); Serial.print(dt.hour); Serial.print(dt.minute); Serial.print(dt.second);
//1976 - 4186
// RTC
  #include <Wire.h>
  #include "RTClib.h"
  RTC_DS1307 rtc;
  DateTime now;
  
// ENCODER
  #define CLK 6
  #define DT 7
  #define SW 8
  
  int currentStateCLK;
  int lastStateCLK;
  unsigned long lastButtonPress = 0;

// SONIC SR04
  #include <SR04.h>
  #include <math.h>
  #define TRIG_PIN 5 
  #define ECHO_PIN 4
  SR04 sr04 = SR04(ECHO_PIN,TRIG_PIN);
  long a;
  long defaultA;
  long timeShortDistance = 0;
  bool dimMode = 0;
  bool isInDimMode = false;
  int lightDelay = 10000;
  bool alreadyChanged = false;

// RFID
  #include <SPI.h>
  #include <MFRC522.h>
  
  #define RST_PIN   9     // Configurable, see typical pin layout above
  #define SS_PIN    10    // Configurable, see typical pin layout above
  
  MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

  MFRC522::MIFARE_Key key;

// LED
  #include <FastLED.h>
  #define DATA_PIN 2
  #define NUM_LEDS 30
  CRGB leds[NUM_LEDS];
  
  int order[] = {28,29,25,26,27,23,24,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};
  int numbers[] = {B1110111,B1100000, B0111110,B1111100,B1101001,B1011101,B1011111,B1110000,B1111111,B1111101};
  int color[] = {15,255,200};
  int colorCode = color[0];
  int valueCode = color[2];
  
  int firstDigits = 0;
  int lastDigits = 0;
  int firstDigit = 0;
  int lastDigit = 0;
  int dotsAreOn = false;
  
  unsigned long start_time;
  unsigned long timed_event;
  unsigned long current_time;

// LOGIC
  int fourState;
  int threeState;

  int currentSetting = 0;
  int s1stage = 0;
  int s0stage = 1;
  int encoderChange = 0;
  bool encoderPressed = false;
  int dayOfWeek = 1;      // 1,2,3,4,5,6,7
  int blinkingSeq = 0;
  unsigned long last_blink;
  int setupSeq = 0;
  int LLhigh = 1;
  int LLlow = 1;
  int LLmiddle = 1;
  int currentLL;
  bool firstTimeS0 = true;
  int s2stage = 0;

  int alarm[] = {0,0};
  int snooze[] = {0,0};
  int alarmStage = 0;
  int typeAlarm;
  int toneSeq;
  int alarmSet = false;

  int s3stage = 0;
  unsigned long s3delay = 0;
  int currentDay = 1;
  int alarms[14];
  int snoozes[14];
  int turnON = 1;
  unsigned long lastAlarm = 0;

int* timeSubtract(int fDigits, int lDigits){
  int fSubtract = fDigits - 8;
  int lSubtract = lDigits;
  if (fSubtract < 0) {
    fSubtract = 60 + fSubtract;
    lSubtract = lSubtract - 1;
  }
  if (lSubtract < 0){
    lSubtract = 24 + lSubtract;
  }
    int* newTime = new int[4];
    newTime[0] = lSubtract;
    newTime[1] = fSubtract;
    return newTime;
}
  
void changeNumber(int toChange,int toNumber){
    int offset;
    int luminosity;
    
    if (toChange == 0 || toChange == 1){
      offset = toChange * 7;
    } else if (toChange == 3 || toChange == 4){
      offset = (toChange - 3) * 7 + 16;
    } else {
      offset = 14;
      if (toNumber == 0){
        leds[14] = CHSV(0,0,0);
        leds[15] = CHSV(0,0,0);
      } else {
        if (dimMode){
          luminosity = 20;
        } else{
          luminosity = color[2];
        }
        CHSV currentColor = CHSV(color[0],color[1],luminosity);
        leds[14] = currentColor;
        leds[15] = currentColor;
      }
      FastLED.show();
      return;
    }
    for (int j = 0; j < 7; j++){
      int ONorOFF = (numbers[toNumber]>>(6-(j))) & 1;
      
      if (dimMode){
        luminosity = 20;
      } else{
        luminosity = color[2];
      }
      CHSV currentColor = CHSV((ONorOFF * color[0]),(ONorOFF * color[1]),(ONorOFF * luminosity));
      leds[order[j + offset]] = currentColor;
    }
    FastLED.show();
    return;
}

void setTime(int fDigits, int lDigits){
  int fDigit;
  int lDigit;
  if (fDigits < 10) {
          changeNumber(1,0);
          changeNumber(0,fDigits);
        } else {
          fDigit = fDigits / 10;
          changeNumber(1, fDigit);
          changeNumber(0, fDigits % 10);
        }
    
        if (lDigits < 10) {
          changeNumber(4,0);
          changeNumber(3,lDigits);
        } else {
          lDigit = lDigits / 10;
          changeNumber(4, lDigit);
          changeNumber(3, lDigits % 10);
        }
}
  
void setup()
{
  Serial.begin(9600);
  // SONIC SR04
    pinMode(6,OUTPUT);

  pinMode(3,OUTPUT);
  
  // LDR
  timeShortDistance = 0;
    
  // RTC
    rtc.begin();
    // Manual (YYYY, MM, DD, HH, II, SS)
    // clock.setDateTime(2022, 1, 4, 19, 07, 15);
    // clock.setDateTime(__DATE__, __TIME__);
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    
  // ENCODER
    pinMode(CLK,INPUT);
    pinMode(DT,INPUT);
    pinMode(SW, INPUT_PULLUP);
    lastStateCLK = digitalRead(CLK);

  // RFID
    SPI.begin();         // Init SPI bus
    mfrc522.PCD_Init();  // Init MFRC522 card

    for (int i = 0; i < 15 ;i++){
      alarms[i] = 69;
    }
    for (int i = 0; i < 15 ;i++){
      snoozes[i] = 69;
    }
    
  // LED
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
    timed_event = 1000;
    current_time = millis();
    start_time = current_time;
    last_blink = current_time;
    delay(50);
    for (int i = 0; i < 30; i++){
        leds[i] = CHSV(0,0,0);
    }
    FastLED.show();
    delay(500);    
}

void loop()
{ 
  current_time = millis();
  now = rtc.now();
  // CHECK FOR INPUTS
  // ENCODER INPUT
    currentStateCLK = digitalRead(CLK);
    if (currentStateCLK != lastStateCLK  && currentStateCLK == 1){
      if (digitalRead(DT) != currentStateCLK) {
        // ENCODER CCW decrease
        encoderChange = 1;
      } else {
        // ENCODER CW increase
        encoderChange = -1;
      }
    } else {
      encoderChange = 0;
    }
    lastStateCLK = currentStateCLK;
    int btnState = digitalRead(SW);
    if (btnState == LOW) {
      if (millis() - lastButtonPress > 50) {
        encoderPressed = true;
      } else {
        encoderPressed = false;
      }
      lastButtonPress = millis();
    }
  // SETUP
    if (setupSeq == 0){
      changeNumber(0,0);
      changeNumber(1,0);
      changeNumber(3,0);
      leds[order[23]] = CHSV(color[0],color[1],color[2]);
      leds[order[24]] = CHSV(color[0],color[1],color[2]);
      leds[order[26]] = CHSV(color[0],color[1],color[2]);
      leds[order[28]] = CHSV(color[0],color[1],color[2]);
      leds[order[29]] = CHSV(color[0],color[1],color[2]);
      FastLED.show();
      fourState = analogRead(A1);
      if (encoderPressed == true){
        LLhigh = analogRead(A3);
        setupSeq = 1;
      }
    } else if (setupSeq == 1){
      changeNumber(0,0);
      changeNumber(1,0);
      changeNumber(3,0);
      leds[order[23]] = CHSV(0,0,0);
      leds[order[24]] = CHSV(0,0,0);
      leds[order[26]] = CHSV(0,0,0);
      leds[order[27]] = CHSV(color[0],color[1],color[2]);
      leds[order[28]] = CHSV(color[0],color[1],color[2]);
      leds[order[29]] = CHSV(color[0],color[1],color[2]);
      FastLED.show();
      if (encoderPressed == true){
        LLlow = analogRead(A3);
        LLmiddle = (LLhigh+LLlow)/2;
        defaultA = sr04.Distance() / 2;
        setupSeq = 2;
      }
    }
    
    if (setupSeq < 2){
      return;
    }
  // CHECK FOR SETTINGS
  // 0-170, 171-511, 512-852, 853-1023
    int fourState = analogRead(A1);
    //Serial.println(analogRead(A1));
    if (fourState >= 0 && fourState <= 170){
      // SHOW TIME

      currentSetting = 0;
      s1stage = 0;
    } else if (fourState >= 171 && fourState <= 511 && currentSetting != 1){
      s1stage = 0;
      currentSetting = 1;\
      
    } else if (fourState >= 171 && fourState <= 511){
      // EDIT TIME/DAY
      // TURN ALL LIGHTS OFF
      currentSetting = 1;
      if (s1stage == 0){
        firstTimeS0 = true;
        for (int i = 0; i < 30; i++){
          leds[i] = CHSV(0,0,0);
        }
        dayOfWeek = now.dayOfTheWeek();
        if (dayOfWeek == 0){
          dayOfWeek = 7;
        }
        changeNumber(0, dayOfWeek);
        s1stage = 1;
      } else if (s1stage == 1){
        if (encoderChange == -1){
          dayOfWeek = dayOfWeek - 1;
          if (dayOfWeek == 0){
            dayOfWeek = 7;
          }
          changeNumber(0,dayOfWeek);
        } else if (encoderChange == 1){
          dayOfWeek = dayOfWeek + 1;
          if (dayOfWeek == 8){
            dayOfWeek = 1;
          }
          changeNumber(0,dayOfWeek);
        } else if (encoderPressed){
          s1stage = 2;
          changeNumber(0,0);
          changeNumber(1,0);
          changeNumber(2,1);
          changeNumber(3,0);
          changeNumber(4,0);
          firstDigits = 0;
          lastDigits = 0; 
        }
      } else if (s1stage == 2) {
        if (encoderChange == -1){
          firstDigits = firstDigits - 1;
          if (firstDigits < 0 ){
            firstDigits = 59;
          }
          setTime(firstDigits, lastDigits);
        } else if (encoderChange == 1){
          firstDigits = firstDigits + 1;
          if (firstDigits == 60){
            firstDigits = 0;
          }
          setTime(firstDigits, lastDigits);
        } else if (encoderPressed){
          s1stage = 3;
        }
      } else if (s1stage == 3){
        if (encoderChange == -1){
          lastDigits = lastDigits - 1;
          if (lastDigits < 0 ){
            lastDigits = 23;
          }
          setTime(firstDigits, lastDigits);
        } else if (encoderChange == 1){
          lastDigits = lastDigits + 1;
          if (lastDigits == 24){
            lastDigits = 0;
          }
          setTime(firstDigits, lastDigits);
        } else if (encoderPressed){
          s1stage = 4;
          // SAVE TIME
          rtc.adjust(DateTime(2021, 11, dayOfWeek, lastDigits, firstDigits, 0));
        }
      } else if (s1stage == 4){
        //blink time and day
        if (current_time - last_blink >= 500){
          if (blinkingSeq == 0 || blinkingSeq == 2){
            for (int i = 0; i < 30; i++){
              leds[i] = CHSV(0,0,0);
            }
            FastLED.show();
          } else if (blinkingSeq == 1){
            changeNumber(0,dayOfWeek);
          } else if (blinkingSeq == 3){
            setTime(firstDigits, lastDigits);
            changeNumber(2,1);
          }
          blinkingSeq = blinkingSeq + 1;
          if (blinkingSeq == 4){
            blinkingSeq = 0;
          }
          last_blink = current_time;
        }
      }
    } else if (fourState >= 512 && fourState <= 852 && currentSetting != 2){
      s2stage = 0;
      currentSetting = 2;
    } else if (fourState >= 512 && fourState <= 852){
      // EDIT ALARM
      currentSetting = 2;
      firstTimeS0 = true;

      if (s2stage == 0){
        changeNumber(2,1);
        firstDigits = alarm[1];
        lastDigits = alarm[0]; 
        setTime(firstDigits,lastDigits);
        s2stage = 1;
        blinkingSeq = 0;
      } else if (s2stage == 1){
        if (encoderChange == -1){
          firstDigits = firstDigits - 1;
          if (firstDigits < 0 ){
            firstDigits = 59;
          }
          setTime(firstDigits, lastDigits);
        } else if (encoderChange == 1){
          firstDigits = firstDigits + 1;
          if (firstDigits == 60){
            firstDigits = 0;
          }
          setTime(firstDigits, lastDigits);
        } else if (encoderPressed){
          s2stage = 2;
        }
      } else if (s2stage == 2){
        if (encoderChange == -1){
          lastDigits = lastDigits - 1;
          if (lastDigits < 0 ){
            lastDigits = 23;
          }
          setTime(firstDigits, lastDigits);
        } else if (encoderChange == 1){
          lastDigits = lastDigits + 1;
          if (lastDigits == 24){
            lastDigits = 0;
          }
          setTime(firstDigits, lastDigits);
        } else if (encoderPressed){
          s2stage = 3;
          // SAVE TIME
          alarmSet = true;
          alarm[1] = firstDigits;
          alarm[0] = lastDigits;
          int *snoozeTime = timeSubtract(firstDigits,lastDigits);
          snooze[0] = snoozeTime[0];
          snooze[1] = snoozeTime[1];
          Serial.println("alarmSet true");
        }
      } else if (s2stage == 3){
        // BLINk
        if (current_time - last_blink >= 500){
          if (blinkingSeq == 0){
            for (int i = 0; i < 30; i++){
              leds[i] = CHSV(0,0,0);
            }
            FastLED.show();
          } else if (blinkingSeq == 1){
            setTime(firstDigits, lastDigits);
            changeNumber(2,1);
          }
          blinkingSeq = blinkingSeq + 1;
          if (blinkingSeq == 2){
            blinkingSeq = 0;
          }
          last_blink = current_time;
        }
      }
    } else if (fourState >= 853 && fourState <= 1023 && currentSetting != 3){
      //Serial.println("Setting 3");
      currentSetting = 3;
      s3stage = 0;
      currentDay = 1;
      firstTimeS0 = true;
      firstDigits = 0;
      lastDigits = 0;
      Serial.println("first time setup setting 3");
    } else if (fourState >= 853 && fourState <= 1023){
      if (s3stage == 0){
        s3delay = current_time;
        Serial.println(s3delay);
        s3stage = 1;
        for (int i = 0; i < 30; i++){
          leds[i] = CHSV(0,0,0);
        }
        FastLED.show();
        changeNumber(4,currentDay);
        Serial.println("ytes stage 0");
      } else if (s3stage == 1 && current_time - 1000 > s3delay){
        Serial.println(current_time);
        s3stage = 2;
        changeNumber(0,1);
      } else if (s3stage == 2){
        if (encoderChange == -1){
          turnON = turnON - 1;
          if (turnON < 0){
            turnON = 2;
          }
          changeNumber(0,turnON);
        } else if (encoderChange == 1){
          turnON = turnON + 1;
          if (turnON > 2){
            turnON = 0;
          }
          changeNumber(0,turnON);
        } else if (encoderPressed){
          Serial.println(turnON);
          if (turnON == 1){
            s3stage = 3;
            if (alarms[2*(currentDay - 1)] < 23 && alarms[2*(currentDay - 1) + 1] < 60){
              setTime(alarms[2*(currentDay - 1)],alarms[2*(currentDay - 1) + 1]);
            } else {
              setTime(0,0);
            }
            
          } else if (turnON == 0){
            // SAVE TIME
            Serial.println("only stage 2");
            Serial.println(2*(currentDay - 1));
            Serial.println(lastDigits);
            alarms[2*(currentDay - 1)] = 24;
            Serial.println(2*(currentDay - 1) + 1);
            Serial.println(firstDigits);
            alarms[2*(currentDay - 1) + 1] = 60;
            snoozes[2*(currentDay - 1)] = 24;
            snoozes[2*(currentDay - 1) + 1] = 60;
            
            currentDay = currentDay + 1;
            turnON = 1;
            firstDigits = 0;
            lastDigits = 0;
            alarmSet = true;
  
            Serial.println("alarmSet true");
            if (currentDay == 8){
              currentDay = 1;
              s3stage = 0;
            } else{
              s3stage = 0;
            }
          } else if (turnON == 2){
            s3stage = 0;
            currentDay = currentDay + 1;
            firstDigits = 0;
            lastDigits = 0;
            turnON = 1;
            alarmSet = true;
            if (currentDay == 8){
              currentDay = 1;
            }
            
          }
        }
      } else if (s3stage == 3){
        if (encoderChange == -1){
          firstDigits = firstDigits - 1;
          if (firstDigits < 0 ){
            firstDigits = 59;
          }
          setTime(firstDigits, lastDigits);
        } else if (encoderChange == 1){
          firstDigits = firstDigits + 1;
          if (firstDigits == 60){
            firstDigits = 0;
          }
          setTime(firstDigits, lastDigits);
        } else if (encoderPressed){
          s3stage = 4;
        }
      } else if (s3stage == 4){
        if (encoderChange == -1){
          lastDigits = lastDigits - 1;
          if (lastDigits < 0 ){
            lastDigits = 23;
          }
          setTime(firstDigits, lastDigits);
        } else if (encoderChange == 1){
          lastDigits = lastDigits + 1;
          if (lastDigits == 24){
            lastDigits = 0;
          }
          setTime(firstDigits, lastDigits);
        } else if (encoderPressed){

          // SAVE TIME
          Serial.println("only stage 4");
          Serial.println(2*(currentDay - 1));
          Serial.println(lastDigits);
          alarms[2*(currentDay - 1)] = lastDigits;
          Serial.println(2*(currentDay - 1) + 1);
          Serial.println(firstDigits);
          alarms[2*(currentDay - 1) + 1] = firstDigits;
          int *snoozeTime = timeSubtract(firstDigits,lastDigits);
          snoozes[2*(currentDay - 1)] = snoozeTime[0];
          snoozes[2*(currentDay - 1) + 1] = snoozeTime[1];
          
          currentDay = currentDay + 1;

          firstDigits = 0;
          lastDigits = 0;
          alarmSet = true;

          Serial.println("alarmSet true");
          if (currentDay == 8){
            currentDay = 1;
            s3stage = 0;
          } else{
            s3stage = 0;
          }
        }
      }
    }

  // SETTING 0
  // UPDATE TIME,2 DOTS EVERY SECOND
    if (currentSetting == 0){
      if (firstTimeS0 && dimMode) {
        timeShortDistance = 0;
        firstTimeS0 = false;
      }

      threeState = analogRead(A0);
      if (threeState >= 0 && threeState <= 255 && typeAlarm != 2){
        // No alarm
        typeAlarm = 2;
        alarmStage = 0;
      } else if (threeState >= 256 && threeState <= 767 && typeAlarm != 1){
        // Standard Alarm
        alarmStage = 0;
        typeAlarm = 1;
        
      } else if (threeState >= 768 && threeState <= 1023 && typeAlarm != 0){
        // Scheduled Alarm
        alarmStage = 0;
        typeAlarm = 0;
      }

      if (typeAlarm == 1 || typeAlarm == 2){
        if (alarmStage == 1){
          if (current_time - last_blink >= 500){
            Serial.println(typeAlarm);
            Serial.println(alarmStage);
            last_blink = current_time;
            if (toneSeq == 0){
              Serial.println("noise 1");
              tone(3,1976,500);
              toneSeq = 1;
            } else if (toneSeq == 1){
              Serial.println("noise 2");
              tone(3,4186,500);
              toneSeq = 0;
            }
          }
          if (lastAlarm + 600000 < current_time){
            alarmStage = 0;
          }
          
          a=sr04.Distance();
          if (a < defaultA){
            alarmStage = 2;
            Serial.println("alarmStage 2");
          }
          // make noise
          // if hand above sensor snooze -> stage 2
        } else if (alarmStage == 3){
          // make noise
          // if pass is detected stop -> stage 0
          if (current_time - last_blink >= 500){
            last_blink = current_time;
            if (toneSeq == 0){
              Serial.println("noise noise 1");
              tone(3,1976,500);
              toneSeq = 1;
            } else if (toneSeq == 1){
              Serial.println("noise noise 2");
              tone(3,4186,500);
              toneSeq = 0;
            }
          }

          if (lastAlarm + 600000 < current_time){
            alarmStage = 0;
          }
          
          if ( mfrc522.PICC_IsNewCardPresent() || mfrc522.PICC_ReadCardSerial() ) {
            alarmStage = 0;
            Serial.println("alarmStage 0");
          }
        }
      }
      
      if (dimMode && (current_time > timeShortDistance + lightDelay) && alreadyChanged == false){
        //Serial.println("lights off");
        for (int i = 0; i < 30; i++){
          leds[i] = CHSV(0,0,0);
        }
        FastLED.show();
      }

      firstDigits = now.minute();
      lastDigits = now.hour();
      if (current_time - start_time >= timed_event){
         if (dimMode == false){
          if (dotsAreOn){
            changeNumber(2,0);
            dotsAreOn = false;
          } else{
            changeNumber(2,1);
            dotsAreOn = true;
          }
           setTime(firstDigits,lastDigits);
           start_time = current_time;
         }
        if (typeAlarm == 1){
          // standard alarm
          if (firstDigits == snooze[1] && lastDigits == snooze[0] && alarmStage == 0 && alarmSet == true && now.second() < 2){
            alarmStage = 1;
            lastAlarm = current_time;
            Serial.println("alarmStage 1");
          } else if (firstDigits == alarm[1] && lastDigits == alarm[0] && alarmStage == 2 && alarmSet == true && now.second() < 2){
            alarmStage = 3;
            lastAlarm = current_time;
            Serial.println("alarmStage 3");
          }
        } else if (typeAlarm == 2){
          // scheduled alarm
          currentDay = now.dayOfTheWeek();
          if (dayOfWeek == 0){
            dayOfWeek = 7;
          }
          if (firstDigits == snoozes[2*(currentDay - 1) + 1] && lastDigits == snoozes[2*(currentDay - 1)] && now.second() < 2 && alarmSet == true && alarmStage == 0){
            // current day
            Serial.println("alarmStage 1 : 2");
            lastAlarm = current_time;
            alarmStage = 1;
          }
          currentDay++;
          if (firstDigits == snoozes[2*((currentDay - 1) % 7) + 1] && lastDigits == snoozes[2*((currentDay - 1) % 7)] && now.second() < 2 && alarmSet == true && alarmStage == 0){
            // next day till 00:07 (23:59)
            if (snoozes[(2*((currentDay - 1) % 7) + 1)] > 51 && snoozes[2*((currentDay - 1) % 7)] == 23){
              Serial.println("alarmStage 1 : 2");
              lastAlarm = current_time;
              alarmStage = 1;
            }
          }
          currentDay--;
          if (firstDigits == alarms[2*(currentDay - 1) + 1] && lastDigits == alarms[2*(currentDay - 1)] && now.second() < 2 && alarmSet == true && alarmStage == 2){
            // current day
            Serial.println("alarmStage 3 : 2");
            lastAlarm = current_time;
            alarmStage = 3;
          }
        }
      }

      if (LLhigh != 1){
        currentLL = analogRead(A3);
        if (currentLL < LLmiddle){
          dimMode = true;
          a=sr04.Distance();

          if (!isInDimMode){
            for (int i = 0; i < 30; i++){
              leds[i] = CHSV(0,0,0);
            }
            FastLED.show();
            isInDimMode = true;
          }
          
          if ((a < defaultA) && (current_time >= timeShortDistance + lightDelay || timeShortDistance == 0)){
            // light up, but dim
            //Serial.println("timeShortDistance ingesteld");
            timeShortDistance = current_time;
          } 
          // lights (off) or (on, but dim)
          Serial.println((current_time - start_time >= timed_event)); /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
          if ((current_time <= timeShortDistance + lightDelay) && (alreadyChanged == false) && (timeShortDistance != 0)){
            // lights on but dim
            Serial.println("SEtupt");
            color[2] = 20;
            color[0] = 15;
            if (dotsAreOn){
              changeNumber(2,0);
              dotsAreOn = false;
            } else{
              changeNumber(2,1);
              dotsAreOn = true;
            }
            setTime(firstDigits,lastDigits);
            alreadyChanged = true;
            start_time = current_time;
          } else if ((current_time <= timeShortDistance + lightDelay) && (alreadyChanged == true) && (current_time - start_time >= timed_event)){
            Serial.println("here");
            if (dotsAreOn){
              changeNumber(2,0);
              dotsAreOn = false;
            } else{
              changeNumber(2,1);
              dotsAreOn = true;
            }
            setTime(firstDigits,lastDigits);
            start_time = current_time;
          } else if ((current_time > timeShortDistance + lightDelay) && alreadyChanged == true) {
            // lights off
            alreadyChanged = false;
            //Serial.println("lights off");
            for (int i = 0; i < 30; i++){
              leds[i] = CHSV(0,0,0);
            }
            FastLED.show();
          } 
          
        } else if (currentLL > LLmiddle){
          dimMode = false;
          isInDimMode = false;
          color[2] = valueCode;
          color[0] = colorCode;
          timeShortDistance = current_time - lightDelay - 5;
          //Serial.println("Normal mode");
          // Normal mode
        }
      }
      
      if (encoderChange == -1 && dimMode == false){
        if (s0stage == 0){
          colorCode = colorCode - 5;
          if (colorCode < 0 ){
            colorCode = 255;
          }
          color[0] = colorCode;
        } else if (s0stage == 1){
          if (valueCode > 5){
          valueCode = valueCode - 5;
          }
          color[2] = valueCode;
        }
        setTime(firstDigits, lastDigits);
        if (dotsAreOn){
          changeNumber(2,1);
        } else{
          changeNumber(2,0);
        }
      } else if (encoderChange == 1 && dimMode == false){
        if (s0stage == 0){
          colorCode = colorCode + 5;
          if (colorCode > 255){
            colorCode = 0;
          }
          color[0] = colorCode;
        } else if (s0stage == 1){
          if (valueCode < 251){
          valueCode = valueCode + 5;
          }
          color[2] = valueCode;
        }
        setTime(firstDigits, lastDigits);
        if (dotsAreOn){
          changeNumber(2,1);
        } else{
          changeNumber(2,0);
        }
      } else if (encoderPressed == true && dimMode == false){
        if (s0stage == 0){
          s0stage = 1;
        } else {
          s0stage = 0;
        }
      }
    }
}
