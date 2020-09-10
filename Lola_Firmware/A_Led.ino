

const byte colors[7][3] = {
  { NEO_BRIGHT,  0,    0   },
  { 0,    NEO_BRIGHT,  0   },
  { 0,    0,    NEO_BRIGHT },
  { NEO_BRIGHT,  NEO_BRIGHT,  0   },
  { 0,    NEO_BRIGHT,  NEO_BRIGHT },
  { NEO_BRIGHT,  0,    NEO_BRIGHT },
  { NEO_BRIGHT,  NEO_BRIGHT,  NEO_BRIGHT },
};



// Rendszer Státuszledjének változtatása
void led(bool enable) {
  digitalWrite(LED, enable);
}


// Rendszer led villogtatása
void blink() {
  led(ON);
  delay(500);
  led(OFF);
}


// Rendszer led villogtatása (röviden)
void short_blink(){
  led(ON);
  delay(100);
  led(OFF);
}


/*
 * LED Blink - Low Battery signal
 */
void lowbatt_blink(){
  
  for(byte i = 0; i < 3; i++){
    neo(RED);
    delay(500);
    neo(BLANK);
    delay(500);
  }
}




// Set neo color
void neo(byte colorindex){

  if(neo_current_color == colorindex){return;}

  if(colorindex == 0){
    neoled.setPixelColor(0, neoled.Color(0,0,0));
  }
  else{
    colorindex--;
    neoled.setPixelColor(0, neoled.Color(
    colors[colorindex][0],
    colors[colorindex][1],
    colors[colorindex][2]));
  }


  //neoled.SetBrightness(bright);
  neoled.show();
  
}


void led_sent_signal(){
  neo(GREEN);
  delay(300);
  neo(BLANK);
}


void led_fail_signal(){
  neo(RED);
  delay(300);
  neo(BLANK);
}


void HandlingLeds(){
  if(MODE == USB){
      if(charge_state){ neo(ORANGE); }
      else{
        neo(BLUE);
      }
  }
}


void INIT_LEDs(){
  Serial.println(F("INIT LEDs"));

  pinMode(LED, OUTPUT);
  neoled.begin();
  neoled.show();
  neo(PURPLE);
  
  Serial.println(F("DONE"));
}
