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
 * Version: 0.1
 * 04/07/2021
 *
 */

#include <avr/wdt.h> //Watchdog
#include <EEPROM.h>  //EEPROM access

// ****************************************************************************
// Configuration

#define InterlockRelayPin 6
#define InterlockPolarity HIGH  //Change if you need to toggle the behavior of the interlock pin (if its HIGH it means that the [InterlockRelayPin] output will be LOW under normal conditions and HIGH when an interlock occurs)

#define ResetButtonPin 11
#define AlarmLedPin 10
#define TripLedPin 12

#define SerialComm true         // Enable serial communications
#define ResetDelay 1000         // Delay for reseting the trip condition
#define LEDDelay 5000           // Led heart beat interval

// ****************************************************************************
 
typedef struct
{
  String label;
  int pin;
  unsigned int timer;
  bool enabled;
  bool active;
  bool newAlarm;
} tSensor;


/* ****************************************************************************
 Declare all sensors using the above template
 "label",pin number,delay,enabled(always true),alarmStatus(always false),newAlarm(always false)

 Label: the name of the sensor, that will help you to intentify it;
 Pin number: the pin wher it is connected;
 Delay: The timer delay to start the interlock from this sensor. If two or more sensors are active, the delay is bypassed; 
 enabled, alarmStatus and newAlarm: Internal use.

*/
tSensor sensors [] =
{
   
   "Fire 1",9,250,true,false,false,
   "Fire 2",8,250,true,false,false,
   "Emergency Button",5,250,true,false,false,
   "Smoke",7,250,true,false,false,
};

// *************************** Don't change after this line **********************************

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
tTimer interlockTimer, resetTimer, ledTimer;

void setup() {

  //Enabling watchdog timer 4s  
  wdt_enable(WDTO_4S);

  if (SerialComm) {
     Serial.begin(9600);
  }
  
  pinMode(AlarmLedPin, OUTPUT);
  digitalWrite(AlarmLedPin, HIGH); //Alarm led test 

  pinMode(TripLedPin, OUTPUT);
  digitalWrite(TripLedPin, HIGH);  //Trip led test
  
  pinMode(ResetButtonPin, INPUT_PULLUP);
  
  //Set sensor input pins
  for (int i =0; i < (sizeof(sensors) / sizeof(sensors[0])); i++) { 
     if (sensors[i].enabled) pinMode(sensors[i].pin, INPUT);
  }
  
  delay(500);

  // Finish Led test  
  digitalWrite(AlarmLedPin, LOW);
  digitalWrite(TripLedPin, LOW);  
    
  // Check EEPROM for the last interlock state
  pinMode(InterlockRelayPin, OUTPUT);
  int firstTimeRun = EEPROM.read(0);
  if (firstTimeRun == 0) { // Verifica se há alguma coisa no endereço 0 da EEPROM (o padrão é 255 quando não gravado)
     interlockStatus = EEPROM.read(1);
     if (interlockStatus) {
        interlock(false,0);
     } else {
        resetInterlock(false);
     }
  } else {
    //Write EEPROM for the first time
    EEPROM.update(0,0); //controll EEprom flag
    EEPROM.update(1,0); // Interlock status
  }
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
  if (SerialComm) {
    // Receive commands from PC
    recvCommandWithStartEndMarkers(); 
  }

  //Reset watchdog timer
  wdt_reset();
}

void recvCommandWithStartEndMarkers() {  
  // Receive commands from PC
  // This function receive serial ASCII data and splits it into a "command" and up to 4 "arguments",
  // The sintax must be:
  // <COMAND ARGUMENT1 ARGUMENT2 ARGUMENT 3 ARGUMENT4> 
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;
  const byte numChars = 38;
  char receivedChars[numChars];
  char command[4] = {0};
  char argument1[8] = {0};
  char argument2[8] = {0};
  char argument3[8] = {0};
  char argument4[8] = {0};
  boolean newData = false;
    
    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }
        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
    if (newData == true) { 

      char * strtokIndx; // this is used by strtok() as an index

      //Splits commands andargumments
      
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
      } else if (strcmp(command,"r1") == 0 or strcmp(command,"R1") == 0) {
        Cmd_r1();
      } else if (strcmp(command,"r2") == 0 or strcmp(command,"R2") == 0) {
        Cmd_r2();
      } else if (strcmp(command,"r3") == 0 or strcmp(command,"R3") == 0) {
        Cmd_r3();
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
   Serial.println("Resseting interlocks");
   resetInterlock(false);
}

void Cmd_r1()
{
   // Serial command to Return Input Status for Octoprint plugin
   Serial.print(String(interlockStatus) + ",");
   for (int i =0; i < (sizeof(sensors) / sizeof(sensors[0])); i++) { 
      Serial.print(String(i) + "," + String(sensors[i].enabled) + "," +  String(sensors[i].active) + "," +  String(sensors[i].newAlarm) + ",");  
   }
   Serial.println();
}

void Cmd_r2()
{
   // Serial command to Return Input Labels for Octoprint plugin
   for (int i =0; i < (sizeof(sensors) / sizeof(sensors[0])); i++) { 
      Serial.print(String(i) + "," + sensors[i].label + ",");  
   }
   Serial.println();
}

void Cmd_r3()
{
   // Serial command to Return Input Status more suitable for humans.
   Serial.println("Interlock Status: " + String(interlockStatus));
   for (int i =0; i < (sizeof(sensors) / sizeof(sensors[0])); i++) { 
      Serial.println(String(i) + ")" + sensors[i].label + " : Enabled: " + String(sensors[i].enabled) + ", Active: " +  String(sensors[i].active) + ", New Alarm: " +  String(sensors[i].newAlarm) + ",");  
   }
}

void checkSensors() {
   // Check all inputs for new alamrs
   int activeAlarmCount = 0;
   for (int i =0; i < (sizeof(sensors) / sizeof(sensors[0])); i++) { 
      if (sensors[i].enabled) {
        if (!digitalRead(sensors[i].pin)) {
           activeAlarmCount++;
           interlock(true,sensors[i].timer);
           if (!sensors[i].active) sensors[i].newAlarm = true;
           sensors[i].active = true;
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
  // Light the Alarm LED [AlarmLedPin].
  digitalWrite(AlarmLedPin, HIGH); 
  if (sensors[index].newAlarm) {
     sensors[index].newAlarm = false;
     alarmCounter++;
  }
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
