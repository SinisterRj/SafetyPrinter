/*
 ************************      Configurations module     *************************
 *
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
*/


// ****************************************************************************
// General Configurations:

// ########################  Interlock Relay  ########################
    
#define INTERLOCK_RELAY_PIN     10          // Arduino's Digital pin connected to the Printer's power supply relay.
#define INTERLOCK_POLARITY      HIGH        // Change if you need to toggle the behavior of the interlock pin (if its HIGH it means that the [INTERLOCK_RELAY_PIN] output will be LOW under normal conditions and HIGH when an interlock occurs)
#define MINIMUM_INTERLOCK_DELAY 5           // Minimum amount of time (in s) that an interlock will be active (to avoid fast switch printer on and off and possible damage it)

// ########################  Reset button  ########################

#define RESET_BUTTON_PIN        11          // Arduino's Digital pin connected to the trip reset switch (OPTIONAL).
#define RESET_DELAY             2500        // Delay for reseting the trip condition in [ms]. The user must press the reset button for this amount of time to reset a trip.

// ########################  Auto-Reset  ########################

#define ARDUINO_RESET_PIN       2           // Arduino's Digital pin connected to RST pin (resets Arduino via serial C9 command to allow firware update from octoprint's plugin).

// ########################  Alarm & trip LED indication  ########################

#define HAS_NEOPIXEL                        // Use a neopixel led or strip to indicate the Safety Printer MCU status (OPTIONAL).
#ifdef HAS_NEOPIXEL
  #define NEOPIXEL_PIN          13          // Arduino's Digital pin connected to the neopixels strip data line.
  #define NEOPIXEL_COUNT        1           // Number of neopixels (LEDs) on the strip.
  #define NEOPIXEL_TYPE         NEO_GRB     // Neopixel color order: NEO_RGB, NEO_GRB or NEO_RGBW.
  #define NEOPIXEL_BRIGHTNESS   125         // Neopixel leds brightness. Any value from 0 to 255.
  #define NORMAL_RGB            60,255,10   // Normal status neopixel color in [Red,Green,Blue,White].
  #define ALARM_RGB             255,255,0   // Alarm status neopixel color in [Red,Green,Blue,White].
  #define TRIP_RGB              255,0,0     // Trip status neopixel color in [Red,Green,Blue,White].
  #define ALARM_BLINK           500         // Alarm color blink interval in [ms]       
#else
  #define ALARM_LED_PIN           13        // Arduino's Digital pin connected to the alarm indication LED (OPTIONAL).
  #define TRIP_LED_PIN            12        // Arduino's Digital pin connected to the trip indication LED (OPTIONAL).
  #define LED_DELAY               5000      // Led heart beat interval in [ms]
#endif

// ########################  Serial Communication  ########################

#define HAS_SERIAL_COMM                     // Uncomment to Enable serial communications. Needed to interface with Octoprint plugin (OPTIONAL)
#ifdef HAS_SERIAL_COMM
  #define BAUD_RATE             115200       // :[2400, 9600, 19200, 38400, 57600, 115200]
#endif

// ########################  LCD  ########################

//#define HAS_LCD                             // Uncomment to Enable the use of a 2x16 I2C LCD. (OPTIONAL)    
#ifdef HAS_LCD
  #define LCD_DELAY             2000        // LCD refresh rate in ms
  #define LCD_ADDRESS           0x27        // LCD I2C address
  #define LCD_SDA_PIN           A4          // I2C data PIN (always use A4 for Arduino Uno or Nano)
  #define LCD_SCL_PIN           A5          // I2C clock PIN (always use A5 for Arduino Uno or Nano)      
#endif

// ########################  Sensors  ########################

/* 

Sensor configuration:

Max: 8 sensors

All sensors have the same structure:

SENSOR_X_LABEL     : Mandatory  : A string with the sensor's NAME - MAX 16 characters. Don't use ",", "#", "$", ":", "<" or ">";
SENSOR_X_PIN       : Mandatory  : Arduino's pin (Analog or digital) connected to the sensor signal;
SENSOR_X_AUX_PIN   : *Optional  : Arduino's digital pin to power the temperature sensor (mandatory for NTC_SENSOR);
SENSOR_X_TYPE      : Mandatory  : [DIGIGTAL_SENSOR] for On/OFF sensors or [NTC_SENSOR] for temperature sensors (NTC thermistor type)
SENSOR_X_TIMER     : Mandatory  : Delay between the sensor signal and raising the alarm. The alarm will be raised IF the sensor signal stays in alarm condition more time than this value. Good to avoid spurious alarms. Expressed in [ms].
SENSOR_X_ALARM_SP  : Mandatory  : Alarm set point. Defines the signal condition to raise the alarm. [HIGH] or [LOW] to define alarm positions on digital sensors, and a [integer] to define a high temperature on temperature sensors.
SENSOR_X_TEMP_TYPE : *Optional  : An integer number with one of temperature table calibration bellow (mandatory for NTC_SENSOR):

 ****************************************************************************
 *    NTC thermistor temperature equivalence (calibration) table:
 *    --NORMAL IS 4.7kohm PULLUP!-- 1kohm pullup can be used on hotend SENSOR_, using correct resistor and table
 * 
 *     0 : not used
 *     1 : 100k thermistor - best choice for EPCOS 100k (4.7k pullup)
 *     2 : 200k thermistor - ATC Semitec 204GT-2 (4.7k pullup)
 *     3 : Mendel-parts thermistor (4.7k pullup)
 *     4 : 10k thermistor !! do not use it for a hotend. It gives bad resolution at high temp. !!
 *     5 : 100K thermistor - ATC Semitec 104GT-2 (Used in ParCan & J-Head) (4.7k pullup)
 *     6 : 100k EPCOS - Not as accurate as table 1 (created using a fluke thermocouple) (4.7k pullup)
 *     7 : 100k Honeywell thermistor 135-104LAG-J01 (4.7k pullup)
 *    71 : 100k Honeywell thermistor 135-104LAF-J01 (4.7k pullup)
 *     8 : 100k 0603 SMD Vishay NTCS0603E3104FXT (4.7k pullup)
 *     9 : 100k GE Sensing AL03006-58.2K-97-G1 (4.7k pullup)
 *    10 : 100k RS thermistor 198-961 (4.7k pullup)
 *    11 : 100k beta 3950 1% thermistor (4.7k pullup)
 *    12 : 100k 0603 SMD Vishay NTCS0603E3104FXT (4.7k pullup) (calibrated for Makibox hot bed)
 *    13 : 100k Hisens 3950  1% up to 300°C for hotend "Simple ONE " & "Hotend "All In ONE"
 *    15 : JGAurora A5 thermistor calibration
 *    17 : Dagoma NTC 100k white thermistor
 *    18 : ATC Semitec 204GT-2 (4.7k pullup) Dagoma.Fr - MKS_Base_DKU001327 
 *    20 : PT100 with INA826 amp on Ultimaker v2.0 electronics
 *    21 : Pt100 with INA826 amp with 3.3v excitation based on "Pt100 with INA826 amp on Ultimaker v2.0 electronics"
 *    22 : Thermistor in a Rostock 301 hot end, calibrated with a multimeter
 *    23 : By AluOne #12622. Formerly 22 above. May need calibration/checking.
 *    30 : Kis3d Silicone mat 24V 200W/300W with 6mm Precision cast plate (EN AW 5083)
 *    60 : 100k Maker's Tool Works Kapton Bed Thermistor beta=3950
 *    61 : Formbot 350°C Thermistor
 *    66 : 4.7M High _TEMPerature thermistor from Dyze Design
 *    67 :  SliceEngineering 450 °C Thermistor
 *    70 : the 100K thermistor found in the bq Hephestos 2
 *    75 : 100k Generic Silicon Heat Pad with NTC 100K MGB18-104F39050L32 thermistor
 *
 *       1k ohm pullup tables - This is atypical, and requires changing out the 4.7k pullup for 1k.
 *                              (but gives greater accuracy and more stable PID)
 *    51 : 100k thermistor - EPCOS (1k pullup)
 *    52 : 200k thermistor - ATC Semitec 204GT-2 (1k pullup)
 *    55 : 100k thermistor - ATC Semitec 104GT-2 (Used in ParCan & J-Head) (1k pullup)
 *
 *       10k ohm pullup tables - This is atypical, and requires changing out the 4.7k pullup for 10k.
 * 
 *    99 : 100k bed thermistor with a 10K pull-up resistor
 * 
 *       3.3 V supply.
 * 
 *   331 : Like table 1, but with 3V3 as input voltage for MEGA
 *   332 : Like table 1, but with 3V3 as input voltage for DUE
 *
 *       Use these for Testing or Development purposes. NEVER for production machine.
 * 
 *   998 : Dummy Table that ALWAYS reads 25°C or the temperature defined below.
 *   999 : Dummy Table that ALWAYS reads 100°C or the temperature defined below.
 *
 *
*/

// Digital Sensor 1:
  #define SENSOR_1_LABEL           "Smoke"       
  #define SENSOR_1_PIN              9                
//#define SENSOR_1_AUX_PIN          0                
  #define SENSOR_1_TYPE             DIGIGTAL_SENSOR
  #define SENSOR_1_TIMER            250              
  #define SENSOR_1_ALARM_SP         LOW              
//#define SENSOR_1_TEMP_TYPE        1                

// Digital Sensor 2
  #define SENSOR_2_LABEL            "Flame 1"
  #define SENSOR_2_PIN              8
//#define SENSOR_2_AUX_PIN          0
  #define SENSOR_2_TYPE             DIGIGTAL_SENSOR
  #define SENSOR_2_TIMER            250
  #define SENSOR_2_ALARM_SP         LOW
//#define SENSOR_2_TEMP_TYPE        1

// Digital Sensor 3
  #define SENSOR_3_LABEL            "Flame 2"
  #define SENSOR_3_PIN              7
//#define SENSOR_3_AUX_PIN          0
  #define SENSOR_3_TYPE             DIGIGTAL_SENSOR
  #define SENSOR_3_TIMER            250
  #define SENSOR_3_ALARM_SP         LOW
//#define SENSOR_3_TEMP_TYPE        1

// Digital Sensor 4  
  #define SENSOR_4_LABEL            "Emergency Button"
  #define SENSOR_4_PIN              6
//#define SENSOR_4_AUX_PIN          0
  #define SENSOR_4_TYPE             DIGIGTAL_SENSOR
  #define SENSOR_4_TIMER            250
  #define SENSOR_4_ALARM_SP         LOW
//#define SENSOR_4_TEMP_TYPE        1

// Temp. Sensor 1
  #define SENSOR_5_LABEL            "HotEnd Temp."
  #define SENSOR_5_PIN              A0 //A6
  #define SENSOR_5_AUX_PIN          5 //2
  #define SENSOR_5_TYPE             NTC_SENSOR
  #define SENSOR_5_TIMER            250
  #define SENSOR_5_ALARM_SP         290
  #define SENSOR_5_TEMP_TYPE        13

// Temp. Sensor 2  
  #define SENSOR_6_LABEL            "Bed Temp."
  #define SENSOR_6_PIN              A1 //A7
  #define SENSOR_6_AUX_PIN          4  //3
  #define SENSOR_6_TYPE             NTC_SENSOR
  #define SENSOR_6_TIMER            250
  #define SENSOR_6_ALARM_SP         150
  #define SENSOR_6_TEMP_TYPE        1
  /* 
  #define SENSOR_7_LABEL            "Spare"
  #define SENSOR_7_PIN              1
  #define SENSOR_7_AUX_PIN          0
  #define SENSOR_7_TYPE             DIGIGTAL_SENSOR
  #define SENSOR_7_TIMER            250
  #define SENSOR_7_ALARM_SP         LOW
  #define SENSOR_7_TEMP_TYPE        1
  */
  /*
  #define SENSOR_8_LABEL            "Spare"
  #define SENSOR_8_PIN              1
  #define SENSOR_8_AUX_PIN          0
  #define SENSOR_8_TYPE             DIGIGTAL_SENSOR
  #define SENSOR_8_TIMER            250
  #define SENSOR_8_ALARM_SP         LOW
  #define SENSOR_8_TEMP_TYPE        1
  */
