//*************************   Configuration.h verification module  *************************
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

#define MIN_PIN           1
#define MAX_PIN           21
#define MAX_PIN_DIGITAL   13

#define A0                14
#define A1                15
#define A2                16
#define A3                17
#define A4                18
#define A5                19
#define A6                20
#define A7                21

// check pin assignment

#ifdef InterlockRelayPin
  #if InterlockRelayPin < MIN_PIN || InterlockRelayPin > MAX_PIN
    #error "Wrong InterlockRelay PIN definition. Check your Configuration.h file."
  #endif
#endif

#ifdef ResetButtonPin
  #if ResetButtonPin < MIN_PIN || ResetButtonPin > MAX_PIN
    #error "Wrong ResetButton PIN definition. Check your Configuration.h file."
  #endif
#endif

#ifdef AlarmLedPin
  #if AlarmLedPin < MIN_PIN || AlarmLedPin > MAX_PIN
    #error "Wrong AlarmLed PIN definition. Check your Configuration.h file."
  #endif
#endif

#ifdef TripLedPin
  #if TripLedPin < MIN_PIN || TripLedPin > MAX_PIN
    #error "Wrong TripLed PIN definition. Check your Configuration.h file."
  #endif 
#endif

#ifdef LCD_SDA_PIN
  #if LCD_SDA_PIN < MIN_PIN || LCD_SDA_PIN > MAX_PIN
    #error "Wrong LCD_SDA PIN definition. Check your Configuration.h file."
  #endif
#endif

#ifdef LCD_SCL_PIN
  #if LCD_SCL_PIN < MIN_PIN || LCD_SCL_PIN > MAX_PIN
    #error "Wrong LCD_SCL PIN definition. Check your Configuration.h file."
  #endif
#endif

#ifdef Sensor1Pin
  #if Sensor1Pin < MIN_PIN || Sensor1Pin > MAX_PIN
    #error "Wrong Sensor 1 PIN definition. Check your Configuration.h file."
  #endif
#endif
#ifdef Sensor1AuxPin
  #if Sensor1AuxPin < MIN_PIN || Sensor1AuxPin > MAX_PIN_DIGITAL
    #error "Wrong Sensor 1 Aux PIN definition. Check your Configuration.h file."
  #endif
#endif
#ifdef Sensor1Type 
  #if Sensor1Type != DigitalSensor && Sensor1Type != NTCSensor
    #error "Wrong Sensor 1 TYPE definition. Check your Configuration.h file."
  #endif
  #if Sensor1Type == DigitalSensor && Sensor1AlarmSP != HIGH && Sensor1AlarmSP != LOW
    #error  "Wrong Sensor 1 ALARM SET POINT definition. Check your Configuration.h file. Sensor disabled."
  #endif
#endif

#ifdef Sensor2Pin
  #if Sensor2Pin < MIN_PIN || Sensor2Pin > MAX_PIN
    #error "Wrong Sensor 2 PIN definition. Check your Configuration.h file."
  #endif
#endif
#ifdef Sensor2AuxPin
  #if Sensor2AuxPin < MIN_PIN || Sensor2AuxPin > MAX_PIN_DIGITAL
    #error "Wrong Sensor 2 Aux PIN definition. Check your Configuration.h file."
  #endif
#endif
#ifdef Sensor2Type 
  #if Sensor2Type != DigitalSensor && Sensor2Type != NTCSensor
    #error "Wrong Sensor 2 TYPE definition. Check your Configuration.h file."
  #endif
  #if Sensor2Type == DigitalSensor && Sensor2AlarmSP != HIGH && Sensor2AlarmSP != LOW
    #error  "Wrong Sensor 2 ALARM SET POINT definition. Check your Configuration.h file. Sensor disabled."
  #endif
#endif

#ifdef Sensor3Pin
  #if Sensor3Pin < MIN_PIN || Sensor3Pin > MAX_PIN
    #error "Wrong Sensor 3 PIN definition. Check your Configuration.h file."
  #endif
#endif
#ifdef Sensor3AuxPin
  #if Sensor3AuxPin < MIN_PIN || Sensor3AuxPin > MAX_PIN_DIGITAL
    #error "Wrong Sensor 3 Aux PIN definition. Check your Configuration.h file."
  #endif
#endif
#ifdef Sensor3Type 
  #if Sensor3Type != DigitalSensor && Sensor3Type != NTCSensor
    #error "Wrong Sensor 3 TYPE definition. Check your Configuration.h file."
  #endif
  #if Sensor3Type == DigitalSensor && Sensor3AlarmSP != HIGH && Sensor3AlarmSP != LOW
    #error  "Wrong Sensor 3 ALARM SET POINT definition. Check your Configuration.h file. Sensor disabled."
  #endif
#endif

#ifdef Sensor4Pin
  #if Sensor4Pin < MIN_PIN || Sensor4Pin > MAX_PIN
    #error "Wrong Sensor 4 PIN definition. Check your Configuration.h file."
  #endif
#endif
#ifdef Sensor4AuxPin
  #if Sensor4AuxPin < MIN_PIN || Sensor4AuxPin > MAX_PIN_DIGITAL
    #error "Wrong Sensor 4 Aux PIN definition. Check your Configuration.h file."
  #endif
#endif
#ifdef Sensor4Type 
  #if Sensor4Type != DigitalSensor && Sensor4Type != NTCSensor
    #error "Wrong Sensor 4 TYPE definition. Check your Configuration.h file."
  #endif
  #if Sensor4Type == DigitalSensor && Sensor4AlarmSP != HIGH && Sensor4AlarmSP != LOW
    #error  "Wrong Sensor 4 ALARM SET POINT definition. Check your Configuration.h file. Sensor disabled."
  #endif
#endif

#ifdef Sensor5Pin
  #if Sensor5Pin < MIN_PIN || Sensor5Pin > MAX_PIN
    #error "Wrong Sensor 5 PIN definition. Check your Configuration.h file."
  #endif
#endif
#ifdef Sensor5AuxPin
  #if Sensor5AuxPin < MIN_PIN || Sensor5AuxPin > MAX_PIN_DIGITAL
    #error "Wrong Sensor 5 Aux PIN definition. Check your Configuration.h file."
  #endif
#endif
#ifdef Sensor5Type 
  #if Sensor5Type != DigitalSensor && Sensor5Type != NTCSensor
    #error "Wrong Sensor 5 TYPE definition. Check your Configuration.h file."
  #endif
  #if Sensor5Type == DigitalSensor && Sensor5AlarmSP != HIGH && Sensor5AlarmSP != LOW
    #error  "Wrong Sensor 5 ALARM SET POINT definition. Check your Configuration.h file. Sensor disabled."
  #endif
#endif

#ifdef Sensor6Pin
  #if Sensor6Pin < MIN_PIN || Sensor6Pin > MAX_PIN
    #error "Wrong Sensor 6 PIN definition. Check your Configuration.h file."
  #endif
#endif
#ifdef Sensor6AuxPin
  #if Sensor6AuxPin < MIN_PIN || Sensor6AuxPin > MAX_PIN_DIGITAL
    #error "Wrong Sensor 6 Aux PIN definition. Check your Configuration.h file."
  #endif
#endif
#ifdef Sensor6Type 
  #if Sensor6Type != DigitalSensor && Sensor6Type != NTCSensor
    #error "Wrong Sensor 6 TYPE definition. Check your Configuration.h file."
  #endif
  #if Sensor6Type == DigitalSensor && Sensor6AlarmSP != HIGH && Sensor6AlarmSP != LOW
    #error  "Wrong Sensor 6 ALARM SET POINT definition. Check your Configuration.h file. Sensor disabled."
  #endif
#endif

#ifdef Sensor7Pin
  #if Sensor7Pin < MIN_PIN || Sensor7Pin > MAX_PIN
    #error "Wrong Sensor 7 PIN definition. Check your Configuration.h file."
  #endif
#endif
#ifdef Sensor7AuxPin
  #if Sensor7AuxPin < MIN_PIN || Sensor7AuxPin > MAX_PIN_DIGITAL
    #error "Wrong Sensor 7 Aux PIN definition. Check your Configuration.h file."
  #endif
#endif
#ifdef Sensor7Type 
  #if Sensor7Type != DigitalSensor && Sensor7Type != NTCSensor
    #error "Wrong Sensor 7 TYPE definition. Check your Configuration.h file."
  #endif
  #if Sensor7Type == DigitalSensor && Sensor7AlarmSP != HIGH && Sensor7AlarmSP != LOW
    #error  "Wrong Sensor 7 ALARM SET POINT definition. Check your Configuration.h file. Sensor disabled."
  #endif
#endif

#ifdef Sensor8Pin
  #if Sensor8Pin < MIN_PIN || Sensor8Pin > MAX_PIN
    #error "Wrong Sensor 8 PIN definition. Check your Configuration.h file."
  #endif
#endif
#ifdef Sensor8AuxPin
  #if Sensor8AuxPin < MIN_PIN || Sensor8AuxPin > MAX_PIN_DIGITAL
    #error "Wrong Sensor 8 Aux PIN definition. Check your Configuration.h file."
  #endif
#endif
#ifdef Sensor8Type 
  #if Sensor8Type != DigitalSensor && Sensor8Type != NTCSensor
    #error "Wrong Sensor 8 TYPE definition. Check your Configuration.h file."
  #endif
  #if Sensor8Type == DigitalSensor && Sensor8AlarmSP != HIGH && Sensor8AlarmSP != LOW
    #error  "Wrong Sensor 8 ALARM SET POINT definition. Check your Configuration.h file. Sensor disabled."
  #endif
#endif

