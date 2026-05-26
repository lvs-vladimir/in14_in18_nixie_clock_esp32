int getSlotValue(byte slotIdx)
{
  byte dropdownIdx = mydata.autoshow_select[slotIdx];
  if (dropdownIdx == 0) return 0;
  int sensorIdx = dropdownIdx - 1;
  if (sensorIdx > 13) return 0;
  log_add('D', "GETVAL slot=%d dropdown=%d sensor=%d", slotIdx, dropdownIdx, sensorIdx);
  switch (sensorIdx) {
    case 0: return (int)mydata.nrd_sens[0];
    case 1: return (int)mydata.nrd_sens[1];
    case 2: return (int)mydata.nrd_sens[2];
    case 3: return (int)mydata.nrd_sens[3];
    case 4: return (int)TempValue;
    case 5: return (int)oppressure;
    case 6: return (int)ophumidity;
    case 7: return (int)pricebtc;
    case 8: return (int)priceeth;
    default: return 0;
  }
}

void loop()
{
  ArduinoOTA.handle();
  ui.tick();
  fd.tick();

  if (display == 0 && second >= 30 && mydata.autoshow_switch && !SwitchDisplayTimer.isRunning()) {
    if (mydata.autoshow_min >= 1) {
      SwitchDisplayTimer.setInterval(mydata.autoshow_min * 1000UL);
      SwitchDisplayTimer.start();
      log_add('I', "AUTOSHOW start: %d sec", mydata.autoshow_min);
    }
  }

  if (SwitchDisplayTimer.isReady())
  {
    int olddisp = display;
    display++;
    if (display > 3) display = 0;
    if (display > mydata.autoshow_slots) display = 0;
    log_add('D', "DISP: %d->%d slots=%d", olddisp, display, mydata.autoshow_slots);

    if (display > 0) {
      timeon = false;
      flip = true;
      Counter = 5;
      off_effects = random(1, 6);
      log_add('D', "OFF_EFF: %d Counter:%d", off_effects, Counter);
      if (off_effects == 2) mooveNixie.setInterval(100);
      if (off_effects == 3) mooveNixie.setInterval(20);
    }
    if (display == 0) log_add('D', "CYCLE complete, back to time");

    int raw_val = 0;
    if (display >= 1 && display <= mydata.autoshow_slots) {
      raw_val = getSlotValue(display);
    }

    int num = raw_val, num1 = 0, num2 = 0, num3 = 0, num4 = 0, num5 = 0, num6 = 0;
    if (display >= 1 && display <= mydata.autoshow_slots) {
      num1 = num % 10; num /= 10;
      num2 = num % 10; num /= 10;
      num3 = num % 10; num /= 10;
      num4 = num % 10; num /= 10;
      num5 = num % 10; num /= 10;
      num6 = num % 10;
      newhour = (num6 * 10) + num5;
      newminute = (num2 * 10) + num1;
      newsecond = (num4 * 10) + num3;
      log_add('D', "DATA: slot=%d sens=%d raw=%d new=%02d:%02d:%02d", display, mydata.autoshow_select[display], raw_val, newhour, newminute, newsecond);
    }

    switch (display) {
      case 0:
        SwitchDisplayTimer.setInterval(mydata.autoshow_min * 1000UL);
        if (timeon) {
          newhour = hour; newminute = minute; newsecond = second;
          SetNixieBufer();
          on_effects = 0; Counter = 6;
        } else {
          timeon = true; flip = true;
          newhour = hour; newminute = minute; newsecond = second;
          SetNixieBufer();
          on_effects = 0; Counter = 6;
        }
        if (flip) {
          newhour = hour; newminute = minute; newsecond = second;
          SetNixieBufer();
          flip = false;
        }
        break;
      default:
        if (display >= 1 && display <= mydata.autoshow_slots) {
          SwitchDisplayTimer.setInterval(mydata.autoshow_select_sec[display] * 1000UL);
          if (flip) {
            flip_nixiebuffer();
            flip = false;
          }
        } else {
          display = 0;
          SwitchDisplayTimer.stop();
        }
        break;
    }

    if (oldminute != minute) {
      int oldfx = effects;
      switch (mydata.anim_change) {
        case 0: effects++; if (effects > 2) effects = 0; break;
        case 1: effects = 0; break;
        case 2: effects = 1; break;
        case 3: effects = 2; break;
      }
      if (effects != oldfx) log_add('D', "EFF: %d->%d (mode=%d)", oldfx, effects, mydata.anim_change);
      oldminute = minute;
    }
  }

  switch (effects) {
    case 0:
      if (display == 0 && timeon) SetNixie();
      break;
    case 1:
      if (display == 0 && timeon) SwitchNumbers();
      break;
    case 2:
      if (display == 0 && timeon) CrossFade();
      break;
  }
  UpdateDisplay();
  switch_effects();

  if (timer1) {
    calculateTime();
    timer1 = false;
    if (second % 10 == 0) log_add('T', "TIM: %02d:%02d:%02d disp=%d to=%d fx=%d off=%d on=%d C=%d",
      hour, minute, second, display, timeon, effects, off_effects, on_effects, Counter);
  }

  if (SensorSelectTimer.isReady()) {
    SensorsAutoShowSelect2 = "";
    for (byte k = 0; k <= 13; k++) SensorsAutoShowSelect2 += SensorsAutoShow[k];
  }

  if (NtpSyncTimer.isReady() && WiFi.status() == WL_CONNECTED) {
    log_add('I', "NTP sync"); NTPClientUpdate();
  }
  if (OwmUpdateTimer.isReady() && WiFi.status() == WL_CONNECTED && strlen(mydata.owMapApiKey) > 0 && strlen(mydata.owCity) > 0) {
    log_add('I', "OWM update"); getTemp2(0);
  }
}