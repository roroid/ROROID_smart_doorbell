//**************************************************************************
//variables for A-GSM
//#define atDebug //uncomment this line if you want to catch OK/ERROR/TIMEOUT modem response

//modify the next 2 line to be convenient for you
char message[]="Hi!\r\nThis message has been sent from a-gsm v2.064 Arduino shield connected with my Arduino board.";  //no more than 160 chars -text SMS maximum lenght
char destinationNumber[]="+40123456789";      //usually phone number with International prefix eg. +40 for Romania - in some networks you must use domestic numbers

/*do not change under this line! Insteed, make one copy for playing with.*/

#define powerPIN     7//Arduino Digital pin used to power up / power down the modem
//#define resetPIN      6//Arduino Digital pin used to reset the modem 
#define statusPIN    5//Arduino Digital pin used to monitor if modem is powered 

#include <SoftwareSerial.h>
SoftwareSerial SSerial(2, 3);  //RX==>2 ,TX soft==>3



int state=0, i=0, powerState = 0; 
char ch;
char buffd[256];
//char IMEI[18];
unsigned long offsetTime;
boolean phoneb=false;

//**************************************************************************
//variables for dorbell

const int a = 440;
const int f = 349;
const int cH = 523;
const int eH = 659;
const int gS = 415;
const int fH = 698;
boolean on=true;

const int buzzerPin = 9; // Digital Pin 9
const int butonPin = 4; // Digital Pin 4
const int relayPin = 11; // Digital Pin 11
String password="1234567#";

int stare_b=0;
int c=0;
//**************************************************************************




/*Arduino hardware setup utility*/
void setup(){
  //**************************************
  //init for A-GSM
  Serial.begin(9600);
  //SSerial.begin(19200);
  SSerial.begin(9600);
  clearHDSerial();
  delay(10);
  //pinMode(powerPIN, OUTPUT);
  pinMode(statusPIN, INPUT);
  //digitalWrite(powerPIN, LOW);    
  delay(100);
  printDebugLN("HW setup ready");
  delay(100);
  //**************************************
  //init for dorbell
  //Setup pin modes
  pinMode(relayPin, OUTPUT); // Digital Pin 11
  pinMode(buzzerPin, OUTPUT); // Digital Pin 9
  pinMode(butonPin,INPUT_PULLUP);  //Digital Pin 4
  digitalWrite(relayPin,LOW);

}



void loop(){
  tasta();


}




