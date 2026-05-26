void loop2 (void* pvParameters) {
  while (1) {
   
  if (WS2812.isReady()){
  //for (int j = 0; j < 255; j += 2) {

    for (int i = 0; i < LEDS_COUNT; i++) {
      strip.setLedColorData(i, strip.Wheel((i * 256 / LEDS_COUNT + WS2812_j) & 255));
    }
    strip.show();
    //delay(10);

    WS2812_j+=2;

    if (WS2812_j>=255) WS2812_j=0;
  
    /*
    if (vemlvalue>=800) vemlvalue=800;
    vemlvalue = map(vemlvalue, 0, 800, 20, 255);
    */
   // Serial.print("LUX: ");
   // Serial.print(vemllux);
   // Serial.print(" BR: ");
   // Serial.println(bright_value);
     
   

  }
  if (timer0){
     vemllux = veml.readLux();
      
      uint8_t bright_value = brigh_value_indi(vemllux, lux_ranges, brigh_values, prev_brigh_value);
      prev_brigh_value = bright_value;
      ledcWrite(PWM_CHANNEL, bright_value);
      Serial.print("LUX: ");
      Serial.print(vemllux);
      Serial.print(" BR: ");
      Serial.println(bright_value);
    timer0 = false;
  }
  }
}