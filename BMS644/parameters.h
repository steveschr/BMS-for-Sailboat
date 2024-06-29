/*
  math for Res Div network
  Vout = Vin * R2 / R1 + R2
  R2/R1+R2 = ratio
  Vin(batteyV) =  Vout / ratio
  C0 = direct read
  C1 = 10 / 20 = .5
  C2 = 5 / 15 = .333333
  C3 = 3.32 / 13.32 = .249249
  Total_Voltage = 4 / 14 = .2857


  TODO
add/modify:
tail current of .05 to .033 ?

Waiting for pack??

pulse audible alarm  use LCD flash routine,   minor alarm, major alarm??
clean up low temp & set to dum mode
verify time in status before actions like start, stop & diconnect.  verify reset timers , counters
accumulate overcharge AHs?  learn effiency?
history??  what kind?  
VERIFY LOGIC     rework HV & LV cut & sanity checks, 
    
Broken:
if 1 cell open display wrong because of ABS calc.  do test
sometimes reboots on eprom save 


v1.2.0 change log:
implement balance with adjustable start V, predefined time 20min 50mV drift to start.  (~1AH = 40 min) , reporting on cell V page, add bal to display cycle
fixup drift, set alarm no lockout
seperate amp sense & cell V sense.  high freq amp sense (~160ms), cell V sense 2 sec interval
lower ADC SPI freq to 1mHz  & take and average 200 samples for amps
Hardware - remove C14, C16 from mainboard.  Remove bias capacitor from shunt.  Verify INA213 50V/V
remove "reset EEPROM" ,remove all load functions , remove several display cycles , add # of T sense option
allow charging after boat off on LV disconnect, remove tail current mode, remove shunt val, menu changes,
auto write eeprom, add .04 Amps for BMS consumption,allows connecting grnd direct to battery negative instead of shunt 
Remove temperature settings from menu.  hardcoded at start=37F stop=100F disconnect=130F
add cell V to cell trim screens, add stats for last charge data to show cells,


v 1.2.1(b)
changed some pin assignments to match new hardware & balance board.  use top cell for pack voltage calc.  
set balance time to 10 min (100mA).  dont record UT event to EEPROM.  Display packvoltage instead of sum of cells
fixup Lv disconnect logic.  set lv alarm 10mV higher than Lv disco if it was lower, reworked menu input - now requires button 
press to set value.  Improved open cell routine.  cell # 0 vol can remain high - no bleed resistor to gnd.  Cell #3 triggers both hv & lv disco

NEED reset lvdisco bounds , cycle count prbly wrong , fixup OC logic

*/


//LCD Menu Logic
const int numOfScreens = 27;
int currentScreen = 0;

const String screens[numOfScreens][4] = {  //Item, Unit, display type, BMS, DUM or ALL display
  {"Bank Capacity", " AH's", "I", "ALL"}, 
  {"Start Charge", " Volts", "F", "BMS"}, 
  {"Stop Charge", " Volts", "F", "BMS"}, 
  {"SOC Start", " %SOC", "I", "BMS"}, 
  {"SOC Stop", " %SOC", "I", "BMS"},   
  {"Charge Mode", " 0=V 1=SOC", "I", "BMS"},  
  {"Chg To Full Now", " 1=Yes", "I", "BMS"}, 
  {"Efficiency", " %", "F", "BMS"}, 
  {"HV Alarm", " Volts", "F", "BMS"}, 
  {"LV Alarm", " Volts", "F", "BMS"}, 
  {"HV Disconnect", " Volts", "F", "BMS"}, 
  {"LV Disconnect", " Volts", "F", "BMS"}, 
  {"MaxDrift", " mV", "I", "BMS"}, 
  {"C1 Trim", " mV", "I", "BMS"}, 
  {"C2 Trim", " mV", "I", "BMS"}, 
  {"C3 Trim", " mV", "I", "BMS"}, 
  {"C4 Trim", " mV", "I", "BMS"}, 
  {"Balance Allow V", " Volts", "F", "BMS"}, 
  {"Show Cells & Vs", " 1=Show", "I", "ALL"}, 
  {"Show Temps", " 1=Show", "I", "ALL"},
  {"Show Alarms", " 1=Show", "I", "ALL"},
  {"Clear Alrm", " 1=Clear", "I", "ALL"},   
  {"Mode 0=BMS 1=OFF", "    2=DUM", "I", "ALL"},  
  {"Encoder Direction", " 0 or 1 ", "I", "ALL"},  
  {"SOC", "AH", "I", "ALL"},
  {"Shunt Polarity" , "-1 or 1", "F" , "ALL"},
  {"# of T Sense" , "   0,1,2,3", "I" , "ALL"},
};


float parameters[numOfScreens] [2] = {
  280, 280, //bank capacity
  3.30, 3.30, //start charge
  3.6, 3.6, //stop charge     
  75, 75, //SOC start
  90, 90, //SOC stop
  0, 0, //Charge Mode   0=V, 1=SOC
  0, 0, //chg to full now
  98, 98, //efficiency
  3.7, 3.7, //hvalarm
  3.1, 3.1, //lvalarm
  3.8, 3.8, //Hv disconnect   
  2.5, 2.5, //Lv disconnect
  100, 100, //drift
  0, 0,  //c0 trim
  0, 0, //c1 trim
  0, 0, //c2 trim
  0, 0, //c3 trim
  3.6, 3.6, //balance v
  0, 0, //show cells
  0, 0, //show temps
  0, 0, //show error
  0, 0, //reset alarm
  1, 1, //op mode BMS, DUM, OFF    <<<<<<<<<initially off to alow cell connect without alarm
  1, 1, //encoder direction
  0, 0, //current SOC
  1.0, 1.0, //shunt polarity
  2, 2  //# of temp sensors
};

const byte SCRbankCapacity = 0;
const byte SCRstartChargeV = 1;
const byte SCRstopChargeV = 2;
const byte SCRSOCstart = 3;
const byte SCRSOCstop = 4;
const byte SCRchargeMode = 5;
const byte SCRchargeToFullNow = 6;
const byte SCRefficiency = 7;
const byte SCRHVAlarm = 8;
const byte SCRLVAlarm = 9;
const byte SCRHVdisconnect = 10;
const byte SCRLVdisconnect = 11;
const byte SCRmaxDrift = 12;
const byte SCRC0Trim = 13;
const byte SCRC1Trim = 14;
const byte SCRC2Trim = 15;
const byte SCRC3Trim = 16;
const byte SCRbalanceV = 17;
const byte SCRshowCells = 18;
const byte SCRshowTemps = 19;   //<<
const byte SCRshowErrors = 20;
const byte SCRresetAlrm = 21; //<<
const byte SCRmode = 22;
const byte SCRencoderDir = 23;
const byte SCRsoc = 24;
const byte SCRshuntPol = 25;
const byte SCRnumTsense = 26;


// Time
unsigned long menuTimer = 0;
unsigned long DISPpreviousMillis = 0 ;
const unsigned long timeInterval = 1000UL ;
unsigned long lastAmpTime = 0;
//unsigned long seconds = 0;
unsigned long lockoutStartTime = 0;
unsigned long lockoutStartTimeSYS = 0;
unsigned long lockoutStartTimeBAL = 0;
unsigned long backlightTimer = 0;
unsigned long balanceTime = 0;
byte dispStatusCycle = 0;


// I/O & Encoder
volatile bool goingUp = false;
volatile bool goingDown = false;
bool encChange = false;
bool encButtonPressed = false;
const byte encpin1 = 2; //pin 16
const byte encpin2 = 3; //pin 17
const byte encButton = 30; //pin 18
int encVal = 0;


//pin assignments
const byte adcU0 = 6; //pin 5
const byte adcU1 = 10;  //pin 3


//                              USE    MPU  ID         
const byte ALARMpin = 24;    //AUX4    24  24   
const byte AcChrge_OFF = 19; //ShOFF   38  19/A5
const byte AcChrge_ON = 18;  //ShON    37  18/A4
const byte Solar_OFF = 17 ;  //SolOFF  36  17/A3
const byte Solar_ON = 16;    //SolON   35  16/A2
const byte Alt_OFF = 29;     //AltOFF  29  29
const byte Alt_ON = 28;      //AltON   28  28
const byte Load_ON = 20;     //LoadON  39  20/A6
const byte Load_OFF = 21;    //LoadOFF 40  21/A7
const byte AUXpin1 = 27;     //AUX1    27  27
const byte AUXpin2 = 26;     //AUX2    26  26
const byte AUXpin3 = 25;     //AUX3    25  25
const byte AUXpin5 = 14;     //AUX5    33  14
const byte balancePins[4] = {0, 1, 8, 9};
const int balanceTempSensor = 4;    //pin # 4 of ADC





//status
bool boatON = false;    //set inital values for startup
bool ChargeON = false;  //are we charging?
bool AlarmFlagOV = false; //error flag for OV set when alarm func is called  - no alarms are cleared
bool AlarmFlagUV = false; //error flag for UV set when alarm func is called
bool AlarmFlagOT = false;  //hi temp
bool AlarmFlagUT = false; //low temp
bool AlarmFlagOTdisconnect = false;  //hi temp disconnect  ?????
bool AlarmFlagOVdisconnect = false;
bool AlarmFlagUVdisconnect = false;
bool AlarmFlagDrift = false; //error flag for high cell  V spread, set in getadc()
bool AlarmFlag = false; // is any alarm set, reflects pin status
bool AlarmFlagOC = false; //Open Cell - large diff between cal'd and measured pack V
bool AlarmFlagSYS = false; //system err
bool balON = false;  //balance status
bool chargeLock = false;
String currentMode = "BMS";
byte flagCountOV = 0 ;
byte flagCountUV = 0 ;
byte flagCountOT = 0 ;
byte flagCountUT = 0 ;
byte flagCountOTdisconnect = 0;
byte flagCountOVdisconnect = 0;
byte flagCountUVdisconnect = 0;
byte flagCountDrift = 0;
byte flagCountSYS = 0;
byte flagCountStartCharge = 0;  //delay charge start in V mode
byte balcell = 0;


const unsigned int alarmHistoryPointer = (numOfScreens * sizeof(float) ) + (sizeof(float) * 2);   // next available loc
int alarmCount = 0;
const unsigned int alarmDataPointer = alarmHistoryPointer + (sizeof(int) * 25) + 1; //max alrm count + pointer
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
*/



//adjustments

const byte RelayOnTime = 20;
const float vref0 = .0005; //2.048 / 4096 measured values of vol references
const float vref1 = .001;  //4.096 / 4096
float shuntR = .00025; //200A-50mV
//float shuntR = .001; //50A-50mV

float shuntPolarity = 1.0;

//Variables
int task = 0;

// temperature related
float tempSensor[4] = {0, 0, 0, 0}; //array for temprature values
const float invBeta = 1.00 / 3430.00;   // replace "Beta" with beta of thermistor
//const float invBeta2 = 1.00 / 3940.00;
const float invT0 = 1.00 / 298.15;   // room temp in Kelvin

// voltage & current measure
float adcVol0 = 0;  //bias
float adcVol1 = 0;  //shunt out
const float cellRdiv1 = ( 10.0 / 20.0 );  // for R divider version
const float cellRdiv2 = ( 5.0 / 15.0 );  //see note on R div ratio aboave
const float cellRdiv3 = ( 3.319 / 13.319 );  // value also in getADC
//const float packRdiv = ( 4.02 / 14.02 );
float Total_Voltage = 0;   //sum of cell voltages
float packVoltage = 0;   //measured pack voltage
float amps0 = 0;
float ampHours = 0;
float ampSeconds = 0;
float SOC = 0;    //also bankAH declared below
float cellV[4] = {0, 0, 0, 0}; //calculated V of cells
float voltageDrift = 0;  //initial setting for drift
byte maxIndex = 0;    //pointers to hi & low cell voltages
byte minIndex = 0;
float bankAH = parameters[SCRbankCapacity][0];  //current measurement
int cycleCount = -1;
unsigned int balanceCount[4] ={0,0,0,0};
float endOfChargeData[6] = {0,0,0,0,0,0};
byte endOfChargeMinCell = 0;
byte endOfChargeMaxCell = 0;

// SPI
//const byte dacRegister = 0xb0; // Sets default DAC registers B00110000, 1st bit choses DAC, A=0 B=1, 2nd Bit bypasses input Buffer, 3rd bit sets output gain to 1x, 4th bit controls active low shutdown. LSB are insignifigant here.
//const int dacSecondaryByteMask = 0b0000000011111111;        // Isolates the last 8 bits of the 12 bit value, B0000000011111111.


const int numberoferrors = 12;
//typedef struct {
  // char description [12];
//} descriptionType;

const char errCode0[] PROGMEM = "XXYYZZ" ;
const char errCode1[] PROGMEM = "OVER T" ;
const char errCode2[] PROGMEM = "UNDER T" ;
const char errCode3[] PROGMEM = "OVER V" ;
const char errCode4[] PROGMEM = "UNDER V" ;
const char errCode5[] PROGMEM = "V DRIFT" ;
const char errCode6[] PROGMEM = "Overflow" ;
const char errCode7[] PROGMEM = "LV Discnt" ;
const char errCode8[] PROGMEM = "HV Discnct" ;
const char errCode9[] PROGMEM = "Open Cell" ;
const char errCode10[] PROGMEM = "OT Discnct" ;
const char errCode11[] PROGMEM = "SYSTEM" ;

const char* const errCodePointers[numberoferrors] PROGMEM = 
{
errCode0, 
errCode1, 
errCode2, 
errCode3, 
errCode4, 
errCode5, 
errCode6, 
errCode7, 
errCode8, 
errCode9, 
errCode10, 
errCode11
};


//functions main
byte getTemps();  //ck temp sensors, returns Hiindex temp cell
void logic();  //main decision loop for BMS mode
int readAdc(byte unit, byte channel); //read ADCs
void getAdc(); // calc V & Amps, determine V differences, sets maxIndex,minIndex, minVoltage, maxVoltage
void alarm(); // activate alarm pin
void alarmOff(); // deactivate alarm pin
void stopCharge();
void startCharge();
void startBoat();// power up
void stopBoat();// total shutdown
void getamphours();
bool writeeprom(byte etask, int code);  //accepts case# as task & val to write to eeprom, returns erpom written if true
void taskManager();
void resetall(int flavor);

//functions display
void printCells();
void clearLine(int RowToClear);
void printScreen(); //menu options
void LCDstatus();     //cycles thru data to display, amps, AH, V, bankAH, alarm hist status
void showMem();
void showTemps();
void decoder();  //rotary encoder
void encoder();  //rotary encoder
void getCurrentScreen();
void cleanDisplay(); //error trap menu input & some var
void startScreen();  //open screens, variable retrieval
bool getNewScreen(); //menus support
void briefMessage(String st1, float val); //place short msg onscreen

//menu & balance
void inputAction(); //menu
void doBalance(); //Cell balance routine.  
