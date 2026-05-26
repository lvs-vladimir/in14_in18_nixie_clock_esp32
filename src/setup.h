void setup()
{
  init_timers();
  Serial.begin(115200);
  NtpSyncTimer.stop();
  OwmUpdateTimer.stop();
  memset(log_entries, 0, sizeof(log_entries));
  log_write_idx = 0;
  log_count = 0;
  log_add('I', "System boot");

  LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED);
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
    mydata.autoshow_select[1] = 5;
    mydata.autoshow_select[2] = 6;
    mydata.autoshow_select[3] = 7;
  }
  if (!mydata.autoshow_switch) mydata.autoshow_switch = true;
  if (!mydata.seconds_switch) mydata.seconds_switch = true;
  if (mydata.autoshow_slots > 0 && (mydata.autoshow_select[1] < 5 || mydata.autoshow_select[1] > 13)) {
    log_add('W', "Slot1 invalid (%d), reset to temp", mydata.autoshow_select[1]);
    mydata.autoshow_select[1] = 5;
  }
  if (mydata.autoshow_slots > 1 && (mydata.autoshow_select[2] < 5 || mydata.autoshow_select[2] > 13)) {
    log_add('W', "Slot2 invalid (%d), reset to pressure", mydata.autoshow_select[2]);
    mydata.autoshow_select[2] = 6;
  }
  if (mydata.autoshow_slots > 2 && (mydata.autoshow_select[3] < 5 || mydata.autoshow_select[3] > 13)) {
    log_add('W', "Slot3 invalid (%d), reset to humidity", mydata.autoshow_select[3]);
    mydata.autoshow_select[3] = 7;
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
  OwmUpdateTimer.start();
  log_add('I', "Init complete, HW: %d/%d, disp=%d", hour, minute, display);

  pricebtc = 42345;
  priceeth = 0;
  TempValue = 18;

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