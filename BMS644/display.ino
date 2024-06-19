void printCells() {
  lcd.clear();
  lcd.setCursor(3, 0);
  // lcd.print(F("Cell Voltages"));
  getAdc();

  lcd.setCursor(0, 0);
  lcd.print(cellV[0], 3);
  lcd.print(F(" "));
  lcd.print(cellV[1], 3);
  
  if (cycleCount > 0){
    lcd.print(F(" "));
    lcd.print(F("Cyc:"));
    lcd.print(cycleCount);
  }
  lcd.setCursor(0, 1);
  lcd.print(cellV[2], 3);
  lcd.print(F(" "));
  lcd.print(cellV[3], 3);
  lcd.print(F("  D:"));
  //float dr = voltageDrift * 1000.0;
  lcd.print((voltageDrift * 1000.0), 0);

  lcd.setCursor(0, 2);
  lcd.print(F("C1:"));
  lcd.print(balanceCount[0]);
  lcd.print(F(" C2:"));
  lcd.print(balanceCount[1]);
  lcd.setCursor(0, 3);
  lcd.print(F("C3:"));
  lcd.print(balanceCount[2]);
  lcd.print(F(" C4:"));
  lcd.print(balanceCount[3]);
  delay(4000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("ShuntV: "));
  lcd.print(adcVol1, 4);  //measured shunt V
  lcd.setCursor(0, 2);
  lcd.print(F("Vref0 "));
  lcd.print(vref0, 4);
  lcd.setCursor(0, 1);
  lcd.print(F("BiasV "));
  lcd.print(adcVol0, 4);  //measured bias V
  lcd.setCursor(0, 3);
  lcd.print(F("Vref1 "));
  lcd.print(vref1, 4);
  delay(3000);

  if (cycleCount > 0){
    lcd.clear();              //show last charge data
    lcd.setCursor(4, 0);
    lcd.print(F("Last Charge"));

    lcd.setCursor(0, 1);
    lcd.print(endOfChargeMinCell);
    lcd.print(F("L="));
    lcd.print(endOfChargeData[0], 3);
    lcd.print(F(" "));
    lcd.print(endOfChargeMaxCell);
    lcd.print(F("H="));
    lcd.print(endOfChargeData[1], 3);

    lcd.setCursor(0, 2);
    lcd.print(F("A="));
    lcd.print(endOfChargeData[2], 1);
    lcd.print(F(" DR="));
    lcd.print((endOfChargeData[5] * 1000 ),0);

    lcd.setCursor(0, 3);
    lcd.print(F("PV="));
    lcd.print(endOfChargeData[4], 1);
    lcd.print(F(" TV="));
    lcd.print(endOfChargeData[3], 1);
    
    delay(5000);
  }
}

void printTemps() {
  float T = getTemps(0);
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print(F("Batt Temps"));
  lcd.setCursor(0, 1);
  for (int i = 0; i < (parameters[SCRnumTsense][0]); i++) {
    lcd.print(tempSensor[i], 0);
    lcd.write(0xDF);  //degree symbol
  }

  lcd.setCursor(3, 2);
  lcd.print(F("Balance Board"));
  lcd.setCursor(4, 3);
  lcd.print(tempSensor[3], 0);
  lcd.write(0xDF);
  delay(5000);
}




void clearLine(int row, int len) {  //pass in row to clear
  lcd.setCursor(0, row);
  for (int j = 0; j <= len; j++) {
    lcd.write(0x20);
    //lcd.print(F(" "));
  }
}


void printScreen() {
  lcd.clear();
  //if (screens[currentScreen][3] == currentMode || screens[currentScreen][3] == "ALL") {
    lcd.setCursor(0, 0);
    lcd.print(screens[currentScreen][0]);
    //lcd.write(0x20);
    //lcd.write(anchorFace);
    //lcd.print((menuTimer-millis()) / 1000 );
    //display cell voltages in cell trim screen
    if (currentScreen == SCRC0Trim || currentScreen == SCRC1Trim || currentScreen == SCRC2Trim || currentScreen == SCRC3Trim) {
      getAdc();  //update cell Vs
      lcd.write(0x20);
      lcd.write(0x20);
      lcd.print(cellV[currentScreen - SCRC0Trim], 3);  //dependant on position of cell trim in array.  position 13
    }

    lcd.setCursor(0, 1);
    if (screens[currentScreen][2] == "I") {
      lcd.print(int(parameters[currentScreen][1]));
    }  //print float or integer
    else {
      lcd.print(parameters[currentScreen][1], 3);
      lcd.write(0x20);
    }
    lcd.print(screens[currentScreen][1]);
  /*} else {
    lcd.setCursor(0, 0);
    lcd.print(F(" Not Available In"));
    lcd.setCursor(4, 1);
    //lcd.print("    ");
    lcd.print(currentMode);
    lcd.print(F(" Mode "));
    lcd.write(lockedFace);
  }
  */
}


void LCDstatus() {

  bool flag = 0;
  float T = getTemps(3);  //get highest temp of 3
  float f = 0;
  int eeAddress = 0;
  EEPROM.get(alarmHistoryPointer, alarmCount);
  int ptr = 0;

  switch (dispStatusCycle) {
      /*    case 0:                       //drift
      clearLine(2, 14);
      lcd.setCursor(0, 2);
      lcd.print(F("Drift: "));
      lcd.print(voltageDrift, 3);
      dispStatusCycle = 1;
      break;
      */
    case 0:
      if (balON) {
        clearLine(2, 14);  //if balancing, show cell
        lcd.setCursor(0, 2);
        lcd.print(F("BAL:"));
        lcd.print(balcell + 1);
        lcd.print(F(":"));
        lcd.print(tempSensor[3], 0);
        lcd.write(0xDF);  
      }
      //dispStatusCycle = 6;
      if (alarmCount > 0) {
        dispStatusCycle = 7;
      } else {
        dispStatusCycle = 0;
      }
      break;
      /*
    case 5:             //<<<<<<<show max & min cell V  
      clearLine(2, 14);
      lcd.setCursor(0, 2);
      lcd.print(cellV[maxIndex], 2);
      lcd.print(F(" "));
      lcd.print(cellV[minIndex], 2);
      dispStatusCycle = 6;
      break;


    case 6:                     //Show temps
      clearLine(2, 14);
      lcd.setCursor(0, 2);
      for (int i = 0; i < (parameters[SCRnumTsense][0]); i++) {
        lcd.print(tempSensor[i], 0);
        lcd.write(0xDF);  //degree symbol
      }
      if (alarmCount > 0 ) { 
      dispStatusCycle = 7;}
      else {
        dispStatusCycle = 0;}

      break;
      */

    case 7:
      if (alarmCount > 0) {  // show last alarm
        lcd.clear();
        eeAddress = alarmHistoryPointer + (alarmCount * sizeof(int));
        EEPROM.get(eeAddress, ptr);
        lcd.setCursor(0, 2);
        lcd.print(alarmCount);
        lcd.print(F(":"));
        char *chptr = (char *)pgm_read_word(&errCodePointers[ptr]);
        char buffer[21];  // must be large enough!
        strcpy_P(buffer, chptr);
        lcd.print(buffer);



        //         lcd.print(ptr);
        //         lcd.print(F(":"));
        //         lcd.print(errorCodes[11]);
        /*
          for (int i = 0; i < numberoferrors; i++)
          {
            lcd.clear();
          char * chptr = (char *) pgm_read_word (&errCodePointers [i]);
          char buffer [21]; // must be large enough!
          strcpy_P (buffer, chptr);
          lcd.print(buffer);
          delay(1000);
          }

        */
      }
      dispStatusCycle = 0;
      break;


    case 99:                                          //alarmHistory
      eeAddress = alarmHistoryPointer + sizeof(int);  //show all alarms in history
      for (int i = 1; i <= alarmCount; i++) {
        EEPROM.get(eeAddress, ptr);
        lcd.setCursor(8, 0);
        lcd.clear();
        lcd.print(alarmCount);
        lcd.setCursor(0, 2);
        lcd.print(i);
        lcd.print(F(":"));
        lcd.print(ptr);
        lcd.print(F(":"));
        //          lcd.print(errorCodes[ptr]);
        char *chptr = (char *)pgm_read_word(&errCodePointers[ptr]);
        char buffer[21];  // must be large enough!
        strcpy_P(buffer, chptr);
        lcd.print(buffer);

        eeAddress += sizeof(int);
        encoder();
        if (encButtonPressed) {  //allow escape from here
          dispStatusCycle = 100;
          break;
        }
        delay(1500);
      }
      dispStatusCycle = 100;
      break;

    case 100:
      lcd.clear();
      lcd.setCursor(0, 0);
      f = 0;
      eeAddress = alarmDataPointer;  // data at time of alarm -6 floats stored
      EEPROM.get(eeAddress, f);      //total Voltage
      lcd.print(F("Tv:"));
      lcd.print(f, 1);
      eeAddress += sizeof(float);
      EEPROM.get(eeAddress, f);  //packVoltage
      lcd.print(F("  Pv:"));
      lcd.print(f, 1);

      lcd.setCursor(0, 1);
      eeAddress += sizeof(float);
      EEPROM.get(eeAddress, f);  //cellV[minIndex]
      lcd.print(F("Cmin:"));
      lcd.print(f, 2);
      eeAddress += sizeof(float);
      EEPROM.get(eeAddress, f);  //cellV[maxIndex]
      lcd.print(F("  Cmax:"));
      lcd.print(f, 2);

      lcd.setCursor(0, 2);
      eeAddress += sizeof(float);
      EEPROM.get(eeAddress, f);  //low temp
      eeAddress += sizeof(float);
      lcd.print(F("LoT:"));
      lcd.print(f, 0);
      EEPROM.get(eeAddress, f);  //hi temp
      lcd.print(F("  HiT:"));
      lcd.print(f, 0);
      delay(10000);

      lcd.clear();          //Now show active flags at time of recoring err
      lcd.setCursor(0, 0);  //3 per line, 1 page,
      eeAddress += sizeof(float);

      EEPROM.get(eeAddress, flag);  //boat on
      if (flag) {
        lcd.print(F("BoatON "));
      }
      eeAddress += sizeof(bool);

      EEPROM.get(eeAddress, flag);  //charging
      if (flag) {
        lcd.print(F("CHGON "));
      }
      eeAddress += sizeof(bool);
      EEPROM.get(eeAddress, flag);  //AlarmFlagOV
      if (flag) {
        lcd.print(F("OvrV"));
      }
      eeAddress += sizeof(bool);

      lcd.setCursor(0, 1);
      EEPROM.get(eeAddress, flag);  //AlarmFlagUV
      if (flag) {
        lcd.print(F("UndrV "));
      }
      eeAddress += sizeof(bool);

      EEPROM.get(eeAddress, flag);  //AlarmFlagOT
      if (flag) {
        lcd.print(F("T Hi  "));
      }
      eeAddress += sizeof(bool);

      EEPROM.get(eeAddress, flag);  //AlarmFlagUT
      if (flag) {
        lcd.print(F("T Low "));
      }
      eeAddress += sizeof(bool);

      lcd.setCursor(0, 2);
      EEPROM.get(eeAddress, flag);  //AlarmFlagOTdisconnect
      if (flag) {
        lcd.print(F("OTDis "));
      }
      eeAddress += sizeof(bool);

      EEPROM.get(eeAddress, flag);  //AlarmFlagDrift
      if (flag) {
        lcd.print(F("DRIFT "));
      }
      eeAddress += sizeof(bool);

      EEPROM.get(eeAddress, flag);  //AlarmFlag
      if (flag) {
        lcd.print(F("AlarmF"));
      }

      lcd.setCursor(0, 3);
      eeAddress += sizeof(bool);
      EEPROM.get(eeAddress, flag);  //AlarmFlagOC
      if (flag) {
        lcd.print(F("OpnCel "));
      }
      eeAddress += sizeof(bool);

      EEPROM.get(eeAddress, flag);  //AlarmFlagSYS
      if (flag) {
        lcd.print(F("SYSTEM "));
      }
      eeAddress += sizeof(bool);

      EEPROM.get(eeAddress, flag);  //balON
      if (flag) {
        lcd.print(F("BalON"));
      }
      eeAddress += sizeof(bool);

      delay(10000);
      lcd.clear();
      EEPROM.get(eeAddress, flag);  //OT Disconnect
      if (flag) {
        lcd.print(F("OT Dis "));
      }
      eeAddress += sizeof(bool);

      EEPROM.get(eeAddress, flag);  //OV Disconnect
      if (flag) {
        lcd.print(F("OV Dis "));
      }
      eeAddress += sizeof(bool);

      EEPROM.get(eeAddress, flag);  //UV diconnect
      if (flag) {
        lcd.print(F("UV Dis"));
      }
      lcd.setCursor(10, 3);
      lcd.print(F("Wait..."));

      delay(5000);
      dispStatusCycle = 0;
      break;
  }



  clearLine(3, 19);  //AMPS & AH
  lcd.setCursor(0, 3);
  lcd.print(F("A:"));
  lcd.print(amps0, 1);
  lcd.setCursor(10, 3);
  lcd.print(F("AH:"));
  lcd.print(ampHours, 1);

  if (currentMode == "BMS") {
    if (parameters[SCRchargeMode][0] == 1) {  //SOC charge mode
      if (ChargeON) {
        lcd.setCursor(0, 1);
        lcd.print(F("To:"));
        lcd.print(parameters[SCRstopChargeV][0], 2);
        lcd.print(F(" OR"));
        lcd.setCursor(10, 1);
        lcd.write(0x20);
        lcd.write(0x20);
        lcd.write(0x20);
        lcd.write(0x20);
        lcd.write(0x20);
        //lcd.print(F("     "));
        lcd.setCursor(10, 1);
        lcd.print(parameters[SCRSOCstop][0], 0);
        lcd.write(0x25);
        lcd.write(0x20);
        lcd.write(upArrowFace);
      } else {
        lcd.setCursor(0, 1);
        lcd.print(cellV[minIndex], 2);
        lcd.write(downArrowFace);
        //lcd.print(parameters[SCRstartChargeV][0] , 2);
        //lcd.print(F(" OR"));
        lcd.setCursor(10, 1);
        lcd.write(0x20);
        lcd.write(0x20);
        lcd.write(0x20);
        lcd.write(0x20);
        lcd.write(0x20);
        //lcd.print(F("     "));
        lcd.setCursor(10, 1);
        lcd.print(parameters[SCRSOCstart][0], 0);
        lcd.write(0x25);
        lcd.write(0x20);
        lcd.write(downArrowFace);
      }
    }
    if (parameters[SCRchargeMode][0] == 0 || parameters[SCRchargeMode][0] == 2) {  // VOL charge mode
      if (ChargeON) {
        lcd.setCursor(0, 1);
        lcd.print(F("    "));
        lcd.setCursor(0, 1);
        lcd.print(cellV[maxIndex], 2);
        lcd.write(0x20);
        lcd.write(upArrowFace);
        lcd.write(0x20);
        lcd.setCursor(9, 1);
        lcd.print(F("    "));
        lcd.setCursor(9, 1);
        lcd.print(parameters[SCRstopChargeV][0], 2);
      } else {
        lcd.setCursor(0, 1);
        lcd.print(F("    "));
        lcd.setCursor(0, 1);
        lcd.print(cellV[minIndex], 2);
        lcd.write(0x20);
        lcd.write(downArrowFace);
        lcd.setCursor(9, 1);
        lcd.print(F("    "));
        lcd.setCursor(9, 1);
        lcd.print(parameters[SCRstartChargeV][0], 2);
      }
    }
  }
  lcd.setCursor(16, 1);  //SOC
  lcd.print(F("    "));
  lcd.setCursor(16, 1);
  lcd.print(SOC, 0);
  lcd.print(F("%"));

  lcd.setCursor(15, 2);  //pack V   
  //lcd.print(Total_Voltage, 1);  
  lcd.print(packVoltage, 1);
  lcd.print(F("V"));

  clearLine(0, 19);

  if (parameters[SCRnumTsense][0] > 0) {  //if temp sense present
    lcd.setCursor(0, 0);
    lcd.print(T, 0);  //show higest temp of 3 sensors
    lcd.write(0xDF);
  }

  if (chargeLock) {
    lcd.setCursor(4, 0);
    lcd.write(lockedFace);
  }


  if (AlarmFlag) {  //major alarm
    lcd.setCursor(6, 0);
    lcd.write(bellFace);
    lcd.write(bellFace);
    flashlcd();
    //audibleAlarm();
    DISPpreviousMillis += 1000;
  }

  bool getestat = writeeprom(4, 0);  //EEPROM available?
  if (!getestat) {
    lcd.setCursor(8, 0);
    lcd.write(emptyBoxFace);
  }

  lcd.setCursor(9, 0);
  if (alarmCount > 0) {
    lcd.print(alarmCount);  //alarm count & bell
    if (AlarmFlagOV || AlarmFlagUV || AlarmFlagOT || AlarmFlagUT || AlarmFlagDrift
        || AlarmFlagOC || AlarmFlagOTdisconnect || AlarmFlagSYS) {
      lcd.write(bellFace);
    } else {
      lcd.write(emptyBellFace);
    }
  } else {
    lcd.write(smileFace);
  }

  lcd.setCursor(13, 0);
  if (currentMode == "BMS") {
    if (parameters[SCRchargeMode][0] == 0) {  //show charge mode
      lcd.print(F("VOL"));
    }
    if (parameters[SCRchargeMode][0] == 1) {
      lcd.print(F("SOC "));
    }
  }

  lcd.setCursor(17, 0);
  lcd.print(currentMode);

  //showMem();

  if (currentMode == "DUM") {
    lcd.setCursor(0, 1);
    lcd.print(F("    "));
    lcd.setCursor(0, 1);
    lcd.print(cellV[maxIndex], 2);
  }
}


void flashlcd() {
  for (int i = 0; i < 4; i++) {
    digitalWrite(ALARMpin, HIGH);
    delay(250);
    lcd.noBacklight();
    delay(75);
    lcd.backlight();
    digitalWrite(ALARMpin, LOW);
  }
}

void audibleAlarm() {
  for (int i = 0; i < 5; i++) {
    digitalWrite(ALARMpin, HIGH);
    delay(1000);
    digitalWrite(ALARMpin, LOW);
    delay(1000);
  }
}



void decoder() {
  //Remember that the routine is only called when pin1
  //changes state, so it's the value of pin2 that we're
  //interrested in here

  if (parameters[SCRencoderDir][0] == 0) {
    if (digitalRead(encpin1) != digitalRead(encpin2)) {
      goingUp = 1;  //if encoder channels are the same, direction is CW
    } else {
      goingDown = 1;  //if they are not the same, direction is CCW
    }
  } else {
    if (digitalRead(encpin1) == digitalRead(encpin2)) {
      goingUp = 1;  //if encoder channels are the same, direction is CW
    } else {
      goingDown = 1;  //if they are not the same, direction is CCW
    }
  }
}


void encoder() {
  while (goingUp == 1)  // CW motion in the rotary encoder
  {
    goingUp = 0;  // Reset the flag
    encChange = 1;
    encVal++;
  }
  while (goingDown == 1)  // CCW motion in rotary encoder
  {
    goingDown = 0;  // clear the flag
    encChange = 1;
    encVal--;
    // if ( encVal <0){ encVal = 0;}
  }
  encButtonPressed = (!digitalRead(encButton));
}


void getCurrentScreen() {
  currentScreen = encVal;
  if (currentScreen >= (numOfScreens - 1)) {
    currentScreen = (numOfScreens - 1);
  }
  if (currentScreen < 0) {
    currentScreen = 0;
  }
  encVal = currentScreen;
}



void cleanDisplay() {
  for (int i = 0; i < 2; i++) {
  if (parameters[SCRstartChargeV][i] < 2.5) parameters[SCRstartChargeV][i] = 2.5;
  if (parameters[SCRstopChargeV][i] < parameters[SCRstartChargeV][i] + .15) parameters[SCRstopChargeV][i] = parameters[SCRstartChargeV][i] + .15;
  if (parameters[SCRHVAlarm][i] < 3.0) parameters[SCRHVAlarm][i] = 3.0;
  if (parameters[SCRLVAlarm][i] < 2.8) parameters[SCRLVAlarm][i] = 2.8;
  if (parameters[SCRHVdisconnect][i] < 3.6) parameters[SCRHVdisconnect][i] = 3.6;
  if (parameters[SCRLVdisconnect][i] < 2.4) parameters[SCRLVdisconnect][i] = 2.4;
  if (parameters[SCRmaxDrift][i] < 0) parameters[SCRmaxDrift][i] = 0;
  if (parameters[SCRC0Trim][i] < -500) parameters[SCRC0Trim][i] = -500;
  if (parameters[SCRC1Trim][i] < -500) parameters[SCRC1Trim][i] = -500;
  if (parameters[SCRC2Trim][i] < -500) parameters[SCRC2Trim][i] = -500;
  if (parameters[SCRC3Trim][i] < -500) parameters[SCRC3Trim][i] = -500;
  if (parameters[SCRshowCells][i] < 0) parameters[SCRshowCells][i] = 0; 
  if (parameters[SCRbalanceV][i] < 3.350) parameters[SCRbalanceV][i] = 3.350;
  if (parameters[SCRbankCapacity][i] < 1) parameters[SCRbankCapacity][i] = 0;
  if (parameters[SCRresetAlrm][i] < 0) parameters[SCRresetAlrm][i] = 0;
  if (parameters[SCRmode][i] < 0) parameters[SCRmode][i] = 0;
  if (parameters[SCRSOCstart][i] < 5) parameters[SCRSOCstart][i] = 5;
  if (parameters[SCRSOCstop][i] < parameters[SCRSOCstart][i] + 3) parameters[SCRSOCstop][i] = parameters[SCRSOCstart][i] + 3;
  if (parameters[SCRchargeToFullNow][i] < 0) parameters[SCRchargeToFullNow][i] = 0;
  if (parameters[SCRshowErrors][i] < 0) parameters[SCRshowErrors][i] = 0;
  if (parameters[SCRefficiency][i] < 0) parameters[SCRefficiency][i] = 0;
  if (parameters[SCRencoderDir][i] < 0) parameters[SCRencoderDir][i] = 0;
  if (parameters[SCRchargeMode][i] < 0) parameters[SCRchargeMode][i] = 0;
  if (parameters[SCRshowTemps][i] < 0) parameters[SCRshowTemps][i] = 0;
  if (parameters[SCRshuntPol][i] < -1) parameters[SCRshuntPol][i] = -1;
  if (parameters[SCRnumTsense][i] < 0) parameters[SCRnumTsense][i] = 0;


  if (parameters[SCRstartChargeV][i] > 3.5 ) parameters[SCRstartChargeV][i] = 3.5;
  if (parameters[SCRstartChargeV][i] > parameters[SCRstopChargeV][i] - .15 ) parameters[SCRstartChargeV][i] = parameters[SCRstopChargeV][i] - .15;
  if (parameters[SCRstopChargeV][i] > 3.65) parameters[SCRstopChargeV][i] = 3.65;
  if (parameters[SCRHVAlarm][i] > 4.0) parameters[SCRHVAlarm][i] = 4.0;
  if (parameters[SCRLVAlarm][i] > 4.0) parameters[SCRLVAlarm][i] = 4.0;
  if (parameters[SCRHVdisconnect][i] > 4.5) parameters[SCRHVdisconnect][i] = 4.5;
  if (parameters[SCRLVdisconnect][i] > 3.4) parameters[SCRLVdisconnect][i] = 3.4;
  if (parameters[SCRLVdisconnect][i] > parameters[SCRLVAlarm][i] ) parameters[SCRLVAlarm][i] = parameters[SCRLVdisconnect][i] + .01; //<<<<
  if (parameters[SCRmaxDrift][i] > 500) parameters[SCRmaxDrift][i] = 500;
  if (parameters[SCRC0Trim][i] > 500) parameters[SCRC0Trim][i] = 500;
  if (parameters[SCRC1Trim][i] > 500) parameters[SCRC1Trim][i] = 500;
  if (parameters[SCRC2Trim][i] > 500) parameters[SCRC2Trim][i] = 500;
  if (parameters[SCRC3Trim][i] > 500) parameters[SCRC3Trim][i] = 500;
  if (parameters[SCRshowCells][i] > 1) parameters[SCRshowCells][i] = 1;
  if (parameters[SCRbalanceV][i] > 3.650) parameters[SCRbalanceV][i] = 3.650;
  if (parameters[SCRbankCapacity][i] > 2000) parameters[SCRbankCapacity][i] = 2000;
  if (parameters[SCRresetAlrm][i] > 1) parameters[SCRresetAlrm][i] = 1;
  if (parameters[SCRmode][i] > 2) parameters[SCRmode][i] = 2;
  if (parameters[SCRSOCstart][i] > 95) parameters[SCRSOCstart][i] = 95;
  if (parameters[SCRSOCstop][i] > 99) parameters[SCRSOCstop][i] = 99;
  if (parameters[SCRshowTemps][i] > 1) parameters[SCRshowTemps][i] = 1;
  if (parameters[SCRchargeToFullNow][i] > 1) parameters[SCRchargeToFullNow][i] = 1;
  if (parameters[SCRshowErrors][i] > 1) parameters[SCRshowErrors][i] = 1;
  if (parameters[SCRefficiency][i] > 100) parameters[SCRefficiency][i] = 100;
  if (parameters[SCRencoderDir][i] > 1) parameters[SCRencoderDir][i] = 1;
  if (parameters[SCRchargeMode][i] > 1) parameters[SCRchargeMode][i] = 1;
  if (parameters[SCRsoc][i] > parameters[SCRbankCapacity][i]) parameters[SCRsoc][i] = parameters[SCRbankCapacity][i];
  if (parameters[SCRshuntPol][i] > 1) parameters[SCRshuntPol][i] = 1;
  if (parameters[SCRnumTsense][i] > 3) parameters[SCRnumTsense][i] = 3;
  }
}

void startScreen() {

  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print(F("Battery  Manager"));
  lcd.setCursor(4, 1);
  lcd.print(F("S/V Skylark"));
  lcd.setCursor(5, 2);
  lcd.print(F("v1.2.1(b)"));
  lcd.setCursor(4, 3);
  lcd.print(F("HW 1.0.2R(b)"));

  for (int j = 0; j < 3; j++) {
    for (int i = 0; i <= 2; i++) {
      lcd.setCursor(0, i);
      lcd.write(anchorFace);
      delay(100);
      lcd.setCursor(0, i);
      lcd.print(F(" "));
    }
    lcd.setCursor(0, 3);
    for (int i = 0; i <= 19; i++) {
      lcd.setCursor(i, 3);
      lcd.write(anchorFace);
      delay(100);
      lcd.setCursor(i, 3);
      lcd.print(F(" "));
      lcd.setCursor(4, 3);
      lcd.print(F("HW 1.0.2R(b)"));
    }
    for (int i = 2; i >= 0; i--) {
      lcd.setCursor(19, i);
      lcd.write(anchorFace);
      delay(100);
      lcd.setCursor(19, i);
      lcd.print(F(" "));
    }
  }

  bool getestat = writeeprom(2, 0);  //restore settings if available
  lcd.clear();
  lcd.setCursor(0, 2);
  if (getestat) {
    lcd.print(F("EEPROM OK "));
    lcd.write(emptyBellFace);
  } else {
    lcd.print(F("EEPROM NA or CORRUPT "));
  }
  delay(2000);
  EEPROM.get(alarmHistoryPointer, alarmCount);
  if (alarmCount > 0) {
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print(F("ALARM HISTORY"));
    lcd.setCursor(4, 1);
    lcd.print(alarmCount);
    lcd.print(F("  EXIST"));
    lcd.setCursor(0, 2);
    lcd.print(F("Check & Clear!"));
    delay(2000);
  }
  parameters[SCRsoc][0] = bankAH;
  shuntPolarity = parameters[SCRshuntPol][0];

  if (parameters[SCRmode][0] == 0) {
    currentMode = "BMS";
  }
  if (parameters[SCRmode][0] == 1) {
    currentMode = "OFF";
  }
  if (parameters[SCRmode][0] == 2) {
    currentMode = "DUM";
  }

  getAdc();
  getAdcAmps();
  getamphours();
  printCells();
  printTemps();
}


bool getNewScreen() {
  bool didsomething = 0;
  menuTimer = millis() + 2000UL;
  //menuTimer += timeInterval;
  //  encVal = 0;
  encChange = 0;
  cleanDisplay();
  printScreen();
  do {
    encoder();
    if (encChange) {
      encChange = 0;
      cleanDisplay();
      getCurrentScreen();
      printScreen();
      menuTimer = millis() + timeInterval * 2;
      didsomething = 1;
    }
  /*    
    lcd.setCursor(0, 2);  //prints previous & next menu item
    cleanDisplay();
    byte scrl = currentScreen;
    byte scrh = currentScreen;
    if ((scrl - 1) <= 0) scrl = 1;
    if (scrh >= numOfScreens - 1) {
      scrh = numOfScreens - 2;
    }
    lcd.print(F("> "));
    lcd.print(screens[scrh + 1][0]);
    lcd.setCursor(0, 3);
    lcd.print(F("< "));
    lcd.print(screens[scrl - 1][0]);
  */
  } while (menuTimer > millis());
  return didsomething;
  lcd.clear();
}


void busy(byte icon) {
  lcd.clear();
  lcd.setCursor(6, 0);
  lcd.print(F("Writing"));
  for (int i = 5; i <= 15; i++) {
    lcd.setCursor(i, 1);
    lcd.write(icon);
  }
}
