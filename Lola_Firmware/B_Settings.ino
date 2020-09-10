
/*
 * EEPROM Storage for Settings
 * 
 */


// CRC Code for EEPROM Validate (could use for auto memory reinit)
const byte crc[] = { 11, 126, 253, 86, 189 };


// Address MAP for EEPROM Saving
#define addr_RF_ADR          8    // Address of the RF Address
#define addr_SEND_INT        9    // Address of the sending interval
#define addr_FACT_LEN        10   // Factory ID Lenght


// Clear EEPROM
void eepromClear(){
  Serial.println(F("Clear EEPROM"));
  for (uint16_t i = 0 ; i < 1000 ; i++) {
    eeprom.write(i, 0);
    if(i % 10 == 0) Serial.print(".");
  }

  Serial.println("DONE");
}


// Validate EEPROM by CRC
// Return false if CRC not match with EEPROM CRC
bool IsValidMemory(){

  for(byte addr = 0; addr < 5; addr++){
    if(crc[addr] != eeprom.read(addr)){
      return false;
    }
  }

  return true;
}


// Clear, CRC write and Saving Settings to EEPROM
void ReinstallMemory(){

  Serial.println(F("Reinstall EEPROM..."));

  eepromClear();

  for(byte addr = 0; addr < 5; addr++){
      eeprom.write(addr, crc[addr]);
  }

  printdone();
  SaveSettings();
}


/*
 * Read Factory ID from EEPROM
 * 
 * return with NULL if fact-id is not exist in memory
 */
char* getFactoryID(){

  byte size = eeprom.read(addr_FACT_LEN);

  if(size == 0){ return NULL; }

  char * factid = (char *) malloc (size + 1);
  unsigned char k;

  for(byte i = 0; i < size; i++){
    k = eeprom.read(addr_FACT_LEN + i + 1);
    factid[i] = k;
  }

  factid[size] = '\0';

//  Serial.println(factid);

  return factid; 
}


/*
 * Factory ID is exist
 */
bool validFactoryID(){
  
  if(eeprom.read(addr_FACT_LEN) < 36){ return false; }

  return true;
}


// Save Settings to EEPROM
void SaveSettings(){

  // System Settings
  eeprom.write(addr_RF_ADR, ADDRESS);
  eeprom.write(addr_SEND_INT, SEND_INT);

  
}


// Load Settings from EEPROM
void LoadSettings(){
  
  if(!IsValidMemory()){
    
    Serial.println(F("\nMemory is invalid"));
    ReinstallMemory();
  }

  ADDRESS = (byte)eeprom.read(addr_RF_ADR);
  SEND_INT = (byte)eeprom.read(addr_SEND_INT);
  
}



// Set System Settings
bool SetSystemSettings(String type, String value){

  if(type == "rf_adr"){
    if(!isValidNumber(value)){ return false; }
    ADDRESS = (byte)value.toInt();
    return true;
  }
  

  else if(type == "interval"){
    if(!isValidNumber(value)){ return false; }
    SEND_INT = (uint8_t)value.toInt();
    return true;
  }

  return false;
  
}


// Get System Settings
void GetSettings(){

  Serial.println(F("======= System Settings ======="));
  
  Serial.print(F("RF Address: "));
  Serial.println(ADDRESS);

  Serial.print(F("Interval: "));
  Serial.print(SEND_INT);
  Serial.println(F(" minute(s)"));

  Serial.print(F("Factory ID: "));
  Serial.println(getFactoryID());
  
}


// INIT Settings from EEPROM
void INIT_SETTINGS(){

  if(!eeprom_enable){
    Serial.println(F("EEPROM ERROR"));
    return;
  }
  
  printinit(F("INIT SETTINGS..."));
  LoadSettings();
  GetSettings();
  printdone();
}


void INIT_EEPROM(){
  printinit(F("EEPROM (AT24C256)"));

  // Ellenőrzi, hogy megtalálható-e a buszon
  if(!I2C_AddressCheck(0x50)){ printreason(F("Not available")); }
  
  eeprom_enable = true;

  printdone();
}
