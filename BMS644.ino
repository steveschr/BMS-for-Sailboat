#include "LiquidCrystal_I2C.h"
LiquidCrystal_I2C lcd(0x27, 20, 4);
#include "parameters.h"
#include <SPI.h>
#include <EEPROM.h>
#include "customChar.h"
#include "MCP3208.h"

MCP3208 adc0(adcU0);
MCP3208 adc1(adcU1);

void setup() {
  pinMode(encpin1, INPUT);  // Pin 2
  pinMode(encpin2, INPUT);  // Pin 3
  pinMode(adcU0, OUTPUT);
  pinMode(adcU1, OUTPUT);
  pinMode(encButton, INPUT);
  pinMode(ALARMpin, OUTPUT);  // AUX4 pin
  pinMode(AcChrge_OFF, OUTPUT);
  pinMode(AcChrge_ON, OUTPUT);
  pinMode(Solar_OFF, OUTPUT);
  pinMode(Solar_ON, OUTPUT);
  pinMode(Alt_OFF, OUTPUT);
  pinMode(Alt_ON, OUTPUT);
  pinMode(Load_ON, OUTPUT);
  pinMode(Load_OFF, OUTPUT);
  pinMode(AUXpin1, OUTPUT);
  pinMode(AUXpin2, OUTPUT);
  pinMode(AUXpin3, OUTPUT);
  pinMode(AUXpin5, OUTPUT);  
  analogReference(DEFAULT);
    for (byte j = 0; j <= 3; j++) {
      for (byte i = 0; i <= 3; i++) {  //Balance outputs to MOSFETs
    pinMode(balancePins[i], OUTPUT);
    digitalWrite(balancePins[i], HIGH);
    delay(100);
    digitalWrite(balancePins[i], LOW);
      }
    }

  digitalWrite(adcU0, HIGH);
  digitalWrite(adcU1, HIGH);
  lcd.init();
  lcd.backlight();
  SPI.beginTransaction(SPISettings(1000, MSBFIRST, SPI_MODE0));  //
  delay(50);
  adc0.begin();
  delay(50);
  adc1.begin();
  delay(50);
  attachInterrupt(0, decoder, FALLING);
  lcd.createChar(smileFace, smiley);
  lcd.createChar(lockedFace, locked);
  lcd.createChar(anchorFace, anchor);
  lcd.createChar(bellFace, bell);
  lcd.createChar(downArrowFace, downArrow);
  lcd.createChar(emptyBoxFace, emptyBox);
  lcd.createChar(upArrowFace, upArrow);
  lcd.createChar(emptyBellFace, emptyBell);
  float T = getTemps(0);  
  startScreen();  //also copies config from EEPROM
  delay(100);
  lcd.clear();
  startBoat();
  ChargeON = 1;
  stopCharge();
  //stopBoat();
  lockoutStartTime = millis();
  chargeLock = true;  
  for (int i = 0; i < numOfScreens; i++) {   //copy working array values to display array
  parameters[i][1] = parameters[i][0];
  }
}


void loop() {
  getAdcAmps();
  getamphours();
  encoder();
  if (encButtonPressed) {
    lcd.backlight();
    bool gns = getNewScreen();
    if (gns) {
      encoder();
      inputAction();
    }
    lcd.clear();
    backlightTimer = millis();
  }

  if (millis() - backlightTimer > (timeInterval * 180UL)) {
    lcd.noBacklight();
  }

  if (millis() - DISPpreviousMillis > (timeInterval * 2UL)) {
    DISPpreviousMillis = millis();
    getAdc();
    if (currentMode == "BMS") {
      logic();
    }
    doBalance();    
    LCDstatus();
    if (millis() - lockoutStartTime > 30000UL) {
      chargeLock = false;
    }
    if (millis() - lockoutStartTimeSYS > 30000UL) {
      AlarmFlagSYS = false;
      flagCountSYS = 0;
      //    alarmOff();   ??   <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<,
    }
  }
}

float getTemps(int req) {  //3 on mainboard to ADC
  int hiindex = 0;
  int loindex = 3;
  float maxtemp = -500;
  float mintemp = 500;
  float K, C, F;
  if (parameters[SCRnumTsense][0] > 0) {
    for (int i = 5; i <= (5 + parameters[SCRnumTsense][0]); i++) {  //channel 5,6,7 ADC1 = temp array positions 0,1,2 for batts T sense
      int t = adc1.analogRead(i);
      K = 1.00 / (invT0 + invBeta * (log(4095 / (float)t - 1.00)));
      C = K - 273.15;                  // convert to Celsius
      F = ((9.0 * C) / 5.00) + 32.00;  // convert to Fahrenheit
      tempSensor[i - 5] = F;
    }
  } 

  int t = adc1.analogRead(balanceTempSensor);
  K = 1.00 / (invT0 + invBeta * (log(4095 / (float)t - 1.00)));
  C = K - 273.15;                  // convert to Celsius
  F = ((9.0 * C) / 5.00) + 32.00;  // convert to Fahrenheit
  tempSensor[3] = F;


 if (parameters[SCRnumTsense][0] > 0){
  for (int i = 0; i < (parameters[SCRnumTsense][0]); i++) {  //get hi lo for batt T sensors
    if (tempSensor[i] > maxtemp) {
      maxtemp = tempSensor[i];
      hiindex = i;
    }
    if (tempSensor[i] < mintemp) {
      mintemp = tempSensor[i];
      loindex = i;
    }
  }
 }
 if (parameters[SCRnumTsense][0] == 1){
  maxtemp = tempSensor[0];
  hiindex = 0;
  mintemp = tempSensor[0];
  loindex = 0;  
 }


  if ((tempSensor[hiindex] >= 100.0) && (tempSensor[hiindex] - tempSensor[loindex] < 25)) {
    if (!AlarmFlagOT) {
      flagCountOT++;
      if (flagCountOT > 64) {
        AlarmFlagOT = true;
        alarm();
        writeeprom(3, 1);
      }
    }
  }

  if ((tempSensor[hiindex] >= 130.0) && (tempSensor[hiindex] - tempSensor[loindex] < 25)) {
    if (!AlarmFlagOTdisconnect) {
      flagCountOTdisconnect++;
      if (flagCountOTdisconnect > 64) {
        AlarmFlagOTdisconnect = true;
        alarm();
        writeeprom(3, 10);
      }
    }
  }

  if (AlarmFlagOT) {
    if (tempSensor[hiindex] < 95.0) {
      AlarmFlagOT = false;
      flagCountOT = 0;
    }
  }

  if ((tempSensor[hiindex] > 100.0) && (tempSensor[hiindex] - tempSensor[loindex] >= 25)) {  // if the sensors dont agree by a wide margin DONT disconnect the boat
    AlarmFlagOTdisconnect = false;
    if (!AlarmFlagSYS) {
      flagCountSYS++;
      if (flagCountSYS > 250) {
        AlarmFlagSYS = true;
        lockoutStartTime = millis();
        lockoutStartTimeSYS = millis();
        chargeLock = true;
        alarm();
        writeeprom(3, 11);
      }
    }
  }


  if (AlarmFlagOTdisconnect) {
    if (tempSensor[hiindex] <= 100.0) {
      AlarmFlagOTdisconnect = false;
      flagCountOTdisconnect = 0;
    }
  }


  if (tempSensor[loindex] < 36.0 && (abs(tempSensor[hiindex] - tempSensor[loindex]) >= 25)) {
    if (!AlarmFlagSYS) {
      flagCountSYS++;
      if (flagCountSYS > 8) {
        AlarmFlagSYS = true;
        lockoutStartTime = millis();
        lockoutStartTimeSYS = millis();
        alarm();
        writeeprom(3, 11);
      }
    }
  }

  if (tempSensor[loindex] < 36.0 && (abs(tempSensor[hiindex] - tempSensor[loindex]) < 25)) {
    if (currentMode == "BMS") {
      chargeLock = true;
      lockoutStartTime = millis();
    }
    if (!AlarmFlagUT) {
      flagCountUT++;
      if (flagCountUT > 64) {
        AlarmFlagUT = true;
        //writeeprom(3, 2);
      }
    }
  }

  if (AlarmFlagUT) {
    if (tempSensor[loindex] >= 39.0) {
      AlarmFlagUT = false;
      flagCountUT = 0;
    }
  }

  switch (req) {
    case 0:
      return loindex;
      break;

    case 1:
      return hiindex;
      break;

    case 2:
      return mintemp;
      break;

    case 3:
      return maxtemp;
      break;
  }
}
/*
  XX - 0
  OT - 1
  UT - 2
  OV - 3
  UV - 4
  DR - 5
  overflow 6
  disconnectLV - 7
  disconnectHV - 8
  OpenCell - 9
  disconnect OT -10
  system err - 11
  writeeprom(3, code);
*/



void logic() {  //add bank out of range,
  //<<<<<<<<<<<<<<<<<<<charging
  if (currentMode == "BMS" && !ChargeON && !balON) {  //if not charging AND not balancing
    if (!AlarmFlagOT && !AlarmFlagUT && !AlarmFlagOV && !AlarmFlagSYS) {  //Temp, Voltage & system integrity
      if (parameters[SCRchargeMode][0] == 0 ) {                                   //0 = Volt mode 
        if (cellV[minIndex] <= parameters[SCRstartChargeV][0] && cellV[maxIndex] <= parameters[SCRstopChargeV][0]) {  //if at LV trigger - charge
          startCharge();
        }
      }
      if (parameters[SCRchargeMode][0] == 1) {  //1 = SOC mode
        if (SOC <= parameters[SCRSOCstart][0] && cellV[maxIndex] < parameters[SCRstopChargeV][0]) {
          startCharge();
        }
      }
    }
  }

  if (ChargeON) {                             //if charging
    if (parameters[SCRchargeMode][0] == 0) {  //volt mode
      if (cellV[maxIndex] > parameters[SCRstopChargeV][0]) {
        stopCharge();
      }
    }
    if (parameters[SCRchargeMode][0] == 1) {          //1 = SOC mode
      if (SOC >= parameters[SCRSOCstop][0]  || cellV[maxIndex] > parameters[SCRstopChargeV][0]) {  //when we reach V or SOC target
        stopCharge();
      }
    }

    if (AlarmFlagOT || AlarmFlagUT || AlarmFlagOV || AlarmFlagSYS) {  //if T or OV or system stop charging
      stopCharge();
    }
  }

  // emergency disconnects
  if (boatON) {
    if (AlarmFlagOVdisconnect && packVoltage > 15.6) {  //Voltage too far out of range - disconnect load
      stopCharge();
      stopBoat();
    }
    if (AlarmFlagUVdisconnect && (packVoltage < 13.6)) {
      //stopCharge();
      stopBoat();
    }
    if (AlarmFlagOTdisconnect) {  //hi temp disconnect
      stopCharge();
      stopBoat();
    }
  }
  if (!boatON){      //restart boat if measurements fall from disconnect to alarm levels but leave major alarm on
   if ( !AlarmFlagOVdisconnect && !AlarmFlagOTdisconnect && !AlarmFlagUVdisconnect) {
     startBoat();
   }
  }
  // sanity checks
  if (cellV[minIndex] < 1.5 || cellV[maxIndex] > 4.1 || amps0 > 100.0 || amps0 < -250.0) {
    if (!AlarmFlagSYS) {
      flagCountSYS++;
      if (flagCountSYS > 64) {
        AlarmFlagSYS = true;
        alarm();
        lockoutStartTime = millis();
        lockoutStartTimeSYS = millis();
        writeeprom(3, 11);
      }
    }
  }
}

void getAdcAmps() {
  long ADCu00val = 0;  //bias
  long ADCu01val = 0;  //shunt
  adcVol0 = 0;
  adcVol1 = 0;                  //shunt
  for (byte j = 0; j < 200; j++) {      //read 200x & average to suppress noise of inverter
    ADCu00val += adc0.analogRead(0);  //bias
    ADCu01val += adc0.analogRead(1);  //shunt
  }
  ADCu00val /= 200;  //get average
  ADCu01val /= 200;
  ADCu01val -= ADCu00val;  //remove bias
  if (ADCu01val == 1 || ADCu01val == -1) {
    ADCu01val = 0;
  }
  adcVol0 = (float(ADCu00val) * vref0);
  adcVol1 = (float(ADCu01val) * vref0);  // 2.048v ref gives .5mV / step
  amps0 = (adcVol1 / 50.0);              //200A/50mV shunt is .25 mV/ Amp, x50 OPA so 12.5mV/A @ ADC
  amps0 /= shuntR;                       // I=V/R
  amps0 *= shuntPolarity;                //invert for shunt polarity if required.   Reverse  AMPS
  amps0 -= .04;                         //subtract for own usage
}

void getAdc() {
  minIndex = 0;
  maxIndex = 0;
  float maxAllowedDrift = parameters[SCRmaxDrift][0] / 1000.0;
  float minVoltage = 4.096;
  float maxVoltage = 0;
  int cellArray[4] = { 0, 0, 0, 0 };

  for (byte i = 0; i < 4; i++) {  //read cell V  4 cells, 5 loops through
    cellArray[i] = 0;
    for (byte j = 0; j < 5; j++) {  //5 times
      cellArray[i] += adc1.analogRead(i);
    }
    cellArray[i] = (cellArray[i] / 5);  //get average
    cellV[i] = (cellArray[i] * vref1);  //convert to V
  }

  cellV[1] /= cellRdiv1;
  cellV[1] -= cellV[0];
  //cellV[1] = abs(cellV[1]);

  cellV[2] /= cellRdiv2;
  cellV[2] -= cellV[0] + cellV[1];
  //cellV[2] = abs(cellV[2]);

  packVoltage = (cellV[3] * 13.319) / 3.319;    // grab ADC output of top cell here Vout = Vs x R2 / (R1 + R2)   >> Vs = ( Vo(r1+R2) ) / R2
  cellV[3] /= cellRdiv3;
  cellV[3] -= cellV[0] + cellV[1] + cellV[2];
  //cellV[3] = abs(cellV[3]);

  cellV[0] += (parameters[SCRC0Trim][0] / 1000.0);  //adjust cell Vs with trim values
  cellV[1] += (parameters[SCRC1Trim][0] / 1000.0);
  cellV[2] += (parameters[SCRC2Trim][0] / 1000.0);
  cellV[3] += (parameters[SCRC3Trim][0] / 1000.0);

  Total_Voltage = 0;
  for (byte i = 0; i < 4; i++) {  //add cells for totalV, 
    Total_Voltage += cellV[i];
  }
  
 

  // get hi lo, drift

  for (byte i = 0; i < 4; i++) {
    if (cellV[i] > maxVoltage) {
      maxVoltage = cellV[i];
      maxIndex = i;
    }
    if (cellV[i] < minVoltage) {
      minVoltage = cellV[i];
      minIndex = i;
    }
  }

  if (maxVoltage >= parameters[SCRHVAlarm][0]) {
    if (!AlarmFlagOV) {
      flagCountOV++;
      if (flagCountOV > 8) {
        AlarmFlagOV = true;
        alarm();
        writeeprom(3, 3);
      }
    }
  }

  if (AlarmFlagOV) {
    if (maxVoltage <= parameters[SCRHVAlarm][0] - .015) {
      AlarmFlagOV = false;
      flagCountOV = 0;
      alarmOff();
    }
  }

  if (maxVoltage >= parameters[SCRHVdisconnect][0]) {
    if (!AlarmFlagOVdisconnect) {
      flagCountOVdisconnect++;
      if (flagCountOVdisconnect > 16) {
        AlarmFlagOVdisconnect = true;
        alarm();
        writeeprom(3, 8);
      }
    }
  }

  if (AlarmFlagOVdisconnect) {
    if (maxVoltage <= parameters[SCRHVAlarm][0]) {
      flagCountOVdisconnect = 0;
      AlarmFlagOVdisconnect = false;
      alarmOff();
    }
  }



  if (minVoltage <= parameters[SCRLVAlarm][0]) {
    if (!AlarmFlagUV) {
      flagCountUV++;
      if (flagCountUV > 8) {
        AlarmFlagUV = true;
        alarm();
        writeeprom(3, 4);
      }
    }
  }

  if (AlarmFlagUV) {
    if (minVoltage >= parameters[SCRLVAlarm][0] + .005) {
      AlarmFlagUV = false;
      flagCountUV = 0;
      alarmOff();
    }
  }

  if (minVoltage <= parameters[SCRLVdisconnect][0]) {
    if (!AlarmFlagUVdisconnect) {
      flagCountUVdisconnect++;
      if (flagCountUVdisconnect > 16) {
        AlarmFlagUVdisconnect = true;
        alarm();
        writeeprom(3, 7);
      }
    }
  }

  if (AlarmFlagUVdisconnect) {
    if (minVoltage >= parameters[SCRLVdisconnect][0] + .02) {    
    //if (minVoltage >= parameters[SCRLVAlarm][0]) {
      flagCountUVdisconnect = 0;
      AlarmFlagUVdisconnect = false;
      alarmOff();
    }
  }
  
    if (abs(Total_Voltage - packVoltage) > 1.0) {
      if (!AlarmFlagOC) {
        AlarmFlagOC = true;
        alarm();
        writeeprom(3, 9);
      }
    }
  
  if (abs(maxVoltage - minVoltage) > 1.0) {
    if (!AlarmFlagSYS) {
      AlarmFlagSYS = true;
      chargeLock = true;
      lockoutStartTime = millis();
      lockoutStartTimeSYS = millis();
      alarm();
      writeeprom(3, 11);
    }
  }


  voltageDrift = maxVoltage - minVoltage;
  if (voltageDrift > maxAllowedDrift) {  //set the alarm if cells drift out of balance
    if (!AlarmFlagDrift) {
      flagCountDrift++;
     /* if (flagCountDrift > 4) {
        lockoutStartTime = millis();
        chargeLock = true;
      }
    */
      if (flagCountDrift > 16) {
        AlarmFlagDrift = true;  //NOK but let it be.  Set the alarm present indicator.  No major notifications.  Alow charging
        writeeprom(3, 5);
      }
    }
  }
  if (voltageDrift <= maxAllowedDrift - .01) {  //10mv hysterisis for canceling alarm
    AlarmFlagDrift = false;
    flagCountDrift = 0;
  }
}



void alarm() {
  digitalWrite(ALARMpin, HIGH);
  AlarmFlag = true;
}

void alarmOff() {
  digitalWrite(ALARMpin, LOW);
  AlarmFlag = false;
}



void startCharge() {
  if (!chargeLock) {
    if (!ChargeON && (currentMode == "BMS" || currentMode == "DUM")) {  //if not charging start charging
      lcd.clear();
      lcd.print("START CHARGE");

      digitalWrite(Solar_ON, HIGH);
      delay(RelayOnTime);
      digitalWrite(Solar_ON, LOW);
      delay(RelayOnTime);
      digitalWrite(Alt_ON, HIGH);
      delay(RelayOnTime);
      digitalWrite(Alt_ON, LOW);
      delay(RelayOnTime);
      digitalWrite(AcChrge_ON, HIGH);
      delay(RelayOnTime);
      digitalWrite(AcChrge_ON, LOW);
      delay(RelayOnTime);
      ChargeON = true;


      delay(1000);
    }
  }
}


void captureEndOfCharge(){            //add total AH added?  
  getAdc();
  endOfChargeData[0] = cellV[minIndex];
  endOfChargeData[1] = cellV[maxIndex];
  endOfChargeData[2] = amps0;
  endOfChargeData[3] = Total_Voltage;
  endOfChargeData[4] = packVoltage;
  endOfChargeData[5] = voltageDrift;
  endOfChargeMinCell = minIndex;
  endOfChargeMaxCell = maxIndex;
}


void stopCharge() {
  if (ChargeON) {  //if charging - stop charge
    lcd.clear();
    lcd.print("STOP CHARGE");
    captureEndOfCharge();
    digitalWrite(Solar_OFF, HIGH);
    delay(RelayOnTime);
    digitalWrite(Solar_OFF, LOW);
    delay(RelayOnTime);
    digitalWrite(Alt_OFF, HIGH);
    delay(RelayOnTime);
    digitalWrite(Alt_OFF, LOW);
    delay(RelayOnTime);
    digitalWrite(AcChrge_OFF, HIGH);
    delay(RelayOnTime);
    digitalWrite(AcChrge_OFF, LOW);
    delay(RelayOnTime);
    ChargeON = false;
    delay(1000);
    lockoutStartTime = millis();
    chargeLock = true;
    cycleCount += 1;
  }
}

void startBoat() {
  if (!boatON) {
    lcd.clear();
    lcd.print("START BOAT");
    digitalWrite(Load_ON, HIGH);
    delay(RelayOnTime);
    digitalWrite(Load_ON, LOW);  
    boatON = true;
    delay(1000);
  }
}


void stopBoat() {
  if (boatON) {
    lcd.clear();
    lcd.print("STOP BOAT");
    digitalWrite(Load_OFF, HIGH);
    delay(RelayOnTime);
    digitalWrite(Load_OFF, LOW);
    boatON = false;
    delay(1000);
  }
}





void getamphours() {  //  show amps charged at charge end?
  if (lastAmpTime > millis()) {
    lastAmpTime = millis();  //rollover.  skip this cycle
  }
  unsigned long elapsedTime = millis() - lastAmpTime;  // millis max value= 4,294,967,295.
  lastAmpTime = millis();
  float increment = (amps0 * elapsedTime);
  increment /= 1000;  // get seconds
  if (amps0 > 0) {    //apply efficiency
    increment *= (parameters[SCRefficiency][0] / 100.0);
  }
  ampSeconds += increment;  //accumulate
  /*
    full synch
     if v > 3.6 its fully charged??.
     if v > ? and tail current < .05 it's fully charged.
    3.475, 1%, 1 minute ??
  */

  ampHours = ampSeconds / 3600.0;
  bankAH = parameters[SCRbankCapacity][0] + ampHours;

  if (bankAH > parameters[SCRbankCapacity][0]) {  //never show greater than selected AH rating
    bankAH = parameters[SCRbankCapacity][0];
  }
  SOC = (bankAH / parameters[SCRbankCapacity][0]) * 100.0;
  if (SOC >= 100) {  //never show greater than 100% SOC
    SOC = 100;
    if (ampHours > 0) {  //never show greater positive AH
      ampHours = 0;
      ampSeconds = 0;
    }
  }
  if (SOC < 0) {  //never show less than 0% SOC
    SOC = 0;
  }
}


/*
  XX - 0
  OT - 1
  UT - 2
  OV - 3
  UV - 4
  DR - 5
  overflow 6
  disconnectLV - 7
  disconnectHV - 8
  OpenCell - 9
  disconnect OT -10
  system err - 11
  writeeprom(3, code);
*/


bool writeeprom(byte etask, int code) {  //use writeeprom(task, code)
  //  noInterrupts();
  bool epromWritten = 0;
  int eeAddress = 0;
  float f = 0;
  float T = 0;
  unsigned long E = 0;
  unsigned long CRC = 0;
  unsigned int elength = EEPROM.length();
  EEPROM.get(0, f);
  switch (etask) {
    case 1:  //write full eprom
      busy(anchorFace);
      EEPROM.put(eeAddress, 99.44);
      eeAddress += sizeof(float);
      for (int i = 0; i <= numOfScreens; i++) {
        EEPROM.put(eeAddress, (parameters[i][0]));
        eeAddress += sizeof(float);
      }
      E = eeprom_crc();
      EEPROM.put((elength - sizeof(long)), E);  //store CRC at end of eeprom
      delay(500);
      lcd.clear();
      break;

    case 2:  //read eprom , restore if available

      EEPROM.get((elength - sizeof(long)), E);  //retrieve stored CRC
      CRC = eeprom_crc();
      if (E == CRC) {
        if (f == 99.44) {
          epromWritten = 1;
          eeAddress = sizeof(float);
          for (int i = 0; i <= numOfScreens; i++) {
            EEPROM.get(eeAddress, (parameters[i][0]));
            eeAddress += sizeof(float);
          }
        }
      } else {
        epromWritten = 0;
        EEPROM.put(0, 0);
      }
      break;


    case 3:  //error history write

      alarmCount++;
      if (alarmCount >= 25) {
        alarmCount = 1;
        eeAddress = alarmHistoryPointer + sizeof(int);
        EEPROM.put(eeAddress, 6);  //record overflow
        alarmCount = 2;            //then record current alarm
      }
      //     delay(2000);
      eeAddress = alarmHistoryPointer;
      EEPROM.put(eeAddress, alarmCount);
      //     delay(200);
      eeAddress = alarmHistoryPointer + (alarmCount * sizeof(int));
      EEPROM.put(eeAddress, code);
      ///now write data at time of alarm -6 floats stored
      eeAddress = alarmDataPointer;
      EEPROM.put(eeAddress, Total_Voltage);
      eeAddress += sizeof(float);
      EEPROM.put(eeAddress, packVoltage);
      eeAddress += sizeof(float);
      EEPROM.put(eeAddress, cellV[minIndex]);
      eeAddress += sizeof(float);
      EEPROM.put(eeAddress, cellV[maxIndex]);
      eeAddress += sizeof(float);
      T = getTemps(2);
      EEPROM.put(eeAddress, T);
      eeAddress += sizeof(float);
      T = getTemps(3);
      EEPROM.put(eeAddress, T);
      eeAddress += sizeof(float);

      //now write flag status
      EEPROM.put(eeAddress, boatON);
      eeAddress += sizeof(bool);
      EEPROM.put(eeAddress, ChargeON);
      eeAddress += sizeof(bool);
      EEPROM.put(eeAddress, AlarmFlagOV);
      eeAddress += sizeof(bool);
      EEPROM.put(eeAddress, AlarmFlagUV);
      eeAddress += sizeof(bool);
      EEPROM.put(eeAddress, AlarmFlagOT);
      eeAddress += sizeof(bool);
      EEPROM.put(eeAddress, AlarmFlagUT);
      eeAddress += sizeof(bool);
      EEPROM.put(eeAddress, AlarmFlagOTdisconnect);
      eeAddress += sizeof(bool);
      EEPROM.put(eeAddress, AlarmFlagDrift);
      eeAddress += sizeof(bool);
      EEPROM.put(eeAddress, AlarmFlag);
      eeAddress += sizeof(bool);
      EEPROM.put(eeAddress, AlarmFlagOC);
      eeAddress += sizeof(bool);
      EEPROM.put(eeAddress, AlarmFlagSYS);
      eeAddress += sizeof(bool);
      EEPROM.put(eeAddress, balON);  //balance
      eeAddress += sizeof(bool);

      EEPROM.put(eeAddress, AlarmFlagOTdisconnect);  //OT diconnect
      eeAddress += sizeof(bool);
      EEPROM.put(eeAddress, AlarmFlagOVdisconnect);  //OV diconnect
      eeAddress += sizeof(bool);
      EEPROM.put(eeAddress, AlarmFlagUVdisconnect);  //UV disconnect
      eeAddress += sizeof(bool);
      // delay(20);
      break;

    case 4:  //was EEPROM written?

      if (f == 99.44) {
        epromWritten = 1;
      }
      break;
  }
  //  interrupts();
  return epromWritten;
}



unsigned long eeprom_crc(void) {

  const unsigned long crc_table[16] = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
  };

  unsigned long crc = ~0L;

  for (unsigned long index = 0; index < (alarmHistoryPointer - 2); ++index) {
    crc = crc_table[(crc ^ EEPROM[index]) & 0x0f] ^ (crc >> 4);
    crc = crc_table[(crc ^ (EEPROM[index] >> 4)) & 0x0f] ^ (crc >> 4);
    crc = ~crc;
  }
  return crc;
}



void taskManager() {
  switch (task) {
  /*    case 2:  //reset all params - full reset
      resetall(1);
      parameters[SCRresetAlrm][0] = 0;
      startBoat();
      break;
  */
    case 1:  //clear alarm count
      EEPROM.put(alarmHistoryPointer, 0);
      parameters[SCRresetAlrm][0] = 0;
      alarmOff();
      alarmCount = 0;
      //busy(anchorFace);
      //delay(500);
      lcd.clear();
      break;

    case 3:  //show all alarms using normal display update routine
      if (alarmCount >= 1) {
        //AlarmFlag = 1;
        dispStatusCycle = 99;
        LCDstatus();
        //AlarmFlag = 0;
      } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("NO ALARMS"));
        delay(1000);
        //briefMessage("NO ALARMS" , alarmCount);
      }
      parameters[currentScreen][0] = 0;
      break;
  /*
    case 4:
      resetFlags();
      resetall(2);
      break;

    case 5:
      resetall(2);  //reset LOD params before save
      writeeprom(1, 0);
      parameters[currentScreen][0] = 0;
      break;
  */
    case 6:  //set DUM mode
      currentMode = "DUM";
      resetFlags();
      startBoat();
      startCharge();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("No Protection Mode"));
      delay(4000);
      parameters[SCRmode][1] = 2;
      parameters[SCRmode][0] = 1;      //set to OFF on restart
      break;

    case 7: //set BMS mode
      currentMode = "BMS";
      parameters[SCRmode][1] = 0;
      parameters[SCRmode][0] = 0;
      startBoat();
      writeeprom(1, 0);
      break;

    case 8: //set OFF mode
      currentMode = "OFF";
      parameters[SCRmode][1] = 1;
      parameters[SCRmode][0] = 1;
      stopCharge();
      writeeprom(1, 0);      
      break;

  }
  task = 0;
}


void resetFlags() {

  AlarmFlagOV = false;            //error flag for OV set when alarm func is called  - no alarms are cleared
  AlarmFlagUV = false;            //error flag for UV set when alarm func is called
  AlarmFlagOT = false;            //hi temp
  AlarmFlagUT = false;            //low temp
  AlarmFlagOTdisconnect = false;  //hi temp disconnect  ?????
  AlarmFlagOVdisconnect = false;
  AlarmFlagUVdisconnect = false;
  AlarmFlagDrift = false;  //error flag for high cell  V spread, set when alarm func is called
  AlarmFlag = false;       // is any alarm set EXCEPT DRIFT, reflects pin status
  AlarmFlagOC = false;     //Open Cell - large diff between call summ and measured pack V
  AlarmFlagSYS = false;    //system err
  balON = false;
  chargeLock = false;
  flagCountOV = 0;
  flagCountUV = 0;
  flagCountOT = 0;
  flagCountUT = 0;
  flagCountOTdisconnect = 0;
  flagCountOVdisconnect = 0;
  flagCountUVdisconnect = 0;
  flagCountDrift = 0;
  flagCountSYS = 0;
}





