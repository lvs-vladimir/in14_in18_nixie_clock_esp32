void CrossFade()
{
  int time1 = 4020;
  int time2 = 0;
  int hourchange, minutecgange, secondchange;
  int ct_sum = 60;
  int delta = 67;

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
}

void SwitchNumbers()
{
  if (flipInit) {
    flipInit = false;
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
        if (Nixie[i] == NixieBuffer[i]) flipIndics[i] = false;
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
    return;
  }
  byte si = mydata.autoshow_select[display] - 1;
  if (si == 4) {
    NixieBuffer[5] = 10; NixieBuffer[4] = 10;
    NixieBuffer[3] = newminute % 10; NixieBuffer[2] = newminute / 10;
    NixieBuffer[1] = 10; NixieBuffer[0] = 10;
  } else {
    SetNixieBufer();
  }
}

void switch_effects(){
  static int timeon_log = 0;
  if (mooveNixie.isReady())
  {
    if (off_effects == 1) {
      for (byte i = 0; i < Counter; i++) Nixie[i] = NixieBuffer[(5 - Counter) + 1 + i];
      Nixie[Counter] = 10;
      Counter--;
    }
    if (off_effects == 2) {
      for (byte i = 1; i < Counter; i++) Nixie[(5 - Counter) + i] = NixieBuffer[i - 1];
      Nixie[5 - Counter] = 10;
      Counter--;
    }
    if (off_effects == 3) {
      Nixie[Counter]--;
      if (Nixie[Counter] < 0) Nixie[Counter] = 9;
      if (Nixie[Counter] == NixieBuffer[Counter]) { Nixie[Counter] = 10; Counter--; }
    }
    if (off_effects == 4) {
      Nixie[5-Counter]--;
      if (Nixie[5-Counter] < 0) Nixie[5-Counter] = 9;
      if (Nixie[5-Counter] == NixieBuffer[5-Counter]) { Nixie[5-Counter] = 10; Counter--; }
    }
    if (off_effects == 5) {
      if (flip_switch) { flip_switch = false; shuffle(rand_arr, 6); }
      Nixie[rand_arr[Counter]]--;
      if (Nixie[rand_arr[Counter]] < 0) Nixie[rand_arr[Counter]] = 9;
      if (Nixie[rand_arr[Counter]] == NixieBuffer[rand_arr[Counter]]) { Nixie[rand_arr[Counter]] = 10; Counter--; }
    }

    if (on_effects == 1) {
      for (byte i = 0; i < Counter; i++) Nixie[(5 - Counter) + i] = NixieBuffer[i];
      Counter++;
    }
    if (on_effects == 2) {
      for (byte i = 0; i < Counter; i++) Nixie[i] = NixieBuffer[(5 - Counter) + i];
      Counter++;
    }
    if (on_effects == 3) {
      Nixie[Counter]--;
      if (Nixie[Counter] < 0) Nixie[Counter] = 9;
      if (Nixie[Counter] == NixieBuffer[Counter]) Counter++;
    }
    if (on_effects == 4) {
      Nixie[5-Counter]--;
      if (Nixie[5-Counter] < 0) Nixie[5-Counter] = 9;
      if (Nixie[5-Counter] == NixieBuffer[5-Counter]) Counter++;
    }
    if (on_effects == 5) {
      if (flip_switch) { flip_switch = false; shuffle(rand_arr, 6); }
      Nixie[rand_arr[Counter]]--;
      if (Nixie[rand_arr[Counter]] < 0) Nixie[rand_arr[Counter]] = 9;
      if (Nixie[rand_arr[Counter]] == NixieBuffer[rand_arr[Counter]]) Counter++;
    }

    if (Counter == 0) {
      if (off_effects > 0) log_add('D', "OFF_DONE off=%d on=%d", off_effects, on_effects);
      off_effects = 0;
      flip_switch = true;
      log_add('D', "EFF flip NixieBuffer display=%d", display);
      flip_nixiebuffer();
      on_effects = random(1, 6);
      if (on_effects == 2) mooveNixie.setInterval(100);
      if (on_effects == 3) mooveNixie.setInterval(20);
      log_add('D', "ON_START on=%d", on_effects);
    }
    if (Counter == 6) {
      if (on_effects > 0) log_add('D', "ON_DONE on=%d timeon=%d", on_effects, timeon);
      flip_switch = true;
      on_effects = 0;
      timeon = (display == 0);
      if (display == 0 && ++timeon_log % 100 == 0) log_add('T', "TIMEON disp=0");
    }
  }
}