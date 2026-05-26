void loop()
{
  ArduinoOTA.handle();
  ui.tick();
  fd.tick();

  if (display == 0 && second >= 30 && mydata.autoshow_switch && !SwitchDisplayTimer.isRunning()) {
    SwitchDisplayTimer.setInterval(mydata.autoshow_min * 1000UL);
    SwitchDisplayTimer.start();
  }

  if (SwitchDisplayTimer.isReady())
  {
    display++;
    if (display > 3) display = 0;
    if (display > 0) {
      timeon = false;
      flip = true;
      off_effects = random(1, 6);
      if (off_effects == 2) mooveNixie.setInterval(100);
      if (off_effects == 3) mooveNixie.setInterval(20);
    }

    int num = 0, num1 = 0, num2 = 0, num3 = 0, num4 = 0, num5 = 0, num6 = 0;
    if (display == 1) num = pricebtc;
    if (display == 2) num = priceeth;
    if (display == 3) num = TempValue;
    if (display >= 1 && display <= 3) {
      num1 = num % 10; num /= 10;
      num2 = num % 10; num /= 10;
      num3 = num % 10; num /= 10;
      num4 = num % 10; num /= 10;
      num5 = num % 10; num /= 10;
      num6 = num % 10;
      newhour = (num6 * 10) + num5;
      newminute = (num2 * 10) + num1;
      newsecond = (num4 * 10) + num3;
    }

    switch (display) {
      case 0:
        SwitchDisplayTimer.setInterval(mydata.autoshow_min * 1000UL);
        if (timeon) {
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
      case 1:
        SwitchDisplayTimer.setInterval(mydata.autoshow_select_sec[1] * 1000UL);
        if (pricebtc > 0) {
          if (flip) { SetNixieBufer(); flip = false; }
          newhour = day; newminute = month; newsecond = year;
        } else { display++; SwitchDisplayTimer.setInterval(10); }
        break;
      case 2:
        SwitchDisplayTimer.setInterval(mydata.autoshow_select_sec[2] * 1000UL);
        if (priceeth > 0) {
          if (flip) {
            newhour = (num6 * 10) + num5;
            newminute = (num4 * 10) + num3;
            newsecond = (num2 * 10) + num1;
            SetNixieBufer(); flip = false;
          }
        } else { display++; SwitchDisplayTimer.setInterval(10); }
        break;
      case 3:
        SwitchDisplayTimer.setInterval(mydata.autoshow_select_sec[3] * 1000UL);
        if (flip) {
          NixieBuffer[5] = 10; NixieBuffer[4] = 10;
          NixieBuffer[3] = newminute % 10; NixieBuffer[2] = newminute / 10;
          NixieBuffer[1] = 10; NixieBuffer[0] = 10;
          flip = false;
        }
        break;
    }

    if (oldminute != minute) {
      switch (mydata.anim_change) {
        case 0: effects++; if (effects > 2) effects = 0; break;
        case 1: effects = 0; break;
        case 2: effects = 1; break;
        case 3: effects = 2; break;
      }
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

  if (timer1) { calculateTime(); timer1 = false; }

  if (SensorSelectTimer.isReady()) {
    SensorsAutoShowSelect2 = "";
    for (byte k = 0; k <= 13; k++) SensorsAutoShowSelect2 += SensorsAutoShow[k];
  }

  if (NtpSyncTimer.isReady() && WiFi.status() == WL_CONNECTED) NTPClientUpdate();
  if (OwmUpdateTimer.isReady() && WiFi.status() == WL_CONNECTED && strlen(mydata.owMapApiKey) > 0 && strlen(mydata.owCity) > 0) getTemp2(0);
}