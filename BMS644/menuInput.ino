
void inputAction() {
  int tempCurrentScreen = currentScreen;  //keep menu location
  menuTimer = millis();
  menuTimer += 3000;
  getCurrentScreen();
  encVal = 0;
  //encButtonPressed = 0;
  cleanDisplay();
  printScreen();
  bool dowewrite = 0;

  do {
    encVal = 0;
    encoder();
    if (encChange) {
      float temp = encVal;
      encButtonPressed = (!digitalRead(encButton));     
      switch (currentScreen) {    //show & modify display array.  If EncButton pressed, copy display to working & eprom


        case SCRbankCapacity:  //bank capacity              <OK>
            parameters[currentScreen][1] += temp * 5 ;                                 //(temp * 5);
            dowewrite = 1;
          break;
        
        case SCRstartChargeV:  //Start Charge         
            parameters[currentScreen][1] += (temp / 100); // 1000 / 100 = 10mv / step
            dowewrite = 1;  
          break;

        case SCRstopChargeV:  //Stop Charge
            parameters[currentScreen][1] += (temp / 100);
            dowewrite = 1;
          break;

        case SCRSOCstart:  //SOC start charge             
            parameters[currentScreen][1] += temp;
            dowewrite = 1;
          break;

        case SCRSOCstop:  //SOC stop charge
            parameters[currentScreen][1] += temp;
            dowewrite = 1;
          break;

        case SCRchargeMode:  //   
            parameters[currentScreen][1] += temp;        
          if (parameters[currentScreen][1] == 1 )  {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(F("Stop SOC ="));
            lcd.print( parameters[SCRSOCstop][0] );
            lcd.setCursor(0, 1);
            lcd.print(F("Stop V ="));
            lcd.print( parameters[SCRstopChargeV][0] );
            //lcd.setCursor(0, 3);
            //lcd.print(F("PRESS TO SAVE"));
            delay(3000);
          }
         if (parameters[currentScreen][1] == 0 ) {
            lcd.clear();          
            lcd.setCursor(0, 0);
            lcd.print(F("Stop V ="));
            lcd.print( parameters[SCRstopChargeV][0] );            
            //lcd.setCursor(0, 3);
            //lcd.print(F("PRESS TO SAVE"));
            delay(3000);
         }
            dowewrite = 1;
          break;

        case SCRchargeToFullNow:                                        //start charging now to stop parameters 
            parameters[currentScreen][1] += temp;
            if (parameters[currentScreen][1] == 1 ) {
              startCharge();
              parameters[SCRchargeToFullNow][1] = 0;
              temp = 0;
            }
          break;

        case SCRefficiency:  //                        
            parameters[currentScreen][1] += (temp / 10);
            dowewrite = 1;
          break;

        case SCRHVAlarm:  //HV Alarm
            parameters[currentScreen][1] += (temp / 20);
            dowewrite = 1;
          break;

        case SCRLVAlarm:  //LV Alarm
            parameters[currentScreen][1] += (temp / 20);
            dowewrite = 1;
          break;

        case SCRHVdisconnect: //HV Cutoff
            parameters[currentScreen][1] += (temp / 20);
            dowewrite = 1;
          break;

        case SCRLVdisconnect:  //LV Cutoff
            parameters[currentScreen][1] += (temp / 20);
            dowewrite = 1;
          break;

        case SCRmaxDrift:  //maxDrift
            temp *= 5;
            parameters[currentScreen][1] += temp;
            dowewrite = 1;
          break;

        case SCRC0Trim:  //cell trim              <OK>
            parameters[currentScreen][1] += temp;
            dowewrite = 1;
          break;

        case SCRC1Trim:  //cell trim              <OK>
            parameters[currentScreen][1] += temp;
            dowewrite = 1;
          break;

        case SCRC2Trim:  //cell trim              <OK>
            parameters[currentScreen][1] += temp;
            dowewrite = 1;
          break;

        case SCRC3Trim:  //cell trim              <OK>
            parameters[currentScreen][1] += temp;
            dowewrite = 1;
          break;

        case SCRshowCells:  // show cells              <OK>
          parameters[currentScreen][1] += temp;
          if (parameters[currentScreen][1] == 1) {
            printCells();
            temp = 0;
            parameters[currentScreen][1] = 0;
          }
          break;


        case SCRshowTemps:  // show temp sense if available              <OK>
          parameters[currentScreen][1] += temp;
          if (parameters[currentScreen][1] == 1) {
            printTemps();
            temp = 0;
            parameters[currentScreen][1] = 0;
          }  
          break;


        case SCRresetAlrm:  //reset alrm              <OK>
          parameters[currentScreen][1] += temp;
          task = 1;  //clear alarm count
          break;

        case SCRmode:  //
          parameters[currentScreen][1] += temp;      
          if (parameters[currentScreen][1] == 0) {
             task = 7;
          }
          if (parameters[currentScreen][1] == 1) {
            task = 8;
          }
          if (parameters[currentScreen][1] == 2) {
            task = 6;
            temp = 0;
          }
          //dowewrite = 1;
          break;

        case SCRbalanceV:  //balance allow V              
            parameters[currentScreen][1] += (temp / 100);
            dowewrite = 1;
          break;


        case SCRencoderDir:  // set encoder direction             <OK>
            parameters[currentScreen][1] += temp;
            dowewrite = 1;
          break;


        case SCRshowErrors:
          parameters[currentScreen][1] += temp;
          if (parameters[currentScreen][1] == 1) {
            task = 3;
            parameters[currentScreen][1] = 0;
          }   
          break;


        case SCRsoc:
          parameters[currentScreen][1] = bankAH;
          ampSeconds += temp * 3600;
          if (ampSeconds == 0) {ampSeconds = 1;}
          ampHours = ampSeconds / 3600.0;
          bankAH = parameters[SCRbankCapacity][0] + ampHours ;
          parameters[currentScreen][1] = bankAH;  
          break;

        case SCRshuntPol:
          parameters[currentScreen][1] += temp; 
          shuntPolarity = parameters[currentScreen][1] ;
          dowewrite = 1;
          break;
          
        case SCRnumTsense:
          parameters[SCRnumTsense][1] += temp;
          dowewrite = 1;
          break;
        
      }
      menuTimer = millis() + 3000;
      encChange = 0;
      lcd.clear();
      cleanDisplay();
      printScreen();
    }
    lcd.setCursor(0, 3);
    cleanDisplay();
    lcd.print(F("> PRESS TO SAVE"));
  } while ((menuTimer > millis() )  &&  !encButtonPressed);
  currentScreen = tempCurrentScreen;  //keep menu location
  encVal = tempCurrentScreen;
  encChange = 0;
  taskManager();
  if (dowewrite && encButtonPressed){
   parameters[currentScreen][0] = parameters[currentScreen][1];
   writeeprom(1, 0);  
  } else{
    parameters[currentScreen][1] = parameters[currentScreen][0];
    }
}
