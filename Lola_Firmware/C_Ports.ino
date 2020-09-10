

void append_float_to_payload(float value){

  if(value == FAIL_READ){
    value = 0;
  }

  union u_tag {
    byte b[4];
    float fval;
  } u;

  u.fval = value;

  for(byte i = 0; i < 4; i++){
    add_payload(u.b[i]);
  }
}



void port_to_rf_frame(){

  Serial.println(F("Meas ports and make package..."));

  clear_payload();

  add_payload(IOT);

  printinit(F("ADC 0"));
  add_payload(A_0);
  append_float_to_payload(read_adc(0));

  printinit(F("ADC 1"));
  add_payload(A_1);
  append_float_to_payload(read_adc(1));

  printinit(F("ADC 2"));
  add_payload(A_2);
  append_float_to_payload(read_adc(2));

  printinit(F("ADC 3"));
  add_payload(A_3);
  append_float_to_payload(read_adc(3));

  printinit(F("Temperature"));
  add_payload(TEMP);
  append_float_to_payload(read_temperature());

  printinit(F("Humidity"));
  add_payload(HUMIDITY);
  append_float_to_payload(read_humidity());

  #ifdef MPL_BAROMETER
  printinit(F("Barometer"));
  add_payload(BAROMETER);
  append_float_to_payload(read_pressure());
  #endif

  printinit(F("Battery"));
  refreshBatteryStat();
  add_payload(BATTERY);
  add_payload(battery_percent);

  add_payload(CHARGE_STAT);
  add_payload(charge_state);
  
}


void print_portname(char* name, byte value, char* unit){
  Serial.print(name);
  Serial.print(": ");
  Serial.print(value);
  Serial.print(unit);
}



// Az összes port értékének kiíratása sorosportra
void printPorts(){
  
  Serial.println(F("\n\nValues\n========================="));
  refreshBatteryStat();
  printValue("Batt.",   battery_voltage,    VOLTAGE);
  printValue("Batt.",   battery_percent,    PERCENT);
  printValue("Chg.",    charge_state,       NONE);
  
  #ifdef MPL_BAROMETER
  printValue("Barom.",  read_pressure(),    PRESSURE);
  #endif
  
  printValue("Temp.",   read_temperature(), CELSIUS);
  printValue("Humid.",  read_humidity(),    PERCENT);
  printValue("A1",      read_adc(0),        MILLIVOLTAGE);
  printValue("A2",      read_adc(1),        MILLIVOLTAGE);
  printValue("A3",      read_adc(2),        MILLIVOLTAGE);
  printValue("A4",      read_adc(3),        MILLIVOLTAGE);
  
}
