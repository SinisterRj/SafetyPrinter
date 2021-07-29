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
  static bool recvInProgress = false;
  static byte ndx = 0;
  char rc;
  char receivedChars[NUMCHARS+1] = "";
  //String receivedStr;
  char command[4] = "";
  char argument1[8] = "";
  char argument2[8] = "";
  char argument3[8] = "";
  char argument4[8] = "";
  bool newData = false;
 
    
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
      if (String(command).equalsIgnoreCase("c1")) {
        Cmd_c1();
      } else if (String(command).equalsIgnoreCase("c2")) {
        Cmd_c2();        
      } else if (String(command).equalsIgnoreCase("c3")) {
        Cmd_c3(argument1,argument2);   
      } else if (String(command).equalsIgnoreCase("c4")) {
        Cmd_c4(argument1,argument2);        
      } else if (String(command).equalsIgnoreCase("c5")) {
        Cmd_c5();      
      } else if (String(command).equalsIgnoreCase("c6")) {
        Cmd_c6(argument1);
      } else if (String(command).equalsIgnoreCase("c7")) {
        Cmd_c7(argument1,argument2);
      } else if (String(command).equalsIgnoreCase("c8")) {
        Cmd_c8(argument1);      
      } else if (String(command).equalsIgnoreCase("r1")) {
        Cmd_r1();
      } else if (String(command).equalsIgnoreCase("r2")) {
        Cmd_r2();
      } else if (String(command).equalsIgnoreCase("r3")) {
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
   if (printerPowered) {
      Serial.println(F("C1: Resseting interlocks."));
   } else {
      Serial.println(F("C1: Resseting interlocks but printer is powered OFF by <C6> command."));
   }
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
   if (String(argument1).equalsIgnoreCase("all")) {
       if (String(argument2).equalsIgnoreCase("on")) {
          for (uint8_t i =0; i < numOfSensors; i++) { 
             sensors[i].enabled = 1;
          }   
          Serial.println(F("C3: ALL sensors are ENABLED.")); 
       } else if (String(argument2).equalsIgnoreCase("off")) {
          for (uint8_t i =0; i < numOfSensors; i++) { 
             sensors[i].enabled = 0;
             sensors[i].active = 0;
          } 
          Serial.println(F("C3: ALL sensors are DISABLED."));  
       }       
   } else {
     int index = atoi(argument1);
     if (index >= 0 and index < numOfSensors and String(argument1).length() > 0) {
       if (String(argument2).equalsIgnoreCase("on")) {
          sensors[index].enabled = 1;   
          Serial.println("C3: Sensor: " + sensors[index].label + " is ENABLED."); 
       } else if (String(argument2).equalsIgnoreCase("off")) {
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
   if (index >= 0 and index < numOfSensors and String(argument1).length() > 0) {
      int newSP = atoi(argument2);
      int oldSP = sensors[index].alarmSP;
      if ((newSP < SPLimits[sensors[index].type][0]) || (newSP > SPLimits[sensors[index].type][1])) {
         // Wrong value.        
         Serial.println("C4: Alarm set point for: " + sensors[index].label + " must be between or equal to: " + SPLimits[sensors[index].type][0] + " and " + SPLimits[sensors[index].type][1] + "."); 
      } else {
         sensors[index].alarmSP = newSP;   
         Serial.println("C4: Sensor: " + sensors[index].label + " set point changed from: " + String(oldSP) + " to: " + String(sensors[index].alarmSP) + "."); 
      }       
   } else {
      ReturnError(1,argument1);
   }
}

void Cmd_c5()
{
   // Update EEPROM sensors enabled status
   byte bytesSaved = 0;
   bytesSaved = writeEEPROMData();
   Serial.println("C5: EEPROM updated. " + String(bytesSaved) + " bytes saved. " + String(EEPROM.length() - bytesSaved) + " bytes free.");   
}

void Cmd_c6(char argument1[8])
{
   // Serial command to turn ON/OFF 3d Printer (switching the interlock relay output without changing trip status)
   if (String(argument1).equalsIgnoreCase("off")) {
      // Turns off printer
      turnOnOff(false);
      Serial.println("C6: Printer turned OFF.");  
   } else if (String(argument1).equalsIgnoreCase("on")) {
      // Turns on printer
      if (!interlockStatus) {
         turnOnOff(true);
         Serial.println("C6: Printer turned ON.");  
      } else {
         Serial.println("C6: Can't turn printer ON with INTERLOCK status ON.");
      }
   } else {
      ReturnError(1,argument1);
   }
}

void Cmd_c7(char argument1[8], char argument2[8])
{
   // Serial command to change timer
   int index = atoi(argument1);
   if (index >= 0 and index < numOfSensors and String(argument1).length() > 0) {
      unsigned long newTimer = atol(argument2);
      unsigned long oldTimer = sensors[index].timer;
      if ((newTimer < 0) || (newTimer > 4294967295)) {
         // Wrong value.        
         Serial.println("C7: Timer for: " + sensors[index].label + " must be between or equal to: 0 and 4,294,967,295."); 
      } else {
         sensors[index].timer = newTimer;   
         Serial.println("C7: Sensor: " + sensors[index].label + " timer changed from: " + String(oldTimer) + "ms to: " + String(sensors[index].timer) + "ms."); 
      }       
   } else {
      ReturnError(1,argument1);
   }
}

void Cmd_c8(char argument1[8])
{
   // Serial command to change sensors enabled, alarm set point and timer to standard values
  if (String(argument1).equalsIgnoreCase("all")) {
      for (int i =0; i < numOfSensors; i++) { 
         sensors[i].enabled = defaultSensors[i].enabled;
         sensors[i].alarmSP = defaultSensors[i].alarmSP;
         sensors[i].timer = defaultSensors[i].timer;
      }   
      Serial.println(F("C8: ALL sensors configurations returned to standard values.")); 
   } else {
     int index = atoi(argument1);
     if (index >= 0 and index < numOfSensors and String(argument1).length() > 0) {
         sensors[index].enabled = defaultSensors[index].enabled;
         sensors[index].alarmSP = defaultSensors[index].alarmSP;
         sensors[index].timer = defaultSensors[index].timer;
         Serial.println("C8: Sensor: " + sensors[index].label + " configurations returned to standard values."); 
     } else {
        ReturnError(1,argument1);
     }
   }
}

void Cmd_r1()
{
   // Serial command to Return Input Status for Octoprint plugin
   Serial.print("R1:"+String(interlockStatus));
   for (int i =0; i < numOfSensors; i++) { 
      Serial.print("#" + String(i) + "," + String(sensors[i].enabled) + "," +  String(sensors[i].active) + "," + String(sensors[i].actualValue) + "," + String(sensors[i].alarmSP) + "," + String(sensors[i].timer) + "," + String(sensors[i].spare1) + "," + String(sensors[i].spare3) + ",");  
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
   Serial.println(F("-------------------------------------------------------------------------------"));
   Serial.print(F("** Interlock Status ** :"));
   if (interlockStatus) Serial.println(F(" ********  Shudown (TRIP) ********")); 
   else Serial.println(F(" Normal Operation")); 
   Serial.println(F("-------------------------------------------------------------------------------"));
   Serial.println(F("#:| Label                           | Enab.| Act.| Value | S.Point | Timer     |"));
   for (int i = 0; i < numOfSensors; i++) { 
      Serial.print(String(i) + ".|." + sensors[i].label);
      int dots = 32-sensors[i].label.length();
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
   Serial.println(F("-------------------------------------------------------------------------------"));
}

void Cmd_r4()
{
   // Serial command to Return firmware version and release date
   Serial.println("R4:" VERSION "," RELEASEDATE "," + String(EEPROMVERSION) + "," + String (COMMPROTOCOL) + ","); 
}

#endif
//***************************************************************************************