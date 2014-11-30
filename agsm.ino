/*
SMS_SS v 0.9/20141117 - a-gsm 2.064 send/read/list SMS example utility
 COPYRIGHT (c) 2014 Dragos Iosub / R&D Software Solutions srl
 
 You are legaly entitled to use this SOFTWARE ONLY IN CONJUNCTION WITH a-gsm DEVICES USAGE. Modifications, derivates and redistribution 
 of this software must include unmodified this COPYRIGHT NOTICE. You can redistribute this SOFTWARE and/or modify it under the terms 
 of this COPYRIGHT NOTICE. Any other usage may be permited only after written notice of Dragos Iosub / R&D Software Solutions srl.
 
 This SOFTWARE is distributed is provide "AS IS" in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
 warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 
 Dragos Iosub, Bucharest 2014.
 http://itbrainpower.net
 */

/*
In order to make your Arduino serial communication reliable (especially for Arduino Uno) with a-gsm shield, you must edit: 
 C:\Program Files\Arduino\libraries\SoftwareSerial\SoftwareSerial.h 
 
 comment the line 42 
 #define _SS_MAX_RX_BUFF 64 
 
 this, will look after that like: 
 //#define _SS_MAX_RX_BUFF 64
 
 and add bellow the next line: 
 
 #define _SS_MAX_RX_BUFF 128
 
 You just increased the RX buffer size for UNO and other "snails". 
 
 Now you can compile and upload your code supporting highier buffered serial input data.
 */

/*
  utility for sending SMS
 phno   - destination phone number, international format eg.:+40123456789 - char*
 phtype - destination phone number type 129 (international)/ 147(domestic) - char*
 message - .....what you want to transmit in your SMS :)) - max. 160 chars - char* 
 
 returns 1 for SUCCESS
 0 for FAILURE
 */

/*
  setupMODEMforSMSusage() -utility to set up the modem for SMS (text mode) usage
 */

/*
  utility for listing SMS (received) from memmory
 store into SSMS the used/total SMSs
 SSMS.noSMS  --- used SMS (locations)... if >0, we have something to read :)
 SSMS.totSMS --- total SMS (locations)  
 */



/*clear buffd preparing it for writing*/
void clearBUFFD(void){//just clear the data buffer
  memset(buffd, 0x00, sizeof(buffd));
}

/*send command -cmd to themodem, waiting Delay_mS msecs after that*/
size_t aGsmCMD(char* cmd, int Delay_mS){
  size_t retv;  
  //printDebugLN("try>>");delay(100);
  //printDebugLN(cmd);delay(100);
  SSerial.println(cmd);
  delay(Delay_mS);
  return retv;
}

/*send string -str to the modem, NOT ADDING \r\n to the line end*/
size_t aGsmWRITE(char* str){
  size_t retv;  
  SSerial.print(str);
  return retv;
}

/*return read char from the modem*/
inline char aGsmREAD(void){
  char retv;
  retv = SSerial.read();
  return retv;
}

/*
Receive data from serial until event:
 SUCCESS string - 1'st argument 
 FAILURE string - second argument 
 TIMEOUT - third argument in secconds!
 return: int 
 -1 TIMEOUT
 0  FAILURE
 1  SUCCESS
 the string collected from modem => buffd
 */
int recUARTDATA( char* ok, char* err, int to){
  int res=0;
  char u8_c;
  int run = 1;
  int i=0;
  unsigned long startTime;

  clearBUFFD();
  startTime = millis();	
  //delay(10);
  while(run){	
    if(strstr(buffd,ok)) {
      delay(200);
#if defined(atDebug)
      printDebugLN("ok");
#endif
      clearHDSerial();
      res=1;
      run=0;
    }
    else if(strstr(buffd,err)) {
#if defined(atDebug)
      printDebugLN("err");
#endif
      //printDebugLN(buffd);
      run=0;
      clearHDSerial();
      clearBUFFD();
    }
    if(millis() - startTime > to *1000) {
#if defined(atDebug)
      printDebugLN("to!");
#endif
      clearHDSerial();
      run=0;
      res=-1;//timeout!
    }

    while(TXavailable()){
      u8_c = aGsmREAD();
      buffd[i]=u8_c;
      i++;
      //if(u8_c == 0x0D) break;
    }
    //delay(10);
  }
  return(res);
}


/*
send AT command, looking for SUCCES STRING(1'st) or FAILURE STRING(second), and TIMEOUT(third)
 return 1 for succes, 2 for failure, -1 for timeout
 modem response is loaded in buffd
 */
int sendATcommand(char* outstr, char* ok, char* err, int to){
  int res=0;
  clearHDSerial();
  clearBUFFD();	
#if defined(atDebug)
  printDebugLN(outstr);
#endif
  aGsmCMD(outstr,1);
  res = recUARTDATA( ok,  err, to);
  return(res);
}


/*
  returns TRUE if chars are available in RX SERIAL_BUFFER (some chars has been received)
 check this function before call aGsmREAD()
 */
bool TXavailable(){
  int retv;
  retv = SSerial.available();    
  return (retv > 0);
}


/* just print debug to Serial*/
void printDebugLN(char* what){
  Serial.println(what);
}

void setAUDIOchannel(){
  /*sendATcommand("AT+QAUDCH=2", "OK","ERROR",2);  //use audio channel(2-standard for a-gsm)
  sendATcommand("AT+QMIC=2,14", "OK","ERROR",2); //set mic channel(2), mic gain
  sendATcommand("AT+CLVL=25", "OK","ERROR",2);   //set output POWER! Do NOT exceed value of 25 if headset is used!
*/
}

/*
int getcallStatus()
 #detect if the voice call is CONNECTED
 #returns: 
 #0 Active   CONNECTED   BOTH
 #1 Held                 BOTH
 #2 Dialing (MO call)    OUTBOUND
 #3 Alerting (MO call)   OUTBOUD
 #4 Incoming (MT call)   INBOUD
 #5 Waiting (MT call)    INBOUD
 */
int getcallStatus(){
  int res=0;
  char content [40];
  printDebugLN("connection status...");
  res = sendATcommand("AT+CLCC", "OK","ERROR",2);//CHECK DIAL STATUS ...RETURN +CLCC: 1,0,0,0,0,"dialed NO", 129 IF ACTIVE CALL 
  if(res==1){
    memset (content,0x00, sizeof(content));
    char * pch0;
    char * pch1;
    char pch;
    if(strstr(buffd,"+CLCC: ")){
      pch0 = strstr(buffd,"+CLCC: ");
      pch1 = strstr(buffd,"OK");
      strncpy(content, pch0+7, 5);
      //printDebugLN(content);
      pch=content[4];
      //printDebugLN((char *)pch);
      if(pch==0x30) {
        printDebugLN("CONNECTED");
        return 0;
      }
      else if(pch==0x32) {
        printDebugLN("DIALING");
        return 2;
      }
      else if(pch==0x33) {
        printDebugLN("RING-OUTBOUND");
        return 3;
      }
      else if(pch==0x34) {
        printDebugLN("RING-INBOUND");
        return 4;
      }
    }
    else{//NO CONNECTION active
      printDebugLN("NO CONNECTION");
      return -1;
    }
  } 
  printDebugLN("CMD ERROR");
  return -2;
}


/*prepare the modem for DTMF usage*/
void setupMODEMforDTMFRusage(){
  int res;
  //is MODEM ready for SMS?
  res = 0; 
  while (res!=1){//wait 
    res = sendATcommand("AT+CPBS?","OK","ERROR",5);
    delay(500);
  }

  //set SIMM memory as active
  sendATcommand("AT+CPBS=\"SM\"","OK","ERROR",7);

  //is MODEM ready for SMS?
  res = 0; 
  while (res!=1){//wait 
    res = sendATcommand("AT+CPBS?","OK","ERROR",5);
    delay(500);
  }				

  //set SMS mode TEXT		
  res = sendATcommand("AT+CMGF=1","OK\r\n","ERROR\r\n",5);
  if(res==1) printDebugLN("Set SMS mode TEXT succeed\r\n");
  //AT+CNMI=2,1,0,0,0//standard mode
  //AT+CNMI=2,0,0,0,0
  res = sendATcommand("AT+CNMI=2,0,0,0,0\r","OK\r\n","ERROR\r\n",5);
  if(res==1) printDebugLN("Disable TE SMS notification succeed\r\n");

  // res = sendATcommand("ATS0=2","OK","ERROR",5);//
  // if(res==1) printDebugLN("Autoanswer at second RING enabled");


  res = sendATcommand("AT+QTONEDET=4,1,3,3,65536","OK","ERROR",5);//some DTMF detection settings
  res = sendATcommand("AT+QTONEDET=1","OK","ERROR",5);//enable DTMF detection
  if(res==1) printDebugLN("DTMF detection enabled");
  res = sendATcommand("AT+QSFR=7","OK","ERROR",5);//use EFR recommended
  if(res==1) printDebugLN("EFR enabled");
  res = sendATcommand("AT+QTDMOD=1,0","OK","ERROR",5);
  if(res==1) printDebugLN("AT+QTDMOD=1,0");

  /*
  AT+QTONEDET=4,1,3,3,65536 for voice application to reduce the ratio of code repeating
   AT+QTONEDET=1 //enable DTMF code detection
   AT+QSFR=7 // ENHANCED FULL RATE
   */
}

/*
Listen for DTMF until "terminator" has been found or "to" (in secs) timeout reached
 return: int 
 -1 TIMEOUT
 1  SUCCESS
 read DTMF string => DTMF
 */
int listen4DTMF(char * DTMF, char * terminator, int to){
  int run = 1;
  int res = 0;
  int i=0, j=0;
  unsigned long startTime;
  char u8_c;
  char* pch0;
  char* pch1;
  char DTMFstr[3];
  char DTMFint;

  clearBUFFD();
  startTime = millis();	
  while(run){
    if(millis() - startTime > (unsigned long)to *1000) {
#if defined(atDebug)
      printDebugLN("to!");
#endif
      clearHDSerial();
      run=0;
      res=-1;//timeout!
    }

    while(TXavailable()){//read it baby
      u8_c = aGsmREAD();
      buffd[i]=u8_c;
      i++;
      if(u8_c == 0x0D) {//found EOL, let's process it
        if (strstr(buffd,"+QTONEDET:")){
          pch0 = strstr(buffd,"+QTONEDET:");
          pch1 = strstr(buffd,"\r\n");
          memset(DTMFstr,0x00,sizeof(DTMFstr));
          strncpy(DTMFstr, pch0+11, 2);
          //printDebugLN("dir:");
          //printDebugLN(DTMFstr);
          DTMFint = (char) atoi(DTMFstr);
          DTMF[j] = DTMFint;
          //printDebugLN("conv:");
          //printDebugLN(DTMF);
          j++;
          if(strstr(DTMF,terminator)){//found logical terminator!
            clearHDSerial();
            run=0;//break loop
            res=1;//go back with success
          }
        }
        clearBUFFD();//prepare for and go to receive next DTMF char
        i=0;
        break;
      }
    }
  }
  return(res);
}

/*
just flush remaining chars from serial
 */
void clearHDSerial(){
  i = 0;
  while(TXavailable()){
    ch = aGsmREAD();
    i++;
  } 
  delay(100);
} 

/*start/restart the modem*/
void restartMODEM(){
  clearHDSerial();
  delay(100); 
  if(digitalRead(statusPIN)){
    printDebugLN("powerOFF"); 
    delay(500);
    digitalWrite(powerPIN, HIGH);    
    delay(1000);                  
    digitalWrite(powerPIN, LOW);  
    powerState=0;
    delay(8000);
  }

  if(!digitalRead(statusPIN)){ // e off
    clearHDSerial();
    delay(100);
    printDebugLN("try restart"); 
    delay(500);
    digitalWrite(powerPIN, HIGH);    
    delay(1000);                  
    digitalWrite(powerPIN, LOW);   
    delay(8000);
    offsetTime=0;
    powerState = 1;
    state=1;
  }
}

void make_call()
{
  int res; 
  char readFileBuffer[128];
  int callStatus;
  switch(state){
  case 0://check modem status
    if(!digitalRead(statusPIN)) restartMODEM();
    else
      state++;
    i=0;
    res= 0;
    while(res != 1){
      res = sendATcommand("AT","OK","ERROR",2);
      if (res != 1) {
        if(i++ >= 10) restartMODEM();
      }
      delay(500);
    }
    sendATcommand("AT+IPR=0;&w","OK","ERROR",2);    
    delay(2000);    
    break;

  case 1:
    clearBUFFD();
    //next some init strings...
    aGsmCMD("AT+QIMODE=0",200);      
    aGsmCMD("AT+QINDI=0",200);      
    aGsmCMD("AT+QIMUX=0",200);      
    aGsmCMD("AT+QIDNSIP=0",200);
    offsetTime=0; 
    clearBUFFD();
    state++;
    break;

  case 2:    
    printDebugLN("try CPIN...");
    if(!offsetTime) offsetTime = millis();
    if ((millis() - offsetTime) > 20000) restartMODEM();
    clearBUFFD();
    aGsmCMD("AT+CPIN?",200);      
    i = 0;
    while(TXavailable()){
      ch = aGsmREAD();
      buffd[i] = ch;
      i++;
    }
    //printDebugLN(buffd); delay(100);
    if(strstr(buffd,"READY")) {
      offsetTime=0; 
      state++;
      printDebugLN("READY");
    } 
    else {
    }
    clearBUFFD();
    clearHDSerial(); 
    delay(100);
    offsetTime = millis();
    //state++;
    break;

  case 3:    
    //getIMEI();//just for fun      
    clearBUFFD();      
    if(!offsetTime) offsetTime = millis();
    if ((millis() - offsetTime) > 30000) restartMODEM(); 

    clearBUFFD();
    printDebugLN("Query GSM registration?");
    aGsmCMD("AT+CREG?",1000);//GSM network registration      
    i = 0;
    while(TXavailable()){
      ch = aGsmREAD();
      buffd[i] = ch;
      i++;
    }
    //printDebugLN(buffd); delay(100);
    if(strstr(buffd,"0,1")) {
      offsetTime=0; 
      state++;
      printDebugLN("Ready");
    } 
    else {
    }
    clearBUFFD();
    clearHDSerial(); 
    offsetTime = millis();
    state++;
    break;

  case 4: //init SIM   
    clearBUFFD();
    printDebugLN("Query State of Initialization");
    aGsmCMD("AT+QINISTAT",200);      
    i = 0;
    while(TXavailable()){
      ch = aGsmREAD();
      buffd[i] = ch;
      i++;
    }

    if(strstr(buffd,"3")) {
      offsetTime=0; 
      state++;
      printDebugLN("Ready"); 
      delay(100);
    } 
    else {
      printDebugLN("Not yet..."); 
      delay(1000);
    }
    clearBUFFD();
    clearHDSerial(); 
    delay(100);
    break; 


  case 5://call defined numb
    setAUDIOchannel();
    sendATcommand("ATD+40766452047;", "OK","ERROR",2);


    state++;
    break; 

  case 6://set modem for DTMF decode
    if(!offsetTime) offsetTime = millis();
    if ((millis() - offsetTime) > 5000) restartMODEM();

    setupMODEMforDTMFRusage();    
    printDebugLN("Let's decode some DTMF!"); 

    //printDebugLN("Waiting for remote call! We has been set ATS0=2 -->auto answer at second ring detection");
    //printDebugLN("Waiting for remote call! Please Start the DTMF SEND Arduino device!");

    callStatus =5;//go to loop and force dial


    while(callStatus>0) {//remmember! We has been set ATS0=2-->auto answer at second ring detection
      callStatus = getcallStatus();
    }
    //if connection succeded enable DTMF
if (callStatus==0){
    printDebugLN("Connected!");
    delay(2000);//wait a little bit
    printDebugLN("Try to receive some data... Waiting for *** terminator or 55 second!");
    //recUARTDATA("DUMMYOK","DUMMYERR",55);//wait 55 second and receive the data


    while ((res>=0)&&(password!=readFileBuffer))
{
    memset(readFileBuffer,0x00,sizeof(readFileBuffer));// clear readFileBuffer for receive DTMF string
  res = listen4DTMF(readFileBuffer,"#", 20);//listen for DTMF
    //printDebugLN("RAW Received data:");
    if(res<0)
      printDebugLN("terminator *** NOT detected - 20 seconds passed");
    else		
      printDebugLN("terminator *** detected"); 

    printDebugLN("\r\nReceived data:");
    printDebugLN(readFileBuffer);
}

if (password==readFileBuffer){
  digitalWrite(relayPin,HIGH);
  delay(15); 
}
    //delay(5000);//wait a little bit


    //sendATcommand("ATH","OK","ERROR",2);//close the call
    //printDebugLN("Hang up active call");

    //sendATcommand("ATS0=0","OK","ERROR",5);//
    //printDebugLN("Autoanswer disabled");

    //sendATcommand("AT+QTONEDET=0","OK","ERROR",5);//disable DTMF detection
    //printDebugLN("DTMF detection disabled");

    clearBUFFD();
    clearHDSerial();

}
//delay(10000);
    offsetTime = millis();
    state++;
    on=true;

    break; 
    /*default:
     //restartMODEM();
     printDebugLN("Done!");
     delay(10000);
     //state=0;
     break;*/
  }
}

