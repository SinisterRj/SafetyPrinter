/**
 * Safety Printer Firmware
 * Copyright (c) 2021 Rodrigo C. C. Silva [https://github.com/SinisterRj/SafetyPrinter]
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
 * Version 0.2.5
 * 09/06/2021
 * Changes:
 * 1) Include CRC check on <r1>,<r2> and <r4> responses;
 * 2) Organize some thermisotr tables and Configuration.h file;
 * 3) Check for some errors on sensor configuration and disable it.
 * 4) Include commands alias;
 * 5) Include free memory, voltage, temperature and cycle time monitoring;
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
 * 6) included command <c8> to return sensors configuration (enabled, alarm set point and timer) back to original values.
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
#define DEBUG_DELAY           1000        // Debug timer renew

#define LOOP_TIME_SAMPLES     25          // Number of samples to calculate main loop time.

#define DigitalSensor         0           // Internal use, don't change
#define NTCSensor             1           // Internal use, don't change
 
#include <Arduino.h>
#include <avr/wdt.h>      //Watchdog
#include <EEPROM.h>       //EEPROM access
#include "Configuration.h"

// *************************** Don't change after this line **********************************
// *******************************************************************************************

#define VERSION         "0.2.5"
#define RELEASEDATE     "Sep 06 2021"
#define EEPROMVERSION   3            // Incremental BYTE. Firmware overwrites the EEPROM with standard values if the number readed from EEPROM is different. change everytime that EEPROM structure is changed.
#define COMMPROTOCOL    2            // Incremental BYTE. Octoprint plugin communication protocol version. 

char forb_chars[] = {',','#','$'};   // forbiden characters for sensor names
//int availableThermistors[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,15,17,18,20,21,22,23,30,51,52,55,60,61,66,67,70,71,75,99,331,332,998,999};
 
typedef struct
{
  String label;
  int8_t pin;
  int8_t auxPin;
  uint8_t type;
  unsigned long timer;
  int alarmSP;
  bool enabled;
  bool active;
  int actualValue;
  bool forceDisable;
  bool spare2;
  int spare3;
  int spare4;
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
 "label",pin number,delay,enabled(always true),alarmStatus(always false),spare1,spare2,spare3,spare4

 Label: the name of the sensor, that will help you to intentify it;
 Pin number: the pin wher it is connected;
 Delay: The timer delay to start the interlock from this sensor. If two or more sensors are active, the delay is bypassed; 
 Normal Condition: The variable status during normal operation. For analog varaibles, use the greater normal value (i.e. 279 if you want to trip with 280)
 enabled, alarmStatus: Internal use.

*/

tSensor sensors [] =
{   
#ifdef Sensor1Label
   Sensor1Label,Sensor1Pin,
   #ifdef Sensor1AuxPin 
      Sensor1AuxPin, 
   #else 
      -1, 
   #endif
   Sensor1Type,Sensor1Timer,Sensor1AlarmSP,true,false,0,false,false,0,0,
   #ifdef Sensor1TempType 
      Sensor1TempType, 
   #else 
      0, 
   #endif
#endif 
#ifdef Sensor2Pin
   Sensor2Label,Sensor2Pin,
   #ifdef Sensor2AuxPin 
      Sensor2AuxPin, 
   #else 
      -1, 
   #endif
   Sensor2Type,Sensor2Timer,Sensor2AlarmSP,true,false,0,false,false,0,0,
      #ifdef Sensor2TempType 
      Sensor2TempType, 
   #else 
      0, 
   #endif
#endif
#ifdef Sensor3Pin
   Sensor3Label,Sensor3Pin,
   #ifdef Sensor3AuxPin 
      Sensor3AuxPin, 
   #else 
      -1, 
   #endif
   Sensor3Type,Sensor3Timer,Sensor3AlarmSP,true,false,0,false,false,0,0,
   #ifdef Sensor3TempType 
      Sensor3TempType, 
   #else 
      0, 
   #endif
#endif
#ifdef Sensor4Pin
   Sensor4Label,Sensor4Pin,
   #ifdef Sensor4AuxPin 
      Sensor4AuxPin, 
   #else 
      -1, 
   #endif
   Sensor4Type,Sensor4Timer,Sensor4AlarmSP,true,false,0,false,false,0,0,
   #ifdef Sensor4TempType 
      Sensor4TempType, 
   #else 
      0, 
   #endif
#endif
#ifdef Sensor5Pin
   Sensor5Label,Sensor5Pin,
   #ifdef Sensor5AuxPin 
      Sensor5AuxPin, 
   #else 
      -1, 
   #endif   
   Sensor5Type,Sensor5Timer,Sensor5AlarmSP,true,false,0,false,false,0,0,
   #ifdef Sensor5TempType 
      Sensor5TempType, 
   #else 
      0, 
   #endif
#endif
#ifdef Sensor6Pin
   Sensor6Label,Sensor6Pin,
   #ifdef Sensor6AuxPin 
      Sensor6AuxPin, 
   #else 
      -1, 
   #endif   
   Sensor6Type,Sensor6Timer,Sensor6AlarmSP,true,false,0,false,false,0,0,
   #ifdef Sensor6TempType 
      Sensor6TempType, 
   #else 
      0, 
   #endif
#endif
#ifdef Sensor7Pin
   Sensor7Label,Sensor7Pin,
   #ifdef Sensor7AuxPin 
      Sensor7AuxPin, 
   #else 
      -1, 
   #endif   
   Sensor7Type,Sensor7Timer,Sensor7AlarmSP,true,false,0,false,false,0,0,
   #ifdef Sensor7TempType 
      Sensor7TempType, 
   #else 
      0, 
   #endif
#endif
#ifdef Sensor8Pin
   Sensor8Label,Sensor8Pin,
   #ifdef Sensor8AuxPin 
      Sensor8AuxPin, 
   #else 
      -1, 
   #endif   
   Sensor8Type,Sensor8Timer,Sensor8AlarmSP,true,false,0,false,false,0,0,
   #ifdef Sensor8TempType 
      Sensor8TempType, 
   #else 
      0, 
   #endif
#endif
};

tDefault defaultSensors[(sizeof(sensors) / sizeof(sensors[0]))];

typedef struct
{
  unsigned long startMs = 0;
  bool started = false;
} tTimer;

//Defines the SP limits for each of the sensor types. 
const int SPLimits[2][2] = {
  { 0, 1 },   // type 0 = Digital
  { 0, 300 }  // type 1 = NTC temperature
};
bool interlockStatus = false;
bool interlockStatusChanged = false;
bool activeFireAlarm = false;
bool activeSmokeAlarm = false;
bool activeEmergencyButtonAlarm = false;
bool statusLed = true;
unsigned int alarmCounter = 0;
uint8_t activeAlarmCount = 0;
uint8_t numOfSensors = 0;
tTimer interlockTimer, resetTimer, ledTimer;
bool printerPowered = true;
unsigned int lTLastRecord = 0;
long lTStartTime;
long lTSum = 0;
long lTMax = 0;
long lTNow, lTLastMax, lTLastSum;
bool memWrng, tempWrng, voltWrng, execWrng  = false;

#ifdef DEBUG
   tTimer debugTimer;
#endif



void setup() {

   //Enabling watchdog timer 4s  
   wdt_enable(WDTO_4S);

   #ifdef SerialComm
   Serial.begin(BaudRate);
   #endif

   //Pins configuration
  
   pinMode(AlarmLedPin, OUTPUT);
   digitalWrite(AlarmLedPin, HIGH); //Alarm led test 

   pinMode(TripLedPin, OUTPUT);
   digitalWrite(TripLedPin, HIGH);  //Trip led test
  
   pinMode(ResetButtonPin, INPUT_PULLUP);

   pinMode(InterlockRelayPin, OUTPUT);
   digitalWrite(InterlockRelayPin, LOW);

   numOfSensors = (sizeof(sensors) / sizeof(sensors[0]));
  
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
   delay(500);

   // Finish Led test  
   digitalWrite(AlarmLedPin, LOW);
   digitalWrite(TripLedPin, LOW);  

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

   // Start heart beat led timer 
   startTimer(&ledTimer.startMs,&ledTimer.started);

   #ifdef DEBUG
   startTimer(&debugTimer.startMs,&debugTimer.started);
   #endif

   #ifdef SerialComm
   Serial.println("Safety Printer MCU ver." VERSION ", release date: " RELEASEDATE); // Boot end msg. Octoprint Plug in looking for this msg in order to indicate that the arduino ir ready to receive commands.
   #endif

}

void loop() {

   // Save the loop start time
   lTStartTime = micros();
 
   //Verify sensors
   checkSensors();

   //Verify reset button
   checkResetButton();

   //Update Interlock and Alarm indication status led
   digitalWrite(AlarmLedPin, LOW); 
   if (interlockStatusChanged) {
      //LED indication of interlock status
      interlockStatusChanged = false;
      digitalWrite(TripLedPin, interlockStatus);
      updateEEPROMinterlock();
   }
  
   // Heart beat (using alarm and trip LEDs to test them)
   if (checkTimer(&ledTimer.startMs,LEDDelay,&ledTimer.started)) { 
      digitalWrite(AlarmLedPin,HIGH);
      delay(25);
      digitalWrite(AlarmLedPin,LOW);
      digitalWrite(TripLedPin,HIGH);
      delay(25);
      digitalWrite(TripLedPin,interlockStatus);
   } else {
      startTimer(&ledTimer.startMs,&ledTimer.started);
   } 
  
   //Manage Serial communications
   #ifdef SerialComm
      // Receive commands from PC
      recvCommandWithStartEndMarkers(); 
   #endif

   //Debug Info
   #ifdef DEBUG
      if (checkTimer(&debugTimer.startMs,DEBUG_DELAY,&debugTimer.started)) { 
         Serial.println();
         Serial.print (F("Free memory [bytes]= "));  // 2048 bytes from datasheet
         Serial.println (freeMemory());
         Serial.print (F("Temperature [C]= "));
         Serial.println (readTemp(), 2);
         Serial.print (F("Voltage [V]= "));
         Serial.println (readVcc(), 2);  // 2.7V to 5.5V from datasheet
         Serial.print (F("Execution time max [us] = "));
         Serial.println (lTLastMax,DEC); 
         Serial.print (F("Execution time average [us] = "));
         Serial.println (lTLastSum/LOOP_TIME_SAMPLES, DEC); 
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
   if ((readVcc() < 3.0) || (readVcc() > 5.2)) { // 2.7 to 5.5 Vcd +-0.3
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
   // Check all sensor configurations
   for (byte i = 0; i < numOfSensors; i++) {
      sensors[i].label = sensors[i].label.substring(0,16);
      for (byte j = 0; j < sizeof(forb_chars); j++) {
         sensors[i].label.replace(forb_chars[j],'?');
      }
      if (sensors[i].pin < 0 || sensors[i].pin > 21) {
         #ifdef SerialComm
            Serial.print(sensors[i].label);
            Serial.println(F(": Wrong sensor PIN definition. Check your Configuration.h file. Sensor disabled."));
         #endif
         sensors[i].pin = 0;
         sensors[i].forceDisable = true;
      }
      if (sensors[i].auxPin < -1 || sensors[i].auxPin > 21) {
         #ifdef SerialComm
            Serial.print(sensors[i].label);
            Serial.println(F(": Wrong sensor AUX_PIN definition. Check your Configuration.h file. Sensor disabled."));
         #endif
         sensors[i].auxPin = 0;
         sensors[i].forceDisable = true;
      }
      if (sensors[i].type != DigitalSensor && sensors[i].type != NTCSensor) {
         #ifdef SerialComm   
            Serial.print(sensors[i].label);      
            Serial.println(F(": Wrong sensor TYPE definition. Check your Configuration.h file. Sensor disabled."));
         #endif
         sensors[i].type = DigitalSensor;
         sensors[i].forceDisable = true;
      }
      if (sensors[i].type == DigitalSensor && sensors[i].alarmSP != 0 && sensors[i].alarmSP != 1) {
         #ifdef SerialComm
            Serial.print(sensors[i].label);
            Serial.println(F(": Wrong sensor ALARM SET POINT definition. Check your Configuration.h file. Sensor disabled."));
         #endif
         sensors[i].alarmSP = 0;
         sensors[i].forceDisable = true;
      }
      if (sensors[i].type == NTCSensor && ((sensors[i].alarmSP < 0) || (sensors[i].alarmSP > highest_temp(i)))) {
         #ifdef SerialComm
            Serial.print(sensors[i].label);
            Serial.println(F(": Wrong sensor ALARM SET POINT definition (above or bellow selected temperature calibration table). Check your Configuration.h file. Sensor disabled."));
         #endif
         sensors[i].alarmSP = 0;
         sensors[i].forceDisable = true;
      }       
   }

}

void checkSensors() {
   // Check all inputs for new alamrs
   activeAlarmCount = 0;
   for (byte i =0; i < numOfSensors; i++) { 
      if (sensors[i].type == DigitalSensor){
        sensors[i].actualValue = digitalRead(sensors[i].pin);
        if (sensors[i].actualValue == sensors[i].alarmSP) {
           setAlarm(i);
        } else {
           sensors[i].active = false;
        }
      }
      else if (sensors[i].type == NTCSensor) {
        sensors[i].actualValue = read_temp(sensors[i].pin, sensors[i].auxPin, i);
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

void checkResetButton() {
  // Check if the reset button [ResetButtonPin] is pressed, else reset the timer.
  if (!digitalRead(ResetButtonPin)){
    resetInterlock(true);   
  } else {
    resetTimer.started = false;
  } 
}

void setAlarm(int index) {
  // Turn an Alarm active, start timers and light Alarm LED [AlarmLedPin].
  if (sensors[index].enabled) {
     activeAlarmCount++;
     interlock(true,sensors[index].timer);
  }
  sensors[index].active = true;
  digitalWrite(AlarmLedPin, HIGH); 
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
   // Change the output [InterlockRelayPin] to an interlock position [InterlockPolarity] after some time to prevent spurious trips
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
       interlockStatus = true;
       interlockStatusChanged = true;
       digitalWrite(InterlockRelayPin,InterlockPolarity);
       digitalWrite(TripLedPin, HIGH); 
    }   
}

void turnOnOff(bool on) {
   // Change the output [InterlockRelayPin]. Diffenret as interlock() because it don't change internal status flags to enable a simple turn on/off.
   if (on && !interlockStatus){
      printerPowered = true;
      digitalWrite(InterlockRelayPin,!InterlockPolarity);
   } else {
      printerPowered = false;
      digitalWrite(InterlockRelayPin,InterlockPolarity);
   }   
}

void resetInterlock(bool useTimer) {
   // Change the output [InterlockRelayPin] to normal position [NOT InterlockPolarity] after some time to prevent spurious resets
   bool resetFlag = false;
   if (!useTimer) {
      resetFlag = true;
   } else { 
     if (checkTimer(&resetTimer.startMs,ResetDelay,&resetTimer.started)) { 
        resetFlag = true;
     } else {
        startTimer(&resetTimer.startMs,&resetTimer.started);
     }
   }
   if (resetFlag) {
      interlockStatus = false;
      interlockStatusChanged = true;
      if (printerPowered) { 
        digitalWrite(InterlockRelayPin,!InterlockPolarity);
      }
   }
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
  /*
  long result; // Read temperature sensor against 1.1V reference 
  ADMUX = _BV(REFS1) | _BV(REFS0) | _BV(MUX3);
  delay(2); // Wait for Vref to settle 
  ADCSRA |= _BV(ADSC); // Convert 
  while (bit_is_set(ADCSRA,ADSC)); 
  result = ADCL;
  result |= ADCH<<8;
  result = (result - 125) * 1075;
  return result;
  */
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
