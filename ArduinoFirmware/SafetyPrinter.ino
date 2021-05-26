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

#include <avr/wdt.h> //Watchdog
#include <EEPROM.h>  //EEPROM access

// ****************************************************************************
// Configuration

#define InterlockRelayPin     6
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
#define Sensor2Pin             9
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
#define Sensor5Pin             7
#define Sensor5AuxPin          2  // Power pin for NTCs Thermistors
#define Sensor5Type            NTCSensor
#define Sensor5Timer           250
#define Sensor5AlarmSP         290
/*
#define Sensor6Label           "Spare"
#define Sensor6Pin             1
#define Sensor6Type            0
#define Sensor6Timer           250
#define Sensor6AlarmSP         0
*/
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

#define VERSION "0.2.1"
 
typedef struct
{
  String label;
  int pin;
  int auxPin;
  int type;
  unsigned int timer;
  int alarmSP;
  bool enabled;
  bool active;
  int actualValue;
  bool spare1;
  bool spare2;
  int spare3;
  int spare4; 
} tSensor;


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

typedef struct
{
  unsigned long startMs = 0;
  bool started = false;
} tTimer;

bool interlockStatus = false;
bool interlockStatusChanged = false;
bool activeFireAlarm = false;
bool activeSmokeAlarm = false;
bool activeEmergencyButtonAlarm = false;
bool statusLed = true;
unsigned long alarmCounter = 0;
int activeAlarmCount = 0;
int numOfSensors = 0;
tTimer interlockTimer, resetTimer, ledTimer;

void setup() {

  //Enabling watchdog timer 4s  
  wdt_enable(WDTO_4S);

  #ifdef SerialComm
  Serial.begin(BaudRate);
  Serial.println("Safety printer ver." VERSION ", release date: " __DATE__); 
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

  readEEPROMData(); 

  // Start heart beat led timer 
  startTimer(&ledTimer.startMs,&ledTimer.started);
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
     EEPROM.update(1, interlockStatus);
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

void readEEPROMData() {
  /* Check EEPROM for the last interlock state
  * EEPROM MAP:
  * [0] : EEPROM flag to check if its written or not;
  * [1] : Interlock Status;
  * [2] : Sensors Enabled status;
  * [3] : Sensor 0 alarm set point (byte 1)
  * [4] : Sensor 0 alarm set point (byte 2)
  * [3] : Sensor 1 alarm set point (byte 1)
  * [4] : Sensor 1 alarm set point (byte 2)
  * [5] : Sensor 2 alarm set point (byte 1)
  * [6] : Sensor 2 alarm set point (byte 2)
  * [7] : Sensor 3 alarm set point (byte 1)
  * [8] : Sensor 3 alarm set point (byte 2)
  * [9] : Sensor 4 alarm set point (byte 1)
  * [10] : Sensor 4 alarm set point (byte 2)
  * [11] : Sensor 5 alarm set point (byte 1)
  * [12] : Sensor 5 alarm set point (byte 2)  
  * [13] : Sensor 6 alarm set point (byte 1)
  * [14] : Sensor 6 alarm set point (byte 2)
  * [15] : Sensor 7 alarm set point (byte 1)
  * [16] : Sensor 7 alarm set point (byte 2)  
  */
  int firstTimeRun = EEPROM.read(0);
  if (firstTimeRun == 0) { // Verify if there is something on EEPROM address 0 (the standard is 255 when its blank)
     interlockStatus = EEPROM.read(1);
     if (interlockStatus) {
        interlock(false,0);
     } else {
        resetInterlock(false);
     }
    // Read Enabled Status from EEPROM
    byte EEPROMSensorsEnabled = EEPROM.read(2); 
    for(int i = 0; i < 8; i++){
      if (7-i < numOfSensors) {
         sensors[7-i].enabled = EEPROMSensorsEnabled % 2;   
      }
      EEPROMSensorsEnabled = EEPROMSensorsEnabled / 2; 

    }
    for(int i = 0; i < numOfSensors; i++){
        byte byte1 = EEPROM.read(3 + (i*2));
        byte byte2 = EEPROM.read(4 + (i*2));
        sensors[i].alarmSP = (byte1 << 8) + byte2;
    }
    
  } else {
    //Write EEPROM for the first time
    EEPROM.update(0,0); // Controll EEprom flag
    EEPROM.update(1,0); // Interlock status
  
    byte EEPROMSensorsEnabled = 0; 
    for(int i = 0; i < numOfSensors; i++){
       EEPROMSensorsEnabled += round(sensors[i].enabled * pow(2,7-i));
    }
    EEPROM.update(2,EEPROMSensorsEnabled);

    for(int i = 0; i < numOfSensors; i++){
       EEPROM.update(3 + (i*2), sensors[i].alarmSP >> 8);
       EEPROM.update(4 + (i*2), sensors[i].alarmSP & 0xFF);
    }    
  }  
}

//*************************   Serial communication module  *************************
#ifdef SerialComm
void recvCommandWithStartEndMarkers() {  
  /* Receive commands from PC
  * 
  * This function receive serial ASCII data and splits it into a "command" and up to 4 "arguments",
  * The sintax must be:
  * <COMAND ARGUMENT1 ARGUMENT2 ARGUMENT3 ARGUMENT4> 
  * 
  */
  #define STARTMARKER '<'
  #define ENDMARKER '>'
  #define NUMCHARS 38
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char rc;
  char receivedChars[NUMCHARS+1] = "";
  //String receivedStr;
  char command[4] = {0};
  char argument1[8] = {0};
  char argument2[8] = {0};
  char argument3[8] = {0};
  char argument4[8] = {0};
  boolean newData = false;
  int test = 0;
    
    
    Serial.flush();
    
    while (Serial.available() > 0 && newData == false) {
        delay(3);
        rc = Serial.read();
        if (rc != -1) {
          if (recvInProgress == true ) {
              if (rc != ENDMARKER) {
                  receivedChars[ndx] = (char)rc;
                  //Serial.print(rc, DEC); 
                  //Serial.println(" " + String(ndx) + ":" + rc + "-" + receivedChars[ndx]);
                  //receivedStr += rc;
                  ndx++;
                  
                  if (ndx >= NUMCHARS) {
                      ndx = NUMCHARS - 1;
                  }
              }
              else {
                
                  receivedChars[ndx] = '\0'; // terminate the string
                  recvInProgress = false;
                  ndx = 0;
                  newData = true;
              }
          }
          else if (rc == STARTMARKER) {
              recvInProgress = true;
          }
        }
    }
    if (newData == true) { 
      char * strtokIndx; // this is used by strtok() as an index

      //Splits commands and argumments
              
      strtokIndx = strtok(receivedChars," "); 
      strcpy(command, strtokIndx);
    
      strtokIndx = strtok(NULL," ");   
      strcpy(argument1, strtokIndx); 
    
      strtokIndx = strtok(NULL," ");     
      strcpy(argument2, strtokIndx); 
    
      strtokIndx = strtok(NULL," ");   
      strcpy(argument3, strtokIndx); 
    
      strtokIndx = strtok(NULL," ");     
      strcpy(argument4, strtokIndx); 

      //Identifyes each command
      //Add here if you need to add new commands
      //Remember that the strcmp is case sensitive
      if (strcmp(command,"c1") == 0 or strcmp(command,"C1") == 0) {
        Cmd_c1();
      } else if (strcmp(command,"c2") == 0 or strcmp(command,"C2") == 0) {
        Cmd_c2();        
      } else if (strcmp(command,"c3") == 0 or strcmp(command,"C3") == 0) {
        Cmd_c3(argument1,argument2);   
      } else if (strcmp(command,"c4") == 0 or strcmp(command,"C4") == 0) {
        Cmd_c4(argument1,argument2);        
      } else if (strcmp(command,"c5") == 0 or strcmp(command,"C5") == 0) {
        Cmd_c5();      
      } else if (strcmp(command,"r1") == 0 or strcmp(command,"R1") == 0) {
        Cmd_r1();
      } else if (strcmp(command,"r2") == 0 or strcmp(command,"R2") == 0) {
        Cmd_r2();
      } else if (strcmp(command,"r3") == 0 or strcmp(command,"R3") == 0) {
        Cmd_r3();
      } else if (strcmp(command,"r4") == 0 or strcmp(command,"R4") == 0) {
        Cmd_r4();
      } else {
        ReturnError(0,command);
      }
    }
}

void ReturnError(int type, char text[8]){
  // Feedbacks wrong serial commands or arguments
  switch (type) {
    case 1:
    case 2:
    case 3:
    case 4:  
         Serial.println("Unknown argument " + String(type) + ":" + String(text)); 
      break;
    default:
          Serial.println("Unknown command: \"" + String(text) + "\"!");
      break;
  }
}

void Cmd_c1()
{
   // Serial command to Reset trip condition
   Serial.println(F("C1: Resseting interlocks."));
   resetInterlock(false);
}

void Cmd_c2()
{
   // Serial command to Trip
   Serial.println(F("C2: External interlock received."));
   interlock(false,0);
}

void Cmd_c3(char argument1[8], char argument2[8])
{
   // Serial command to enable or dissable a sensor
   if ((strcmp(argument1, "all") == 0)) {
       if (strcmp(argument2, "on") == 0) {
          for (int i =0; i < numOfSensors; i++) { 
             sensors[i].enabled = 1;
          }   
          Serial.println(F("C3: ALL sensors are ENABLED.")); 
       } else if (strcmp(argument2, "off") == 0) {
          for (int i =0; i < numOfSensors; i++) { 
             sensors[i].enabled = 0;
             sensors[i].active = 0;
          } 
          Serial.println(F("C3: ALL sensors are DISABLED."));  
       }       
   } else {
     int index = atoi(argument1);
     if (index >= 0 and index < numOfSensors) {
       if (strcmp(argument2, "on") == 0) {
          sensors[index].enabled = 1;   
          Serial.println("C3: Sensor: " + sensors[index].label + " is ENABLED."); 
       } else if (strcmp(argument2, "off") == 0) {
          sensors[index].enabled = 0;
          sensors[index].active = 0;
          Serial.println("C3: Sensor: " + sensors[index].label + " is DISABLED.");  
       } else {
          ReturnError(2,argument2);
       }       
     } else {
        ReturnError(1,argument1);
     }
   }
}

void Cmd_c4(char argument1[8], char argument2[8])
{
   // Serial command to change set point
   int index = atoi(argument1);
   if (index >= 0 and index < numOfSensors) {
    int newSP = atoi(argument2);
    int oldSP = sensors[index].alarmSP;
    if (sensors[index].type == DigitalSensor and (newSP == 0 or newSP == 1)){
        sensors[index].alarmSP = newSP;   
        Serial.println("C4: Sensor: " + sensors[index].label + " set point changed from:" + String(oldSP) + " to: " + String(sensors[index].alarmSP) + "."); 
     } else if (sensors[index].type == NTCSensor) { 
        sensors[index].alarmSP = newSP; 
        Serial.println("C4: Sensor: " + sensors[index].label + " set point changed from:" + String(oldSP) + " to: " + String(sensors[index].alarmSP) + ".");  
     } else {
        ReturnError(2,argument2);
     }       
   } else {
      ReturnError(1,argument1);
   }
}

void Cmd_c5()
{
   // Update EEPROM sensors enabled status
   byte EEPROMSensorsEnabled = 0; 
   for(int i = 0; i < numOfSensors; i++){
      EEPROMSensorsEnabled += round(sensors[i].enabled * pow(2,7-i));
   }
   EEPROM.update(2,EEPROMSensorsEnabled);
   
   // Update EEPROM sensors alarm set point
   for(int i = 0; i < numOfSensors; i++){
      EEPROM.update(3 + (i*2), sensors[i].alarmSP >> 8);
      EEPROM.update(4 + (i*2), sensors[i].alarmSP & 0xFF);
   }
   Serial.println("C5: EEPROM updated.");   
}

void Cmd_r1()
{
   // Serial command to Return Input Status for Octoprint plugin
   Serial.print("R1:"+String(interlockStatus));
   for (int i =0; i < numOfSensors; i++) { 
      Serial.print("#" + String(i) + "," + String(sensors[i].enabled) + "," +  String(sensors[i].active) + "," + String(sensors[i].actualValue) + "," + String(sensors[i].alarmSP) + "," + String(sensors[i].spare1) + "," + String(sensors[i].spare3) + ",");  
   }
   Serial.println();
}

void Cmd_r2()
{
   // Serial command to Return Input Labels for Octoprint plugin
   Serial.print(F("R2:"));
   for (int i =0; i < numOfSensors; i++) { 
      Serial.print("#" + String(i) + "," + sensors[i].label + "," + String(sensors[i].type) + "," + String(sensors[i].spare2) + "," + String(sensors[i].spare4) + ",");  
   }
   Serial.println();
}

void Cmd_r3()
{
   // Serial command to Return Input Status more suitable for humans.
   Serial.println(F("R3: Safety Printer Status"));
   Serial.println(F("---------------------------------------------------------------------------"));
   Serial.print(F("** Interlock Status ** :"));
   if (interlockStatus) Serial.println(F(" ********  Shudown (TRIP) ********")); 
   else Serial.println(F(" Normal Operation")); 
   Serial.println(F("---------------------------------------------------------------------------"));
   Serial.println(F("#:| Label                                   | Enab.| Act.| Value | S.Point |"));
   for (int i = 0; i < numOfSensors; i++) { 
      Serial.print(String(i) + ".|." + sensors[i].label);
      int dots = 40-sensors[i].label.length();
      for (int j = 0; j< dots ;j++)
      {
         Serial.print(F("."));
      }
      Serial.print(F("|."));
      if (sensors[i].enabled) {
         Serial.print(F("Yes"));
      } else {
         Serial.print(F("No."));
      }
      Serial.print(F("..|."));
      if (sensors[i].active) {
         Serial.print(F("Yes"));
      } else {
         Serial.print(F("No."));
      }
      Serial.print(".|." + String(sensors[i].actualValue));
      dots = 6-String(sensors[i].actualValue).length();
      for (int j = 0; j< dots;j++)
      {
         Serial.print(".");
      }
      Serial.print("|." + String(sensors[i].alarmSP));
      dots = 8-String(sensors[i].alarmSP).length();
      for (int j = 0; j< dots;j++)
      {
         Serial.print(".");
      }
      Serial.println("|");
   }
   Serial.println("---------------------------------------------------------------------------");
}

void Cmd_r4()
{
   // Serial command to Return firmware version and release date
   Serial.println("R4:" VERSION "," __DATE__); 
}

#endif
//***************************************************************************************

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
      digitalWrite(InterlockRelayPin,!InterlockPolarity);
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
