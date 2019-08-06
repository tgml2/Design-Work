#include "AirQuality.h"
#include "Arduino.h"
#include <Arduino.h>
#include "sensirion_common.h"
#include "sgp30.h"

AirQuality airqualitysensor;
int current_quality =-1;
void setup()
{
    Serial.begin(9600);
    airqualitysensor.init(A0);
}
void loop()
{
    current_quality=airqualitysensor.slope();
    if (current_quality >= 0)// if a valid data returned.
    {
        if (current_quality==0)
            Serial.println("High pollution! Force signal active");
        else if (current_quality==1)
            Serial.println("High pollution!");
        else if (current_quality==2)
            Serial.println("Low pollution!");
        else if (current_quality ==3)
            Serial.println("Fresh air");
    }
}
ISR(TIMER2_OVF_vect)
{
    if(airqualitysensor.counter==122)//set 2 seconds as a detected duty
    {
        airqualitysensor.last_vol=airqualitysensor.first_vol;
        airqualitysensor.first_vol=analogRead(A0);
        airqualitysensor.counter=0;
        airqualitysensor.timer_index=1;
        PORTB=PORTB^0x20;
    }
    else
    {
        airqualitysensor.counter++;
    }
}

//SGP30 setup
void setup() {
    s16 err;
    u16 scaled_ethanol_signal, scaled_h2_signal;
  Serial.begin(115200);
  Serial.println("serial start!!");

 /*For wio link!*/
  #if defined(ESP8266)
          pinMode(15,OUTPUT);
          digitalWrite(15,1);
          Serial.println("Set wio link power!");
          delay(500);
  #endif
  /*Init module,Reset all baseline,The initialization takes up to around 15 seconds, during which
all APIs measuring IAQ(Indoor air quality ) output will not change.Default value is 400(ppm) for co2,0(ppb) for tvoc*/
  while (sgp_probe() != STATUS_OK) {
         Serial.println("SGP failed");
         while(1);
    }
    /*Read H2 and Ethanol signal in the way of blocking*/
    err = sgp_measure_signals_blocking_read(&scaled_ethanol_signal,
                                            &scaled_h2_signal);
    if (err == STATUS_OK) {
        Serial.println("get ram signal!");
    } else {
         Serial.println("error reading signals"); 
    }
     err = sgp_iaq_init();
     //
}

void loop() {
  s16 err=0;
  u16 tvoc_ppb, co2_eq_ppm;
  err = sgp_measure_iaq_blocking_read(&tvoc_ppb, &co2_eq_ppm);
        if (err == STATUS_OK) {
             Serial.print("tVOC  Concentration:");
             Serial.print(tvoc_ppb);
             Serial.println("ppb");
             
              Serial.print("CO2eq Concentration:");
              Serial.print(co2_eq_ppm);
               Serial.println("ppm");
        } else {
             Serial.println("error reading IAQ values\n"); 
        }
  delay(1000);
}

//dust sensor

//#include "Dust_SensorLib.h"

const int Vout2 = A0;
const int Vout1 = A1;

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(9600);
  pinMode(Vout2, INPUT);
  pinMode(Vout1, INPUT);
}

// the loop function runs over and over again until power down or reset
void loop() {
  Serial.println(analogRead(Vout1));
  Serial.println(analogRead(Vout2));
  Serial.println("==============");
  delay(1000);
}

//LED ring
