void setup()
{
  init_timers();
  Serial.begin(115200);
  NtpSyncTimer.stop();
  OwmUpdateTimer.stop();
  CoinUpdateTimer.stop();
  memset(log_entries, 0, sizeof(log_entries));
  log_write_idx = 0;
  log_count = 0;
  log_add('I', "System boot");

  LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED);
  log_add('I', "LittleFS mounted");
  
  if (LittleFS.exists("/webpass.txt")) {
    log_add('I', "Password file exists, reading...");
    File f = LittleFS.open("/webpass.txt", "r");
    if (f) {
      String pass = f.readStringUntil('\n');
      f.close();
      pass.trim();
      log_add('I', "Read from file: len=%d val=%s", pass.length(), pass.c_str());
      if (pass.length() > 0) {
        pass.toCharArray(webPass, sizeof(webPass));
        log_add('I', "Password loaded: %s", webPass);
      } else {
        log_add('W', "File empty, using default: %s", webPass);
      }
    } else {
      log_add('W', "Failed to open file, using default: %s", webPass);
    }
  } else {
    log_add('I', "Password file not found, creating with default: %s", webPass);
    File f = LittleFS.open("/webpass.txt", "w");
    if (f) {
      f.print(webPass);
      f.close();
      log_add('I', "Password file created");
    } else {
      log_add('W', "Failed to create password file");
    }
  }
  ui.disableAuth();
  log_add('I', "Web auth enabled, current password: %s", webPass);

  FDstat_t stat = fd.read();
  if (stat != FD_READ) {
    log_add('W', "Settings read: %d, using defaults", stat);
    Serial.println("Using default settings");
  } else {
    log_add('I', "Settings read OK");
    Serial.println("Settings read OK");
  }
  if (mydata.ssid[0] == '\0') {
    strcpy(mydata.ssid, "WAY");
    strcpy(mydata.pass, "lukjanow");
  }
  if (mydata.NTPserver[0] == '\0') strcpy(mydata.NTPserver, "pool.ntp.org");
  if (mydata.GMT == 0) mydata.GMT = 7;
  if (mydata.autoshow_slots == 0) {
    mydata.autoshow_slots = 3;
    mydata.autoshow_min = 10;
    for (byte i = 0; i < 7; i++) mydata.autoshow_select_sec[i] = 10;
    mydata.autoshow_select_sec[0] = 0;
    mydata.autoshow_switch = true;
    mydata.autoshow_select[1] = 1;
    mydata.autoshow_select[2] = 2;
    mydata.autoshow_select[3] = 3;
  }
  if (!mydata.autoshow_switch) mydata.autoshow_switch = true;
  if (!mydata.seconds_switch) mydata.seconds_switch = true;
  if (mydata.animdots > 7) mydata.animdots = 0;
  if (mydata.autoshow_slots > 5) mydata.autoshow_slots = 5;
  if (mydata.autoshow_select[0] != 0) {
    byte slots = mydata.autoshow_slots;
    if (slots > 5) slots = 5;
    log_add('W', "Migrate autoshow select 0-based slots=%d s0=%d s1=%d s2=%d", slots,
            mydata.autoshow_select[0], mydata.autoshow_select[1], mydata.autoshow_select[2]);
    for (int idx = slots; idx >= 1; idx--) mydata.autoshow_select[idx] = mydata.autoshow_select[idx - 1];
    mydata.autoshow_select[0] = 0;
    fd.updateNow();
  }
  bool autoshowMigratedCompact = false;
  for (byte idx = 1; idx <= mydata.autoshow_slots; idx++) {
    if (mydata.autoshow_select[idx] >= 5) {
      log_add('W', "Compact autoshow slot %d old=%d new=%d", idx, mydata.autoshow_select[idx], mydata.autoshow_select[idx] - 4);
      mydata.autoshow_select[idx] -= 4;
      autoshowMigratedCompact = true;
    }
  }
  if (autoshowMigratedCompact) fd.updateNow();
  if (mydata.autoshow_slots > 0 && mydata.autoshow_select[1] > 5) {
    log_add('W', "Slot1 invalid (%d), reset to temp", mydata.autoshow_select[1]);
    mydata.autoshow_select[1] = 1;
  }
  if (mydata.autoshow_slots > 1 && mydata.autoshow_select[2] > 5) {
    log_add('W', "Slot2 invalid (%d), reset to pressure", mydata.autoshow_select[2]);
    mydata.autoshow_select[2] = 2;
  }
  if (mydata.autoshow_slots > 2 && mydata.autoshow_select[3] > 5) {
    log_add('W', "Slot3 invalid (%d), reset to humidity", mydata.autoshow_select[3]);
    mydata.autoshow_select[3] = 3;
  }

  log_add('I', "SSID: %s", mydata.ssid);

  WiFiConnect_APcreate();
  OtaUpdate();

  ui.attachBuild(build);
  ui.attach(action);
  ui.uploadAuto(true);
  ui.enableOTA();
  ui.downloadAuto(true);
  ui.start();

  pinMode(HSPI_SS, OUTPUT);
  pinMode(BL, OUTPUT);
  digitalWrite(BL, 1);

  hspi = new SPIClass(HSPI);
  hspi->begin(HSPI_SCLK, HSPI_MISO, HSPI_MOSI, HSPI_SS);

  strip.begin();
  strip.setBrightness(0);

  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LED_OUTPUT_PIN, PWM_CHANNEL);
  ledcWrite(PWM_CHANNEL, 255);

  hour = 23;
  minute = 22;
  second = 00;
  oldminute = minute;

  NTPClientUpdate();
  NtpSyncTimer.start();
  SwitchDisplayTimer.stop();

  if (WiFi.status() == WL_CONNECTED) {
    log_add('I', "OWM init update");
    if (strlen(mydata.owMapApiKey) > 0 && strlen(mydata.owCity) > 0) getTemp2(0);
  }
  rebuildSensorsAutoShowSelect();
  log_add('I', "Sensor select list initialized: len=%d", SensorsAutoShowSelect2.length());
  OwmUpdateTimer.start();
  log_add('I', "Init complete, HW: %d/%d, disp=%d", hour, minute, display);

  if (TempValue == 0 && optemperature != 0) TempValue = optemperature;
  if (WiFi.status() == WL_CONNECTED) updateCryptoRates();
  CoinUpdateTimer.start();

  SwitchDisplayTimer.stop();
  veml.begin();
  veml.setLowThreshold(10000);
  veml.setHighThreshold(20000);
  veml.interruptEnable(true);

  xTaskCreatePinnedToCore (
    loop2,
    "loop2",
    10000,
    NULL,
    0,
    NULL,
    0
  );
}