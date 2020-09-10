#define MISS_ID   1
#define WAIT_ADR  2
#define ADR_OK    3


void state_manager(){

  // Handling Measurements
  if(need_send_iot_pack && isMeasEnabled){
    send_iot_pack();
  }

  // Handling Sleeping
  if(need_goto_sleep && isSleepEnabled){
    sleep();
  }

}


void send_iot_pack(){

  neo(ORANGE);

  // If RF Address is OK
  if(address_manager()){

    // Szenzorok áramellátásának bekapcsolása
    output_power(ON);

    // Adatok konvertálása küldhettő csomaggá
    port_to_rf_frame();

    // Adatcsomag kiiratása
    print_payload_hex();

    // Üzenet elküldése
    if(!rf_send_iot()){ 
      led_fail_signal();
      printfail(); }
    else{ 
      led_sent_signal();
      printdone(); }

    // Alvó mód engedélyezése
    need_send_iot_pack = false;
    need_goto_sleep = isSleepEnabled;
  }
  else{
    neo(RED);
    delay(1000);
    
  }
  
}


bool address_manager(){

  if(ADR_MAN_STAT == ADR_OK){ return true; }
  
  // Address is OK
  if(ADDRESS > 9){ 
    ADR_MAN_STAT = ADR_OK;
    return true; }

  // Address is default
  if(ADDRESS == 9){

    if(!validFactoryID()){ 
      ADR_MAN_STAT = MISS_ID;
      return false; } 

    if(ADR_MAN_STAT != WAIT_ADR){
      
      // Request RF Address
      Serial.print(F("Request RF Address..."));
      if(request_rf_address()){
        ADR_MAN_STAT = WAIT_ADR;
        printdone();
      }
      
      printfail();
      
    }
    
  }
  
  return false;

}


void print_adr_state(){

  switch(ADR_MAN_STAT){

    case ADR_OK:
      Serial.println(F("Address OK"));
      break;

    case WAIT_ADR:
      Serial.println(F("Waiting!"));
      break;

    case MISS_ID:
      Serial.println(F("NO Factory ID"));
      break;
    
  }
  
}
