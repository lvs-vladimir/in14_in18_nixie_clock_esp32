void CrossFade()
{
  int time1 = 4020;
  int time2 = 0;
  int hourchange, minutecgange, secondchange;
  int ct_sum = 60;
  int delta = 67;

  log_add('A', "XFADE start old=%02d:%02d:%02d new=%02d:%02d:%02d",
          old_hour, old_minute, old_second, hour, minute, second);

  for (int ct = 0; ct < ct_sum; ct++) {
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
    log_add('A', "SWNUM init N=%d%d:%d%d:%d%d B=%d%d:%d%d:%d%d",
            Nixie[0], Nixie[1], Nixie[2], Nixie[3], Nixie[4], Nixie[5],
            NixieBuffer[0], NixieBuffer[1], NixieBuffer[2], NixieBuffer[3], NixieBuffer[4], NixieBuffer[5]);
    for (int i = 0; i < 6; i++) flipIndics[i] = (Nixie[i] != NixieBuffer[i]);
  }

  if (ChangeCathodeTimer.isReady()) {
    int flipCounter = 0;
    for (int i = 0; i < 6; i++) {
      if (flipIndics[i]) {
        if (NixieBuffer[i] >= 10) {
          Nixie[i] = 10;
          flipIndics[i] = false;
          log_add('A', "SWNUM idx=%d blank", i);
        } else {
          if (Nixie[i] >= 10) Nixie[i] = 9;
          else Nixie[i]--;
          if (Nixie[i] < 0) Nixie[i] = 9;
          if (Nixie[i] == NixieBuffer[i]) {
            flipIndics[i] = false;
            log_add('A', "SWNUM idx=%d target=%d", i, NixieBuffer[i]);
          }
        }
      } else {
        flipCounter++;
      }
    }
    if (flipCounter == 6) flipInit = true;
  }
}

void shuffle(int* arr, int len)
{
  for (int i = len - 1; i > 0; i--) {
    int j = rand() % (i + 1);
    int tmp = arr[j];
    arr[j] = arr[i];
    arr[i] = tmp;
  }
}

void flip_nixiebuffer()
{
  if (display == 0) {
    newhour = hour;
    newminute = minute;
    newsecond = second;
    SetNixieBufer();
    log_add('A', "FLIPBUF time B=%d%d:%d%d:%d%d new=%02d:%02d:%02d",
            NixieBuffer[0], NixieBuffer[1], NixieBuffer[2], NixieBuffer[3], NixieBuffer[4], NixieBuffer[5],
            newhour, newminute, newsecond);
    return;
  }

  byte dropdownIdx = mydata.autoshow_select[display];
  byte si = dropdownIdx > 0 ? dropdownIdx - 1 : 0;
  if (si == 4) {
    NixieBuffer[0] = 10;
    NixieBuffer[1] = 10;
    NixieBuffer[2] = newminute / 10;
    NixieBuffer[3] = newminute % 10;
    NixieBuffer[4] = 10;
    NixieBuffer[5] = 10;
    log_add('A', "FLIPBUF temp slot=%d raw=%02d B=%d%d:%d%d:%d%d",
            display, newminute,
            NixieBuffer[0], NixieBuffer[1], NixieBuffer[2], NixieBuffer[3], NixieBuffer[4], NixieBuffer[5]);
  } else {
    SetNixieBufer();
    log_add('A', "FLIPBUF sensor slot=%d sensor=%d B=%d%d:%d%d:%d%d new=%02d:%02d:%02d",
            display, si,
            NixieBuffer[0], NixieBuffer[1], NixieBuffer[2], NixieBuffer[3], NixieBuffer[4], NixieBuffer[5],
            newhour, newminute, newsecond);
  }
}

const char* modeStateName(DisplayModeState state)
{
  switch (state) {
    case MODE_TIME: return "TIME_HOLD";
    case MODE_TIME_OFF: return "TIME_OFF";
    case MODE_SENSOR_ON: return "SENSOR_ON";
    case MODE_SENSOR_HOLD: return "SENSOR_HOLD";
    case MODE_SENSOR_OFF: return "SENSOR_OFF";
    case MODE_TIME_ON: return "TIME_ON";
  }
  return "UNKNOWN";
}

void logModeDigits(const char* tag)
{
  log_add('S', "%s st=%s disp=%d next=%d tr=%d->%d off=%d on=%d C=%d to=%d N=%d%d:%d%d:%d%d B=%d%d:%d%d:%d%d",
          tag, modeStateName(displayState), display, nextDisplay, transitionFromDisplay, transitionToDisplay,
          off_effects, on_effects, Counter, timeon,
          Nixie[0], Nixie[1], Nixie[2], Nixie[3], Nixie[4], Nixie[5],
          NixieBuffer[0], NixieBuffer[1], NixieBuffer[2], NixieBuffer[3], NixieBuffer[4], NixieBuffer[5]);
}

byte pickDisplayModeEffect()
{
  if (mydata.animdots == 0) return random(1, 8);
  if (mydata.animdots > 7) return 1;
  return mydata.animdots;
}

const char* modeAnimName(byte effect)
{
  switch (effect) {
    case 1: return "LTR";
    case 2: return "RTL";
    case 3: return "CENTER";
    case 4: return "OUTWARD";
    case 5: return "RANDOM";
    case 6: return "EVEN_ODD";
    case 7: return "ODD_EVEN";
  }
  return "UNKNOWN";
}

void setMoveIntervalForEffect(byte effect)
{
  mooveNixie.setInterval(120);
  if (effect == 5) mooveNixie.setInterval(90);
  if (effect == 6 || effect == 7) mooveNixie.setInterval(110);
}

void startHoldTimerForCurrentDisplay()
{
  uint32_t intervalSec = (display == 0) ? mydata.autoshow_min : mydata.autoshow_select_sec[display];
  if (intervalSec < 1) intervalSec = 1;
  SwitchDisplayTimer.setInterval(intervalSec * 1000UL);
  SwitchDisplayTimer.start();
  modeHoldStarted = millis();
  log_add('S', "HOLD_START st=%s disp=%d sec=%lu", modeStateName(displayState), display, (unsigned long)intervalSec);
  logModeDigits("HOLD_START_DIGITS");
}

void beginModeOffTransition(byte fromDisplay, byte toDisplay, DisplayModeState offState)
{
  transitionFromDisplay = fromDisplay;
  transitionToDisplay = toDisplay;
  nextDisplay = toDisplay;
  display = toDisplay;
  displayState = offState;
  timeon = false;
  flip = true;
  flip_switch = true;
  on_effects = 0;
  off_effects = pickDisplayModeEffect();
  activeTransitionEffect = off_effects;
  Counter = 0;
  SwitchDisplayTimer.stop();
  setMoveIntervalForEffect(off_effects);
  log_add('S', "OFF_START st=%s from=%d to=%d off=%d/%s menu=%d", modeStateName(displayState), fromDisplay, toDisplay, off_effects, modeAnimName(off_effects), mydata.animdots);
  logModeDigits("OFF_START_DIGITS");
}

int transitionIndexForEffect(byte effect, int step)
{
  static const byte centerOrder[6] = {0, 5, 1, 4, 2, 3};
  static const byte outwardOrder[6] = {2, 3, 1, 4, 0, 5};
  static const byte evenOddOrder[6] = {0, 2, 4, 1, 3, 5};
  static const byte oddEvenOrder[6] = {1, 3, 5, 0, 2, 4};

  if (effect == 2) return 5 - step;
  if (effect == 3) return centerOrder[step];
  if (effect == 4) return outwardOrder[step];
  if (effect == 5) return rand_arr[step];
  if (effect == 6) return evenOddOrder[step];
  if (effect == 7) return oddEvenOrder[step];
  return step;
}

void prepareNextOnEffect()
{
  flip_nixiebuffer();
  on_effects = off_effects ? off_effects : activeTransitionEffect;
  if (on_effects == 0) on_effects = pickDisplayModeEffect();
  activeTransitionEffect = on_effects;
  Counter = 0;
  flip_switch = true;
  setMoveIntervalForEffect(on_effects);
  displayState = (display == 0) ? MODE_TIME_ON : MODE_SENSOR_ON;
  log_add('S', "ON_START st=%s from=%d to=%d on=%d/%s menu=%d", modeStateName(displayState), transitionFromDisplay, transitionToDisplay, on_effects, modeAnimName(on_effects), mydata.animdots);
  logModeDigits("ON_START_DIGITS");
}

void completeOnEffect()
{
  for (int i = 0; i < 6; i++) Nixie[i] = NixieBuffer[i];
  on_effects = 0;
  Counter = 0;
  flip_switch = true;
  timeon = (display == 0);
  displayState = (display == 0) ? MODE_TIME : MODE_SENSOR_HOLD;
  log_add('S', "ON_DONE st=%s disp=%d timeon=%d", modeStateName(displayState), display, timeon);
  logModeDigits("ON_DONE_DIGITS");
  startHoldTimerForCurrentDisplay();
}

void switch_effects()
{
  if (!mooveNixie.isReady()) return;

  if (off_effects > 0) {
    if (Counter == 0 && off_effects == 5 && flip_switch) {
      flip_switch = false;
      shuffle(rand_arr, 6);
      log_add('A', "OFF_ORDER random=%d,%d,%d,%d,%d,%d", rand_arr[0], rand_arr[1], rand_arr[2], rand_arr[3], rand_arr[4], rand_arr[5]);
    } else if (Counter == 0 && flip_switch) {
      flip_switch = false;
      log_add('A', "OFF_ORDER effect=%d/%s", off_effects, modeAnimName(off_effects));
    }

    int idx = transitionIndexForEffect(off_effects, Counter);
    if (idx < 0) idx = 0;
    if (idx > 5) idx = 5;
    Nixie[idx] = 10;
    log_add('A', "OFF_STEP st=%s off=%d step=%d idx=%d N=%d%d:%d%d:%d%d",
            modeStateName(displayState), off_effects, Counter + 1, idx,
            Nixie[0], Nixie[1], Nixie[2], Nixie[3], Nixie[4], Nixie[5]);

    Counter++;
    if (Counter >= 6) {
      byte doneEffect = off_effects;
      log_add('A', "OFF_DONE st=%s off=%d/%s from=%d to=%d", modeStateName(displayState), doneEffect, modeAnimName(doneEffect), transitionFromDisplay, transitionToDisplay);
      off_effects = 0;
      activeTransitionEffect = doneEffect;
      Counter = 0;
      flip_switch = true;
      prepareNextOnEffect();
    }
    return;
  }

  if (on_effects > 0) {
    if (Counter == 0 && on_effects == 5 && flip_switch) {
      flip_switch = false;
      shuffle(rand_arr, 6);
      log_add('A', "ON_ORDER random=%d,%d,%d,%d,%d,%d", rand_arr[0], rand_arr[1], rand_arr[2], rand_arr[3], rand_arr[4], rand_arr[5]);
    } else if (Counter == 0 && flip_switch) {
      flip_switch = false;
      log_add('A', "ON_ORDER effect=%d/%s", on_effects, modeAnimName(on_effects));
    }

    int idx = transitionIndexForEffect(on_effects, Counter);
    if (idx < 0) idx = 0;
    if (idx > 5) idx = 5;
    Nixie[idx] = NixieBuffer[idx];
    log_add('A', "ON_STEP st=%s on=%d step=%d idx=%d N=%d%d:%d%d:%d%d B=%d%d:%d%d:%d%d",
            modeStateName(displayState), on_effects, Counter + 1, idx,
            Nixie[0], Nixie[1], Nixie[2], Nixie[3], Nixie[4], Nixie[5],
            NixieBuffer[0], NixieBuffer[1], NixieBuffer[2], NixieBuffer[3], NixieBuffer[4], NixieBuffer[5]);

    Counter++;
    if (Counter >= 6) completeOnEffect();
    return;
  }
}
