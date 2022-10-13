//*************************   LCD module  *************************
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
 * WARNIG: DON'T CHANGE ANYTHING IN THIS FILE! ALL CONFIGURATIONS SHOULD BE DONE IN "Configurations.h".
*/
#ifdef HAS_LCD

//#include <avr/pgmspace.h>  // Needed for PROGMEM stuff

#define MSG_BUFFER_SIZE 20

LiquidCrystal_I2C lcd(LCD_ADDRESS,LCD_SDA_PIN,LCD_SCL_PIN);

const byte ok[8] = {
 B00001,
 B00010,
 B00010,
 B00100,
 B10100,
 B01000,
 B01000,
};
const byte bell[8] = {
 B00000,
 B00100,
 B01110,
 B01110,
 B11111,
 B00100,
 B00000,
};

// extra msgs       Size[16]: "                "
const char MSG_0[]  PROGMEM = "Command ERROR";
const char MSG_1[]  PROGMEM = "Interlock reset";
const char MSG_2[]  PROGMEM = "Enabled";
const char MSG_3[]  PROGMEM = "Disabled";
const char MSG_4[]  PROGMEM = "Set point change";
const char MSG_5[]  PROGMEM = "EEPROM updated";
const char MSG_6[]  PROGMEM = "Printer OFF";
const char MSG_7[]  PROGMEM = "Printer ON";
const char MSG_8[]  PROGMEM = "Timer changed";
const char MSG_9[]  PROGMEM = "Standard values";
const char MSG_10[] PROGMEM = "HOLD to reset";
const char MSG_11[] PROGMEM = "Reset inhibited";

typedef struct
{
  byte sensorIndex;
  byte message;
  bool printLabel;
} tExtraMsg;

tExtraMsg extraMsgs[MSG_BUFFER_SIZE];

bool tripAnim = false;
byte lcdSensor = 0;
byte lastSetedMsg = 0;
byte lastPrintedMsg = 0; 

void lcd_Init() {
   lcd.init();
   //lcd.autoscroll();
   lcd.setBacklight(HIGH);
   lcd.setCursor(0,0);
   lcd.print(F(" Safety Printer"));
   const char ver[16] = "ver.:" VERSION; 
   lcd.setCursor(8-strlen(ver)/2,1);
   lcd.print(ver);
   lcd.createChar(0, ok);
   lcd.createChar(1, bell);
}

void updateLCD(bool forceShow) {  
  if (checkTimer(&LCDTimer.startMs,LCD_DELAY,&LCDTimer.started)) {
     if (tripLCD){ 
        // Shutdown Message
        if (!forceShow) {
           //Ignore other messages
           lastSetedMsg = lastPrintedMsg; 
        }
        lcd.clear();
        lcd.setCursor(1,0);
        if (tripAnim) {
            lcd.print(F(" "));
            lcd.write(byte(1));
            lcd.print(F(" "));
            lcd.write(byte(1));
        } else {
            lcd.write(byte(1));
            lcd.print(F(" "));
            lcd.write(byte(1));   
            lcd.print(F(" "));
        }
        lcd.print(F(" TRIP "));
        if (tripAnim) {
            lcd.print(F(" "));
            lcd.write(byte(1));
            lcd.print(F(" "));
            lcd.write(byte(1));
        } else {
            lcd.write(byte(1));
            lcd.print(F(" "));
            lcd.write(byte(1));   
            lcd.print(F(" "));
        }
        tripAnim = !tripAnim;         
        if (triggerIndex == 255) {
            lcd.setCursor(0,1);
            lcd.print(F("Saved / external"));
        } else {
            lcd.setCursor(8-strlen(sensors[triggerIndex].label)/2,1);
            lcd.print(sensors[triggerIndex].label);
        }
     } 
     if (lastSetedMsg != lastPrintedMsg) {
        // Print extra messages
        lcd.clear();
        lastPrintedMsg++;
        if (lastPrintedMsg >= MSG_BUFFER_SIZE) {
          lastPrintedMsg = 0;
        }
        byte line = 0;
        if (extraMsgs[lastPrintedMsg].printLabel){
           if (extraMsgs[lastPrintedMsg].sensorIndex < numOfSensors) {
              lcd.setCursor(8-strlen(sensors[extraMsgs[lastPrintedMsg].sensorIndex].label)/2,0);
              lcd.print(sensors[extraMsgs[lastPrintedMsg].sensorIndex].label);
           } else {
              lcd.setCursor(3,0);
              lcd.print(F("All sensors"));
           }
           line = 1;
        }
        char msg[16] = "";
        switch (extraMsgs[lastPrintedMsg].message) {
          case 0:
             strcpy_P(msg,MSG_0);
             break;
          case 1:
             strcpy_P(msg,MSG_1);
             break;
          case 2:
             strcpy_P(msg,MSG_2);
             break;
          case 3:
             strcpy_P(msg,MSG_3);
             break;
          case 4:
             strcpy_P(msg,MSG_4);
             break;
          case 5:
             strcpy_P(msg,MSG_5);
             break;
          case 6:
             strcpy_P(msg,MSG_6);
             break;
          case 7:
             strcpy_P(msg,MSG_7);
             break;
          case 8:
             strcpy_P(msg,MSG_8);
             break;
          case 9:
             strcpy_P(msg,MSG_9);
             break;
          case 10:
             strcpy_P(msg,MSG_10);
             break;
          case 11:
             strcpy_P(msg,MSG_11);
             break;
        }
        lcd.setCursor(8-strlen(msg)/2,line);
        lcd.print(msg);
     
     } else if (!tripLCD){
        lcd.clear();
        //Normal sensors status (if there is nothing else to print)
        lcd.setCursor(8-strlen(sensors[lcdSensor].label)/2,0);
        lcd.print(sensors[lcdSensor].label);
        lcd.setCursor(0,1);
        lcd.print(F("Value:"));
        if (sensors[lcdSensor].active) {
            lcd.write(byte(1));
        } else {
            lcd.write(byte(0));
        }
        if (!sensors[lcdSensor].enabled) {
            lcd.print(F(" Disabled"));
        } else if (sensors[lcdSensor].type == NTC_SENSOR) {
            lcd.print(F(" "));
            lcd.print(sensors[lcdSensor].actualValue);
            lcd.print((char) 223); //º character https://arduino.stackexchange.com/questions/46828/how-to-show-the-%C2%BA-character-in-a-lcd
        } 
     }
     lcdSensor++;
     if (lcdSensor == numOfSensors) {
        lcdSensor = 0;
     }
  } else {
     startTimer(&LCDTimer.startMs,&LCDTimer.started);
  } 
}

void includeExtraMsg(byte index, byte msg, bool printSensor) {
  lastSetedMsg++;
  if (lastSetedMsg >= MSG_BUFFER_SIZE) {
    lastSetedMsg = 0;
  }
  extraMsgs[lastSetedMsg].sensorIndex = index;
  extraMsgs[lastSetedMsg].message = msg;
  extraMsgs[lastSetedMsg].printLabel = printSensor;
}
#endif
//***************************************************************************************
