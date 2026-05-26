void CrossFade()
{
  int time1 = 4020;
  int time2 = 0;
  int hourchange, minutecgange, secondchange;
  int ct_sum = 60;
  int delta = 67;
  log_add('A', "XFADE start old=%02d:%02d:%02d new=%02d:%02d:%02d",
    old_hour, old_minute, old_second, hour, minute, second);

  for (int ct = 0; ct < ct_sum; ct++)
  {
    Nixie[0] = old_hour / 10;
    Nixie[1] = old_hour % 10;
    Nixie[2] = old_minute / 10;
    Nixie[3] = old_minute % 10;
    Nixie[4] = old_second / 10;
    Nixie[5] = old_second % 10;
    UpdateDisplay();
    delayMicroseconds(time1);
    time1 = time1 - delta;

    if ((display == 0) && timeon) {
      hourchange = hour;
      minutecgange = minute;
      secondchange = second;
    } else {
      hourchange = newhour;
      minutecgange = newminute;
      secondchange = newsecond;
    }

    Nixie[0] = hourchange / 10;
    Nixie[1] = hourchange % 10;
    Nixie[2] = minutecgange / 10;
    Nixie[3] = minutecgange % 10;
    Nixie[4] = secondchange / 10;
    Nixie[5] = secondchange % 10;
    UpdateDisplay();
    delayMicroseconds(time2);
    time2 = time2 + delta;
  }

  if ((display == 0) && timeon) {
    old_second = second;
    old_minute = minute;
    old_hour = hour;
  } else {
    old_second = newsecond;
    old_minute = newminute;
    old_hour = newhour;
  }
  log_add('A', "XFADE done Nixie=%d%d:%d%d:%d%d",
    Nixie[0], Nixie[1], Nixie[2], Nixie[3], Nixie[4], Nixie[5]);
}

void SwitchNumbers()
{
  if (flipInit) {
    flipInit = false;
    log_add('A', "SWNUM init Nixie=%d%d:%d%d:%d%d Buf=%d%d:%d%d:%d%d",
      Nixie[0], Nixie[1], Nixie[2], Nixie[3], Nixie[4], Nixie[5],
      NixieBuffer[0], NixieBuffer[1], NixieBuffer[2],
      NixieBuffer[3], NixieBuffer[4], NixieBuffer[5]);
    for (int i = 0; i < 6; i++) {
      if (Nixie[i] != NixieBuffer[i]) flipIndics[i] = true;
      else flipIndics[i] = false;
    }
  }

  if (ChangeCathodeTimer.isReady()) {
    int flipCounter = 0;
    for (int i = 0; i < 6; i++) {
      if (flipIndics[i]) {
        Nixie[i]--;
        if (Nixie[i] < 0) Nixie[i] = 9;
        if (Nixie[i] == NixieBuffer[i]) {
          flipIndics[i] = false;
          log_add('A', "SWNUM idx=%d reached target=%d", i, NixieBuffer[i]);
        }
      } else {
        flipCounter++;
      }
    }
    if (flipCounter == 6) flipInit = true;
  }
}

void shuffle(int* arr, int n)
{
  for (int i = n - 1; i > 0; i--) {
    int j = rand() % (i + 1);
    int tmp = arr[j];
    arr[j] = arr[i];
    arr[i] = tmp;
  }
}

void flip_nixiebuffer()
{
  if (display == 0) {
    newhour = hour; newminute = minute; newsecond = second;
    SetNixieBufer();
    log_add('A', "FLIPBUF time -> NixieBuf=%d%d:%d%d:%d%d",
      NixieBuffer[0], NixieBuffer[1], NixieBuffer[2],
      NixieBuffer[3], NixieBuffer[4], NixieBuffer[5]);
    return;
  }
  byte si = mydata.autoshow_select[display] - 1;
  if (si == 4) {
    NixieBuffer[5] = 10; NixieBuffer[4] = 10;
    NixieBuffer[3] = newminute % 10; NixieBuffer[2] = newminute / 10;
    NixieBuffer[1] = 10; NixieBuffer[0] = 10;
    log_add('A', "FLIPBUF temp -> NixieBuf=%d%d:%d%d:%d%d",
      NixieBuffer[0], NixieBuffer[1], NixieBuffer[2],
      NixieBuffer[3], NixieBuffer[4], NixieBuffer[5]);
  } else {
    SetNixieBufer();
    log_add('A', "FLIPBUF sensor=%d -> NixieBuf=%d%d:%d%d:%d%d",
      si,
      NixieBuffer[0], NixieBuffer[1], NixieBuffer[2],
      NixieBuffer[3], NixieBuffer[4], NixieBuffer[5]);
  }
}

void switch_effects(){
  static int timeon_log = 0;
  if (mooveNixie.isReady())
  {
    // OFF effects
    if (off_effects == 1) {
      for (byte i = 0; i < Counter; i++) Nixie[i] = NixieBuffer[(5 - Counter) + 1 + i];
      Nixie[Counter] = 10;
      Counter--;
      log_add('A', "OFF1 C=%d Nixie=%d%d:%d%d:%d%d", Counter+1,
        Nixie[0], Nixie[1], Nixie[2], Nixie[3], Nixie[4], Nixie[5]);
    }
    if (off_effects == 2) {
      for (byte i = 1; i < Counter; i++) Nixie[(5 - Counter) + i] = NixieBuffer[i - 1];
      Nixie[5 - Counter] = 10;
      Counter--;
      log_add('A', "OFF2 C=%d Nixie=%d%d:%d%d:%d%d", Counter+1,
        Nixie[0], Nixie[1], Nixie[2], Nixie[3], Nixie[4], Nixie[5]);
    }
    if (off_effects == 3) {
      if (NixieBuffer[Counter] >= 10) { Nixie[Counter] = 10; Counter--; }
      else {
        Nixie[Counter]--;
        if (Nixie[Counter] < 0) Nixie[Counter] = 9;
        if (Nixie[Counter] == NixieBuffer[Counter]) { Nixie[Counter] = 10; Counter--; }
      }
      log_add('A', "OFF3 C=%d Nixie=%d%d:%d%d:%d%d", Counter,
        Nixie[0], Nixie[1], Nixie[2], Nixie[3], Nixie[4], Nixie[5]);
    }
    if (off_effects == 4) {
      if (NixieBuffer[5-Counter] >= 10) { Nixie[5-Counter] = 10; Counter--; }
      else {
        Nixie[5-Counter]--;
        if (Nixie[5-Counter] < 0) Nixie[5-Counter] = 9;
        if (Nixie[5-Counter] == NixieBuffer[5-Counter]) { Nixie[5-Counter] = 10; Counter--; }
      }
      log_add('A', "OFF4 C=%d Nixie=%d%d:%d%d:%d%d", Counter,
        Nixie[0], Nixie[1], Nixie[2], Nixie[3], Nixie[4], Nixie[5]);
    }
    if (off_effects == 5) {
      if (flip_switch) { flip_switch = false; shuffle(rand_arr, 6); }
      int idx = rand_arr[Counter];
      if (NixieBuffer[idx] >= 10) { Nixie[idx] = 10; Counter--; }
      else {
        Nixie[idx]--;
        if (Nixie[idx] < 0) Nixie[idx] = 9;
        if (Nixie[idx] == NixieBuffer[idx]) { Nixie[idx] = 10; Counter--; }
      }
      log_add('A', "OFF5 C=%d rand=%d Nixie=%d%d:%d%d:%d%d", Counter, idx,
        Nixie[0], Nixie[1], Nixie[2], Nixie[3], Nixie[4], Nixie[5]);
    }

    // ON effects
    if (on_effects == 1) {
      for (byte i = 0; i < Counter; i++) Nixie[(5 - Counter + 1) + i] = NixieBuffer[i];
      Counter++;
      log_add('A', "ON1 C=%d Nixie=%d%d:%d%d:%d%d", Counter-1,
        Nixie[0], Nixie[1], Nixie[2], Nixie[3], Nixie[4], Nixie[5]);
      if (Counter > 6) {
        for (int i = 0; i < 6; i++) Nixie[i] = NixieBuffer[i];
        flip_switch = true; on_effects = 0; timeon = (display == 0);
        log_add('A', "ON_DONE on=1 timeon=%d Nixie=%d%d:%d%d:%d%d", timeon,
          Nixie[0], Nixie[1], Nixie[2], Nixie[3], Nixie[4], Nixie[5]);
      }
    }
    if (on_effects == 2) {
      for (byte i = 0; i < Counter; i++) Nixie[i] = NixieBuffer[(5 - Counter + 1) + i];
      Counter++;
      log_add('A', "ON2 C=%d Nixie=%d%d:%d%d:%d%d", Counter-1,
        Nixie[0], Nixie[1], Nixie[2], Nixie[3], Nixie[4], Nixie[5]);
      if (Counter > 6) {
        for (int i = 0; i < 6; i++) Nixie[i] = NixieBuffer[i];
        flip_switch = true; on_effects = 0; timeon = (display == 0);
        log_add('A', "ON_DONE on=2 timeon=%d Nixie=%d%d:%d%d:%d%d", timeon,
          Nixie[0], Nixie[1], Nixie[2], Nixie[3], Nixie[4], Nixie[5]);
      }
    }
    if (on_effects == 3) {
      Nixie[Counter]--;
      if (Nixie[Counter] < 0) Nixie[Counter] = 9;
      if (Nixie[Counter] == NixieBuffer[Counter]) Counter++;
      log_add('A', "ON3 C=%d Nixie=%d%d:%d%d:%d%d", Counter,
        Nixie[0], Nixie[1], Nixie[2], Nixie[3], Nixie[4], Nixie[5]);
    }
    if (on_effects == 4) {
      Nixie[5-Counter]--;
      if (Nixie[5-Counter] < 0) Nixie[5-Counter] = 9;
      if (Nixie[5-Counter] == NixieBuffer[5-Counter]) Counter++;
      log_add('A', "ON4 C=%d Nixie=%d%d:%d%d:%d%d", Counter,
        Nixie[0], Nixie[1], Nixie[2], Nixie[3], Nixie[4], Nixie[5]);
    }
    if (on_effects == 5) {
      if (flip_switch) { flip_switch = false; shuffle(rand_arr, 6); }
      Nixie[rand_arr[Counter]]--;
      if (Nixie[rand_arr[Counter]] < 0) Nixie[rand_arr[Counter]] = 9;
      if (Nixie[rand_arr[Counter]] == NixieBuffer[rand_arr[Counter]]) Counter++;
      log_add('A', "ON5 C=%d rand=%d Nixie=%d%d:%d%d:%d%d", Counter, rand_arr[Counter],
        Nixie[0], Nixie[1], Nixie[2], Nixie[3], Nixie[4], Nixie[5]);
    }

    // Effect completion
    if (Counter == 0) {
      if (off_effects > 0) log_add('A', "OFF_DONE off=%d", off_effects);
      off_effects = 0;
      flip_switch = true;
      flip = true;
      log_add('A', "FLIPBUF call display=%d", display);
      flip_nixiebuffer();
      on_effects = random(1, 6);
      if (on_effects == 2) mooveNixie.setInterval(100);
      if (on_effects == 3) mooveNixie.setInterval(20);
      log_add('A', "ON_START on=%d", on_effects);
    }
    if (Counter == 6 && on_effects > 0) {
      if (on_effects > 0) log_add('A', "ON_DONE on=%d timeon=%d", on_effects, timeon);
      for (int i = 0; i < 6; i++) Nixie[i] = NixieBuffer[i];
      flip_switch = true; on_effects = 0; timeon = (display == 0);
      log_add('A', "FORCE Nixie=Buffer Nixie=%d%d:%d%d:%d%d",
        Nixie[0], Nixie[1], Nixie[2], Nixie[3], Nixie[4], Nixie[5]);
    }
  }
}