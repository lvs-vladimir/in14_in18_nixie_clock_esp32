void disablePing();

void setup()
{
  setCpuFrequencyMhz(240);
  init_timers();
  //Serial.begin(115200);
  NtpSyncTimer.stop();
  OwmUpdateTimer.stop();
  CoinUpdateTimer.stop();
  
  if (mydata.ntp_sync_enable == true && mydata.ntp_sync_interval == 0) {
    mydata.ntp_sync_enable = true;
    mydata.ntp_sync_interval = 60;
  }
  if (mydata.ntp_sync_enable) {
    NtpSyncTimer.setInterval(mydata.ntp_sync_interval * 60000UL);
    NtpSyncTimer.start();
  }
memset(log_entries, 0, sizeof(log_entries));
  log_write_idx = 0;
  log_count = 0;
  log_add('I', "System boot");

  LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED);
  log_add('I', "LittleFS mounted");

  // migrate password from /webpass.txt if present
  if (LittleFS.exists("/webpass.txt")) {
    File f = LittleFS.open("/webpass.txt", "r");
    if (f) {
      String pass = f.readStringUntil('\n');
      f.close();
      pass.trim();
      if (pass.length() > 0) {
        pass.toCharArray(mydata.webPass, sizeof(mydata.webPass));
        log_add('I', "Migrated password from /webpass.txt: %s", mydata.webPass);
      }
      LittleFS.remove("/webpass.txt");
    }
  }

  fd.addWithoutWipe(true);
  FDstat_t stat = fd.read();
  if (stat != FD_READ && stat != FD_ADD) {
    log_add('W', "Settings read: %d, using defaults", stat);
    Serial.println("Using default settings");
  } else {
    log_add('I', "Settings read OK");
    Serial.println("Settings read OK");
  }
  if (mydata.webPass[0] == '\0') strcpy(mydata.webPass, "admin");
  strcpy(webPass, mydata.webPass);
  ui.disableAuth();
  log_add('I', "Web auth enabled, current password: %s", webPass);

  if (mydata.ssid[0] == '\0') {
    if (stat != FD_READ && stat != FD_ADD) {
      strcpy(mydata.ssid, "WAY");
      strcpy(mydata.pass, "lukjanow");
    }
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
  if (mydata.offtime_end_h == 0 && mydata.offtime_end_m == 0) mydata.offtime_end_h = 8;
  if (mydata.nrd_sens_count == 0 || mydata.nrd_sens_count > 5) mydata.nrd_sens_count = 3;
  if (mydata.buzzer_interval != 5 && mydata.buzzer_interval != 10 && mydata.buzzer_interval != 30 && mydata.buzzer_interval != 60) mydata.buzzer_interval = 5;
  if (mydata.buzzer_duration == 0 || mydata.buzzer_duration > 2000) mydata.buzzer_duration = 150;
  if (mydata.buzzer_volume == 0) mydata.buzzer_volume = 100;
  if (mydata.buzzer_volume > 100) mydata.buzzer_volume = 100;
  if (mydata.buzzer_melody_idx >= BUZZER_MELODY_COUNT) mydata.buzzer_melody_idx = 0;
  if (!mydata.alarm_enable && mydata.alarm_hour == 0 && mydata.alarm_minute == 0) {
    mydata.alarm_hour = 8;
    mydata.alarm_volume = 100;
  }
  if (mydata.alarm_volume > 100) mydata.alarm_volume = 100;
  if (mydata.alarm_duration == 0) mydata.alarm_duration = 30;
  if (!mydata.reboot_enable && mydata.reboot_hour == 0 && mydata.reboot_minute == 0) {
    mydata.reboot_hour = 3;
  }
  if (mydata.animdots > 12) mydata.animdots = 0;
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
  byte max_select = 7 + mydata.nrd_sens_count;
  if (mydata.autoshow_slots > 0 && mydata.autoshow_select[1] > max_select) {
    log_add('W', "Slot1 invalid (%d), reset to temp", mydata.autoshow_select[1]);
    mydata.autoshow_select[1] = 1;
  }
  if (mydata.autoshow_slots > 1 && mydata.autoshow_select[2] > max_select) {
    log_add('W', "Slot2 invalid (%d), reset to pressure", mydata.autoshow_select[2]);
    mydata.autoshow_select[2] = 2;
  }
  if (mydata.autoshow_slots > 2 && mydata.autoshow_select[3] > max_select) {
    log_add('W', "Slot3 invalid (%d), reset to humidity", mydata.autoshow_select[3]);
    mydata.autoshow_select[3] = 3;
  }

  log_add('I', "SSID: %s", mydata.ssid);

  WiFiConnect_APcreate();
  OtaUpdate();
  disablePing();

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

  if (mydata.ws2812_brightness == 0) mydata.ws2812_brightness = 100;
  if (mydata.ws2812_random_sec == 0) mydata.ws2812_random_sec = 10;
  if (mydata.ws2812_br_ranges == 0) {
    mydata.ws2812_br_ranges = 4;
    mydata.ws2812_lux_min[0] = 0; mydata.ws2812_lux_max[0] = 15; mydata.ws2812_bright_val[0] = 10;
    mydata.ws2812_lux_min[1] = 20; mydata.ws2812_lux_max[1] = 200; mydata.ws2812_bright_val[1] = 60;
    mydata.ws2812_lux_min[2] = 300; mydata.ws2812_lux_max[2] = 700; mydata.ws2812_bright_val[2] = 130;
    mydata.ws2812_lux_min[3] = 1000; mydata.ws2812_lux_max[3] = 15000; mydata.ws2812_bright_val[3] = 200;
  }
  if (mydata.nixie_lux_min[0] == 0 && mydata.nixie_lux_max[0] == 0) {
    mydata.nixie_lux_min[0] = 0; mydata.nixie_lux_max[0] = 12; mydata.nixie_bright_val[0] = 15;
    mydata.nixie_lux_min[1] = 15; mydata.nixie_lux_max[1] = 47; mydata.nixie_bright_val[1] = 40;
    mydata.nixie_lux_min[2] = 50; mydata.nixie_lux_max[2] = 97; mydata.nixie_bright_val[2] = 80;
    mydata.nixie_lux_min[3] = 100; mydata.nixie_lux_max[3] = 297; mydata.nixie_bright_val[3] = 150;
    mydata.nixie_lux_min[4] = 300; mydata.nixie_lux_max[4] = 697; mydata.nixie_bright_val[4] = 200;
    mydata.nixie_lux_min[5] = 700; mydata.nixie_lux_max[5] = 15000; mydata.nixie_bright_val[5] = 255;
  }

  ws2812_set_brightness(mydata.ws2812_enable ? mydata.ws2812_brightness : 0);

  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LED_OUTPUT_PIN, PWM_CHANNEL);
  ledcWrite(PWM_CHANNEL, 255);



  ledcSetup(BUZZER_CHANNEL, BUZZER_FREQ, PWM_RESOLUTION);
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
  ledcWrite(BUZZER_CHANNEL, 0);

  hour = 23;
  minute = 22;
  second = 00;
  oldminute = minute;
  if (mydata.anim_change_sec == 0) mydata.anim_change_sec = 30;

  if (WiFi.status() == WL_CONNECTED) {
    log_add('I', "NTP init update");
    NTPClientUpdate();
  } else {
    log_add('W', "WiFi not connected, skipping NTP init");
  }
  NtpSyncTimer.start();
  SwitchDisplayTimer.stop();

  if (WiFi.status() == WL_CONNECTED) {
    log_add('I', "OWM init update");
    if (strlen(mydata.owMapApiKey) > 0 && strlen(mydata.owCity) > 0) getTemp2(0);
  }
  log_add('I', "Narodmon init update");
  narodmonUpdate();
  rebuildSensorsAutoShowSelect();
  log_add('I', "Sensor select list initialized: len=%d", SensorsAutoShowSelect2.length());
  OwmUpdateTimer.start();
  log_add('I', "Init complete, HW: %d/%d, disp=%d", hour, minute, display);

  if (TempValue == 0 && optemperature != 0) TempValue = optemperature;
  if (WiFi.status() == WL_CONNECTED) updateCryptoRates();
  CoinUpdateTimer.start();

  SwitchDisplayTimer.stop();
  Wire.setTimeOut(50);
  veml_ok = veml.begin();
  if (veml_ok) {
    veml.setLowThreshold(10000);
    veml.setHighThreshold(20000);
    veml.interruptEnable(true);
  } else {
    log_add('W', "VEML7700 not found, lux disabled");
  }

  //pinMode(BUZZER_PIN, OUTPUT);
  //digitalWrite(BUZZER_PIN, LOW);



  xTaskCreatePinnedToCore (
    ws2812Task,
    "ws2812",
    4096,
    NULL,
    2,
    &ws2812TaskHandle,
    0
  );
  xTaskCreatePinnedToCore (
    vemlTask,
    "veml",
    4096,
    NULL,
    0,
    &vemlTaskHandle,
    1
  );
  xTaskCreatePinnedToCore (
    networkTask,
    "network",
    8192,
    NULL,
    0,
    NULL,
    0
  );
  xTaskCreatePinnedToCore (
    loop2,
    "loop2",
    8192,
    NULL,
    1,
    NULL,
    0
  );
   xTaskCreatePinnedToCore (
    loop1,
    "loop1",
    12288,//12288
    NULL,
    1,
    NULL,
    1
  );
  vTaskDelete(NULL); // удалить loopTask
}

static struct raw_pcb *ping_pcb = NULL;

static u8_t ping_filter(void *arg, struct raw_pcb *pcb, struct pbuf *p, const ip_addr_t *addr) {
  pbuf_free(p);
  return 1;
}

void disablePing() {
  ping_pcb = raw_new(IP_PROTO_ICMP);
  if (ping_pcb) {
    raw_recv(ping_pcb, ping_filter, NULL);
    raw_bind(ping_pcb, IP_ADDR_ANY);
    log_add('I', "Ping blocked");
  }
}