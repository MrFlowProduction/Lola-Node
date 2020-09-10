
// Return true if the input parameter is number (working with float number too)
bool isValidNumber(String str){
   bool isNum=false;
   if( str.charAt(0) != '+' && str.charAt(0) != '-' && !isDigit(str.charAt(0)) ) return false;

   for(byte i=1;i<str.length();i++)
   {
       if(!(isDigit(str.charAt(i)) || str.charAt(i) == '.')) return false;
   }
   return true;
}


void INIT_I2C(){
  Serial.print(F("INIT I2C Bus...."));

  //Wire.setClock(400000);  // Set I2C freq (default is 400KHz)
  Wire.begin();

  // Ping own address if the return value match with zero then the I2C bus is working
  if(0 != I2C_AddressCheck(0)){ Serial.print(F("FAIL\nI2C bus init fault!\nFault code: ")); Serial.println(I2C_AddressCheck(0), DEC); return; }

  I2C_ENABLE = true;

  Serial.println(F("...DONE"));
}


// Checking address and return error code
byte I2C_AddressCheck(byte address){
  Wire.beginTransmission(address);
  delay(2);
  return Wire.endTransmission();
}


// I2C address scanner (Serial)
void I2C_Scanner(){
  byte error, address;
  uint8_t nDevices;
  
  delay(15); // Sensors PowerUp Time
  Wire.begin();

  Serial.println(F("Scanning..."));

  nDevices = 0;
   for(address = 0; address <= 127; address++ )
  {
     error = I2C_AddressCheck(address);

    if (error == 0)
    {
      Serial.print(F("I2C device found at address 0x"));
      if (address<16)
        Serial.print("0");
      Serial.print(address,HEX);
      Serial.println(" !");

      nDevices++;
    }
    else if (error==4)
    {
       Serial.print(F("Unknow error at address 0x"));
       if (address<16)
        Serial.print("0");
       Serial.println(address,HEX);
     }
     delay(20);
   }
   if (nDevices == 0)
     Serial.println(F("No I2C devices found\n"));
   else
     Serial.println(F("done\n"));
}



void printSensorData(String text, float value, char* unit){

  Serial.print(text);
  Serial.print(value, 2);
  Serial.println(unit);
}


bool checkHumidity(float value){

  if(value < 0){   printSensorData(F("Humidity is too LOW! "), value, "%"); return false; }
  if(value > 130){ printSensorData(F("Humidity is too HIGH! "), value, "%"); return false; }

  return true;
}


bool checkTemperature(float value){

  if(value < -50){ printSensorData(F("Temperature is too HIGH! "), value, "C"); return false; }
  if(value > 100){ printSensorData(F("Temperature is too LOW! "), value, "C"); return false; }

  return true;
}


bool checkPressure(float value){

  if(value > 1600){ printSensorData(F("Pressure is too HIGH! "), value, "mbar"); return false; }
  if(value < 600){  printSensorData(F("Pressure is too LOW! "), value, "mbar"); return false; }

  return true;
}


// Check ADC chip returned value
bool CheckADC_Value(float mV){
  if(mV < 0 || mV > 2700){ return false; }

  return true;
}


// Check VWC Value (constrain)
float VWC_Check(float Value){

  Value = Value * 100;
  
  if(Value < 0.0){
    return 0;
  }
  
  else if(Value > 100.0){ return 100.0; }

  else { return Value; }
}


float UvCheck(float value){
  if(value < 0.0){ return 0; }
  if(value > 19.0){ return 19; }
  return value;
}

float SolarCheck(float value){
  if(value < 0.0){ return 0; }
  if(value > 1800.0){ return 19; }
  return value;
}
