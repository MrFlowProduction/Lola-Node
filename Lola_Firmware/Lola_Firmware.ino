

/*
   ########################################
   #                                      #
   #        Lola Firmware V2.1.3          #
   #            by MR.Flow                #
   #                                      #
   ########################################

   for Indux Production and Project
   GateWay Station: Indigo

   Programmer:
   Florian Szekely
   florian.szekely@aseso.hu
   +36 30 556 2270


   Supported Products:
   - Lola Rev. 4

   

*/

// ######################### Compile Configurations #################################
#define NODE          // NODE mode (SLEEP, ONLY MEAS)
//#define SHT21         // SHT21 Sensor
#define SHT31         // SHT31 Sensor
//#define MPL_BAROMETER // MPL3115A2 Barometer
// ##################################################################################


#include <Wire.h>               // I2C Bus library
#include <AT24CX.h>             // EEPROM IC at24c256c

#include "ADS1115.h"            // ADS1115 ADC 
#include <SPI.h>                // SPI library
#include <RFM69.h>              // RF Radio library
#include <stdio.h>              // this library need to float - byte array conversion
#include <LowPower.h>           // Low Power library for sleep mode
#include <SDISerial.h>          // SDI Serial Sensors
#include <Adafruit_NeoPixel.h>  // Adafruit Neopixel led

#ifdef MPL_BAROMETER
#include "SparkFunMPL3115A2.h"  // MPL3115A2 Barometer Library
#endif

#ifdef SHT21
#include "SHT2x.h"              // SHT21 Temp & Humidity Sensor Library
#endif

#ifdef SHT31
#include "Adafruit_SHT31.h"
#endif


struct sdi_pack
{
   byte adr;
   float temp;
   float moist;
   float ec;
};

typedef struct sdi_pack   SdiPack;

// ___________________________ PINOUT ___________________________________
// =======================================================================
//      NAME              PIN          DESCRIPTION
//========================================================================
#define LED               9         // Stat LED pin
#define BATT_VOLTAGE      A3        // Battery Voltage sense pin (analog)
#define SUPPLY_EN         A1        // Sensors supply enable pin
#define RF_INT            3         // RF module interrupt pin
#define RF_SS             6         // RF module slave select pin
#define SDI               2         // SDI-12 Bus pin
#define ADC_RDY           5         // ADC Ready pin
#define NEO_LED           4         // Neo Pixel led output
#define BUZZER            8         // Buzzer output
#define BUTTON            10        // User Button input
#define CHARGE            A2        // Battery charge status input
#define BUCK_EN           A0        // 5V Buck Boost supply enable
//========================================================================


// _________________________________ PORTS ENUMs__________________________________
// ===============================================================================
//      NAME              VALUE          DESCRIPTION
// ===============================================================================
#define A_0               1           // Analog Port
#define A_1               2           // Analog Port
#define A_2               3           // Analog Port
#define A_3               4           // Analog Port

#define TEMP              51          // Temperature Port
#define HUMIDITY          52          // Humidity Port
#define BATTERY           53          // Battery Port
#define BAROMETER         54          // Barometer Port
#define CHARGE_STAT       55          // Charge Status
// ===============================================================================


// ________________________________ RF ENUMS _____________________________________
// ===============================================================================
//      NAME              VALUE          DESCRIPTION
// ===============================================================================
#define RF_NOP            0           // NO OPERATION (Ex.: test message)
#define REG               1           // Node Registration
#define ADR               2           // Set Node Address
#define IOT               3           // IOT message
#define SPS               4           // Set Port State
#define NOW               5           // Master: Request Current DateTime/NODE: Set Current DateTime
#define STP               168         // Message End Flag
// ===============================================================================



// __________________________________ Configuration ____________________________________________
// =============================================================================================
//      NAME                  VALUE                    DESCRIPTION
// =============================================================================================
const char* version =         "2.1.3b";             // SW Version    
#define ADC_NEXTMEAS          100                   // ADC next meas time
#define FREQUENCY             RF69_868MHZ           // RF Radio Frequency and type
#define NETWORKID             9                     // Must be the same for all nodes (0 to 255)
#define OWN_ID                9                     // OWN ID in RF network
#define BOSS_ID               1                     // My node ID (0 to 255)
#define BROADCAST             255                   // Broadcast Address
#define FAIL_READ             -99999                // FAIL_READ - for sensor reading
#define DATA_BUFFER_SIZE      50                    // Data buffer size for sensor datas (RF stat package)
#define RF_ATTEMPT_COUNT      3                     // RF send attempt count
#define RF_RESEND_INTERVAL    500                   // RF resend package interval
#define ADC_SAMPLE_RATE       10.0                  // ADC read sample rate
#define RF_ENCRYPT_KEY        "A$je3L22!<dI9cA-"    // RF Encrypt key
#define PAYLOAD_SIZE          80                    // Payload max size
#define CUT_VOLTAGE           3.6                   // Battery cutdown voltage
#define ADC_POWERUP_TIME      200                   // ADC inputs power up wait time
#define STAT_INTERVAL         200                   // LED Blink Interval (ms)
#define RESET_INTERVAL        960                   // Hágy alvás ciklus után resetelje magát az eszközt (960 az kb 2 naponta 3 perces időzítéssel)
#define SDI_RDY_TIME          3000                  // SDI-12 busz powerup sensor time
#define NEO_BRIGHT            30                    // NEO led max bright
#define SDI_TIMEOUT           1000                  // SDI szenzor válasz időkorlát
// =============================================================================================



// ____________________________________________ Globals ___________________________________________________
// ========================================================================================================
// TYPE               NAME                  DEF. VALUE         DESCRIPTION
// ========================================================================================================
bool                  I2C_ENABLE =          false;          // I2C Bus enabled (FAIL report)
bool                  ADC_ENABLE =          false;          // ADC chip enable (FAIL report)
bool                  sht_enable =          false;          // SHT Sensor enable (FAIL report)
bool                  barometric_enable =   false;          // MPL Sensor enable (FAIL report)
bool                  sdi_enable =          false;          // SDI Sensors enabled
bool                  wait_for_address =    false;          // Waiting for address mode
bool                  radio_ready =         false;          // RF module is ready
bool                  need_send_iot_pack =  false;          // Need send iot pack indicator
bool                  need_goto_sleep =     false;          // Need goto sleep mode
bool                  isSleepEnabled =      false;          // Sleep Enable/Disable flag
bool                  isMeasEnabled =       false;          // Measurement enable or disable
bool                  eeprom_enable =       false;          // EEPROM enabled
bool                  charge_state =        false;

static byte           ADR_MAN_STAT =        0;              // Address Manager status
static byte           MODE =                0;              // Device work mode (NORMAL, USB, DEV)
static byte           SysStat =             0;              // System Status
static byte           send_fail_counter =   0;              // Package sending fail counter
static byte           cycle_counter =       0;              // Sleep Counter for Sleep mode
static byte           SEND_INT =            3;              // Data send interval (minutes) - stored in EEPROM
static byte           ADDRESS =             OWN_ID;         // RF Address
static byte           battery_level =       0;              // Battery Charge Level
static byte           battery_percent =     0;              // Battery Level in Percent
static byte           payload[PAYLOAD_SIZE + 1];            // RF Global Payload buffer 
static byte           rf_index =            0;              // RF Payload indexer
static byte           neo_current_color =   255;            // Neoled current color

float                 battery_voltage =     0;              // Battery current voltage

int16_t               rssi =                0;              // RF RSSI value
unsigned long         stat_read_timer =     0;              // Status Reading timer
int                   sleep_counter =       0;              // Sleep counter for reset

static char c;
// =========================================================================================================


// ________________________________ Objects_______________________________________
// ===============================================================================
// Type             Name                                   DESCRIPTION
// ===============================================================================
ADS1115             adc(ADS1115_DEFAULT_ADDRESS);       // ADC Chip class
RFM69               radio(RF_SS, RF_INT, true);         // RF Radio Core Library
Adafruit_NeoPixel   neoled = Adafruit_NeoPixel(1, NEO_LED, NEO_GRB + NEO_KHZ800);
AT24CX              eeprom;                             // EEPROM Library

#ifdef MPL_BAROMETER
MPL3115A2           barometer;                          // MPL Barometric sensor chip class
#endif

#ifdef SHT21
SHT2x               sht;                                // SHT Sensor chip class
#endif

#ifdef SHT31
Adafruit_SHT31 sht31 = Adafruit_SHT31();
#endif

SDISerial           sdi(SDI, 1);
// ===============================================================================


// SDI Commands
#define SDI_INFO " I!"


// Enums
#define ON      1
#define OFF     0
#define ENABLE  1
#define DISABLE 0
#define NONE    0

// Diag message types
#define INFO  0
#define WARN  1
#define ERR   2


// Modes
#define NORMAL  0   // Normal mode with sleeping
#define USB     1   // USB settings/diagnostic mode (device keep alive)
#define DEV     2   // Development mode (device keep alive and goto debug mode)


// Meas type
#define VOLTAGE       1
#define PERCENT       2
#define CELSIUS       3
#define PRESSURE      4
#define MILLIVOLTAGE  5


// Colors
#define BLANK             0
#define RED               1
#define GREEN             2
#define BLUE              3
#define LIGHT_BLUE        4
#define ORANGE            5
#define PURPLE            6
#define PINK              7


// SOFT RESET
void(* resetFunc) (void) = 0; //declare reset function @ address 0



void setLowPowerConsumptionMode(){
  // Kikapcsolunk mindent fogyasztót
  output_power(OFF);
  sleep_radio();
  neo(BLANK);
  led(OFF);
}


// Sensor Power Enable/Disable
void output_power(bool enable) {

  Serial.print(F("Set Supply - "));

  if(enable){    
    // Supply Power OFF
    pinMode(SUPPLY_EN, OUTPUT);
    digitalWrite(SUPPLY_EN, LOW);
    pinMode(BUCK_EN, OUTPUT);
    digitalWrite(BUCK_EN, HIGH);
    delay(ADC_POWERUP_TIME);
  }
  else{
    digitalWrite(SUPPLY_EN, HIGH);
    digitalWrite(BUCK_EN, LOW);
    delay(100);
    pinMode(SUPPLY_EN, INPUT);
    pinMode(BUCK_EN, INPUT);
  }

  printdone();
}


void INIT_GPIO() {
  pinMode(BATT_VOLTAGE, INPUT);
}


void INIT(){
  // Initials
  INIT_GPIO();

  INIT_SERIAL();
  INIT_EEPROM();
  INIT_SETTINGS();
  INIT_LEDs();
  INIT_SENSORS();
  INIT_RF();
}


/*
 * Configure Run mode
 * 
 * sleep, send, led, etc...
 */
void RuntimeConf(){

  if(MODE != USB){
    isSleepEnabled = true;
    isMeasEnabled = true;
    neo(BLANK);
  }
  else{
    neo(BLUE);
  }

}


/*
 * Runtime diagnostic
 */
void RuntimeCheck(){

  if(stat_read_timer < millis()){
    
    refreshBatteryStat();

    if(MODE == NORMAL){

      if(battery_percent == 0){
        lowbatt_blink();
        sleep();
      }

      if(isSleepEnabled && validFactoryID()){
        need_send_iot_pack = true;
      }
      
    }
    
    stat_read_timer = millis() + STAT_INTERVAL;
  }
}



void setup() {

  INIT();

  Serial.println(F("RDY"));

  WaitForServiceMode();

  RuntimeConf();
}



void loop() {
  
  RuntimeCheck();
  uart_handling();
  state_manager();
  RadioHandler();
  HandlingLeds();
}
