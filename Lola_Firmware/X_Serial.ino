
void INIT_SERIAL(){

  Serial.begin(9600);

  printSysData();
  
}



void uart_handling(){

  // If USB MODE Deactivated return
  if(MODE == NORMAL){ return; }

  if(Serial.available() > 0){

    neo(PURPLE);

    String line = "";

    while(Serial.available() > 0){
      c = (char)Serial.read();

      if(c == '\n' || c == '\r') { break; }

      line += c;
      delay(2);
    }


    // ========================== General ==================================

    if(line == "scan"){
      I2C_Scanner();
    }

    else if(line == "reset"){
      neo(RED);
      resetFunc();
    }

    else if(line == "power on"){
      output_power(ON);
    }

    else if(line == "power off"){
      output_power(OFF);
    }

    else if(line == "battery"){
      Serial.print(F("Batt. level: "));
      Serial.print(battery_voltage, 2);
      Serial.println(F("V"));
      Serial.print(F("Charge status: "));
      Serial.println(getChargeStatus());
    }

    else if(line == "sleep test"){
      Serial.print(millis(), DEC);
      Serial.println(F("GOTO SLEEP"));
      delay(30);
      sleep();
      Serial.println(F("GOOD :)"));
    }


    // ========================== Sensors ==================================


    else if(line == "test ports"){
      printPorts();
    }

    else if(line == "get pressure"){
      Serial.print(F("Air Pressure: "));
      Serial.print(read_pressure(), 2);
      Serial.println(F("mbar"));
    }


    else if(line == "get temperature"){
      Serial.print(F("Air Temperatue: "));
      Serial.print(read_temperature(), 2);
      Serial.println(F("C"));
    }


    else if(line == "get humidity"){
      Serial.print(F("Humidity: "));
      Serial.print(read_humidity(), 2);
      Serial.println(F("%"));
    }

    else if(line == "read sdi"){
      meas_sdi_sensor();
    }


    // ================================== SETTINGS =======================================
    
    else if(line == "factory reset"){
      ReinstallMemory();
      resetFunc();
    }
    
    else if(line == "setup system"){
      SetupSystem();
    }

    else if(line == "save settings"){
      SaveSettings();
      Serial.println(F("SAVED"));
    }

    else if(line == "settings"){
      GetSettings();
    }

    else if(line == "send iot msg"){
      need_send_iot_pack = true;
    }

    else if(line == "ping master"){
      rf_ping_master();
    }

    else if(line == "request address"){
      address_manager();
    }

    else if(line == "reset rf address"){
      ADDRESS = 9;
      radio.setAddress(ADDRESS);
      SaveSettings();
    }

    else if(line == "iot"){
      send_iot_pack();
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


String readLine(){
  waitToAnsware();

  String line = "";

  while(Serial.available() > 0){
    c = (char)Serial.read();
    if(c == '\r'){ continue; }
    if(c == '\n'){ return line; }

    line += c;    
    delay(2);
  }

  return line;
}


void printValueLine(const char* name, int value){

  //Serial.printf("%s: %d\n", name, value);

}



bool serial_usb_connected(){

  if(Serial.available() > 0){
    if(readLine() == "USB-MODE"){
      Serial.println(F("USB-MODE OK"));
      return true;
    }
  }

  return false;
}


void CloseSerial(){

  Serial.end(); 
  pinMode(0, INPUT);
  pinMode(1, INPUT);
  digitalWrite(0, LOW);
  digitalWrite(1, LOW);
  
}


void SetupSystem(){

  String line = "";
  String type, value;
  
  bool isDataType = true;

  Serial.println(F("RDY"));

  while(true){
    line = readLine();
    if(line == ""){ Serial.println(F("ERROR")); continue; }

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
        printfail(); 
      }

      type = "";
      value = "";     
    }
  
    if(line == "#"){ break; }
  }

  SaveSettings();
  Serial.println(F("SAVED"));
}



void WaitForServiceMode(){

  // WAITING FOR SERIAL ===> USB_MODE (10 sec On Start)
  if (Serial) {

    int usb_wait = millis() + 10000;

    while (true) {

      if (serial_usb_connected()) {
        MODE = USB;
        break;
      }

      if (usb_wait < millis()) {
        CloseSerial();
        break;
      }

      
      led(ON);
      delay(10);
      led(OFF);
      delay(300);
    }
  }
  
}
