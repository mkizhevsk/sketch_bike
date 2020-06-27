#include <LCD5110_Graph.h>
#include <Wire.h> 
#include <BMP085.h>
#include <avr/sleep.h>
#include <avr/power.h>

LCD5110 myGLCD(3,4,5,7,6);
extern uint8_t TinyFont[];
extern uint8_t SmallFont[];
extern uint8_t MediumNumbers[];

BMP085 dps = BMP085();    
 
long Temperature = 0, Pressure = 0;

long previousMillis = 0;

//int volt = 0;
//int prov = 0;
int pres = 0;
int contr = 65;  //яркость/контрастность
int pressureX = 687;
int key = 0;
//int dvavos = 28;
int x = 0;
int k = 0;
//int sensorCurrent = 0;
//int delaysum = 0;
int delayconst = 50;
float s = 0;
float v = 0;
float distancePerSecond = 0;
float xtotal = 0;
//float voltage = 0;
float previousPressure = 99000;
float alt = 0;
float temper = 0;
float current = 0;
float currentSum = 0;
boolean showTemperature = true;
boolean buttonOn = false;
int y = 0;
int middleValueY = 29;
int maxValueY = 34;
int minValueY = 24;
int rpm[7] = {0, 0, 0, 0, 0, 0, 0};
float currentAngle = 0;
float angle[5] = {0, 0, 0, 0, 0};
int pointY[31] = {middleValueY, middleValueY, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100};
int previousY = middleValueY;
  
void setup() {
  Serial.begin(9600);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  Wire.begin(); 
  delay(1000);
  dps.init();  
  
  myGLCD.InitLCD();
  myGLCD.setContrast(contr);
 
  pinMode(8, INPUT); //кнопка
  pinMode(2, INPUT); //геркон
}

void loop() {
  //previousMillis = millis();
  k++;
  myGLCD.clrScr();
  
  //Геркон и кнопка
  for (int i=0; i <960; i++) {
    if (digitalRead(2) == HIGH) {
      x++; //инкрементируем срабатывание геркона
      delay(delayconst);
      //delaysum = delaysum + delayconst; //0+50
      i = i + delayconst;
    } else {
      delay(1);
      if (y > 0) {
        y--;
      }
    }
    if ((!buttonOn) && (y == 0)) {
      if (digitalRead(8) == HIGH) {
        //Serial.println(i);
        showTemperature = (showTemperature == true) ? false : true;
        if (i > 800) {
          y = 960; //задержка в случае срабатывания в конце цикла
        } 
        buttonOn = true; 
        //myGLCD.enableSleep();
        //EnterSleep();
      } 
    } 
  }
  buttonOn = false; 
  //Serial.println(millis());
  //previousMillis = millis();

  //Батарея
  int sensorBattery = analogRead(A6);
  float batteryVoltage = 0.00488 * sensorBattery;
 
  //Давление, температура и высота
  if (k == 1) {
    //dvavos = 28;
    dps.getPressure(&Pressure);
    //опять ловим геркон
    if (digitalRead(2) == HIGH) {
      x++;
      delay(delayconst);
    }
    dps.getTemperature(&Temperature);
    pres = (Pressure/133.3);
    //Serial.print(Pressure);
    temper = Temperature*0.1;
    //сдвигаем точки кривой рельефа местности
    if (Pressure > (previousPressure + 8)) {  
      pointY[30] = previousY + 1;
      //Serial.print(" + ");
      //Serial.print(pointY[30]);
    } else if (Pressure < (previousPressure - 8)) {  
      pointY[30] = previousY - 1;
      //Serial.print(" - ");
      //Serial.print(pointY[30]);
    }
    if ((pointY[30] > maxValueY) || (pointY[30] < minValueY)) {
      pointY[30] = middleValueY;
    }
    for (int i=0; i<30; i++) {
      pointY[i] = pointY[i+1];
    }
    previousY = pointY[30];
    previousPressure = Pressure;
    //Serial.println("");
  }
  //прорисовываем рельеф местности
  for (int i = 0; i < 30; i++) {
    myGLCD.setPixel (45 + i, pointY[i]);
  }
  //myGLCD.drawLine (46,36,76,36);

  //давление
  myGLCD.drawLine (43, 4, 84, 4);
  myGLCD.setPixel (43, 3);
  myGLCD.setPixel (53, 3);
  myGLCD.setPixel (63, 3);
  myGLCD.setPixel (73, 3);
  myGLCD.setPixel (83, 3);
  myGLCD.drawRect (pres-pressureX, 0, pres+1-pressureX, 2);
  
  //температура
  myGLCD.setFont(SmallFont); 
  if (showTemperature == true) {
    myGLCD.printNumF (temper, 1, 59, 7);   // Температура в градусах Цельсия  
  } else {
    myGLCD.printNumF (batteryVoltage, 1, 59, 7);
  }
    
  //Напряжение
  int sensorValue = analogRead(A0);
  float voltage = sensorValue*0.0296;
  myGLCD.setFont(SmallFont);
  //voltage = 12,3;
  myGLCD.printNumF (voltage, 1, 0, 41);
  //int volt = (voltage*100-2200)/8.3;
  //volt = 0;

  //Ток
  int sensorCurrent = analogRead(A2);
  if (sensorCurrent > 512) {
    current = (sensorCurrent - 512) * 0.0586;
    currentSum = current/3.6 + currentSum;
  }
  myGLCD.setFont(SmallFont);
  //myGLCD.printNumI (sensorCurrent,35,9);
  myGLCD.drawLine (0,38,84,38);
  myGLCD.drawLine (29,38,29,48);
  myGLCD.printNumF (current,1,34,41);
  myGLCD.drawLine (54,46,58,42);
  myGLCD.printNumF (currentSum/1000,2,60,41);
  current = 0;

  //Скорость
  for (int i = 0; i < 6; i++) {
    rpm[6-i] = rpm[5-i];
  }
  rpm[0] = x;
  for (int i = 0; i < 7; i++) {
    xtotal = rpm[i] + xtotal;
  }
  v = xtotal * 1.0267;
  if (rpm[0] == 0 && rpm [1] == 0) {
    v = 0;
  }
  distancePerSecond = v/3600;
  s = s + distancePerSecond;
  //myGLCD.setFont(MediumNumbers);
  //v = 12;
  DrawNumbers(v);
  //myGLCD.printNumF (v, 0, 6, -1);
  myGLCD.setFont(TinyFont);
  //s = 23.7;
  myGLCD.printNumF (s, 1, 15, 24);
  x = 0;
  xtotal = 0;
    
  if (k == 3) {
    k = 0;
  }

  //myGLCD.fillScr();
  myGLCD.update();
  
}

void DrawNumbers(float currentSpeed) {
  int initXforFirst = 12;
  int intXforSecond = 24;
  int speed = currentSpeed;
  int firstDigit = speed / 10;
  float secondDigit = speed % 10;
  //myGLCD.setPixel (intXforSecond+2, 13);
  //myGLCD.drawLine (intXforSecond-10,0,intXforSecond-10,14); 
  
  if (firstDigit == 1) {
    myGLCD.drawLine (initXforFirst-1,1,initXforFirst-1,14);
    myGLCD.drawLine (initXforFirst,0,initXforFirst,14);
  }
  if (firstDigit == 2) {
    myGLCD.drawLine (initXforFirst-1,0,initXforFirst-8,0);
    myGLCD.drawLine (initXforFirst,1,initXforFirst-7,1);
    
    myGLCD.drawLine (initXforFirst-1,0,initXforFirst-1,8);
    myGLCD.drawLine (initXforFirst,1,initXforFirst,7);
    
    myGLCD.drawLine (initXforFirst,6,initXforFirst-7,6);
    myGLCD.drawLine (initXforFirst-1,7,initXforFirst-8,7);
    
    myGLCD.drawLine (initXforFirst-8,7,initXforFirst-8,12);
    myGLCD.drawLine (initXforFirst-7,6,initXforFirst-7,13);
    
    myGLCD.drawLine (initXforFirst-8,12,initXforFirst,12);
    myGLCD.drawLine (initXforFirst-7,13,initXforFirst+1,13);
  }
    if (firstDigit == 3) {
    myGLCD.drawLine (initXforFirst-1,0,initXforFirst-7,0);
    myGLCD.drawLine (initXforFirst,1,initXforFirst-8,1);
    
    myGLCD.drawLine (initXforFirst-1,0,initXforFirst-1,8);
    myGLCD.drawLine (initXforFirst,1,initXforFirst,7);
    
    myGLCD.drawLine (initXforFirst,6,initXforFirst-7,6);
    myGLCD.drawLine (initXforFirst-1,7,initXforFirst-7,7);
    
    myGLCD.drawLine (initXforFirst-1,7,initXforFirst-1,12);
    myGLCD.drawLine (initXforFirst,6,initXforFirst,13);
    
    myGLCD.drawLine (initXforFirst-8,12,initXforFirst,12);
    myGLCD.drawLine (initXforFirst-7,13,initXforFirst,13);
  }

   if (secondDigit == 0) {
    myGLCD.drawLine (intXforSecond-1,0,intXforSecond-7,0);
    myGLCD.drawLine (intXforSecond,1,intXforSecond-8,1);
    
    myGLCD.drawLine (intXforSecond-1,0,intXforSecond-1,13);
    myGLCD.drawLine (intXforSecond,1,intXforSecond,12);
        
    myGLCD.drawLine (intXforSecond-8,1,intXforSecond-8,12);
    myGLCD.drawLine (intXforSecond-7,0,intXforSecond-7,13);
    
    myGLCD.drawLine (intXforSecond+1,12,intXforSecond-8,12);
    myGLCD.drawLine (intXforSecond,13,intXforSecond-7,13);
  }
  if (secondDigit == 1) {
    myGLCD.drawLine (intXforSecond-1,1,intXforSecond-1,14);
    myGLCD.drawLine (intXforSecond,0,intXforSecond,14);
  }
  if (secondDigit == 2) {
    myGLCD.drawLine (intXforSecond-1,0,intXforSecond-8,0);
    myGLCD.drawLine (intXforSecond,1,intXforSecond-7,1);
    
    myGLCD.drawLine (intXforSecond-1,0,intXforSecond-1,8);
    myGLCD.drawLine (intXforSecond,1,intXforSecond,7);
    
    myGLCD.drawLine (intXforSecond,6,intXforSecond-7,6);
    myGLCD.drawLine (intXforSecond-1,7,intXforSecond-8,7);
    
    myGLCD.drawLine (intXforSecond-8,7,intXforSecond-8,12);
    myGLCD.drawLine (intXforSecond-7,6,intXforSecond-7,13);
    
    myGLCD.drawLine (intXforSecond-8,12,intXforSecond,12);
    myGLCD.drawLine (intXforSecond-7,13,intXforSecond+1,13);
  }
      if (secondDigit == 3) {
    myGLCD.drawLine (intXforSecond-1,0,intXforSecond-7,0);
    myGLCD.drawLine (intXforSecond,1,intXforSecond-8,1);
    
    myGLCD.drawLine (intXforSecond-1,0,intXforSecond-1,8);
    myGLCD.drawLine (intXforSecond,1,intXforSecond,7);
    
    myGLCD.drawLine (intXforSecond,6,intXforSecond-7,6);
    myGLCD.drawLine (intXforSecond-1,7,intXforSecond-7,7);
    
    myGLCD.drawLine (intXforSecond-1,7,intXforSecond-1,12);
    myGLCD.drawLine (intXforSecond,6,intXforSecond,13);
    
    myGLCD.drawLine (intXforSecond-8,12,intXforSecond,12);
    myGLCD.drawLine (intXforSecond-7,13,intXforSecond,13);
  }
  if (secondDigit == 4) {
    //myGLCD.drawLine (intXforSecond-1,0,intXforSecond-8,0);
    //myGLCD.drawLine (intXforSecond,1,intXforSecond-7,1);
    
    myGLCD.drawLine (intXforSecond-1,1,intXforSecond-1,13);
    myGLCD.drawLine (intXforSecond,0,intXforSecond,14);
    
    myGLCD.drawLine (intXforSecond,6,intXforSecond-8,6);
    myGLCD.drawLine (intXforSecond,7,intXforSecond-7,7);
    
    myGLCD.drawLine (intXforSecond-8,1,intXforSecond-8,6);
    myGLCD.drawLine (intXforSecond-7,0,intXforSecond-7,7);
    
    //myGLCD.drawLine (intXforSecond-8,12,intXforSecond,12);
    //myGLCD.drawLine (intXforSecond-7,13,intXforSecond+1,13);
  }
  if (secondDigit == 5) {
    myGLCD.drawLine (intXforSecond+1,0,intXforSecond-7,0);
    myGLCD.drawLine (intXforSecond,1,intXforSecond-8,1);
    
    myGLCD.drawLine (intXforSecond-8,1,intXforSecond-8,7);
    myGLCD.drawLine (intXforSecond-7,0,intXforSecond-7,8);
    
    myGLCD.drawLine (intXforSecond-8,6,intXforSecond,6);
    myGLCD.drawLine (intXforSecond-7,7,intXforSecond+1,7);
    
    myGLCD.drawLine (intXforSecond-1,7,intXforSecond-1,12);
    myGLCD.drawLine (intXforSecond,8,intXforSecond,13);
    
    myGLCD.drawLine (intXforSecond,12,intXforSecond-7,12);
    myGLCD.drawLine (intXforSecond-1,13,intXforSecond-8,13);
  }
  if (secondDigit == 6) {
    myGLCD.drawLine (intXforSecond+1,0,intXforSecond-7,0);
    myGLCD.drawLine (intXforSecond,1,intXforSecond-8,1);
    
    myGLCD.drawLine (intXforSecond-8,1,intXforSecond-8,13);
    myGLCD.drawLine (intXforSecond-7,0,intXforSecond-7,14);
    
    myGLCD.drawLine (intXforSecond-8,6,intXforSecond,6);
    myGLCD.drawLine (intXforSecond-7,7,intXforSecond+1,7);
    
    myGLCD.drawLine (intXforSecond-1,7,intXforSecond-1,12);
    myGLCD.drawLine (intXforSecond,8,intXforSecond,13);
    
    myGLCD.drawLine (intXforSecond,12,intXforSecond-7,12);
    myGLCD.drawLine (intXforSecond,13,intXforSecond-7,13);
  }
  if (secondDigit == 7) {
    myGLCD.drawLine (intXforSecond-1,0,intXforSecond-8,0);
    myGLCD.drawLine (intXforSecond,1,intXforSecond-7,1);
    
    myGLCD.drawLine (intXforSecond-1,0,intXforSecond-1,13);
    myGLCD.drawLine (intXforSecond,1,intXforSecond,14);
  }
  if (secondDigit == 8) {
    myGLCD.drawLine (intXforSecond,0,intXforSecond-7,0);
    myGLCD.drawLine (intXforSecond,1,intXforSecond-8,1);
    
    myGLCD.drawLine (intXforSecond-8,1,intXforSecond-8,13);
    myGLCD.drawLine (intXforSecond-7,0,intXforSecond-7,14);
    
    myGLCD.drawLine (intXforSecond-8,6,intXforSecond,6);
    myGLCD.drawLine (intXforSecond-7,7,intXforSecond+1,7);
    
    myGLCD.drawLine (intXforSecond-1,0,intXforSecond-1,14);
    myGLCD.drawLine (intXforSecond,1,intXforSecond,13);
    
    myGLCD.drawLine (intXforSecond,12,intXforSecond-7,12);
    myGLCD.drawLine (intXforSecond,13,intXforSecond-7,13);
  }
  if (secondDigit == 9) {
    myGLCD.drawLine (intXforSecond,0,intXforSecond-7,0);
    myGLCD.drawLine (intXforSecond,1,intXforSecond-8,1);
    
    myGLCD.drawLine (intXforSecond-8,1,intXforSecond-8,6);
    myGLCD.drawLine (intXforSecond-7,0,intXforSecond-7,7);
    
    myGLCD.drawLine (intXforSecond-8,6,intXforSecond,6);
    myGLCD.drawLine (intXforSecond-7,7,intXforSecond+1,7);
    
    myGLCD.drawLine (intXforSecond-1,0,intXforSecond-1,14);
    myGLCD.drawLine (intXforSecond,1,intXforSecond,13);
    
    myGLCD.drawLine (intXforSecond,12,intXforSecond-7,12);
    myGLCD.drawLine (intXforSecond,13,intXforSecond-8,13);
  } 
  //Serial.print(firstDigit);
  //Serial.print("  ");
  //Serial.println(secondDigit); 
}

//void EnterSleep() {
//  //attachInterrupt(0, wakeUp, LOW); //Если на 0-вом прерываниии - ноль, то просыпаемся.
//  delay(100);
//  sleep_enable(); //Разрешаем спящий режим
//  sleep_mode(); //Спим (Прерывания продолжают работать.) Программа останавливается.
//  sleep_disable(); //Запрещаем спящий режим
//}

