int getSlotValue(byte slotIdx)
{
  byte dropdownIdx = mydata.autoshow_select[slotIdx];
  if (dropdownIdx == 0) {
    log_add('D', "GETVAL slot=%d dropdown=0 sensor=none value=0", slotIdx);
    return 0;
  }

  int sensorIdx = dropdownIdx + 3;
  if (dropdownIdx >= 4) sensorIdx = dropdownIdx + 3;
  if (sensorIdx > 13) {
    log_add('W', "GETVAL slot=%d dropdown=%d invalid sensor=%d", slotIdx, dropdownIdx, sensorIdx);
    return 0;
  }

  int value = 0;
  switch (sensorIdx) {
    case 0: value = (int)mydata.nrd_sens[0]; break;
    case 1: value = (int)mydata.nrd_sens[1]; break;
    case 2: value = (int)mydata.nrd_sens[2]; break;
    case 3: value = (int)mydata.nrd_sens[3]; break;
    case 4: value = (int)optemperature; break;
    case 5: value = (int)oppressure; break;
    case 6: value = (int)ophumidity; break;
    case 7: value = (int)pricebtc; break;
    case 8: value = (int)priceeth; break;
    case 9: value = (int)usdRubRate; break;
    case 10: value = dayOfMonth * 10000 + (month + 1) * 100 + year; break;
    default: value = 0; break;
  }

  log_add('D', "GETVAL slot=%d dropdown=%d sensor=%d value=%d", slotIdx, dropdownIdx, sensorIdx, value);
  return value;
}

byte getConfiguredSlotCount()
{
  byte slots = mydata.autoshow_slots;
  if (slots > 5) slots = 5;
  return slots;
}

void prepareDisplayTarget(byte targetDisplay)
{
  if (targetDisplay == 0) {
    newhour = hour;
    newminute = minute;
    newsecond = second;
    SetNixieBufer();
    log_add('S', "TARGET time new=%02d:%02d:%02d", newhour, newminute, newsecond);
    return;
  }

  byte slots = getConfiguredSlotCount();
  if (targetDisplay < 1 || targetDisplay > slots) {
    log_add('W', "TARGET invalid display=%d slots=%d", targetDisplay, slots);
    targetDisplay = 0;
    newhour = hour;
    newminute = minute;
    newsecond = second;
    SetNixieBufer();
    return;
  }

  int raw_val = getSlotValue(targetDisplay);
  sensorDisplayValue = raw_val;
  int abs_val = abs(raw_val);
  sensorDisplayDigits = 1;
  int tmp_val = abs_val;
  while (tmp_val >= 10 && sensorDisplayDigits < 6) {
    tmp_val /= 10;
    sensorDisplayDigits++;
  }
  newhour = 0;
  newminute = 0;
  newsecond = 0;

  log_add('S', "TARGET sensor slot=%d dropdown=%d raw=%d digits=%d hold=%d",
          targetDisplay, mydata.autoshow_select[targetDisplay], raw_val,
          sensorDisplayDigits, mydata.autoshow_select_sec[targetDisplay]);
}

void loop()
{
  ArduinoOTA.handle();
  ui.tick();
  fd.tick();

  byte slots = getConfiguredSlotCount();

  if (mydata.autoshow_switch && slots > 0 && displayState == MODE_TIME && display == 0 &&
      off_effects == 0 && on_effects == 0 && !SwitchDisplayTimer.isRunning()) {
    startHoldTimerForCurrentDisplay();
  }

  if (!mydata.autoshow_switch && SwitchDisplayTimer.isRunning()) {
    SwitchDisplayTimer.stop();
    log_add('S', "AUTOSHOW disabled, timer stopped st=%s disp=%d", modeStateName(displayState), display);
  }

  if (mydata.autoshow_switch && SwitchDisplayTimer.isReady()) {
    uint32_t elapsed = (millis() - modeHoldStarted) / 1000UL;
    log_add('S', "HOLD_TIMER_READY st=%s disp=%d slots=%d elapsed=%lu off=%d on=%d C=%d",
            modeStateName(displayState), display, slots, (unsigned long)elapsed, off_effects, on_effects, Counter);

    if (displayState == MODE_TIME && display == 0 && slots > 0) {
      byte target = 1;
      prepareDisplayTarget(target);
      log_add('S', "SEQ TIME_HOLD -> TIME_OFF -> SENSOR%d_ON", target);
      beginModeOffTransition(0, target, MODE_TIME_OFF);
    } else if (displayState == MODE_SENSOR_HOLD && display >= 1 && display <= slots) {
      byte fromDisplay = display;
      byte target = fromDisplay + 1;
      if (target > slots) target = 0;
      prepareDisplayTarget(target);
      log_add('S', "SEQ SENSOR%d_HOLD -> SENSOR_OFF -> %s", fromDisplay, target == 0 ? "TIME_ON" : "SENSOR_ON");
      beginModeOffTransition(fromDisplay, target, MODE_SENSOR_OFF);
    } else {
      log_add('W', "HOLD_TIMER_READY ignored st=%s disp=%d slots=%d", modeStateName(displayState), display, slots);
      SwitchDisplayTimer.stop();
    }
  }

  static int lastCrossFadeSecond = -1;
  if (displayState == MODE_TIME && display == 0 && timeon) {
    if (mydata.seconds_switch) {
      switch (effects) {
        case 1:
          newhour = hour;
          newminute = minute;
          newsecond = second;
          SetNixieBufer();
          SwitchNumbers();
          break;
        case 2:
          if (lastCrossFadeSecond != second) {
            lastCrossFadeSecond = second;
            CrossFade();
          } else {
            SetNixie();
          }
          break;
        default:
          SetNixie();
          break;
      }
    } else {
      SetNixie();
    }
  }

  
        lamp_celsius_hv31 = false;
  lamp_percent_hv32 = false;
  lamp_dot_hv31 = false;
  lamp_plus_hv32 = false;
  if (!mydata.ap_mode && display > 0 && off_effects == 0 && on_effects == 0) {
    byte sel = mydata.autoshow_select[display];
    if (sel == 1) {
      lamp_celsius_hv31 = true;
      if (sensorDisplayValue >= 0) {
        lamp_plus_hv32 = true;
      } else {
        lamp_dot_hv31 = true;
      }
    }
    else if (sel == 3) lamp_percent_hv32 = true;
  }
  UpdateDisplay();
  switch_effects();

  if (timer1) {
    calculateTime();
    timer1 = false;

    {
  if (mydata.anim_by_mode) {
    if (displayState == MODE_TIME) effects = mydata.anim_time_mode;
    else effects = mydata.anim_data_mode;
  } else {
  static byte rotateCounter = 0;
  int oldfx = effects;
  if (mydata.anim_change == 0) {
    byte interval = mydata.anim_change_sec;
    if (interval == 0) interval = 30;
    rotateCounter++;
    if (rotateCounter >= interval) {
      rotateCounter = 0;
      effects++;
      if (effects >= 2) effects = 0;
      log_add('D', "EFF cycle: (int=%ds)", effects, interval);
    }
  } else {
    rotateCounter = 0;
    byte target;
    switch (mydata.anim_change) {
      case 1: target = 0; break;
      case 2: target = 1; break;
      case 3: target = 2; break;
      default: target = effects; break;
    }
    if (target != effects) {
      effects = target;
      log_add('D', "EFF set: (mode=%d)", effects, mydata.anim_change);
    }
  }
  }
}

    uint32_t holdElapsed = SwitchDisplayTimer.isRunning() ? ((millis() - modeHoldStarted) / 1000UL) : 0;
    log_add('T', "SEC %02d:%02d:%02d st=%s disp=%d to=%d fx=%d off=%d on=%d C=%d hold=%lu/%d run=%d",
            hour, minute, second, modeStateName(displayState), display, timeon, effects,
            off_effects, on_effects, Counter, (unsigned long)holdElapsed,
            display == 0 ? mydata.autoshow_min : mydata.autoshow_select_sec[display],
            SwitchDisplayTimer.isRunning());
  }

  if (SensorSelectTimer.isReady()) {
    rebuildSensorsAutoShowSelect();
  }

  if (mydata.ntp_sync_enable && NtpSyncTimer.isReady() && WiFi.status() == WL_CONNECTED) {
    log_add('I', "NTP sync");
    NTPClientUpdate();
    NtpSyncTimer.setInterval(mydata.ntp_sync_interval * 60000UL);
    NtpSyncTimer.start();
  }

  static unsigned long lastNtpRetry = 0;
  if (WiFi.status() == WL_CONNECTED && (year < 20 || year > 50) && millis() - lastNtpRetry > 30000) {
    lastNtpRetry = millis();
    log_add('W', "Bad time year=%d, forcing NTP sync", year);
    NTPClientUpdate();
    NtpSyncTimer.setInterval(mydata.ntp_sync_interval * 60000UL);
    NtpSyncTimer.start();
  }

  if (OwmUpdateTimer.isReady() && WiFi.status() == WL_CONNECTED && strlen(mydata.owMapApiKey) > 0 && strlen(mydata.owCity) > 0) {
    log_add('I', "OWM update");
    getTemp2(0);
  }

  if (CoinUpdateTimer.isReady() && WiFi.status() == WL_CONNECTED) {
    updateCryptoRates();
  }
}
