void loop2 (void* pvParameters) {
  while (1) {
    if (WS2812.isReady()) {
      if (mydata.ws2812_enable) {
        strip.setBrightness(mydata.ws2812_brightness);
        switch (mydata.ws2812_anim) {
          case 0: // Rainbow
            for (int i = 0; i < LEDS_COUNT; i++)
              strip.setLedColorData(i, strip.Wheel((i * 256 / LEDS_COUNT + WS2812_j) & 255));
            WS2812_j += 2;
            if (WS2812_j >= 255) WS2812_j = 0;
            break;

          case 1: // Static warm white
            for (int i = 0; i < LEDS_COUNT; i++)
              strip.setLedColorData(i, 255, 160, 60);
            break;

          case 2: // Breathing
            {
              int phase = ws2812_phase % 256;
              float f = sin(phase * 3.14159 / 128);
              int bright = (f + 1) * 127;
              if (bright > 255) bright = 255;
              if (bright < 0) bright = 0;
              int r = 255 * bright / 255;
              int g = 160 * bright / 255;
              int b = 60 * bright / 255;
              for (int i = 0; i < LEDS_COUNT; i++)
                strip.setLedColorData(i, r, g, b);
              ws2812_phase += 4;
            }
            break;

          case 3: // Color wipe
            {
              int pos = (ws2812_phase / 8) % (LEDS_COUNT + 4);
              for (int i = 0; i < LEDS_COUNT; i++) {
                if (i == pos) strip.setLedColorData(i, 255, 160, 60);
                else strip.setLedColorData(i, 0, 0, 0);
              }
              ws2812_phase++;
            }
            break;

          case 4: // Theater chase
            {
              int shift = ws2812_phase / 4;
              for (int i = 0; i < LEDS_COUNT; i++) {
                if (((i + shift) % 3) == 0) strip.setLedColorData(i, 255, 160, 60);
                else strip.setLedColorData(i, 0, 0, 0);
              }
              ws2812_phase++;
            }
            break;
        }
        strip.show();
      } else {
        strip.setBrightness(0);
        for (int i = 0; i < LEDS_COUNT; i++)
          strip.setLedColorData(i, 0, 0, 0);
        strip.show();
      }
    }

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
