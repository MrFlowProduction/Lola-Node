
void INIT_RF(){

  printinit(F("RFM69HW"), true);
  SPI.begin();

  if(!radio.initialize(FREQUENCY, ADDRESS, NETWORKID)){
    printreason(F("Not available!"));
    return;
  }

  radio.setHighPower(true);

  radio.encrypt(RF_ENCRYPT_KEY);

  radio.rcCalibration();
  
  radio_ready = true;

  printdone();
}



/*
 * Sending RF data package/payload with retry ACK
 * 
 * data - payload
 * length - payload size
 * address - destination device network address
 */
bool SendData(byte *data, byte length, byte address){

  if(!radio_ready){ printreason(F("RF is Ready!")); }

  Serial.println(F("RF - Send package..."));

  if(radio.sendWithRetry(address, data, length, RF_ATTEMPT_COUNT, RF_RESEND_INTERVAL)){
    printdone();
    return true;
  }

  printfail();
  return false;
}


/*
 * Read radio modul CMOS temperature
 */
byte readRadioTemp(){

  if(!radio_ready){ return 0; }

  return radio.readTemperature(-1);
  
}


/*
 * Set radio to sleep mode
 */
void sleep_radio(){
  if(!radio_ready){ return; }
  radio.sleep();
  radio_ready = false;
}


/*
 * Wake Up radio after sleep
 */
void ReinitRadio(){
   if(radio_ready){ return; }
   INIT_RF();
}


/*
 * Set Sleep Mode radio module
 */
void SleepRadio(){

  if(!radio_ready){ return; }
  
  radio.sleep();
  radio_ready = false;
  Serial.println(F("RADIO - Goto sleep mode"));
}


/*
 * Store RF Address by Master
 */
void store_rf_address(){

  if(wait_for_address && radio.DATALEN > 2){
    Serial.print(F("RF: New Slave ID\nAddress: "));
    ADDRESS = (byte)radio.DATA[1];
    radio.sendACK();
    radio.setAddress(ADDRESS);
    Serial.println(ADDRESS);
    SaveSettings();
  }

}



/*
 * Request RF address by Factory ID
 */
bool request_rf_address(){

  if(!radio_ready){ return false; }

  byte size = 1;
  byte buf[45];

  byte fact_size;

  char* factory_id = getFactoryID();

  fact_size = strlen(factory_id);

  if(fact_size == 0){
    return false;
  }

  Serial.print(F("Factory ID lenght: "));
  Serial.println(fact_size);

  buf[0] = REG;
  for(byte i = 0; i < fact_size; i++){
    buf[i + 1] = (byte)factory_id[i];

    Serial.print(char(payload[i+1]));
    size++;
  }

  buf[size] = STP;
  size++;

    Serial.println(F("\nSending..."));
    if(radio.sendWithRetry(BOSS_ID, buf, size, RF_ATTEMPT_COUNT, RF_RESEND_INTERVAL)){
      wait_for_address = true;
      return true;
    }
  

  return false;
}



/*
 * Request Current DateTime from Master by RF
 */
void rf_request_now(){

  if(!radio_ready){ return false; }

//  Serial.println(F("RF - Request Current Date"));
//
//  byte payload[2];
//
//  payload[0] = NOW;
//  payload[1] = STP;
//
//  if(radio.sendWithRetry(BOSS_ID, payload, 2, RF_ATTEMPT_COUNT, RF_RESEND_INTERVAL)){
//    printdone();
//  }
//  else{
//    printfail();
//  }
}


/*
 * Parse RF Current DateTime message
 */
bool rf_now(){

  if(!radio_ready){ return false; }

  Serial.println(F("Current Time Payload"));

  if(radio.DATALEN < 8){
    return false;
  }

//  SetRTC(
//    radio.DATA[4] + 2000, // YEAR
//    radio.DATA[5],        // 
//    radio.DATA[6],
//    radio.DATA[1],
//    radio.DATA[2],
//    radio.DATA[3]); 
  
  return true;
}


/*
 * RF Payload parser
 * 
 * payload protocol [op. code] [arg. 0] [arg. 1] ... [arg. n]
 */
void rf_parser(){
  

  // Check package length
  if(radio.DATALEN == 0){
        return;
  }

  byte sender_adr = radio.SENDERID;

  switch(radio.DATA[0]){
    
    case RF_NOP: // No Operation (ex.: ping message)
      radio.sendACK();
      return;

    case ADR: // Store RF Address
      store_rf_address();
      radio.sendACK();
      return;

    case NOW: // No Operation (ex.: ping message)
      rf_now();
      radio.sendACK();
      return; 
  }
}


void rf_ping_master(){

  if(!radio_ready){ return false; }

  Serial.println(F("PING"));
  byte buf[3];
  buf[0] = RF_NOP;
  buf[1] = STP;

  Serial.print(F("MASTER Address: "));
  Serial.println(BOSS_ID);
  Serial.print(F("Network ID: "));
  Serial.println(NETWORKID);

  if(radio.sendWithRetry(BOSS_ID, buf, 2, RF_ATTEMPT_COUNT, RF_RESEND_INTERVAL)){
    printdone();
    rssi = radio.readRSSI();
    return;    
  }
  
  printfail();
}


/*
 * Send IOT Package to Master
 */
bool rf_send_iot(){

  if(!radio_ready){ return false; }
  
  if(radio.sendWithRetry(BOSS_ID, payload, rf_index, RF_ATTEMPT_COUNT, RF_RESEND_INTERVAL)){
    rssi = radio.readRSSI();
    short_blink();
    return true;    
  }

  blink();
  return false;
  
}


/*
 * Handling Radio Received messages
 */
void RadioHandler(){
  
  if(!radio_ready){ return; }

  if(radio.receiveDone()){

    Serial.print(F("Sender: "));
    Serial.println(radio.SENDERID);
    Serial.print(F("Length: "));
    Serial.println(radio.DATALEN);
    
    rssi = radio.readRSSI();
    
    rf_parser();
  }

}
