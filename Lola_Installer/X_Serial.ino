void INIT_SERIAL(){

  Serial.begin(9600);
}



void uart_handling(){

  if(Serial.available() > 0){

    line = "";
    
    while(Serial.available() > 0){
      c = (char)Serial.read();

      if(c == '\n' || c == '\r') { break; }

      line += c;
      delay(1);
    }


    // ========================== General ==================================

    if(line == "scan"){
    }

   

    // ================================== SETTINGS =======================================
    else if(line == "setup system"){
      SetupSystem();
    }

   
    else{
      Serial.println(line);
      Serial.println(F("UNKNOWN COMMAND"));
    }

    Serial.flush();
    
  }
  
}




void waitToAnsware(){

  while(Serial.available() == 0){
    delay(10);
  }
  
}


bool readLine(){
  waitToAnsware();

  line = "";

  while(Serial.available() > 0){
    c = (char)Serial.read();
    if(c == '\r'){ continue; }
    if(c == '\n'){ return true; }

    line += c;
    
    delay(1);
  }

  return false;
}


void printValueLine(const char* name, int value){

  //Serial.printf("%s: %d\n", name, value);

}



void SetupSystem(){

  line = "";
  String type, value;
  
  bool isDataType = true;

  Serial.println(F("RDY"));

  while(true){

    if(!readLine()){ Serial.println(F("ERROR")); continue; }

    if(isDataType){ 
      type = line; isDataType = false; 
      Serial.print(type);
      Serial.print(": ");
    }
    else{ 
      
      value = line;
      Serial.println(value);
      
      isDataType = true;
      if(SetSystemSettings(type, value)){
        Serial.println(F("OK"));
      }
      else{
        Serial.println(F("FAIL")); 
      }

      type = "";
      value = "";     
    }
  
    if(line == "#"){ break; }
  }

  SaveSettings();
  Serial.println(F("SAVED"));
}
