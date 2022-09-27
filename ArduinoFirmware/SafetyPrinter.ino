/**
 * Safety Printer Firmware
 * Copyright (c) 2021~22 Rodrigo C. C. Silva [https://github.com/SinisterRj/SafetyPrinter]
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * 
 * WARNING: DON'T CHANGE ANYTHING IN THIS FILE! ALL CONFIGURATIONS SHOULD BE DONE IN "Configurations.h".
 * 
 * Version 0.2.6rc1
 * 30/08/22
 * Changes:
 * 1) New default pin out to match with the official Safety Printer Arduino Shield;
 * 2) Include <R6> command to repeat intro msg;
 * 3) Remove "booting..." string from the setup;
 * 4) Auto detects different Arduino boards;
 * 5) Include command <r7> to report Printer's power status (user request) - Integration with PSU Control;
 * 6) Include command <c9> to reset the Safety Printer MCU. This command allows firmware update.
 * 7) Command <R4> returns board Type;
 * 8) Include ARDUINO_RESET_PIN;
 * 9) Include neopixels as status indicator;
 * 10) Fix a bug that makes wrong temperature readings if your declared sensor that don't match with the sensor's count.
 * 
 * Version 0.2.5
 * 10/21/2021
 * Changes:
 * 1) Include CRC check on <r1>,<r2> and <r4> responses;
 * 2) Organize some thermisotr tables and Configuration.h file;
 * 3) Check for some errors on sensor configuration and disable it.
 * 4) Include commands alias;
 * 5) Include free memory, voltage, temperature and cycle time monitoring;
 * 6) Include I2C 16x2 LCD support
 * 7) Creates "lowSP" and "highSP" sensor values (now user can choose different sensors);
 * 8) <R2> now sends "forceEnable", "lowSP" and "highSP" values;
 * 9) Include a timer to avoid sucessive trips/resets or turns on/off on printer and damage it.
 * 
 * Version 0.2.4
 * 07/30/2021
 * Changes:
 * 1) Include Marlin Thermistor files for better thermistor configuration;
 * 2) Configuration.h created.
 *
 * Version 0.2.3
 * Changes:
 * 1) Fix a bug when reading EEPROM data.
 * 2) Split project into multiple files.
 * 3) Checks EEPROM for CRC.
 * 4) Update <c4> command to send EEPROM and Serial Protocol versions
 * 
 * Version 0.2.2
 * Changes:
 * 1) Defined limits for set point definition.
 * 2) Now EEPROM read uses the addr 0 as a version flag. If it is different from EEPROMVERSION (because new release changed EEPROM structure), the firmware will overwrite it.
 * 3) Include command C5 to turn off the printer.
 * 4) Commands and arguments are now case insensitive.
 * 5) Included command <c7> to change sensor timers.
 * 6) included command <c1> to return sensors configuration (enabled, alarm set point and timer) back to original values.
 * 7) Fix a bug that interpretates no argument as argument 0 in some commands.
 * 
 * Version: 0.2.1
 * 05/26/2021
 * Changes: 
 * 1) Include analog type of sensor to allow Hotend temperature measurement.
 * 2) included command <c5> to save on EEPROM sensors Enabled status and alarm set points
 * 3) 2 new serial commands: Remote shutdown and enable/disable sensors 
 * 4) Now it indicates Alarm even if the sensor is disabled (but not starts de trip)
 * 5) Included command <r4> to return firmware version and release date
 * 
 */
 
//#define DEBUG   
#ifdef DEBUG                                // Just for debug porpouses
  #define DEBUG_DELAY           1000        // Debug timer renew
#endif

#define LOOP_TIME_SAMPLES       25          // Number of samples to calculate main loop time.

#define DIGIGTAL_SENSOR         0           // Internal use, don't change
#define NTC_SENSOR              1           // Internal use, don't change

// Board Codes
#define UNO                     1
#define NANO                    2
#define LEONARDO                3

#include <Arduino.h>                        // Needed to compile with Platformio
#include <avr/wdt.h>                        // Watchdog
#include <EEPROM.h>                         // EEPROM access
#include "Configuration.h"

#ifdef HAS_LCD
  #include <Wire.h>
  #include <LiquidCrystal_I2C.h>
#endif

// *************************** Don't change after this line **********************************
// *******************************************************************************************

#define VERSION                 "0.2.6"
#define RELEASEDATE             "Aug 30 2022"
#define EEPROMVERSION           3            // Incremental BYTE. Firmware overwrites the EEPROM with standard values if the number readed from EEPROM is different. change everytime that EEPROM structure is changed.
#define COMMPROTOCOL            5            // Incremental BYTE. Octoprint plugin communication protocol version. 

char forb_chars[] = {',','#','$',':','<','>'};   // forbiden characters for sensor names
//int availableThermistors[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,15,17,18,20,21,22,23,30,51,52,55,60,61,66,67,70,71,75,99,331,332,998,999};
 
// another .ino functions:
int highest_temp(byte sensorNumber);
int lowest_temp(byte sensorNumber);

typedef struct
{
  int8_t index;
  char label[17];
  int8_t pin;
  int8_t auxPin;
  uint8_t type;
  unsigned long timer;
  int alarmSP;
  bool enabled;
  bool active;
  int actualValue;
  bool forceDisable;
  bool trigger;
  int lowSP;
  int highSP;
  int16_t tTI; //temp table index
} tSensor;

typedef struct
{
  unsigned long timer;
  int alarmSP;
  bool enabled;
} tDefault;

/* ****************************************************************************
 Declare all sensors using the above template
 "label",pin number,delay,enabled,alarmStatus,forceDisable,trigger,spare3,spare4

 Label: the name of the sensor, that will help you to intentify it;
 Pin number: the pin wher it is connected;
 Delay: The timer delay to start the interlock from this sensor. If two or more sensors are active, the delay is bypassed; 
 Normal Condition: The variable status during normal operation. For analog variables, use the greater normal value (i.e. 279 if you want to trip with 280)
 enabled, alarmStatus: Internal use.

*/

tSensor sensors [] =
{   
#ifdef SENSOR_1_LABEL
   0,SENSOR_1_LABEL,SENSOR_1_PIN,
   #ifdef SENSOR_1_AUX_PIN 
      SENSOR_1_AUX_PIN, 
   #else 
      -1, 
   #endif
   SENSOR_1_TYPE,SENSOR_1_TIMER,SENSOR_1_ALARM_SP,true,false,0,false,false,
   #ifdef SENSOR_1_TEMP_TYPE 
      0,0,SENSOR_1_TEMP_TYPE, 
   #else 
      0,1,0,
   #endif
#endif 
#ifdef SENSOR_2_LABEL
   1,SENSOR_2_LABEL,SENSOR_2_PIN,
   #ifdef SENSOR_2_AUX_PIN 
      SENSOR_2_AUX_PIN, 
   #else 
      -1, 
   #endif
   SENSOR_2_TYPE,SENSOR_2_TIMER,SENSOR_2_ALARM_SP,true,false,0,false,false,
   #ifdef SENSOR_2_TEMP_TYPE 
      0,0,SENSOR_2_TEMP_TYPE, 
   #else 
      0,1,0,
   #endif
#endif 
#ifdef SENSOR_3_LABEL
   2,SENSOR_3_LABEL,SENSOR_3_PIN,
   #ifdef SENSOR_3_AUX_PIN 
      SENSOR_3_AUX_PIN, 
   #else 
      -1, 
   #endif
   SENSOR_3_TYPE,SENSOR_3_TIMER,SENSOR_3_ALARM_SP,true,false,0,false,false,
   #ifdef SENSOR_3_TEMP_TYPE 
      0,0,SENSOR_3_TEMP_TYPE, 
   #else 
      0,1,0, 
   #endif
#endif 
#ifdef SENSOR_4_LABEL
   3,SENSOR_4_LABEL,SENSOR_4_PIN,
   #ifdef SENSOR_4_AUX_PIN 
      SENSOR_4_AUX_PIN, 
   #else 
      -1, 
   #endif
   SENSOR_4_TYPE,SENSOR_4_TIMER,SENSOR_4_ALARM_SP,true,false,0,false,false,
   #ifdef SENSOR_4_TEMP_TYPE 
      0,0,SENSOR_4_TEMP_TYPE, 
   #else 
      0,1,0, 
   #endif
#endif 
#ifdef SENSOR_5_LABEL
   4,SENSOR_5_LABEL,SENSOR_5_PIN,
   #ifdef SENSOR_5_AUX_PIN 
      SENSOR_5_AUX_PIN, 
   #else 
      -1, 
   #endif
   SENSOR_5_TYPE,SENSOR_5_TIMER,SENSOR_5_ALARM_SP,true,false,0,false,false,
   #ifdef SENSOR_5_TEMP_TYPE 
      0,0,SENSOR_5_TEMP_TYPE, 
   #else 
      0,1,0, 
   #endif
#endif 
#ifdef SENSOR_6_LABEL
   5,SENSOR_6_LABEL,SENSOR_6_PIN,
   #ifdef SENSOR_6_AUX_PIN 
      SENSOR_6_AUX_PIN, 
   #else 
      -1, 
   #endif
   SENSOR_6_TYPE,SENSOR_6_TIMER,SENSOR_6_ALARM_SP,true,false,0,false,false,
   #ifdef SENSOR_6_TEMP_TYPE 
      0,0,SENSOR_6_TEMP_TYPE, 
   #else 
      0,0,0, 
   #endif
#endif 
#ifdef SENSOR_7_LABEL
   6,SENSOR_7_LABEL,SENSOR_7_PIN,
   #ifdef SENSOR_7_AUX_PIN 
      SENSOR_7_AUX_PIN, 
   #else 
      -1, 
   #endif
   SENSOR_7_TYPE,SENSOR_7_TIMER,SENSOR_7_ALARM_SP,true,false,0,false,false,
   #ifdef SENSOR_7_TEMP_TYPE 
      0,0,SENSOR_7_TEMP_TYPE, 
   #else 
      0,1,0, 
   #endif
#endif 
#ifdef SENSOR_8_LABEL
   7,SENSOR_8_LABEL,SENSOR_8_PIN,
   #ifdef SENSOR_8_AUX_PIN 
      SENSOR_8_AUX_PIN, 
   #else 
      -1, 
   #endif
   SENSOR_8_TYPE,SENSOR_8_TIMER,SENSOR_8_ALARM_SP,true,false,0,false,false,
   #ifdef SENSOR_8_TEMP_TYPE 
      0,0,SENSOR_8_TEMP_TYPE, 
   #else 
      0,1,0, 
   #endif
#endif 
};

tDefault defaultSensors[(sizeof(sensors) / sizeof(sensors[0]))];

typedef struct
{
  unsigned long startMs = 0;
  bool started = false;
} tTimer;

/*
//Defines the SP limits for each of the sensor types. 
int SPLimits[2][2] = {
  { 0, 1 },   // type 0 = Digital
  { 0, 300 }  // type 1 = NTC temperature
};*/
bool interlockStatus = false;
bool interlockStatusChanged = false;
bool anyAlarm = false;
uint8_t activeAlarmCount = 0;
uint8_t numOfSensors = 0;
tTimer interlockTimer, resetTimer, minimumInterlockTimer;
bool printerPowered = true;
unsigned int lTLastRecord = 0;
long lTStartTime;
long lTSum = 0;
long lTMax = 0;
long lTNow, lTLastMax, lTLastSum;
bool memWrng, tempWrng, voltWrng, execWrng  = false;
byte triggerIndex = 255;
bool resetBtnPressed = false;



#ifdef DEBUG
   tTimer debugTimer;
#endif

#ifdef HAS_LCD
  tTimer LCDTimer;
  bool tripLCD = false;
#endif

#ifdef HAS_SERIAL_COMM
  #define SERIAL Serial
#endif

void setup() {

   //Enabling watchdog timer 4s  
   wdt_enable(WDTO_4S);

   pinMode(ARDUINO_RESET_PIN, OUTPUT);
   digitalWrite(ARDUINO_RESET_PIN, HIGH);
   
   #ifdef HAS_SERIAL_COMM
      SERIAL.begin(BAUD_RATE);
   #endif

   #ifdef HAS_LCD
      lcd_Init();
   #endif
   
   #ifdef HAS_NEOPIXEL ||  ALARM_LED_PIN || TRIP_LED_PIN  
      led_Init();
   #endif
  
   #ifdef RESET_BUTTON_PIN
      pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
   #endif

   pinMode(INTERLOCK_RELAY_PIN, OUTPUT);
   digitalWrite(INTERLOCK_RELAY_PIN, INTERLOCK_POLARITY);
   startTimer(&minimumInterlockTimer.startMs,&minimumInterlockTimer.started);

   numOfSensors = (sizeof(sensors) / sizeof(sensors[0]));
  
   // check sensor configuration
   validateSensorsInfo();
   
   //Set sensor input pins
   for (int i =0; i < numOfSensors; i++) { 
      if (sensors[i].enabled && !sensors[i].forceDisable) 
      {
         pinMode(sensors[i].pin, INPUT);
         if (sensors[i].auxPin != -1) {
            pinMode(sensors[i].auxPin, OUTPUT);  // Configure power pin. This pin will be used to power the thermistor
         }
      }          
   }

   for (int i =0; i < numOfSensors; i++) { 
      defaultSensors[i].timer = sensors[i].timer;
      defaultSensors[i].alarmSP = sensors[i].alarmSP;
      if (!sensors[i].forceDisable) {
         defaultSensors[i].enabled = sensors[i].enabled;
      } else {
         defaultSensors[i].enabled = false;
      }
   }

   readEEPROMData(); 

   #ifdef DEBUG
      startTimer(&debugTimer.startMs,&debugTimer.started);
   #endif

   #ifdef HAS_LCD
      startTimer(&LCDTimer.startMs,&LCDTimer.started);
   #endif
}

void loop() {

   // Save the loop start time
   lTStartTime = micros();
 
   //Verify sensors
   checkSensors();

   //Verify reset button
   #ifdef RESET_BUTTON_PIN
      checkResetButton();
   #endif

   //Update LEDs
   updateLEDs();   
  
   //Manage Serial communications
   #ifdef HAS_SERIAL_COMM
      // Receive commands from PC
      recvCommandWithStartEndMarkers(); 
   #endif

   //update LCD information 
   #ifdef HAS_LCD
      updateLCD(resetBtnPressed);
   #endif   

   //Debug Info
   #ifdef DEBUG
      if (checkTimer(&debugTimer.startMs,DEBUG_DELAY,&debugTimer.started)) { 
         SERIAL.println();
         SERIAL.print (F("Free memory [bytes]= "));  // 2048 bytes from datasheet
         SERIAL.println (freeMemory());
         SERIAL.print (F("Temperature [C]= "));
         SERIAL.println (readTemp(), 2);
         SERIAL.print (F("Voltage [V]= "));
         SERIAL.println (readVcc(), 2);  // 2.7V to 5.5V from datasheet
         SERIAL.print (F("Execution time max [us] = "));
         SERIAL.println (lTLastMax,DEC); 
         SERIAL.print (F("Execution time average [us] = "));
         SERIAL.println (lTLastSum/LOOP_TIME_SAMPLES, DEC); 
      } else {
         startTimer(&debugTimer.startMs,&debugTimer.started);
      } 
   #endif

   // check board health:
   if (freeMemory() < 205) {  // 10% of total SRAM
      memWrng = true;
   }
   if ((readTemp() > 110) || (readTemp() < -25)) {  // -40 to +125 +-15
      tempWrng = true;
   }
   if ((readVcc() < 2.7) || (readVcc() > 5.5)) { // 2.7 to 5.5 Vcd 
       voltWrng = true;
   }

   // Measures the execution time
   lTLastRecord++;   
   lTNow = micros() - lTStartTime;   
   lTSum += lTNow;
   
   if (lTNow > lTMax) {
      lTMax = lTNow;    
   }
   
   if (lTLastRecord >= LOOP_TIME_SAMPLES) {
      lTLastSum = lTSum;
      lTSum = 0;
      lTLastMax = lTMax;
      lTMax = 0;      
      lTLastRecord = 0;
      if (lTLastMax > 500000) {   // 500 ms
         execWrng = true;
      }
   } 
   
   //Reset watchdog timer
   wdt_reset();
}

void validateSensorsInfo() {   
   // Check sensor configurations that cannot be done by pre-processor
   byte usedPins[21];
   byte arrayPos = 0;
   #ifdef INTERLOCK_RELAY_PIN
      usedPins[arrayPos] = INTERLOCK_RELAY_PIN;
      arrayPos++;
   #endif

   #ifdef RESET_BUTTON_PIN
      usedPins[arrayPos] = RESET_BUTTON_PIN;
      arrayPos++;
   #endif
    
   #ifdef ALARM_LED_PIN
      usedPins[arrayPos] = ALARM_LED_PIN;
      arrayPos++;
   #endif
    
   #ifdef TRIP_LED_PIN
      usedPins[arrayPos] = TRIP_LED_PIN;
      arrayPos++;
   #endif
    
   #ifdef LCD_SDA_PIN
      usedPins[arrayPos] = LCD_SDA_PIN;
      arrayPos++;
   #endif
    
   #ifdef LCD_SCL_PIN
      usedPins[arrayPos] = LCD_SCL_PIN;
      arrayPos++;
   #endif
   
   for (byte i = 0; i < numOfSensors; i++) {
      //sensors[i].label = sensors[i].label.substring(0,16);
      for (byte j = 0; j < sizeof(sensors[i].label); j++) {
         for (byte x = 0; x < sizeof(forb_chars); x++) {
            //sensors[i].label.replace(forb_chars[j],'?');
            if (sensors[i].label[j] == forb_chars[x]) {
               sensors[i].label[j] = '?';
            }
         }
      }
      usedPins[arrayPos] = sensors[i].pin;
      arrayPos++;
      if (sensors[i].auxPin != -1) {
         usedPins[arrayPos] = sensors[i].auxPin;
         arrayPos++;
      }
      if (sensors[i].type == NTC_SENSOR) {
         sensors[i].highSP = highest_temp(sensors[i].index);
      }
      if (sensors[i].type == NTC_SENSOR) {
         sensors[i].lowSP = lowest_temp(sensors[i].index);
      }     
      if ((sensors[i].alarmSP < sensors[i].lowSP) || (sensors[i].alarmSP > sensors[i].highSP)) {
         #ifdef HAS_SERIAL_COMM
            SERIAL.print(sensors[i].label);
            SERIAL.print(F(": Wrong sensor ALARM SET POINT ("));
            SERIAL.print(sensors[i].alarmSP);
            SERIAL.print(F("). Range: "));
            SERIAL.print(sensors[i].lowSP);
            SERIAL.print(F(" to "));
            SERIAL.print(sensors[i].highSP);
            SERIAL.println(F(". Check your Configuration.h file. Sensor disabled."));
         #endif
         sensors[i].alarmSP = 0;
         sensors[i].enabled = false;
         sensors[i].forceDisable = true;
      }    
   }
   for (byte i = 0; i < arrayPos-1; i++) {
      for (byte j = i+1; j < arrayPos; j++) {
         if (usedPins[i] == usedPins[j] && usedPins[i] != -1){
            // Duplicated pin assignment found.
            for (byte x = 0; x < numOfSensors; x++){
               if (sensors[x].pin == usedPins[i] || sensors[x].auxPin == usedPins[i]) {
                  sensors[x].enabled = false;
                  sensors[x].forceDisable = true;
                  #ifdef HAS_SERIAL_COMM
                     SERIAL.print(sensors[x].label);
                     SERIAL.println(F(": Multiple assings to the same I/O pin. Check your Configuration.h file. Sensor disabled."));
                  #endif
               }
            }
         }
      }
   }
}

void checkSensors() {
   // Check all inputs for new alamrs
   activeAlarmCount = 0;
   anyAlarm = false;
   for (byte i =0; i < numOfSensors; i++) { 
      if (sensors[i].type == DIGIGTAL_SENSOR){
        sensors[i].actualValue = digitalRead(sensors[i].pin);
        if (sensors[i].actualValue == sensors[i].alarmSP) {
           setAlarm(i);
        } else {
           sensors[i].active = false;
        }
      }
      else if (sensors[i].type == NTC_SENSOR) {
        sensors[i].actualValue = read_temp(sensors[i].pin, sensors[i].auxPin, sensors[i].index);
        if (sensors[i].actualValue >= sensors[i].alarmSP) {
           setAlarm(i);
        } else {
           sensors[i].active = false;
        }
      }
   }
   if ((activeAlarmCount == 0) and interlockTimer.started) {
      interlockTimer.started = false;
   } else if (activeAlarmCount > 1) {
      interlock(false,0);      
   }
}

#ifdef RESET_BUTTON_PIN
void checkResetButton() {
   // Check if the reset button [RESET_BUTTON_PIN] is pressed, else reset the timer.
   if (interlockStatus) {
      if (!digitalRead(RESET_BUTTON_PIN)){
         #ifdef HAS_LCD
            if (!resetBtnPressed) {
               resetBtnPressed = true;
               includeExtraMsg(0,10,false);
            }
         #endif
         if (resetInterlock(true)) {
            #ifdef HAS_LCD
               includeExtraMsg(0,1,false);
            #endif      
         } 
      } else {
        resetBtnPressed = false;
        resetTimer.started = false;
      } 
   }
}
#endif

void updateLEDs() {
  //Update Interlock and Alarm indication status led
  if (!anyAlarm) { 
    alarmLedLow();    
  }
  if (interlockStatusChanged) {
    //LED indication of interlock status
    interlockStatusChanged = false;
    #if defined(ALARM_LED_PIN) || defined(TRIP_LED_PIN) || defined(HAS_NEOPIXEL)
      updateInterlockLed(interlockStatus);
    #endif

    updateEEPROMinterlock();
    #ifdef HAS_LCD
      tripLCD = interlockStatus;
    #endif
  }  
   // Heart beat (using alarm and trip LEDs to test them)
   ledHeartBeat();
}

void setAlarm(byte index) {
  // Turn an Alarm active, start timers and light Alarm LED [ALARM_LED_PIN].
  anyAlarm = true;
  if (sensors[index].enabled) {
     activeAlarmCount++;
     if (triggerIndex == 255) {
        triggerIndex = index;
        sensors[index].trigger = true;
     }
     interlock(true,sensors[index].timer);
  }
  sensors[index].active = true;
  updateAlarmLed();
}

void startTimer(unsigned long *pStartMS, bool *timerStatus) {
   // Starts any timer
   if (!*timerStatus) {
      *pStartMS = millis();
      *timerStatus = true;
   }
}

bool checkTimer(unsigned long *pStartMS, unsigned int delayms, bool *timerStatus){
   // Checks if the timer is finished returning True on that condition
   if (!*timerStatus) {
     return false;
   } else {
      unsigned long nowMs = millis();
      if (nowMs > *pStartMS) {
          if ((nowMs - *pStartMS) > delayms) {   
            *timerStatus = false;
            return true;
        } else{
            return false;
        }
      } else {
         *pStartMS = millis();
         return false;
      }  
   }     
}

void interlock(bool useTimer, int interlockDelay) {
   // Change the output [INTERLOCK_RELAY_PIN] to an interlock position [INTERLOCK_POLARITY] after some time to prevent spurious trips
   bool interlockFlag = false;
   if (!useTimer) {
      interlockFlag = true;
   } else {
      if (checkTimer(&interlockTimer.startMs,interlockDelay,&interlockTimer.started)) { 
         if (!interlockStatus) {
            interlockFlag = true;
         }
      } else {
         startTimer(&interlockTimer.startMs,&interlockTimer.started);
      } 
   }  
   if (interlockFlag) {
       if (!minimumInterlockTimer.started) {
          startTimer(&minimumInterlockTimer.startMs,&minimumInterlockTimer.started);
       }
       interlockStatus = true;
       interlockStatusChanged = true;
       digitalWrite(INTERLOCK_RELAY_PIN,INTERLOCK_POLARITY);
       #ifdef TRIP_LED_PIN
          digitalWrite(TRIP_LED_PIN, HIGH); 
       #endif
   }   
}

bool turnOnOff(bool on) {
   // Change the output [INTERLOCK_RELAY_PIN]. Different as interlock() because it don't change internal status flags to enable a simple turn on/off. Return true if command was applied.
   if (on && !interlockStatus){
      if (checkTimer(&minimumInterlockTimer.startMs,MINIMUM_INTERLOCK_DELAY * 1000,&minimumInterlockTimer.started)) {
        printerPowered = true;
        digitalWrite(INTERLOCK_RELAY_PIN,!INTERLOCK_POLARITY);
        return true;
      } else {
        return false;
      }
   } else {
      printerPowered = false;
      digitalWrite(INTERLOCK_RELAY_PIN,INTERLOCK_POLARITY);
      if (!minimumInterlockTimer.started) {
         startTimer(&minimumInterlockTimer.startMs,&minimumInterlockTimer.started);
      }
      return true;
   } 
   return false;  
}

bool resetInterlock(bool useTimer) {
   // Change the output [INTERLOCK_RELAY_PIN] to normal position [NOT INTERLOCK_POLARITY] after some time to prevent spurious resets. Return "true" if MINIMUM_INTERLOCK_DELAY
   bool resetFlag = false;
   if (!useTimer) {
      resetFlag = true;
   } else { 
     if (checkTimer(&resetTimer.startMs,RESET_DELAY,&resetTimer.started)) { 
        resetFlag = true;
     } else {
        startTimer(&resetTimer.startMs,&resetTimer.started);
     }
   }
   if (resetFlag) {
      if (checkTimer(&minimumInterlockTimer.startMs,MINIMUM_INTERLOCK_DELAY * 1000,&minimumInterlockTimer.started)) {
         interlockStatus = false;
         interlockStatusChanged = true;
         triggerIndex = 255;
         for (byte i = 0; i < numOfSensors; i++){
            sensors[i].trigger = false;
         }
         if (printerPowered) { 
           digitalWrite(INTERLOCK_RELAY_PIN,!INTERLOCK_POLARITY);
         }
         return true;
      }
      else {
         return false;
      }
   }
   return false;
}

#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

int freeMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}

float readTemp() { 

  unsigned int wADC;
  float t;

  // The internal temperature has to be used
  // with the internal reference of 1.1V.
  // Channel 8 can not be selected with
  // the analogRead function yet.

  // Set the internal reference and mux.
  ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
  ADCSRA |= _BV(ADEN);  // enable the ADC

  delay(20);            // wait for voltages to become stable.

  ADCSRA |= _BV(ADSC);  // Start the ADC

  // Detect end-of-conversion
  while (bit_is_set(ADCSRA,ADSC));

  // Reading register "ADCW" takes care of how to read ADCL and ADCH.
  wADC = ADCW;

  // The offset of 324.31 could be wrong. It is just an indication.
  t = (wADC - 324.31 ) / 1.22;

  // The returned temperature is in degrees Celcius.
  return (t);
  
}

float readVcc() { 
   long result; // Read 1.1V reference against AVcc   
   ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
   delay(2); // Wait for Vref to settle 
   ADCSRA |= _BV(ADSC); // Convert 
   while (bit_is_set(ADCSRA,ADSC)); 
  
   result = ADCL;
   result |= ADCH<<8; 
   result = 1126400L / result; // Back-calculate AVcc in mV 

   return result / 1000.0; // convert to volts
}
