void doBalance() {  //test, discharge, rest, test

if ( cellV[maxIndex] > parameters[SCRbalanceV][0] ){  //Cell V above Bal setting
  if ((voltageDrift > .050) && tempSensor[balanceTempSensor] <= 95 && !AlarmFlag && (millis() - lockoutStartTimeBAL > 60000)) {  
    if ( ! balON ) {
       //30 sec delay before bal allowed again, 50mV spread, check temp sensors
     digitalWrite(balancePins[maxIndex], HIGH);
     balcell = maxIndex;
     balanceTime = millis();
     balON = true;
     balanceCount[balcell] += 1;
    }
  }
}

  if ( balON ) {
      float T = getTemps(balanceTempSensor);
      if ( millis() - balanceTime > ( 600000) || tempSensor[balanceTempSensor] > 100 ||  AlarmFlag ) {   //600,000 = 10min = 100mA  1A load x .1H
      balON = false;
      digitalWrite(balancePins[balcell], LOW);
      lockoutStartTimeBAL = millis();
    }
  }
}

void balanceOff(){   //not in use.  turns off all balance pins
  for (int i = 0; i < 3; i++) {
  digitalWrite(balancePins[i], LOW);
  }
}


/*

  rate 3.4v @ 3 Ohm = 1A   
  measured 1.5A @ 3.3 V thru 2 Ohm  so ~ 40min / Amp

*/
