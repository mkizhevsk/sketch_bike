#include <Wire.h> 
#include <BMP085.h>
#include <avr/sleep.h>
#include <avr/power.h>

BMP085 dps = BMP085();    
 
long Temperature = 0, Pressure = 0;

int pres = 0;
float temper = 0;
float current = 0;
float currentSum = 0;
float displayedCurrentSum = 0;

int pressureCounter = 0;
int pressureCounterMax = 3;

float batteryVoltage = 0;
int batteryCounter = 0;
int batteryCounterMax = 10;
  
void setup() {
  Serial.begin(9600);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  Wire.begin(); 
  delay(1000);
  dps.init();  
}

void loop() {
  delay(1000);

  //Давление и температура
  if (pressureCounter == 1) {
    dps.getPressure(&Pressure);
    dps.getTemperature(&Temperature);
    pres = (Pressure/133.3);
    temper = Temperature*0.1;
  }

  //Батарея
  if (batteryCounter == 1) {
    int sensorBattery = analogRead(A6);
    batteryVoltage = 0.00488 * sensorBattery;
  }
 
  //Напряжение
  int sensorValue = analogRead(A0);
  float voltage = sensorValue*0.0296;

  //Ток
  int sensorCurrent = analogRead(A2);
  //Serial.println(sensorCurrent);
  if (sensorCurrent < 510) {
    current = (510 - sensorCurrent) * 0.067;
    currentSum = current/3.6 + currentSum;
  } else {
    current = 0;
  }
  displayedCurrentSum = currentSum/1000;

  Serial.print(batteryVoltage);
  Serial.print("/");
  Serial.print(pres);
  Serial.print("/");
  Serial.print(temper);
  Serial.print("/");
  Serial.print(voltage);
  Serial.print("/");
  Serial.print(current);
  Serial.print("/");
  Serial.println(displayedCurrentSum);
    
  if (pressureCounter == pressureCounterMax) {
    pressureCounter = 0;
  } else {
    pressureCounter++;
  }

  if (batteryCounter == batteryCounterMax) {
    batteryCounter = 0;
  } else {
    batteryCounter++;
  }
}


