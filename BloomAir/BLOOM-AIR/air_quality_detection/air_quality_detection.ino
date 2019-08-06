//Air Quality Sensor

#include "Air_Quality_Sensor.h"

const int AirQualitySensor::FORCE_SIGNAL   = 0;
const int AirQualitySensor::HIGH_POLLUTION = 1;
const int AirQualitySensor::LOW_POLLUTION  = 2;
const int AirQualitySensor::FRESH_AIR      = 3;

AirQualitySensor::AirQualitySensor(int pin) 
: _pin(pin), _voltageSum(0), _volSumCount(0) {
    // do nothing
}

bool AirQualitySensor::init(void) {
    int initVoltage = analogRead(_pin);

    if (10 < initVoltage && initVoltage < 798) {
        _currentVoltage = initVoltage;
        _lastVoltage = _currentVoltage;

        _standardVoltage = initVoltage;
        _lastStdVolUpdated = millis();

        return true;
    }
    else {
        return false;
    }
}

int AirQualitySensor::slope(void) {
    _lastVoltage = _currentVoltage;
    _currentVoltage = analogRead(_pin);

    _voltageSum += _currentVoltage;
    _volSumCount += 1;

    updateStandardVoltage();
    if (_currentVoltage - _lastVoltage > 400 || _currentVoltage > 700) {
        return AirQualitySensor::FORCE_SIGNAL;
    }
    else if ((_currentVoltage - _lastVoltage > 400 && _currentVoltage < 700)
             || _currentVoltage - _standardVoltage > 150) {
        return AirQualitySensor::HIGH_POLLUTION;
    }
    else if ((_currentVoltage - _lastVoltage > 200 && _currentVoltage < 700)
             || _currentVoltage - _standardVoltage > 50) {
        return AirQualitySensor::LOW_POLLUTION;
    }
    else {
        return AirQualitySensor::FRESH_AIR;
    }

    return -1;
}

int AirQualitySensor::getValue(void) {
    return _currentVoltage;
}

void AirQualitySensor::updateStandardVoltage(void) {
    if (millis() - _lastStdVolUpdated > 500000) {
        _standardVoltage = _voltageSum / _volSumCount;
        _lastStdVolUpdated = millis();

        _voltageSum = 0;
        _volSumCount = 0;
    }
}

//Dust Sensor

#include <math.h>
int pin = 8;
unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 1000;//sampe 1s ;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;
int gLed = 7;
int yLed = 6;
int rLed = 5;

void setup() {
  Serial.begin(9600);
  pinMode(8,INPUT);
  pinMode(gLed,OUTPUT);
  pinMode(yLed,OUTPUT);
  pinMode(rLed,OUTPUT);
  starttime = millis();//get the current time;
}

void loop() {
  duration = pulseIn(pin, LOW);
  lowpulseoccupancy = lowpulseoccupancy+duration;

  if ((millis()-starttime) > sampletime_ms)//if the sampel time == 30s
  {
    ratio = lowpulseoccupancy/(sampletime_ms*10.0);  // Integer percentage 0=>100
    concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62; // using spec sheet curve
    Serial.print("concentration = ");
    Serial.print(concentration);
    Serial.print(" pcs/0.01cf  -  ");
    if (concentration < 1.0) {
     Serial.println("It's a smokeless and dustless environment"); 
     digitalWrite(gLed, HIGH);
     digitalWrite(yLed, LOW);
     digitalWrite(rLed, LOW);
  }
    if (concentration > 1.0 && concentration < 20000) {
     Serial.println("It's probably only you blowing air to the sensor :)"); 
     digitalWrite(gLed, HIGH);
     digitalWrite(yLed, LOW);
     digitalWrite(rLed, LOW);
    }
    
    if (concentration > 20000 && concentration < 315000) {
     Serial.println("Smokes from matches detected!"); 
     digitalWrite(gLed, LOW);
     digitalWrite(yLed, HIGH);
     digitalWrite(rLed, LOW);
    }
      if (concentration > 315000) {
     Serial.println("Smokes from cigarettes detected! Or It might be a huge fire! Beware!"); 
     digitalWrite(gLed, LOW);
     digitalWrite(yLed, LOW);
     digitalWrite(rLed, HIGH);
  }
    
    lowpulseoccupancy = 0;
    starttime = millis();
  }
}

//-----------------------------------------------------------------------------------------------------------------

//Gas Sensor

#include "Particle.h"
 #include "Adafruit_SGP30.h"

 Adafruit_SGP30 sgp;

 void setup() {
   Serial.begin(9600);
   Serial.println("SGP30 test");

   if (! sgp.begin()){
     Serial.println("Sensor not found :(");
     while (1);
   }
   Serial.print("Found SGP30 serial #");
   Serial.print(sgp.serialnumber[0], HEX);
   Serial.print(sgp.serialnumber[1], HEX);
   Serial.println(sgp.serialnumber[2], HEX);

   // If you have a baseline measurement from before you can assign it to start, to 'self-calibrate'
   // sgp.setIAQBaseline(0x8E68, 0x8F41);  // Will vary for each sensor!
 }

 int counter = 0;
 void loop() {
   if (! sgp.IAQmeasure()) {
     Serial.println("Measurement failed");
     return;
   }
   Serial.print("TVOC "); Serial.print(sgp.TVOC); Serial.print(" ppb\t");
   Serial.print("eCO2 "); Serial.print(sgp.eCO2); Serial.println(" ppm");
   delay(1000);

   counter++;
   if (counter == 30) {
     counter = 0;

     uint16_t TVOC_base, eCO2_base;
     if (! sgp.getIAQBaseline(&eCO2_base, &TVOC_base)) {
       Serial.println("Failed to get baseline readings");
       return;
     }
     Serial.print("****Baseline values: eCO2: 0x"); Serial.print(eCO2_base, HEX);
     Serial.print(" & TVOC: 0x"); Serial.println(TVOC_base, HEX);
   }
 }
