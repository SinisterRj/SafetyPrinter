//*************************      Configurations module     *************************
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
*/


// ****************************************************************************
// General Configuration:

#define InterlockRelayPin     6           // Arduino's Digital pin connected to the Printer's power supply relay.
#define InterlockPolarity     HIGH        // Change if you need to toggle the behavior of the interlock pin (if its HIGH it means that the [InterlockRelayPin] output will be LOW under normal conditions and HIGH when an interlock occurs)

#define ResetButtonPin        11          // Arduino's Digital pin connected to the trip reset switch.
#define ResetDelay            1000        // Delay for reseting the trip condition in [ms]

#define AlarmLedPin           10          // Arduino's Digital pin connected to the alarm indication LED.
#define TripLedPin            12          // Arduino's Digital pin connected to the trip indication LED.
#define LEDDelay              5000        // Led heart beat interval in [ms]

#define SerialComm            true        // [true] to Enable serial communications.
#define BaudRate              115200      // :[2400, 9600, 19200, 38400, 57600, 115200, 250000, 500000, 1000000]

/* 
****************************************************************************
Sensor configuration:

Max: 8 sensors

All sensors have the same structure:

SensorXLabel    : Mandatory  : A string with the sensor's NAME;
SensorXPin      : Mandatory  : Arduino's pin (Analog or digital) connected to the sensor signal;
SensorXAuxPin   : *Optional  : Arduino's digital pin to power the temperature sensor;
SensorXType     : Mandatory  : [DigitalSensor] for On/OFF sensors or [NTCSensor] for temperature sensors (NTC thermistor type)
SensorXTimer    : Mandatory  : Delay between the sensor signal and raising the alarm. The alarm will be raised IF the sensor signal stays in alarm condition more time than this value. Good to avoid spurious alarms. Expressed in [ms].
SensorXAlarmSP  : Mandatory  : Alarm set point. Defines the signal condition to raise the alarm. [0] or [1] to define alarm positions on digital sensors, and a [integer] to define a high temperature on temperature sensors.
Sensor5TempType : *Optional  : An integer number with one of temperature table calibration bellow:

 ****************************************************************************
 *    NTC thermistor temperature equivalence (calibration) table:
 *    --NORMAL IS 4.7kohm PULLUP!-- 1kohm pullup can be used on hotend sensor, using correct resistor and table
 * 
 *    -3 : thermocouple with MAX31855 (only for sensor 0)
 *    -2 : thermocouple with MAX6675 (only for sensor 0)
 *    -1 : thermocouple with AD595
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
 *    20 : the PT100 circuit found in the Ultimainboard V2.x
 *    60 : 100k Maker's Tool Works Kapton Bed Thermistor beta=3950
 *    66 : 4.7M High Temperature thermistor from Dyze Design
 *    70 : the 100K thermistor found in the bq Hephestos 2
 *    75 : 100k Generic Silicon Heat Pad with NTC 100K MGB18-104F39050L32 thermistor
 *
 *       1k ohm pullup tables - This is atypical, and requires changing out the 4.7k pullup for 1k.
 *                              (but gives greater accuracy and more stable PID)
 *    51 : 100k thermistor - EPCOS (1k pullup)
 *    52 : 200k thermistor - ATC Semitec 204GT-2 (1k pullup)
 *    55 : 100k thermistor - ATC Semitec 104GT-2 (Used in ParCan & J-Head) (1k pullup)
 *
 *  1047 : Pt1000 with 4k7 pullup
 *  1010 : Pt1000 with 1k pullup (non standard)
 *   147 : Pt100 with 4k7 pullup
 *   110 : Pt100 with 1k pullup (non standard)
 *
 *         Use these for Testing or Development purposes. NEVER for production machine.
 *   998 : Dummy Table that ALWAYS reads 25°C or the temperature defined below.
 *   999 : Dummy Table that ALWAYS reads 100°C or the temperature defined below.
 *
 * :{ '0': "Not used", '1':"100k / 4.7k - EPCOS", '2':"200k / 4.7k - ATC Semitec 204GT-2", '3':"Mendel-parts / 4.7k", '4':"10k !! do not use for a hotend. Bad resolution at high temp. !!", '5':"100K / 4.7k - ATC Semitec 104GT-2 (Used in ParCan & J-Head)", '6':"100k / 4.7k EPCOS - Not as accurate as Table 1", '7':"100k / 4.7k Honeywell 135-104LAG-J01", '8':"100k / 4.7k 0603 SMD Vishay NTCS0603E3104FXT", '9':"100k / 4.7k GE Sensing AL03006-58.2K-97-G1", '10':"100k / 4.7k RS 198-961", '11':"100k / 4.7k beta 3950 1%", '12':"100k / 4.7k 0603 SMD Vishay NTCS0603E3104FXT (calibrated for Makibox hot bed)", '13':"100k Hisens 3950  1% up to 300°C for hotend 'Simple ONE ' & hotend 'All In ONE'", '20':"PT100 (Ultimainboard V2.x)", '51':"100k / 1k - EPCOS", '52':"200k / 1k - ATC Semitec 204GT-2", '55':"100k / 1k - ATC Semitec 104GT-2 (Used in ParCan & J-Head)", '60':"100k Maker's Tool Works Kapton Bed Thermistor beta=3950", '66':"Dyze Design 4.7M High Temperature thermistor", '70':"the 100K thermistor found in the bq Hephestos 2", '71':"100k / 4.7k Honeywell 135-104LAF-J01", '147':"Pt100 / 4.7k", '1047':"Pt1000 / 4.7k", '110':"Pt100 / 1k (non-standard)", '1010':"Pt1000 / 1k (non standard)", '-3':"Thermocouple + MAX31855 (only for sensor 0)", '-2':"Thermocouple + MAX6675 (only for sensor 0)", '-1':"Thermocouple + AD595",'998':"Dummy 1", '999':"Dummy 2" }
 *
*/

#define Sensor1Label           "Flame 1"
#define Sensor1Pin             9
#define Sensor1Type            DigitalSensor
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
#define Sensor5AuxPin          3
#define Sensor5Type            NTCSensor
#define Sensor5Timer           250
#define Sensor5AlarmSP         290
#define Sensor5TempType        1

#define Sensor6Label           "Bed Temp."
#define Sensor6Pin             A6
#define Sensor6AuxPin          2
#define Sensor6Type            NTCSensor
#define Sensor6Timer           250
#define Sensor6AlarmSP         150
#define Sensor6TempType        1

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
