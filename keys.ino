void buton(){
  if (on==true){
    c++;
    Serial.println(c);      
    on=false;   
    if(c>3){
      c=0;
      state=0;
      while (state<=6){
      make_call();}  
    }  
  else
   {melodie();}
}
}
void tasta(){
  stare_b=digitalRead(butonPin);
  if (stare_b == HIGH) {     
    buton();
  }
  delayMicroseconds(100);  
}


