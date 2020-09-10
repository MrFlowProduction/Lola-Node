


void INIT_SENSORS(){


  printinit(F("ADS1115-IDGST ADC"));

  if(0 != I2C_AddressCheck(0x48)){ 
    printreason(F("Not available!")); }
  else{ adc.initialize();
        adc.setMode(ADS1115_MODE_SINGLESHOT);
        adc.setRate(ADS1115_RATE_8);
        adc.setGain(ADS1115_PGA_6P144);
        pinMode(ADC_RDY, INPUT_PULLUP);
        adc.setConversionReadyPinMode();
        ADC_ENABLE = true;
        printdone();
  }

  #ifdef SHT21
  printinit(F("SHT21 Temperature & Humidity sensor"));

  if(0 != I2C_AddressCheck(0x40)){ 
    printreason(F("Not available!")); }
  else{
    printdone();
    sht_enable = true;
  }
  #endif

  #ifdef SHT31
  printinit(F("SHT31 Temperature & Humidity sensor"));

  if(!sht31.begin(0x44)){ // Set to 0x45 for alternate i2c addr
    printreason(F("Not available!")); }
  else{
    printdone();
    sht_enable = true;
  }
  #endif


  #ifdef MPL_BAROMETER
  printinit(F("MPL3115A2 Barometric sensor"));
  
  if(0 != I2C_AddressCheck(96)){ 
    printreason(F("Not available!")); }
  else{ 
        barometer.begin();
        barometer.setModeBarometer();
        barometer.setOversampleRate(7);
        barometer.enableEventFlags();
        barometric_enable = true;
        printdone();
  }
  #endif


  printinit(F("SDI-12 Sensors"));

  sdi.begin();
  
  printdone();
  
  
}


/*
 * Poll the assigned pin for conversion status 
 */
bool pollAlertReadyPin() {

  pinMode(ADC_RDY, INPUT_PULLUP);
  delay(1);
  
  for (uint32_t i = 0; i<100000; i++){
    if (!digitalRead(ADC_RDY)) { pinMode(ADC_RDY, INPUT); return true; }
  }

  pinMode(ADC_RDY, INPUT);

  Serial.println("Failed to wait for AlertReadyPin, it's stuck high!");
  return false;
}


/*
 * Check SHT21 read value result
 */
bool CheckSHT21result(uint16_t result){
  #ifdef SHT21
  if(result == ERROR_TIMEOUT){ Serial.println(F("Sensor TIMEOUT error")); return false; }
  if(result == ERROR_CRC){ Serial.println(F("Sensor CRC error")); return false; }
  #endif
  return true;
}


/*
 * Read Barometric Pressure Value
 */
float read_pressure(){

  float pressure = FAIL_READ;

  #ifdef MPL_BAROMETER
  if(!barometric_enable){ return FAIL_READ; }

  pressure = barometer.readPressure() / 100;
  
  if(pressure == 0.0){
    delay(100);
    pressure = barometer.readPressure() / 100;
  }

  #endif
  return pressure;
}


/*
 * Read Temperature Value
 */
float read_temperature(){

  if(!sht_enable){ return FAIL_READ; }

  #ifdef SHT21
  // Read Sensor
  uint16_t rawt = sht.readSensor(TRIGGER_TEMP_MEASURE_NOHOLD);

  if(!CheckSHT21result(rawt)){ return FAIL_READ; }

  float temp = (-46.85 + 175.72 / 65536.0 * (float)(rawt));
  #endif

  #ifdef SHT31
  float temp = sht31.readTemperature();
  if(isnan(temp)){ return FAIL_READ; }
  #endif

  return temp;
  
}


/*
 * Read Humidity Value
 */
float read_humidity(){

  if(!sht_enable){ return FAIL_READ; }

  #ifdef SHT21
  // Read Sensor
  uint16_t rawh = sht.readSensor(TRIGGER_HUMD_MEASURE_NOHOLD);
  
  if(!CheckSHT21result(rawh)){ return FAIL_READ; }

  float temp = (-6.0 + 125.0 / 65536.0 * (float)(rawh));
  #endif

  #ifdef SHT31
  float temp = sht31.readHumidity();
  if(isnan(temp)){ return FAIL_READ; }
  #endif

  return temp;
  
}



/*
 * Read ADC by channel ID
 * 
 * channel - channel (0-3)
 */
float read_adc(byte channel){

  if(!ADC_ENABLE){ return FAIL_READ; }

  adc.setMultiplexer(channel + 4);
  adc.triggerConversion();
  pollAlertReadyPin();
  
  float value = 0;

  for(byte i = 0; i < ADC_SAMPLE_RATE; i++){
    value += (float)adc.getMilliVolts(false);
    delay(10);
  }

  return value / ADC_SAMPLE_RATE;
}



/*
 * SDI Serial Query
 * kéréseket intéz a szenzor vele
 * a parancsot úgy kell megadni, hogy az első karaktert üresen kell hagyni az addressnek
 */
char* sdi_query(char* cmd, byte address = 255){
  
  cmd[0] = address == 255 ? '?' : (address + 48);
  return sdi.sdi_query(cmd, SDI_TIMEOUT);
}


/*
 * SDI Sensor Info
 * SDI szenzor adatok
 */
char* sdi_get_sensorinfo(){
  return sdi_query(SDI_INFO);
}


/*
 * Meas SDI Sensor
 * SDI szenzor mérés begyűjtése (5TE)
 */
SdiPack meas_sdi_sensor(){

  SdiPack pack;
  
  Serial.println(F("Query ?M!"));
  sdi_query("?M!");

  sdi.wait_for_response(1000);

  Serial.println(F("Wait for response..."));
  //dont worry about waiting too long it will return once it gets a response
  char* result = sdi_query("?D0!");
  Serial.println(result);
  Serial.println(strlen(result));

  if(strlen(result) <= 8){ return pack; }

  byte counter = 0;
  String temp = "";
  
  for(byte i = 0; i < strlen(result) - 1; i++){

      if(result[i] == '+'){
        Serial.println(temp);
             if(counter == 0){ pack.adr   = (byte)temp.toInt(); }
        else if(counter == 1){ pack.moist = (byte)temp.toFloat(); }
        else if(counter == 2){ pack.ec    = (byte)temp.toFloat(); }
        temp = "";
        counter++;
        continue;
      }

    temp += result[i];
  }

  if(counter == 3){ pack.temp  = (byte)temp.toFloat(); }

  Serial.println(pack.adr);
  Serial.println(pack.moist);
  Serial.println(pack.ec);
  Serial.println(pack.temp);

  return pack;
}


void sdi_test(){
  
  char* info = sdi_get_sensorinfo();
  
  if(sizeof(info) == 0){
    return;
  }

  Serial.println(info);

 
}
