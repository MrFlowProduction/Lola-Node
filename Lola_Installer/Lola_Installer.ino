#include <Wire.h>               // I2C Bus library
#include <AT24CX.h>             // EEPROM IC at24c256c

AT24CX              eeprom;                             // EEPROM Library

static byte           SEND_INT =            3;              // Data send interval (minutes) - stored in EEPROM
static byte           ADDRESS =             9;              // RF Address

static String line = "";
static char c;

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

// Checking address and return error code
byte I2C_AddressCheck(byte address){
  Wire.beginTransmission(address);
  delay(2);
  return Wire.endTransmission();
}


void setup() {
  
  // put your setup code here, to run once:
  INIT_SERIAL();
  INIT_EEPROM();
  INIT_SETTINGS();
}

void loop() {
  // put your main code here, to run repeatedly:
  uart_handling();
}
