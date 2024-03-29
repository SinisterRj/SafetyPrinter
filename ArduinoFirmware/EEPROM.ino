//*************************          EEPROM module         *************************
/*
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
 *
 * EEPROM MAP:
 * [0] : EEPROM version flag to check if its written or not;
 * [1] : Interlock Status;
 * [2 ~ 5] : EEPROM CRC;
 * [6]~[END]  : Sensor information;  
 * 
 * 
 * WARNIG: DON'T CHANGE ANYTHING IN THIS FILE! ALL CONFIGURATIONS SHOULD BE DONE IN "Configurations.h".
*/

#define VERSION_ADR 0
#define INTERLOCK_ADR 1
#define CRC_ADR 2
#define DATA_ADR 6

typedef struct 
{
  bool sensorEnabled[8];
  int sensorAlarmSP[8];
  unsigned long sensorTimer[8];
} tEEPROM;


void readEEPROMData() {
   // Check EEPROM for the last interlock state
   tEEPROM eRead;
   unsigned long saved_EEPROM_CRC, actual_EEPROM_CRC;
   bool firstLoop = true;
   EEPROM.get(CRC_ADR,saved_EEPROM_CRC);
   actual_EEPROM_CRC = eeprom_crc();
   if (saved_EEPROM_CRC == actual_EEPROM_CRC) {
      if (EEPROM.read(VERSION_ADR) == EEPROMVERSION) { // Verify if there is EEPROMVERSION on EEPROM address VERSION_ADR (the standard is 255 when its blank)

         EEPROM.get(DATA_ADR, eRead );
         for(uint8_t i = 0; i < numOfSensors; i++){
            if (!sensors[i].forceDisable) {
               sensors[i].enabled = eRead.sensorEnabled[i];
            } else {
               sensors[i].enabled = false;
            }
            sensors[i].alarmSP = eRead.sensorAlarmSP[i];
            if ((sensors[i].alarmSP < sensors[i].lowSP) || (sensors[i].alarmSP > sensors[i].highSP)) {
               // Wrong value. Change back to standard:           
               #ifdef HAS_SERIAL_COMM
               SERIAL.println("Invalid EEPROM set point read (" + String(sensors[i].alarmSP) +"). Defining standard set point to " + sensors[i].label + " (" + String(defaultSensors[i].alarmSP) + ").");
               #endif
               sensors[i].alarmSP = defaultSensors[i].alarmSP;
            }
            sensors[i].timer = eRead.sensorTimer[i];
         }
         
         interlockStatus = EEPROM.read(INTERLOCK_ADR);
         if (interlockStatus) {
           interlock(false,0);
         } else {
           while (!resetInterlock(false)) {
              if (firstLoop) {
                  firstLoop = false;
                  #ifdef HAS_SERIAL_COMM
                    SERIAL.print(F("Waiting "));
                    SERIAL.print(MINIMUM_INTERLOCK_DELAY);
                    SERIAL.println(F("s to turn on printer."));
                  #endif
              }
              delay(250);
              wdt_reset();
           }
         }
      } else {
         //Write EEPROM for the first time
         #ifdef HAS_SERIAL_COMM
          SERIAL.println(F("Wrong EEPROM version. Overwriting EEPROM with standard values."));
         #endif
         writeEEPROMData();
     }
   }
   else {
         //EEPROM corrupted. Write standard values
         #ifdef HAS_SERIAL_COMM
          SERIAL.println(F("EEPROM corrupted. Overwriting EEPROM with standard values."));
         #endif
         writeEEPROMData();    
   }  
}

byte writeEEPROMData() {
   tEEPROM eWrite;
   //eWrite.firstTimeRun = EEPROMVERSION; // Controll EEPROM version
   //eWrite.interlockStatus = interlockStatus; // Interlock status

   for(uint8_t i = 0; i < numOfSensors; i++){
      eWrite.sensorEnabled[i] = sensors[i].enabled;
      eWrite.sensorAlarmSP[i] = sensors[i].alarmSP;
      eWrite.sensorTimer[i] = sensors[i].timer;
   }
   EEPROM.update(VERSION_ADR,EEPROMVERSION);
   EEPROM.update(INTERLOCK_ADR,interlockStatus);
   EEPROM.put(DATA_ADR,eWrite);
   EEPROM.put(CRC_ADR,eeprom_crc());
   return DATA_ADR + sizeof(eWrite);
}

void updateEEPROMinterlock() {
   EEPROM.update(INTERLOCK_ADR,interlockStatus);
}

unsigned long eeprom_crc(void) {
   /***
    Written by Christopher Andrews.
    CRC algorithm generated by pycrc, MIT licence ( https://github.com/tpircher/pycrc ).
    https://www.arduino.cc/en/Tutorial/LibraryExamples/EEPROMCrc
   ***/

  const unsigned long crc_table[16] = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
  };

  unsigned long crc = ~0L;
  for (word index = DATA_ADR ; index < EEPROM.length()  ; ++index) {
    crc = crc_table[(crc ^ EEPROM[index]) & 0x0f] ^ (crc >> 4);
    crc = crc_table[(crc ^ (EEPROM[index] >> 4)) & 0x0f] ^ (crc >> 4);
    crc = ~crc;
  }
  return crc;
}

//***************************************************************************************
