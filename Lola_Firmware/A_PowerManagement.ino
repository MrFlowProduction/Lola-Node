


// Sleep Function
void sleep(){

  // Alvás ciklusszámláló
  cycle_counter = 0;

  // Legalacsonyabb fogyasztási mód
  setLowPowerConsumptionMode();
  
  // Akkumulátor töltöttség figyelése
  // Ha túl alacsony, akkor teljesen kikapcsolja magát a rendszert
  if(battery_percent == 0){ LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); return; }

  // Calculate Loop Cicle count
  byte destination_count = (byte)((SEND_INT * 60) / 8.0);

  // Sleep and Weak up by Watchdog timer
  while(cycle_counter < destination_count){
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    cycle_counter ++;
  }

  INIT_RF();
  output_power(ON);
  stat_read_timer = 0;
  need_goto_sleep = false;

  sleep_counter++;

  if(sleep_counter >= RESET_INTERVAL){ resetFunc(); }
}
