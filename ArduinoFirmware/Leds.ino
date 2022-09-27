//*************************        LEDs module       *************************
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

#ifdef HAS_NEOPIXEL
  #include <Adafruit_NeoPixel.h>
  Adafruit_NeoPixel strip(NEOPIXEL_COUNT,NEOPIXEL_PIN,NEOPIXEL_TYPE);
#endif

tTimer ledTimer, neopixelAlarmTimer;
uint32_t tripNeopixelColor;
bool neopixelAlarmColor = false;

void led_Init() {
  #ifdef HAS_NEOPIXEL  
      strip.begin();
      strip.show();
      strip.setBrightness(NEOPIXEL_BRIGHTNESS);
  #endif
  //Pins configuration
  
  #ifdef ALARM_LED_PIN
      pinMode(ALARM_LED_PIN, OUTPUT);
      digitalWrite(ALARM_LED_PIN, HIGH); //Alarm led test 
  #endif
   
  #ifdef TRIP_LED_PIN
      pinMode(TRIP_LED_PIN, OUTPUT);
      digitalWrite(TRIP_LED_PIN, HIGH);  //Trip led test
  #endif

  #if defined(ALARM_LED_PIN) || defined(TRIP_LED_PIN)  
    delay(500);
  #endif

   // Finish Led test
  #ifdef ALARM_LED_PIN  
      digitalWrite(ALARM_LED_PIN, LOW);
  #endif
  #ifdef TRIP_LED_PIN
      digitalWrite(TRIP_LED_PIN, LOW);  
  #endif
  #ifdef HAS_NEOPIXEL
    for(long firstPixelHue = 0; firstPixelHue < 65536; firstPixelHue += 512) {
      for(int i=0; i<strip.numPixels(); i++) { 
        int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
        strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
      }
      strip.show();
      delay(10);
    }
  #endif

   // Start heart beat led timer 
   startTimer(&ledTimer.startMs,&ledTimer.started);
}

void updateAlarmLed() {
  bool changeLedFlag = false;
  #ifdef ALARM_LED_PIN
    digitalWrite(ALARM_LED_PIN, HIGH); 
  #endif
  #ifdef HAS_NEOPIXEL
    if (checkTimer(&neopixelAlarmTimer.startMs,ALARM_BLINK,&neopixelAlarmTimer.started)) { 
      if (neopixelAlarmColor) {
        neopixelAlarmColor = false;
        strip.fill(tripNeopixelColor, 0, NEOPIXEL_COUNT);                             
      } else {
        neopixelAlarmColor = true;
        strip.fill(strip.Color(ALARM_RGB), 0, NEOPIXEL_COUNT);       
      }      
      strip.show();      
    } else {
       startTimer(&neopixelAlarmTimer.startMs,&neopixelAlarmTimer.started);
    }  
  #endif  
}

void updateInterlockLed(bool interlockStatus) {
  #ifdef TRIP_LED_PIN
      digitalWrite(TRIP_LED_PIN, interlockStatus);
  #endif
  #ifdef HAS_NEOPIXEL
    if (interlockStatus) {  
      strip.fill(strip.Color(TRIP_RGB), 0, NEOPIXEL_COUNT);
      tripNeopixelColor = strip.Color(TRIP_RGB);
    } else {
      strip.fill(strip.Color(NORMAL_RGB), 0, NEOPIXEL_COUNT); 
      tripNeopixelColor = strip.Color(NORMAL_RGB);      
    }
    strip.show();
  #endif 
}

void ledHeartBeat() {
   #if defined(ALARM_LED_PIN) || defined(TRIP_LED_PIN)
   if (checkTimer(&ledTimer.startMs,LED_DELAY,&ledTimer.started)) { 
      #ifdef ALARM_LED_PIN
          digitalWrite(ALARM_LED_PIN,HIGH);
          delay(25);
          digitalWrite(ALARM_LED_PIN,LOW);
      #endif
      #ifdef TRIP_LED_PIN
          digitalWrite(TRIP_LED_PIN,HIGH);
          delay(25);
          digitalWrite(TRIP_LED_PIN,interlockStatus);
      #endif
   } else {
      startTimer(&ledTimer.startMs,&ledTimer.started);
   }
   #endif
}

void alarmLedLow() {
   #ifdef ALARM_LED_PIN
      digitalWrite(ALARM_LED_PIN, LOW); 
   #endif
   #ifdef HAS_NEOPIXEL
      neopixelAlarmColor = false;
      strip.fill(tripNeopixelColor, 0, NEOPIXEL_COUNT); 
      strip.show();   
   #endif  
}