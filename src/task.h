void getTemp2(byte i);
void narodmonUpdate();
void updateCryptoRates();

static byte hue = 0;
static byte idex = 0;
static int bouncedirection = 0;
static int phase = 0;
static byte fire_heat[LEDS_COUNT];
static int ball_pos[3];
static int ball_speed[3];
static byte ball_init = 0;
static byte ws2812_random_counter = 0;
static byte ws2812_rand_anim = 1;

void ws2812_effect() {
  bool ws2812_enable = mydata.ws2812_enable;
  bool ap_mode = mydata.ap_mode;
  uint8_t ws2812_brightness = mydata.ws2812_brightness;
  bool veml_enable = mydata.veml_enable;
  uint8_t ws2812_bright_val[4];
  memcpy(ws2812_bright_val, mydata.ws2812_bright_val, sizeof(ws2812_bright_val));
  uint8_t ws2812_br_ranges = mydata.ws2812_br_ranges;
  int ws2812_lux_min[4];
  memcpy(ws2812_lux_min, mydata.ws2812_lux_min, sizeof(ws2812_lux_min));
  uint8_t ws2812_anim = mydata.ws2812_anim;
  uint8_t anim_by_mode = mydata.anim_by_mode;
  uint8_t anim_time_mode = mydata.anim_time_mode;
  uint8_t anim_data_mode = mydata.anim_data_mode;
  uint8_t autoshow_min = mydata.autoshow_min;
  uint8_t autoshow_select_sec[7];
  memcpy(autoshow_select_sec, mydata.autoshow_select_sec, sizeof(mydata.autoshow_select_sec));
  int display = ::display;
  int displayState = ::displayState;

  if (!ws2812_enable && !ap_mode) {
    ws2812_set_brightness(0);
    fill_solid(leds, LEDS_COUNT, CRGB::Black);
  ws2812_show(leds, LEDS_COUNT, LEDS_PIN);
  if (ap_mode) hue = 0;
    return;
  }

  byte target_bright = ws2812_brightness;
  if (veml_enable) {
    byte found = ws2812_bright_val[0];
    byte num = ws2812_br_ranges;
    for (int i = 0; i < 4 && i < num; i++) {
      if ((int)vemllux >= ws2812_lux_min[i]) {
        found = ws2812_bright_val[i];
      }
    }
    target_bright = found;
  }
  ws2812_set_brightness(target_bright);

  byte anim = ws2812_anim;
  if (anim_by_mode) {
    if (displayState == MODE_TIME) anim = anim_time_mode;
    else anim = anim_data_mode;
  }
  if (anim == 0) {
    byte interval;
    if (display == 0) interval = max((byte)1, autoshow_min);
    else interval = max((byte)1, autoshow_select_sec[display]);
    static byte prev_display = 0;
    if (display != prev_display) {
      ws2812_random_counter = 0;
      byte next;
      do { next = random(1, 20); } while (next == ws2812_rand_anim);
      ws2812_rand_anim = next;
      prev_display = display;
    }
    ws2812_random_counter++;
    if (ws2812_random_counter >= interval * 33) {
      ws2812_random_counter = 0;
      byte next;
      do { next = random(1, 20); } while (next == ws2812_rand_anim);
      ws2812_rand_anim = next;
    }
    anim = ws2812_rand_anim;
  }

  if (ap_mode) {
    hue = 0;
    anim = 15;
  }

  switch (anim) {
    case 1: // Rainbow fade
      fill_rainbow(leds, LEDS_COUNT, hue, 7);
      hue++;
      break;

    case 2: // Running rainbow
      leds[idex] = CRGB(CHSV(hue, 255, 255));
      fadeToBlackBy(leds, LEDS_COUNT, 20);
      idex = (idex + 1) % LEDS_COUNT;
      hue += 4;
      break;

    case 3: // Random colors
      if (phase % 30 == 0)
        for (int i = 0; i < LEDS_COUNT; i++)
          leds[i] = CHSV(random(256), 255, 255);
      phase++;
      break;

    case 4: // Color bounce
      fadeToBlackBy(leds, LEDS_COUNT, 40);
      leds[idex] = CRGB(CHSV(hue, 255, 255));
      if (bouncedirection == 0) { idex++; if (idex >= LEDS_COUNT) { bouncedirection = 1; idex--; } }
      else { idex--; if (idex < 0) { bouncedirection = 0; idex++; } }
      hue += 2;
      break;

    case 5: { // Color bounce fade
      fadeToBlackBy(leds, LEDS_COUNT, 60);
      leds[idex] = CRGB(CHSV(hue, 255, 255));
      int iL1 = (idex + 1) % LEDS_COUNT;
      int iL2 = (idex + 2) % LEDS_COUNT;
      int iR1 = (idex - 1 + LEDS_COUNT) % LEDS_COUNT;
      int iR2 = (idex - 2 + LEDS_COUNT) % LEDS_COUNT;
      leds[iL1] = CRGB(CHSV(hue, 255, 150));
      leds[iR1] = CRGB(CHSV(hue, 255, 150));
      leds[iL2] = CRGB(CHSV(hue, 255, 60));
      leds[iR2] = CRGB(CHSV(hue, 255, 60));
      if (bouncedirection == 0) { idex++; if (idex >= LEDS_COUNT) { bouncedirection = 1; idex--; } }
      else { idex--; if (idex < 0) { bouncedirection = 0; idex++; } }
      hue += 2;
      break;
    }

    case 6: { // Fire
      for (int i = LEDS_COUNT - 1; i > 0; i--)
        fire_heat[i] = (fire_heat[i - 1] + fire_heat[i]) / 2;
      fire_heat[0] = random8(255);
      for (int i = 0; i < LEDS_COUNT; i++)
        leds[i] = HeatColor(fire_heat[i]);
      break;
    }

    case 7: // Twinkle
      for (int i = 0; i < LEDS_COUNT; i++) {
        if (random8() < 20) leds[i] = CHSV(random(256), 255, 255);
        else leds[i].nscale8(230);
      }
      break;

    case 8: // Strobe
      fill_solid(leds, LEDS_COUNT, (phase % 8 == 0) ? CRGB(CHSV(hue, 255, 255)) : CRGB::Black);
      hue += 8;
      phase++;
      break;


    case 9: // Sparkle
      for (int i = 0; i < LEDS_COUNT; i++) {
        if (random8() < 5) leds[i] = CRGB::White;
        else leds[i].nscale8(220);
      }
      break;

    case 10: // Static warm white
      fill_solid(leds, LEDS_COUNT, CRGB(255, 160, 60));
      break;

    case 11: // Theater chase
      for (int i = 0; i < LEDS_COUNT; i++)
        leds[i] = ((i + phase / 4) % 3 == 0) ? CRGB(CHSV(hue, 255, 255)) : CRGB::Black;
      phase++;
      hue++;
      break;

    case 12: // Theater chase rainbow
      for (int i = 0; i < LEDS_COUNT; i++)
        leds[i] = ((i + phase / 4) % 3 == 0) ? CRGB(CHSV(hue + i * 30, 255, 255)) : CRGB::Black;
      phase++;
      hue++;
      break;

    case 13: // Running lights
      for (int i = 0; i < LEDS_COUNT; i++) {
        int p = (i + phase / 2) * 64 / LEDS_COUNT;
        leds[i] = CHSV(hue, 255, sin8(p));
      }
      hue++;
      phase++;
      break;

    case 14: // Color wipe
      leds[(phase / 4) % LEDS_COUNT] = CRGB(CHSV(hue, 255, 255));
      if (phase % 4 == 0)
        if ((phase / 4) % LEDS_COUNT == 0) hue += 8;
      phase++;
      break;

    case 15: { // Pulse one color
  static uint8_t hp = 0;
  uint8_t b;
  if (hp < 8) b = sin8(hp * 32);
  else if (hp < 12) b = 0;
  else if (hp < 22) b = sin8((hp - 12) * 18);
  else if (hp < 40) b = 0;
  else b = 0, hp = 0;
  hp++;
  uint8_t cp = LEDS_COUNT / 2;
  for (int i = 0; i < LEDS_COUNT; i++) {
    uint8_t dist = abs(i - cp);
    uint8_t br = (b > dist * 40) ? b - dist * 40 : 0;
    leds[i] = CHSV(hue, 255, br);
  }
  if (hp % 8 == 0) hue++;
break;
    }

    case 16: { // Bouncing balls
      if (!ball_init) { ball_init = 1; for (int i = 0; i < 3; i++) { ball_pos[i] = i * 85; ball_speed[i] = 4 + i * 2; } }
      for (int i = 0; i < 3; i++) {
        ball_pos[i] += ball_speed[i];
        if (ball_pos[i] >= (LEDS_COUNT - 1) * 255) { ball_speed[i] = -ball_speed[i]; ball_pos[i] = (LEDS_COUNT - 1) * 255; }
        if (ball_pos[i] < 0) { ball_speed[i] = -ball_speed[i]; ball_pos[i] = 0; }
      }
      fadeToBlackBy(leds, LEDS_COUNT, 40);
      for (int i = 0; i < 3; i++) {
        byte idx = ball_pos[i] / 255;
        byte bright = ball_pos[i] % 255;
        leds[idx] += CHSV(i * 85, 255, bright);
      }
      break;
    }

    case 17: { // Pacman
      byte pos = (phase / 4) % (LEDS_COUNT + 3);
      byte on = (phase / 4) % (LEDS_COUNT * 2) < LEDS_COUNT;
      fadeToBlackBy(leds, LEDS_COUNT, 20);
      for (int i = 0; i < LEDS_COUNT; i++)
        if (i == pos && on) leds[i] = CRGB(CHSV(32, 255, 255));
      phase++;
      break;
    }

    case 18: // Matrix rain
      for (int i = 0; i < LEDS_COUNT; i++) {
        if (random8() < 10) leds[i] = CRGB::Green;
        else if (random8() < 30) leds[i] = CRGB(0, 50, 0);
        else leds[i].nscale8(200);
      }
      break;

case 19: // Snow sparkle - falling snow
{
  static uint8_t snow_mask = 0;
  static uint8_t snow_step = 0;
  snow_step++;
  if (snow_step >= 5) {
    snow_step = 0;
    snow_mask = (snow_mask << 1) & 0x3F;
    if (random(100) < 25) snow_mask |= 1;
  }
  fill_solid(leds, LEDS_COUNT, CHSV(160, 50, 80));
  for (int i = 0; i < LEDS_COUNT; i++)
    if (snow_mask & (1 << i)) leds[i] = CRGB::White;
  break;
}
  default: break;
  }
  ws2812_show(leds, LEDS_COUNT, LEDS_PIN);
}
void loop2 (void* pvParameters) {
  while (1) {
    if (offtime_active) {
      ledcWrite(PWM_CHANNEL, 0);
      if (ws2812_timer_flag) {
        ws2812_timer_flag = false;
        ws2812_set_brightness(0);
        fill_solid(leds, LEDS_COUNT, CRGB::Black);
        ws2812_show(leds, LEDS_COUNT, LEDS_PIN);
      }
    } else {
     if (vemlRead.isReady()) {
      float lux = veml.readLux();
        vemllux = lux;
        uint8_t bright_value = brigh_value_indi(vemllux, mydata.nixie_lux_min, mydata.nixie_lux_max, mydata.nixie_bright_val, prev_brigh_value);
        prev_brigh_value = bright_value;
        ledcWrite(PWM_CHANNEL, bright_value);
      }
    if (ws2812_timer_flag) {
      ws2812_timer_flag = false;
      ws2812_effect();
    }
    }

  if (alarm_state == ALARM_PLAYING) {
    if (alarmTimer.isReady()) {
      alarm_note_idx++;
      byte idx = mydata.alarm_melody_idx;
      if (idx >= MELODY_COUNT) idx = 0;
      MelodyNote n = melodies[idx][alarm_note_idx];
      if (n.freq == 0 && n.dur == 0) {
        if (mydata.alarm_duration > 0
            && millis() - alarm_start_ms < (unsigned long)mydata.alarm_duration * 1000UL) {
          alarm_note_idx = 0;
          n = melodies[idx][0];
          if (n.freq > 0) {
            ledcWriteTone(BUZZER_CHANNEL, n.freq);
            ledcWrite(BUZZER_CHANNEL, ((uint16_t)mydata.alarm_volume * 511UL) / 100UL);
          } else {
            ledcWrite(BUZZER_CHANNEL, 0);
          }
          alarmTimer.setInterval(n.dur);
          alarmTimer.reset();
        } else {
          ledcWrite(BUZZER_CHANNEL, 0);
          alarm_state = ALARM_DONE;
        }
      } else if (n.freq > 0) {
        ledcWriteTone(BUZZER_CHANNEL, n.freq);
        ledcWrite(BUZZER_CHANNEL, ((uint16_t)mydata.alarm_volume * 511UL) / 100UL);
        alarmTimer.setInterval(n.dur);
        alarmTimer.reset();
      } else {
        ledcWrite(BUZZER_CHANNEL, 0);
        alarmTimer.setInterval(n.dur);
        alarmTimer.reset();
      }
    }
  } else if (mydata.alarm_enable && alarm_state == ALARM_IDLE
      && hour == mydata.alarm_hour && minute == mydata.alarm_minute && second < 3) {
    alarm_note_idx = 0;
    alarm_state = ALARM_PLAYING;
    alarm_start_ms = millis();
    buzzer_state = IDLE;
    byte idx = mydata.alarm_melody_idx;
    if (idx >= MELODY_COUNT) idx = 0;
    ledcWrite(BUZZER_CHANNEL, 0);
    MelodyNote n = melodies[idx][0];
    if (n.freq > 0) {
      ledcWriteTone(BUZZER_CHANNEL, n.freq);
      ledcWrite(BUZZER_CHANNEL, ((uint16_t)mydata.alarm_volume * 511UL) / 100UL);
    }
    alarmTimer.setInterval(n.dur);
    alarmTimer.reset();
  } else if (alarm_state == ALARM_DONE
      && (hour != mydata.alarm_hour || minute != mydata.alarm_minute || second > 10)) {
    alarm_state = ALARM_IDLE;
  }

  if (alarm_state != ALARM_PLAYING && mydata.buzzer_enable) {
    if (buzzer_state == IDLE && minute % mydata.buzzer_interval == 0 && second == 0) {
      ledcSetup(BUZZER_CHANNEL, BUZZER_FREQ, PWM_RESOLUTION);
      ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
      ledcWrite(BUZZER_CHANNEL, 255);
      buzzer_state = ACTIVE;
      buzzerTimer.setInterval(mydata.buzzer_duration);
      buzzerTimer.reset();
    }
    if (buzzer_state == ACTIVE && buzzerTimer.isReady()) {
      ledcWrite(BUZZER_CHANNEL, 0);
      buzzer_state = COOLDOWN;
    }
    if (buzzer_state == COOLDOWN && second > 0) {
      buzzer_state = IDLE;
    }
  }

  if (alarm_state != ALARM_PLAYING) {
    if (OwmUpdateTimer.isReady() && WiFi.status() == WL_CONNECTED && strlen(mydata.owMapApiKey) > 0 && strlen(mydata.owCity) > 0) {
      log_add('I', "OWM update");
      getTemp2(0);
    }

    if (strlen(mydata.NarodmoonApi) > 0 && strlen(mydata.NarodmoonID) > 0) {
      narodmonUpdate();
    }

    if (CoinUpdateTimer.isReady() && WiFi.status() == WL_CONNECTED) {
      updateCryptoRates();
    }
  }

    vTaskDelay(2);
  }
}




