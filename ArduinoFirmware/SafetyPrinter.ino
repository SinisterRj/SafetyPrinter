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

 //*********************** A fazer: incluir os status de enabled na eeprom: Tá dando um erro bizarro que o arduino para de entender os caracteres que eu mando assim que eu gravo alguma coisa na eeprom
#include <Arduino.h>
#include <avr/wdt.h> //Watchdog
#include <EEPROM.h>  //EEPROM access

// ****************************************************************************
// Configuration

#define InterlockRelayPin     6 //13
#define InterlockPolarity     HIGH  //Change if you need to toggle the behavior of the interlock pin (if its HIGH it means that the [InterlockRelayPin] output will be LOW under normal conditions and HIGH when an interlock occurs)

#define ResetButtonPin        11
#define AlarmLedPin           10
#define TripLedPin            12
#define TempPowerPin          2

#define SerialComm            true         // Enable serial communications
#define BaudRate              115200       // :[2400, 9600, 19200, 38400, 57600, 115200, 250000, 500000, 1000000]
#define ResetDelay            1000         // Delay for reseting the trip condition
#define LEDDelay              5000         // Led heart beat interval

#define Samples               20           // num of Samples for NTC thermistor
#define DigitalSensor         0
#define NTCSensor             1

// Sensors - Max 8 sensors

#define Sensor1Label           "Flame 1"
#define Sensor1Pin             9
#define Sensor1Type            DigitalSensor    // 0 for digital, 1 for Analog temperature
#define Sensor1Timer           250
#define Sensor1AlarmSP         0

#define Sensor2Label          "Flame 2"
#define Sensor2Pin             8
#define Sensor2Type            DigitalSensor
#define Sensor2Timer           250
#define Sensor2AlarmSP         0

#define Sensor3Label           "Emergency Button"
#define Sensor3Pin             5
#define Sensor3Type            DigitalSensor
#define Sensor3Timer           250
#define Sensor3AlarmSP         0

#define Sensor4Label           "Smoke"
#define Sensor4Pin             7
#define Sensor4Type            DigitalSensor
#define Sensor4Timer           250
#define Sensor4AlarmSP         0

#define Sensor5Label           "HotEnd Temp."
#define Sensor5Pin             A7
#define Sensor5AuxPin          3  // Power pin for NTCs Thermistors
#define Sensor5Type            NTCSensor
#define Sensor5Timer           250
#define Sensor5AlarmSP         290

#define Sensor6Label           "Bed Temp."
#define Sensor6Pin             A6
#define Sensor6AuxPin          2  // Power pin for NTCs Thermistors
#define Sensor6Type            NTCSensor
#define Sensor6Timer           250
#define Sensor6AlarmSP         150

/*
#define Sensor7Label           "Spare"
#define Sensor7Pin             1
#define Sensor7Type            0
#define Sensor7Timer           250
#define Sensor7AlarmSP         0
*/
/*
#define Sensor8Label           "Spare"
#define Sensor8Pin             1
#define Sensor8Type            0
#define Sensor8Timer           250
#define Sensor8AlarmSP         0
*/
#define NTC   //enable thermistor temperature measurement

#ifdef NTC
  // NTC thermistor temperature equivalence table. 
  // R25 = 100 kOhm, beta25 = 4092 K, 4.7 kOhm pull-up, bed thermistor (thermistor_1.h from Marlin). Change it if you find a better suit for yout sensor
  #define NUMTEMPS              64  // Number of entries on the tabble.
  short temptable[NUMTEMPS][2] = {
     // Standart creality
    {   23, 300 },
    {   25, 295 },
    {   27, 290 },
    {   28, 285 },
    {   31, 280 },
    {   33, 275 },
    {   35, 270 },
    {   38, 265 },
    {   41, 260 },
    {   44, 255 },
    {   48, 250 },
    {   52, 245 },
    {   56, 240 },
    {   61, 235 },
    {   66, 230 },
    {   71, 225 },
    {   78, 220 },
    {   84, 215 },
    {   92, 210 },
    {  100, 205 },
    {  109, 200 },
    {  120, 195 },
    {  131, 190 },
    {  143, 185 },
    {  156, 180 },
    {  171, 175 },
    {  187, 170 },
    {  205, 165 },
    {  224, 160 },
    {  245, 155 },
    {  268, 150 },
    {  293, 145 },
    {  320, 140 },
    {  348, 135 },
    {  379, 130 },
    {  411, 125 },
    {  445, 120 },
    {  480, 115 },
    {  516, 110 },
    {  553, 105 },
    {  591, 100 },
    {  628,  95 },
    {  665,  90 },
    {  702,  85 },
    {  737,  80 },
    {  770,  75 },
    {  801,  70 },
    {  830,  65 },
    {  857,  60 },
    {  881,  55 },
    {  903,  50 },
    {  922,  45 },
    {  939,  40 },
    {  954,  35 },
    {  966,  30 },
    {  977,  25 },
    {  985,  20 },
    {  993,  15 },
    {  999,  10 },
    { 1004,   5 },
    { 1008,   0 },
    { 1012,  -5 },
    { 1016, -10 },
    { 1020, -15 }
  };
#endif

// *************************** Don't change after this line **********************************

#define VERSION         "0.2.3"
#define RELEASEDATE     "Jul 21 2021"
#define EEPROMVERSION   3            // Incremental BYTE. Firmware overwrites the EEPROM with standard values if the number readed from EEPROM is different. change everytime that EEPROM structure is changed.
#define COMMPROTOCOL    1            // Incremental BYTE. Octoprint plugin communication protocol version. 
 
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
  bool spare1;
  bool spare2;
  int spare3;
  int spare4; 
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
#endif
#ifdef Sensor2Pin
   Sensor2Label,Sensor2Pin,
   #ifdef Sensor2AuxPin 
      Sensor2AuxPin, 
   #else 
      -1, 
   #endif
   Sensor2Type,Sensor2Timer,Sensor2AlarmSP,true,false,0,false,false,0,0,
#endif
#ifdef Sensor3Pin
   Sensor3Label,Sensor3Pin,
   #ifdef Sensor3AuxPin 
      Sensor3AuxPin, 
   #else 
      -1, 
   #endif
   Sensor3Type,Sensor3Timer,Sensor3AlarmSP,true,false,0,false,false,0,0,
#endif
#ifdef Sensor4Pin
   Sensor4Label,Sensor4Pin,
   #ifdef Sensor4AuxPin 
      Sensor4AuxPin, 
   #else 
      -1, 
   #endif
   Sensor4Type,Sensor4Timer,Sensor4AlarmSP,true,false,0,false,false,0,0,
#endif
#ifdef Sensor5Pin
   Sensor5Label,Sensor5Pin,
   #ifdef Sensor5AuxPin 
      Sensor5AuxPin, 
   #else 
      -1, 
   #endif   
   Sensor5Type,Sensor5Timer,Sensor5AlarmSP,true,false,0,false,false,0,0,
#endif
#ifdef Sensor6Pin
   Sensor6Label,Sensor6Pin,
   #ifdef Sensor6AuxPin 
      Sensor6AuxPin, 
   #else 
      -1, 
   #endif   
   Sensor6Type,Sensor6Timer,Sensor6AlarmSP,true,false,0,false,false,0,0,
#endif
#ifdef Sensor7Pin
   Sensor7Label,Sensor7Pin,
   #ifdef Sensor7AuxPin 
      Sensor7AuxPin, 
   #else 
      -1, 
   #endif   
   Sensor7Type,Sensor7Timer,Sensor7AlarmSP,true,false,0,false,false,0,0,
#endif
#ifdef Sensor8Pin
   Sensor8Label,Sensor8Pin,
   #ifdef Sensor8AuxPin 
      Sensor8AuxPin, 
   #else 
      -1, 
   #endif   
   Sensor8Type,Sensor8Timer,Sensor8AlarmSP,true,false,0,false,false,0,0,
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
  //Set sensor input pins
  for (int i =0; i < numOfSensors; i++) { 
     if (sensors[i].enabled) 
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
     defaultSensors[i].enabled = sensors[i].enabled;
  }

  readEEPROMData(); 

  // Start heart beat led timer 
  startTimer(&ledTimer.startMs,&ledTimer.started);

  #ifdef SerialComm
  Serial.println("Safety Printer MCU ver." VERSION ", release date: " RELEASEDATE); // Boot end msg. Octoprint Plug in looking for this msg in order to indicate that the arduino ir ready to receive commands.
  #endif

}

void loop() {
 
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

  //Reset watchdog timer
  wdt_reset();
}

void checkSensors() {
   // Check all inputs for new alamrs
   activeAlarmCount = 0;
   for (int i =0; i < numOfSensors; i++) { 
      if (sensors[i].type == DigitalSensor){
        sensors[i].actualValue = digitalRead(sensors[i].pin);
        if (sensors[i].actualValue == sensors[i].alarmSP) {
           setAlarm(i);
        } else {
           sensors[i].active = false;
        }
      }
      else if (sensors[i].type == NTCSensor) {
        sensors[i].actualValue = read_temp(sensors[i].pin, sensors[i].auxPin);
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

#ifdef NTC
int read_temp(int NTC_Pin, int NTC_Power_Pin)
{
   digitalWrite(NTC_Power_Pin, HIGH);        // Power thermistor
   int rawAdc = 0 ;
   for (int i = 0; i< Samples; i++) {
      rawAdc += analogRead(NTC_Pin);
   }   
    digitalWrite(NTC_Power_Pin, LOW);       // Unpower thermistor
   rawAdc /= Samples; 
   int current_celsius = 0;

   byte i;
   for (i=1; i<NUMTEMPS; i++)
   {
      if (temptable[i][0] > rawAdc)
      {
         int realtemp  = temptable[i-1][1] + (rawAdc - temptable[i-1][0]) * (temptable[i][1] - temptable[i-1][1]) / (temptable[i][0] - temptable[i-1][0]);
         if (realtemp > 255.0)
            realtemp = 255.0; 

         current_celsius = realtemp;

         break;
      }
   }

   // Overflow: We just clamp to 500 degrees celsius
   if (i == NUMTEMPS)
   current_celsius = 500;

   return current_celsius;
}
#endif
