//*************************   Serial communication module  *************************
/*
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
 * 
 * WARNIG: DON'T CHANGE ANYTHING IN THIS FILE! ALL CONFIGURATIONS SHOULD BE DONE IN "Configurations.h".
*/
#ifdef HAS_SERIAL_COMM

#include <util/crc16.h>
#include <string.h>

#define SENSOR_CHAR  "#"
#define SEP_CHAR     ","
#define CRC_CHAR     "$"
#define COMMANDSIZE  9
#define ARGUMENTSIZE 8

#define STARTMARKER '<'
#define ENDMARKER '>'
#define NUMCHARS 38

const char * const BoolToString(bool b)
{
  return b ? "T" : "F";
}

void recvCommandWithStartEndMarkers() {  
  /* Receive commands from PC
  * 
  * This function receive serial ASCII data and splits it into a "command" and up to 4 "arguments",
  * The sintax must be:
  * <COMAND ARGUMENT1 ARGUMENT2 ARGUMENT3 ARGUMENT4> 
  * 
  */

  static bool recvInProgress = false;
  static byte ndx = 0;
  char rc;
  char receivedChars[NUMCHARS+1] = "";
  //String receivedStr;
  char command[COMMANDSIZE] = "";
  char argument1[ARGUMENTSIZE] = "";
  char argument2[ARGUMENTSIZE] = "";
  char argument3[ARGUMENTSIZE] = "";
  char argument4[ARGUMENTSIZE] = "";
  bool newData = false;
 
  Serial.flush();
  
  while (Serial.available() > 0 && newData == false) {
      delay(3);
      rc = Serial.read();
      if (rc != -1) {
        if (recvInProgress == true ) {
            if (rc != ENDMARKER) {
                receivedChars[ndx] = (char)rc;
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
    //Add here if you need to add new commands: MAX 8 chars
    Serial.print(command);
    if (String(command).equalsIgnoreCase("c1") || String(command).equalsIgnoreCase("reset")) {
      Cmd_c1();
    } else if ((String(command).equalsIgnoreCase("c2")) || (String(command).equalsIgnoreCase("trip"))) {
      Cmd_c2();        
    } else if ((String(command).equalsIgnoreCase("c3")) || (String(command).equalsIgnoreCase("enable"))) {
      Cmd_c3(argument1,argument2);   
    } else if ((String(command).equalsIgnoreCase("c4")) || (String(command).equalsIgnoreCase("setpoint"))) {
      Cmd_c4(argument1,argument2);        
    } else if ((String(command).equalsIgnoreCase("c5")) || (String(command).equalsIgnoreCase("save"))) {
      Cmd_c5();      
    } else if ((String(command).equalsIgnoreCase("c6")) || (String(command).equalsIgnoreCase("turn"))) {
      Cmd_c6(argument1);
    } else if ((String(command).equalsIgnoreCase("c7")) || (String(command).equalsIgnoreCase("timer"))) {
      Cmd_c7(argument1,argument2);
    } else if ((String(command).equalsIgnoreCase("c8")) || (String(command).equalsIgnoreCase("standard"))) {
      Cmd_c8(argument1);      
    } else if (String(command).equalsIgnoreCase("r1")) {
      Cmd_r1();
    } else if (String(command).equalsIgnoreCase("r2")) {
      Cmd_r2();
    } else if ((String(command).equalsIgnoreCase("r3")) || (String(command).equalsIgnoreCase("info"))) {
      Cmd_r3();
    } else if (String(command).equalsIgnoreCase("r4")) {
      Cmd_r4();
    } else if (String(command).equalsIgnoreCase("r5")) {
      Cmd_r5();
    } else if ((String(command).equalsIgnoreCase("r6")) || (String(command).equalsIgnoreCase("ispowered"))) {
      Cmd_r6();
    } else if (String(command).equalsIgnoreCase("d1")) {
      Cmd_d1(argument1);
    } else {
      ReturnError(0,command);
    }
  }
}

void ReturnError(int type, char text[ARGUMENTSIZE]){
   // Feedbacks wrong serial commands or arguments
   switch (type) {
      case 1:
      case 2:
      case 3:
      case 4:  
         Serial.print(F(": Unknown argument "));
         Serial.print(type);
         Serial.print(F(":"));
         Serial.println(text); 
      break;
      default:
          Serial.println(F(": Unknown command!"));
      break;
  }
  #ifdef DEBUG
      Serial.print (F("ReturnError: Free memory [bytes]= "));  // 2048 bytes from datasheet
      Serial.println (freeMemory());
  #endif
  #ifdef HAS_LCD
      includeExtraMsg(0,0,false);
  #endif
}

void Cmd_c1()
{
   // Serial command to Reset trip condition
   if (interlockStatus) {
      if (resetInterlock(false)) {
         if (printerPowered) {     
            Serial.println(F(": Resseting interlocks."));      
         } else {
            Serial.println(F(": Resseting interlocks but printer is powered OFF by <C6> command."));      
         } 
         #ifdef HAS_LCD
            includeExtraMsg(0,1,false);
         #endif
      } else {
         Serial.print(F(": Can't reset now. Wait "));
         Serial.print(MINIMUM_INTERLOCK_DELAY);
         Serial.println(F("s and try again."));  
      }
   } else {
      Serial.println(F(": No interlock to reset."));   
   }
   #ifdef DEBUG
      Serial.print (F("Cmd_c1: Free memory [bytes]= "));  // 2048 bytes from datasheet
      Serial.println (freeMemory());
   #endif
}

void Cmd_c2()
{
   // Serial command to Trip
   Serial.println(F(": External interlock received."));
   interlock(false,0);
   #ifdef DEBUG
      Serial.print (F("Cmd_c2: Free memory [bytes]= "));  // 2048 bytes from datasheet
      Serial.println (freeMemory());
   #endif
}

void Cmd_c3(char argument1[ARGUMENTSIZE], char argument2[ARGUMENTSIZE])
{
   // Serial command to enable or dissable a sensor
   if (String(argument1).equalsIgnoreCase("all")) {
      if (String(argument2).equalsIgnoreCase("on")) {
         for (uint8_t i =0; i < numOfSensors; i++) { 
            if (!sensors[i].forceDisable) {
               sensors[i].enabled = true;
            } else {
               sensors[i].enabled = false;
            }
         }   
         Serial.println(F(": ALL sensors are ENABLED.")); 
         #ifdef HAS_LCD
             includeExtraMsg(255,2,true);
         #endif 
      } else if (String(argument2).equalsIgnoreCase("off")) {
         for (uint8_t i =0; i < numOfSensors; i++) { 
            sensors[i].enabled = false;
            sensors[i].active = false;
         } 
         Serial.println(F(": ALL sensors are DISABLED.")); 
         #ifdef HAS_LCD
             includeExtraMsg(255,3,true);
         #endif  
      }       
   } else {
      int i = atoi(argument1);
      if (i >= 0 and i < numOfSensors and String(argument1).length() > 0) {
         if (String(argument2).equalsIgnoreCase("on")) {
            Serial.print(F(": "));
            Serial.print(sensors[i].label);
            if (!sensors[i].forceDisable) {
               sensors[i].enabled = true;
               Serial.println(F(" is ENABLED."));
                  #ifdef HAS_LCD
                     includeExtraMsg(i,2,true);
                  #endif 
            } else {
               sensors[i].enabled = false;
               Serial.println(F(" ERROR: cannot be enabled due to configuration problem.")); 
               #ifdef HAS_LCD
                  includeExtraMsg(0,0,false);
               #endif
            }  
         } else if (String(argument2).equalsIgnoreCase("off")) {
            sensors[i].enabled = false;
            sensors[i].active = false;
            Serial.print(F(": "));
            Serial.print(sensors[i].label);
            Serial.println(F(" is DISABLED."));
            #ifdef HAS_LCD
               includeExtraMsg(i,3,true);
            #endif   
         } else {
            ReturnError(2,argument2);
         }       
      } else {
         ReturnError(1,argument1);
      }
   }
   #ifdef DEBUG
      Serial.print (F("Cmd_c3: Free memory [bytes]= "));  // 2048 bytes from datasheet
      Serial.println (freeMemory());
   #endif
}

void Cmd_c4(char argument1[ARGUMENTSIZE], char argument2[ARGUMENTSIZE])
{
   // Serial command to change set point
   int i = atoi(argument1);
   if (i >= 0 and i < numOfSensors and String(argument1).length() > 0) {
      int newSP = atoi(argument2);
      int oldSP = sensors[i].alarmSP;
      if ((newSP < sensors[i].lowSP) || (newSP > sensors[i].highSP)) {
         // Wrong value.        
         Serial.print(F(": Alarm set point for: "));
         Serial.print(sensors[i].label);
         Serial.print(F(" must be between or equal to: "));
         Serial.print(sensors[i].lowSP);
         Serial.print(F(" and "));
         Serial.print(sensors[i].highSP);
         #ifdef HAS_LCD
            includeExtraMsg(0,0,false);
         #endif
      } else {
         sensors[i].alarmSP = newSP;   
         Serial.print(F(": Sensor: "));
         Serial.print(sensors[i].label); 
         Serial.print(F(" set point changed from: ")); 
         Serial.print(oldSP); 
         Serial.print(F(" to: ")); 
         Serial.print(sensors[i].alarmSP);
         #ifdef HAS_LCD
             includeExtraMsg(i,4,true);
         #endif 
      }   
      Serial.println(F("."));     
   } else {
      ReturnError(1,argument1);
   }
   #ifdef DEBUG
      Serial.print (F("Cmd_c4: Free memory [bytes]= "));  // 2048 bytes from datasheet
      Serial.println (freeMemory());
   #endif
}

void Cmd_c5()
{
   // Update EEPROM sensors status
   byte bytesSaved = 0;
   bytesSaved = writeEEPROMData();
   Serial.print(F(": EEPROM updated. "));
   Serial.print(bytesSaved);
   Serial.print(F(" bytes saved. "));
   Serial.print(EEPROM.length() - bytesSaved);
   Serial.println(F(" bytes free."));
   #ifdef HAS_LCD
      includeExtraMsg(0,5,false);
   #endif 
   #ifdef DEBUG
      Serial.print (F("Cmd_c5: Free memory [bytes]= "));  // 2048 bytes from datasheet
      Serial.println (freeMemory());
   #endif   
}

void Cmd_c6(char argument1[ARGUMENTSIZE])
{
   // Serial command to turn ON/OFF 3d Printer (switching the interlock relay output without changing trip status)
   if (String(argument1).equalsIgnoreCase("off")) {
      // Turns off printer
      if (turnOnOff(false)) {
         Serial.println(F(": Printer turned OFF.")); 
         #ifdef HAS_LCD
            includeExtraMsg(0,6,false);
         #endif 
      } else {
         Serial.println(F(": Can't turn OFF printer.")); 
      }
   } else if (String(argument1).equalsIgnoreCase("on")) {
      // Turns on printer

      if (!interlockStatus) {
         if (turnOnOff(true)) {
            Serial.println(F(": Printer turned ON.")); 
            #ifdef HAS_LCD
               includeExtraMsg(0,7,false);
            #endif  
         } else {
            Serial.print(F(": Can't turn printer ON now. Wait "));
            Serial.print(MINIMUM_INTERLOCK_DELAY);
            Serial.println(F("s and try again."));
         }
      } else {
         Serial.println(F(": Can't turn printer ON with INTERLOCK status ON."));
      }
   } else {
      ReturnError(1,argument1);
   }
   #ifdef DEBUG
      Serial.print (F("Cmd_c6: Free memory [bytes]= "));  // 2048 bytes from datasheet
      Serial.println (freeMemory());
   #endif
}

void Cmd_c7(char argument1[ARGUMENTSIZE], char argument2[ARGUMENTSIZE])
{
   // Serial command to change timer
   int i = atoi(argument1);
   if (i >= 0 and i < numOfSensors and String(argument1).length() > 0) {
      unsigned long newTimer = atol(argument2);
      unsigned long oldTimer = sensors[i].timer;
      if ((newTimer < 0) || (newTimer > 4294967295)) {
         // Wrong value.        
         Serial.print(F(": Timer for: "));
         Serial.print(sensors[i].label);
         Serial.println(F(" must be between or equal to: 0 and 4,294,967,295.")); 
         #ifdef HAS_LCD
            includeExtraMsg(0,0,false);
         #endif
      } else {
         sensors[i].timer = newTimer;   
         Serial.print(F(": Sensor: "));
         Serial.print(sensors[i].label); 
         Serial.print(F(" timer changed from: "));
         Serial.print(oldTimer);
         Serial.print(F("ms to: "));
         Serial.print(sensors[i].timer);
         Serial.println(F("ms."));
         #ifdef HAS_LCD
            includeExtraMsg(i,8,true);
         #endif  
      }       
   } else {
      ReturnError(1,argument1);
   }
   #ifdef DEBUG
      Serial.print (F("Cmd_c7: Free memory [bytes]= "));  // 2048 bytes from datasheet
      Serial.println (freeMemory());
   #endif
}

void Cmd_c8(char argument1[ARGUMENTSIZE])
{
   // Serial command to change sensors enabled, alarm set point and timer to standard values
   if (String(argument1).equalsIgnoreCase("all")) {
      for (int i =0; i < numOfSensors; i++) { 
         sensors[i].enabled = defaultSensors[i].enabled;
         sensors[i].alarmSP = defaultSensors[i].alarmSP;
         sensors[i].timer = defaultSensors[i].timer;
      }   
      Serial.println(F(": ALL sensors configurations returned to standard values."));
      #ifdef HAS_LCD
         includeExtraMsg(255,9,true);
      #endif  
   } else {
     int i = atoi(argument1);
     if (i >= 0 and i < numOfSensors and String(argument1).length() > 0) {
         sensors[i].enabled = defaultSensors[i].enabled;
         sensors[i].alarmSP = defaultSensors[i].alarmSP;
         sensors[i].timer = defaultSensors[i].timer;
         Serial.print(F(": Sensor: "));
         Serial.print(sensors[i].label);
         Serial.println(F(" configurations returned to standard values.")); 
         #ifdef HAS_LCD
            includeExtraMsg(i,9,true);
         #endif  
     } else {
        ReturnError(1,argument1);
     }
   }
   #ifdef DEBUG
      Serial.print (F("Cmd_c8: Free memory [bytes]= "));  // 2048 bytes from datasheet
      Serial.println (freeMemory());
   #endif
}

void Cmd_r1() //530
{
   // Serial command to Return Input Status for Octoprint plugin
   char bfr[20] = "";

   String response = BoolToString(interlockStatus);
   response += SEP_CHAR;
   
   // board status
   response += BoolToString(memWrng);
   response += SEP_CHAR;
   memWrng = false;

   response += BoolToString(execWrng);
   response += SEP_CHAR;
   execWrng = false;

   response += BoolToString(tempWrng);
   response += SEP_CHAR;
   tempWrng = false;

   response += BoolToString(voltWrng);
   response += SEP_CHAR;
   voltWrng = false;

   for (int i =0; i < numOfSensors; i++) { 
      response += SENSOR_CHAR;

      memset(bfr, 0, sizeof bfr);
      itoa(i,bfr,10);
      response += bfr;
      response += SEP_CHAR;

      response += BoolToString(sensors[i].enabled);
      response += SEP_CHAR;

      response += BoolToString(sensors[i].active);
      response += SEP_CHAR;

      memset(bfr, 0, sizeof bfr);
      itoa(sensors[i].actualValue,bfr,10);
      response += bfr;
      response += SEP_CHAR;

      memset(bfr, 0, sizeof bfr);
      itoa(sensors[i].alarmSP,bfr,10);
      response += bfr;
      response += SEP_CHAR;
      
      memset(bfr, 0, sizeof bfr);
      itoa(sensors[i].timer,bfr,10);
      response += bfr;
      response += SEP_CHAR;

      response += BoolToString(sensors[i].trigger);
      response += SEP_CHAR;

   }   
   send(response.c_str());
   #ifdef DEBUG
      Serial.print (F("Cmd_r1: Free memory [bytes]= "));  // 2048 bytes from datasheet
      Serial.println (freeMemory());
   #endif 
}

void Cmd_r2()
{
   // Serial command to Return Input Labels for Octoprint plugin
   String response = "";
   char bfr[20] = "";

   for (int i =0; i < numOfSensors; i++) { 
      response += SENSOR_CHAR;
      itoa(i,bfr,10);
      response += bfr;
      response += SEP_CHAR;
      response += sensors[i].label;
      response += SEP_CHAR;

      memset(bfr, 0, sizeof bfr);
      itoa(sensors[i].type,bfr,10);
      response += bfr;
      response += SEP_CHAR;

      response += BoolToString(sensors[i].forceDisable);
      response += SEP_CHAR;

      memset(bfr, 0, sizeof bfr);
      itoa(sensors[i].lowSP,bfr,10);
      response += bfr;
      response += SEP_CHAR;
      
      memset(bfr, 0, sizeof bfr);
      itoa(sensors[i].highSP,bfr,10);
      response += bfr;
      response += SEP_CHAR;
   }
   send(response.c_str()); 
   #ifdef DEBUG
      Serial.print (F("Cmd_r2: Free memory [bytes]= "));  // 2048 bytes from datasheet
      Serial.println (freeMemory());
   #endif  
}

void Cmd_r3()
{
   // Serial command to Return Input Status more suitable for humans.
   Serial.println(F(": Safety Printer Status"));
   Serial.println(F("------------------------------------------------------------------------"));
   Serial.print(F("** Interlock Status ** :"));
   if (interlockStatus) Serial.println(F(" ********  Shudown (TRIP) ********")); 
   else Serial.println(F(" Normal Operation")); 
   Serial.println(F("------------------------------------------------------------------------"));
   Serial.println(F("#:| Label             | Enab.| Act.| Trig.| Value | S.Point | Timer     |"));
   for (int i = 0; i < numOfSensors; i++) { 
      Serial.print(String(i) + ".|." + sensors[i].label);
      int dots = 18 - strlen(sensors[i].label); //.length();
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
      Serial.print(F(".|."));
      if (sensors[i].trigger) {
         Serial.print(F("Yes."));
      } else {
         Serial.print(F("No.."));
      }
      Serial.print(".|." + String(sensors[i].actualValue));
      dots = 6-String(sensors[i].actualValue).length();
      for (int j = 0; j< dots;j++)
      {
         Serial.print(F("."));
      }
      Serial.print("|." + String(sensors[i].alarmSP));
      dots = 8-String(sensors[i].alarmSP).length();
      for (int j = 0; j< dots;j++)
      {
         Serial.print(F("."));
      }
      Serial.print("|." + String(sensors[i].timer));
      dots = 10-String(sensors[i].timer).length();
      for (int j = 0; j< dots;j++)
      {
         Serial.print(F("."));
      }
      Serial.println(F("|"));
   }
   Serial.println(F("------------------------------------------------------------------------"));
   #ifdef DEBUG
      Serial.print (F("Cmd_r3: Free memory [bytes]= "));  // 2048 bytes from datasheet
      Serial.println (freeMemory());
   #endif
}

void Cmd_r4()
{
   // Serial command to Return firmware version and release date 
   String response =  VERSION SEP_CHAR RELEASEDATE SEP_CHAR;
   char bfr[20] = "";
  
   itoa(EEPROMVERSION,bfr,10);
   response += bfr;
   response += SEP_CHAR;

   memset(bfr, 0, sizeof bfr);
   itoa(COMMPROTOCOL,bfr,10);
   response += bfr;
   response += SEP_CHAR;

   send(response.c_str()); 
   #ifdef DEBUG
      Serial.print (F("Cmd_r4: Free memory [bytes]= "));  // 2048 bytes from datasheet
      Serial.println (freeMemory());
   #endif  
}

void Cmd_r5() 
{
   // Serial command to Return Warning Status for Octoprint plugin
   char bfr[20] = "";

   memset(bfr, 0, sizeof bfr);
   itoa(freeMemory(),bfr,10);
   String response = bfr;
   response += SEP_CHAR;

   memset(bfr, 0, sizeof bfr);
   dtostrf(readTemp(),0,2,bfr);
   response += bfr;
   response += SEP_CHAR;

   memset(bfr, 0, sizeof bfr);
   dtostrf(readVcc(),0,2,bfr);
   response += bfr;
   response += SEP_CHAR;
   
   memset(bfr, 0, sizeof bfr);
   ltoa(lTLastMax,bfr,10);
   response += bfr;
   response += SEP_CHAR;

   memset(bfr, 0, sizeof bfr);
   ltoa(lTLastSum/LOOP_TIME_SAMPLES,bfr,10);
   response += bfr;
   response += SEP_CHAR;

   send(response.c_str());
   #ifdef DEBUG
      Serial.print (F("Cmd_r5: Free memory [bytes]= "));  // 2048 bytes from datasheet
      Serial.println (freeMemory());
   #endif 
}

void Cmd_r6() {
  send(BoolToString(printerPowered));
}

void Cmd_d1(char argument1[ARGUMENTSIZE]) 
{
   int i = atoi(argument1);

   if (i == 1) {
      memWrng = true;
   } else if (i == 2) {
      execWrng = true;
   } else if (i == 3) {
      tempWrng = true;
   } else if (i == 4) {
      voltWrng = true;
   } else {
      ReturnError(1,argument1);
      return;
   }
   Serial.println(": Debug command OK!");
   #ifdef DEBUG
      Serial.print (F("Cmd_d1: Free memory [bytes]= "));  // 2048 bytes from datasheet
      Serial.println (freeMemory());
   #endif
}
 
uint16_t calcCRC(const char* str)
{
  uint16_t crc=0; // starting value as you like, must be the same before each calculation
  for (unsigned int i=0;i<strlen(str);i++) // for each character in the string
  {
    crc= _crc16_update (crc, str[i]); // update the crc value
  }
  return crc;
}
/*
uint16_t _crc16_update(uint16_t crc, uint8_t a)
{
  int i;
  crc ^= a;
  for (i = 0; i < 8; ++i)
  {
    if (crc & 1)
    crc = (crc >> 1) ^ 0xA001;
    else
    crc = (crc >> 1);
  }
  return crc;
}
*/
void send(const char* payload) {
   Serial.print(":" CRC_CHAR); 
   Serial.print(calcCRC(payload));
   Serial.print(CRC_CHAR);
   Serial.println(payload);
}



#endif
//***************************************************************************************
