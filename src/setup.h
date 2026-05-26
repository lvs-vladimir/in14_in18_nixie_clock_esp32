void setup()
{
  init_timers();
  Serial.begin(115200);
  memset(log_entries, 0, sizeof(log_entries));
  log_write_idx = 0;
  log_count = 0;
  log_add('I', "System boot");

  if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
    log_add('E', "LittleFS Mount Failed");
    Serial.println("LittleFS Mount Failed");
    return;
  }

  bool need_defaults = false;
  File file = LittleFS.open("/setting.dat");
  if (!file) {
    log_add('W', "No settings file, applying defaults");
    Serial.println("No settings file, applying defaults");
    need_defaults = true;
  } else {
    file.close();
    FDstat_t stat = fd.read();
    if (stat == FD_READ) {
      log_add('I', "Settings loaded OK");
      Serial.println("Settings loaded OK");
    } else {
      log_add('W', "Settings read: %d, applying defaults", stat);
      need_defaults = true;
    }
  }
  if (need_defaults) {
    memset(&mydata, 0, sizeof(mydata));
    strcpy(mydata.NTPserver, "pool.ntp.org");
    mydata.GMT = 7;
    mydata.lng = 0;
    mydata.animdots = 0;
    mydata.dots_switch = true;
    mydata.seconds_switch = true;
    mydata.autoshow_slots = 3;
    mydata.autoshow_min = 10;
    for (byte i = 0; i < 7; i++) mydata.autoshow_select_sec[i] = 10;
    mydata.autoshow_select_sec[0] = 0;
    mydata.autoshow_switch = true;
    fd.read();
  }

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