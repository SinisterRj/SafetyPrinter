//*************************        Thermistor module       *************************
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

#define SAMPLES               20          // Number of samples for NTC thermistor. Smaller number will add some noise to signal, greater number will add some delay on sample acquisition.

#define OV(N) int16_t(N)   //((N)*(SAMPLES))
#define COUNT(a) (sizeof(a)/sizeof(*a))

typedef int16_t celsius_t;
typedef struct { int16_t value; celsius_t celsius; } temp_entry_t;

#define ANY_THERMISTOR_IS(n) (Sensor1TempType == n || Sensor2TempType == n || Sensor3TempType == n || Sensor4TempType == n || Sensor5TempType == n || Sensor6TempType == n || Sensor7TempType == n || Sensor8TempType == n )

#if ANY_THERMISTOR_IS(1) // 100k bed thermistor
  #include "tables/thermistor_1.h"
#endif
#if ANY_THERMISTOR_IS(2) // 200k bed thermistor
  #include "tables/thermistor_2.h"
#endif
#if ANY_THERMISTOR_IS(3) // mendel-parts
  #include "tables/thermistor_3.h"
#endif
#if ANY_THERMISTOR_IS(4) // 10k thermistor
  #include "tables/thermistor_4.h"
#endif
#if ANY_THERMISTOR_IS(5) // 100k ParCan thermistor (104GT-2)
  #include "tables/thermistor_5.h"
#endif
#if ANY_THERMISTOR_IS(6) // 100k Epcos thermistor
  #include "tables/thermistor_6.h"
#endif
#if ANY_THERMISTOR_IS(7) // 100k Honeywell 135-104LAG-J01
  #include "tables/thermistor_7.h"
#endif
#if ANY_THERMISTOR_IS(71) // 100k Honeywell 135-104LAF-J01
  #include "tables/thermistor_71.h"
#endif
#if ANY_THERMISTOR_IS(8) // 100k 0603 SMD Vishay NTCS0603E3104FXT (4.7k pullup)
  #include "tables/thermistor_8.h"
#endif
#if ANY_THERMISTOR_IS(9) // 100k GE Sensing AL03006-58.2K-97-G1 (4.7k pullup)
  #include "tables/thermistor_9.h"
#endif
#if ANY_THERMISTOR_IS(10) // 100k RS thermistor 198-961 (4.7k pullup)
  #include "tables/thermistor_10.h"
#endif
#if ANY_THERMISTOR_IS(11) // QU-BD silicone bed QWG-104F-3950 thermistor
  #include "tables/thermistor_11.h"
#endif
#if ANY_THERMISTOR_IS(13) // Hisens thermistor B25/50 =3950 +/-1%
  #include "tables/thermistor_13.h"
#endif
#if ANY_THERMISTOR_IS(20) // PT100 with INA826 amp on Ultimaker v2.0 electronics
  #include "tables/thermistor_20.h"
#endif
#if ANY_THERMISTOR_IS(51) // 100k EPCOS (WITH 1kohm RESISTOR FOR PULLUP, R9 ON SANGUINOLOLU! NOT FOR 4.7kohm PULLUP! THIS IS NOT NORMAL!)
  #include "tables/thermistor_51.h"
#endif
#if ANY_THERMISTOR_IS(52) // 200k ATC Semitec 204GT-2 (WITH 1kohm RESISTOR FOR PULLUP, R9 ON SANGUINOLOLU! NOT FOR 4.7kohm PULLUP! THIS IS NOT NORMAL!)
  #include "tables/thermistor_52.h"
#endif
#if ANY_THERMISTOR_IS(55) // 100k ATC Semitec 104GT-2 (Used on ParCan) (WITH 1kohm RESISTOR FOR PULLUP, R9 ON SANGUINOLOLU! NOT FOR 4.7kohm PULLUP! THIS IS NOT NORMAL!)
  #include "tables/thermistor_55.h"
#endif
#if ANY_THERMISTOR_IS(60) // Maker's Tool Works Kapton Bed Thermistor
  #include "tables/thermistor_60.h"
#endif
#if ANY_THERMISTOR_IS(66) // DyzeDesign 500°C Thermistor
  #include "tables/thermistor_66.h"
#endif
#if ANY_THERMISTOR_IS(12) // 100k 0603 SMD Vishay NTCS0603E3104FXT (4.7k pullup) (calibrated for Makibox hot bed)
  #include "tables/thermistor_12.h"
#endif
#if ANY_THERMISTOR_IS(70) // bqh2 stock thermistor
  #include "tables/thermistor_70.h"
#endif
#if ANY_THERMISTOR_IS(75) // Many of the generic silicon heat pads use the MGB18-104F39050L32 Thermistor
  #include "tables/thermistor_75.h"
#endif
#if ANY_THERMISTOR_IS(110) // Pt100 with 1k0 pullup
  #include "tables/thermistor_110.h"
#endif
#if ANY_THERMISTOR_IS(147) // Pt100 with 4k7 pullup
  #include "tables/thermistor_147.h"
#endif
#if ANY_THERMISTOR_IS(1010) // Pt1000 with 1k0 pullup
  #include "tables/thermistor_1010.h"
#endif
#if ANY_THERMISTOR_IS(1047) // Pt1000 with 4k7 pullup
  #include "tables/thermistor_1047.h"
#endif
#if ANY_THERMISTOR_IS(998) // User-defined table 1
  #include "tables/thermistor_998.h"
#endif
#if ANY_THERMISTOR_IS(999) // User-defined table 2
  #include "tables/thermistor_999.h"
#endif
#if ANY_THERMISTOR_IS(80) // 3950 thermistor
  #include "tables/thermistor_80.h"
#endif

#ifdef Sensor1TempType
  #define Sensor1_TEMPTABLE TT_NAME(Sensor1TempType)  
  #define Sensor1_TEMPTABLE_LEN COUNT(Sensor1_TEMPTABLE)
#else
  #define Sensor1_TEMPTABLE NULL
  #define Sensor1_TEMPTABLE_LEN 0
#endif

#ifdef Sensor2TempType
  #define Sensor2_TEMPTABLE TT_NAME(Sensor2TempType) 
  #define Sensor2_TEMPTABLE_LEN COUNT(Sensor2_TEMPTABLE) 
#else
  #define Sensor2_TEMPTABLE NULL
  #define Sensor2_TEMPTABLE_LEN 0
#endif

#ifdef Sensor3TempType
  #define Sensor3_TEMPTABLE TT_NAME(Sensor3TempType)  
  #define Sensor3_TEMPTABLE_LEN COUNT(Sensor3_TEMPTABLE)
#else
  #define Sensor3_TEMPTABLE NULL
  #define Sensor3_TEMPTABLE_LEN 0
#endif

#ifdef Sensor4TempType
  #define Sensor4_TEMPTABLE TT_NAME(Sensor4TempType) 
  #define Sensor4_TEMPTABLE_LEN COUNT(Sensor4_TEMPTABLE) 
#else
  #define Sensor4_TEMPTABLE NULL
  #define Sensor4_TEMPTABLE_LEN 0
#endif

#ifdef Sensor5TempType
  #define Sensor5_TEMPTABLE TT_NAME(Sensor5TempType)  
  #define Sensor5_TEMPTABLE_LEN COUNT(Sensor5_TEMPTABLE)
#else
  #define Sensor5_TEMPTABLE NULL
  #define Sensor5_TEMPTABLE_LEN 0
#endif

#ifdef Sensor6TempType
  #define Sensor6_TEMPTABLE TT_NAME(Sensor6TempType)
  #define Sensor6_TEMPTABLE_LEN COUNT(Sensor6_TEMPTABLE)  
#else
  #define Sensor6_TEMPTABLE NULL
  #define Sensor6_TEMPTABLE_LEN 0
#endif

#ifdef Sensor7TempType
  #define Sensor7_TEMPTABLE TT_NAME(Sensor7TempType)
  #define Sensor7_TEMPTABLE_LEN COUNT(Sensor7_TEMPTABLE)  
#else
  #define Sensor7_TEMPTABLE NULL
  #define Sensor7_TEMPTABLE_LEN 0
#endif

#ifdef Sensor8TempType
  #define Sensor8_TEMPTABLE TT_NAME(Sensor8TempType) 
  #define Sensor8_TEMPTABLE_LEN COUNT(Sensor8_TEMPTABLE) 
#else
  #define Sensor8_TEMPTABLE NULL
  #define Sensor8_TEMPTABLE_LEN 0
#endif

#define _TT_NAME(_N) temptable_ ## _N
#define TT_NAME(_N) _TT_NAME(_N)

#define PGM_RD_W(x)   (short)pgm_read_word(&x)

int read_temp(byte NTC_Pin, byte NTC_Power_Pin, byte sensorNumber) 
{
    digitalWrite(NTC_Power_Pin, HIGH);        // Power thermistor
    int16_t rawAdc = 0 ;
    for (int i = 0; i< SAMPLES; i++) {
        rawAdc += analogRead(NTC_Pin);
    }   
    digitalWrite(NTC_Power_Pin, LOW);         // Unpower thermistor
    rawAdc /= SAMPLES; 
    int current_celsius = 0;

    word numTemps = 0;
    switch (sensorNumber) {
        case 0:
            numTemps = Sensor1_TEMPTABLE_LEN;
            break;
        case 1:
            numTemps = Sensor2_TEMPTABLE_LEN;
            break;
        case 2:
            numTemps = Sensor3_TEMPTABLE_LEN;
            break;
        case 3:
            numTemps = Sensor4_TEMPTABLE_LEN;
            break;
        case 4:
            numTemps = Sensor5_TEMPTABLE_LEN;
            break;
        case 5:
            numTemps = Sensor6_TEMPTABLE_LEN;
            break;
        case 6:
            numTemps = Sensor7_TEMPTABLE_LEN;
            break;
        case 7:
            numTemps = Sensor8_TEMPTABLE_LEN;
            break;
    }

    temp_entry_t t[2] = {{OV(0),0},{OV(0),0}};

    word i;
    for (i=1; i<numTemps; i++)
    {      
        switch (sensorNumber) {
        case 0:
            #ifdef Sensor1TempType
                t[0].value =    PGM_RD_W(Sensor1_TEMPTABLE[i-1].value);
                t[0].celsius =  PGM_RD_W(Sensor1_TEMPTABLE[i-1].celsius);
                t[1].value =    PGM_RD_W(Sensor1_TEMPTABLE[i].value);
                t[1].celsius =  PGM_RD_W(Sensor1_TEMPTABLE[i].celsius);
            #endif           
            break;
        case 1:
            #ifdef Sensor2TempType
                t[0].value =    PGM_RD_W(Sensor2_TEMPTABLE[i-1].value);
                t[0].celsius =  PGM_RD_W(Sensor2_TEMPTABLE[i-1].celsius);
                t[1].value =    PGM_RD_W(Sensor2_TEMPTABLE[i].value);
                t[1].celsius =  PGM_RD_W(Sensor2_TEMPTABLE[i].celsius);  
            #endif  
            break;
        case 2:
            #ifdef Sensor3TempType
                t[0].value =    PGM_RD_W(Sensor3_TEMPTABLE[i-1].value);
                t[0].celsius =  PGM_RD_W(Sensor3_TEMPTABLE[i-1].celsius);
                t[1].value =    PGM_RD_W(Sensor3_TEMPTABLE[i].value);
                t[1].celsius =  PGM_RD_W(Sensor3_TEMPTABLE[i].celsius);  
            #endif  
            break;
        case 3:
            #ifdef Sensor4TempType
                t[0].value =    PGM_RD_W(Sensor4_TEMPTABLE[i-1].value);
                t[0].celsius =  PGM_RD_W(Sensor4_TEMPTABLE[i-1].celsius);
                t[1].value =    PGM_RD_W(Sensor4_TEMPTABLE[i].value);
                t[1].celsius =  PGM_RD_W(Sensor4_TEMPTABLE[i].celsius); 
            #endif  
            break;
        case 4:
            #ifdef Sensor5TempType
                t[0].value =    PGM_RD_W(Sensor5_TEMPTABLE[i-1].value);
                t[0].celsius =  PGM_RD_W(Sensor5_TEMPTABLE[i-1].celsius);
                t[1].value =    PGM_RD_W(Sensor5_TEMPTABLE[i].value);
                t[1].celsius =  PGM_RD_W(Sensor5_TEMPTABLE[i].celsius); 
            #endif  
            break;
        case 5:
            #ifdef Sensor6TempType
                t[0].value =    PGM_RD_W(Sensor6_TEMPTABLE[i-1].value);
                t[0].celsius =  PGM_RD_W(Sensor6_TEMPTABLE[i-1].celsius);
                t[1].value =    PGM_RD_W(Sensor6_TEMPTABLE[i].value);
                t[1].celsius =  PGM_RD_W(Sensor6_TEMPTABLE[i].celsius); 
            #endif  
            break;
        case 6:
            #ifdef Sensor7TempType
                t[0].value =    PGM_RD_W(Sensor7_TEMPTABLE[i-1].value);
                t[0].celsius =  PGM_RD_W(Sensor7_TEMPTABLE[i-1].celsius);
                t[1].value =    PGM_RD_W(Sensor7_TEMPTABLE[i].value);
                t[1].celsius =  PGM_RD_W(Sensor7_TEMPTABLE[i].celsius); 
            #endif  
            break;
        case 7:
            #ifdef Sensor8TempType
                t[0].value =    PGM_RD_W(Sensor8_TEMPTABLE[i-1].value);
                t[0].celsius =  PGM_RD_W(Sensor8_TEMPTABLE[i-1].celsius);
                t[1].value =    PGM_RD_W(Sensor8_TEMPTABLE[i].value);
                t[1].celsius =  PGM_RD_W(Sensor8_TEMPTABLE[i].celsius);
            #endif  
            break;
        }              
        if (t[1].value > rawAdc)
        {
            current_celsius  = t[0].celsius + (rawAdc - t[0].value) * (t[1].celsius - t[0].celsius) / (t[1].value - t[0].value);
            break;
        }        
    }
    // Overflow: We just clamp to 500 degrees celsius
    if (i == numTemps) {        
        current_celsius = 500;
    }
    return current_celsius;
}
