static byte hue = 0;
static byte idex = 0;
static int bouncedirection = 0;
static int phase = 0;
static byte fire_heat[LEDS_COUNT];

void ws2812_effect() {
  if (!mydata.ws2812_enable) {
    FastLED.setBrightness(0);
    fill_solid(leds, LEDS_COUNT, CRGB::Black);
    FastLED.show();
    return;
  }
  FastLED.setBrightness(mydata.ws2812_brightness);
  switch (mydata.ws2812_anim) {
    case 0: // Rainbow fade
      fill_rainbow(leds, LEDS_COUNT, hue, 7);
      hue++;
      break;

    case 1: // Rainbow loop
      leds[idex] = CHSV(hue, 255, 255);
      fadeToBlackBy(leds, LEDS_COUNT, 20);
      idex = (idex + 1) % LEDS_COUNT;
      hue += 4;
      break;

    case 2: // Random colors
      if (phase % 30 == 0)
        for (int i = 0; i < LEDS_COUNT; i++)
          leds[i] = CHSV(random(256), 255, 255);
      phase++;
      break;

    case 3: // Color bounce
      fadeToBlackBy(leds, LEDS_COUNT, 40);
      leds[idex] = CHSV(hue, 255, 255);
      if (bouncedirection == 0) { idex++; if (idex >= LEDS_COUNT) { bouncedirection = 1; idex--; } }
      else { idex--; if (idex < 0) { bouncedirection = 0; idex++; } }
      hue += 2;
      break;

    case 4: // Fire
      for (int i = 0; i < LEDS_COUNT; i++) {
        fire_heat[i] = qsub8(fire_heat[i], random8(0, 40));
        int j = (LEDS_COUNT - 1) - i;
        if (j > 0) {
          int sp = fire_heat[j - 1];
          int s = (sp + fire_heat[j] + fire_heat[j]) / 3;
          leds[i] = HeatColor(fire_heat[i] + 80);
        }
      }
      break;

    case 5: // Twinkle
      for (int i = 0; i < LEDS_COUNT; i++) {
        if (random8() < 20) leds[i] = CHSV(random(256), 255, 255);
        else leds[i].nscale8(230);
      }
      break;

    case 6: // Strobe
      if (phase % 4 == 0) {
        if (phase % 8 == 0) fill_solid(leds, LEDS_COUNT, CHSV(hue, 255, 255));
        else fill_solid(leds, LEDS_COUNT, CRGB::Black);
        hue += 8;
      }
      phase++;
      break;

    case 7: // Scanner
      fadeToBlackBy(leds, LEDS_COUNT, 30);
      leds[idex] = CHSV(hue, 255, 255);
      if (bouncedirection == 0) { idex++; if (idex >= LEDS_COUNT) { bouncedirection = 1; idex--; } }
      else { idex--; if (idex < 0) { bouncedirection = 0; idex++; } }
      break;

    case 8: // Sparkle
      for (int i = 0; i < LEDS_COUNT; i++) {
        if (random8() < 5) leds[i] = CRGB::White;
        else leds[i].nscale8(220);
      }
      break;

    case 9: // Static warm white
      fill_solid(leds, LEDS_COUNT, CRGB(255, 160, 60));
      break;
  }
  FastLED.show();
}

void loop2 (void* pvParameters) {
  while (1) {
    ws2812_effect();
    delay(30);

    if (timer0) {
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
