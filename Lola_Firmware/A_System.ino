char* boolToText(bool value){
  if(value){ return "HIGH"; }
  return "LOW";
}


void printSysData(){

  Serial.print(F("\n\nLola Firmware "));
  Serial.println(version);
  
}


void printtestpassed(){
  Serial.println(F("[TEST PASSED]\n"));
}


void printtestfailed(){
  Serial.println(F("[TEST FAILED]\n"));
}


void printdone(){
  Serial.println(F("[DONE]"));
}

void printfail(){
  Serial.println(F("[FAIL]"));
}


void printinit(String text, bool withNewLine = false){

  Serial.print(F("\n[INIT]\n"));
  Serial.print(text);
  Serial.print("...");

  if(withNewLine){
    Serial.println();
  }

  
}

void printreason(String text, bool withFail = true){

  if(withFail){
    printfail();
  }

  Serial.print(F("[REASON]: "));
  Serial.print(text);
  Serial.println();

}

// Mértékegység típus printelése
void printType(byte type, bool withEnter = true){

  switch(type){

    case VOLTAGE:
      Serial.print("V");
      break;

    case MILLIVOLTAGE:
      Serial.print("mV");
      break;

    case PERCENT:
      Serial.print("%");
      break;

    case CELSIUS:
      Serial.print("C");
      break;

    case PRESSURE:
      Serial.print("mBar");
      break;
  }

  if(withEnter){
    Serial.print("\n");
  }
}

// Flaot érték printelése
void printValue(String name, float value, byte type){
  Serial.print(name);
  Serial.print(F(": "));

  if(value == FAIL_READ){ Serial.println("NO DATA"); return; }
 
  Serial.print(value, 2);
  printType(type);
}


// Akkumulátor töltés állapotának olvasása
bool getChargeStatus(){
  pinMode(CHARGE, INPUT_PULLUP);
  delay(10);
  bool status = !digitalRead(CHARGE);
  pinMode(CHARGE, INPUT);
  return status;
}


// Akkumulátor töltöttségének a szintje
byte getBatteryLevel(){

  if(battery_percent < 10){ return 0; }
  if(battery_percent < 21){ return 1; }
  if(battery_percent < 31){ return 2; }
  if(battery_percent < 51){ return 3; }
  if(battery_percent < 61){ return 4; }
  if(battery_percent < 81){ return 5; }

  return 6;
  
}

// Akkumulátor töltöttség kiszámolása százalékban
byte getBatteryPercent(){

  if(battery_voltage < 3.6){ return 0; }
  else if(battery_voltage > 4.15){ return 100; }

  int percent = (int)(((-3.6 + battery_voltage) / 0.55) * 100);

  if(percent > 100){ return 100; }

  if(percent < 0){ return 0; }
  
  return (byte)percent;
  
}


// Akkumulátor töltésének mérése analógporton feszültségben
float getBatteryVoltage(){

  float value = 0;

  for(byte i = 0; i < 10; i++){
    value += analogRead(BATT_VOLTAGE);
    delay(5);
  }

  value /= 10.0;

  // value to voltage
  value = value * (3.3 / 1023.0);

  // Devide with voltage divider value
  //               R2  (R1 + R2)
  return (value / (100 / 200.0));
}

// Meas Current Battery voltage, level, percent and store global variable
void refreshBatteryStat(){
  
  battery_voltage = getBatteryVoltage();
  battery_percent = getBatteryPercent();
  battery_level = getBatteryLevel();
  charge_state = getChargeStatus();
  
}

// Formatted Battery Status text
String getBatteryStatText(){
  String batt = String(getBatteryVoltage(), 2);
  batt += "V";

  return batt;
}

// Byte érték konvertálása bool-ba
bool stateToBool(byte state){
  
  if(state == 0){ 
    Serial.println("LOW");
    return false; }

  Serial.println("HIGH");
  return true;
}





/*
 * Add byte to payload
 */
void add_payload(byte val){
  
  payload[rf_index] = val;

  rf_index++;

  if(rf_index >= PAYLOAD_SIZE){
    rf_index = 0;
  }
 
}


/*
 * Clear Payload Content
 */
void clear_payload(){

  for(byte i = 0; i < PAYLOAD_SIZE; i++){
    payload[i] = 0;
  }

  rf_index = 0;
}


/*
 * Print Payload content in HEX array format
 */
void print_payload_hex(){

  Serial.println(F("Payload (HEX):"));
  Serial.println(rf_index);

  for(byte i = 0; i < rf_index + 1; i++){
    
    if (payload[i] < 16) Serial.print("0");
    
    Serial.print(payload[i], HEX);
    Serial.print(" | ");
  }

  Serial.println();
  
}
