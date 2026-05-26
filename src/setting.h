void TimeUpdate()
{
  timeClient.update();
  if (timeClient.getHours() >= 0 && timeClient.getMinutes() >= 0 && timeClient.getSeconds() >= 0) {
    hour = timeClient.getHours();
    minute = timeClient.getMinutes();
    second = timeClient.getSeconds();
  }

  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  if (timeinfo.tm_hour >= 0 && timeinfo.tm_min >= 0 && timeinfo.tm_sec >= 0) {
    hour = timeinfo.tm_hour;
    minute = timeinfo.tm_min;
    second = timeinfo.tm_sec;
    day = timeinfo.tm_mday;
    month = timeinfo.tm_mon;
    year = timeinfo.tm_year;
    year = year % 100;
  }
}

void NTPClientUpdate()
{
  timeClient.setPoolServerName(mydata.NTPserver);
  timeClient.setTimeOffset(3600 * mydata.GMT);
  TimeUpdate();
}

bool initWiFi()
{
  if (mydata.ssid[0] == '\0') {
    Serial.println("SSID not defined");
    return false;
  }
  char HOSTNAME[30];
  sprintf_P(HOSTNAME, (PGM_P)F("%S-%llX"), YOUR_HOSTNAME, ESP.getEfuseMac());
  WiFi.setHostname(HOSTNAME);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(mydata.ssid, mydata.pass);
  Serial.println("Подключаемся к WiFi...");
  unsigned long currentMillis = millis();
  previousMillis = currentMillis;
  while (WiFi.status() != WL_CONNECTED) {
    currentMillis = millis();
    if (currentMillis - previousMillis >= 10000) {
      Serial.println("Ошибка подключения к WIFI");
      return false;
    }
  }
  Serial.println(WiFi.localIP());
  return true;
}

void WiFiConnect_APcreate()
{
  if (initWiFi()) {
    Serial.println("WiFi успешно подключен");
    ap_show_scroll = false;
  } else {
    char HOSTNAME[30];
    sprintf_P(HOSTNAME, (PGM_P)F("%S-%llX"), "IN18-AP", ESP.getEfuseMac());
    log_add('W', "WiFi not connected, creating AP");
    Serial.println("WiFi не подключен, создаем точку");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(HOSTNAME);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
  }
}

void OtaUpdate()
{
  char otaHostname[30];
  sprintf_P(otaHostname, (PGM_P)F("%S"), YOUR_HOSTNAME);
  ArduinoOTA.setHostname(otaHostname);
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else
      type = "filesystem";
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void calculateTime()
{
  second++;
  if (second > 59) {
    second = 0;
    minute++;
  }
  if (minute > 59) {
    minute = 0;
    hour++;
  }
  if (hour > 23) {
    hour = 0;
  }
}

uint8_t brigh_value_indi(uint16_t veml_value, Range lux_ranges[], uint8_t brigh_values[], uint8_t prev_brigh_value)
{
  for (byte i = 0; i < num_ranges; i++) {
    if (veml_value >= lux_ranges[i].min && veml_value <= lux_ranges[i].max) {
      return prev_brigh_value != brigh_values[i] ? brigh_values[i] : prev_brigh_value;
    }
  }
  return prev_brigh_value;
}

void IRAM_ATTR brightness_timer()
{
  timer0=true;
}

void IRAM_ATTR second_timer()
{
  timer1=true;
}

void init_timers(){
  Timer0_Cfg = timerBegin(0, 80, true);
  timerAttachInterrupt(Timer0_Cfg, &brightness_timer, true);
  timerAlarmWrite(Timer0_Cfg, 2000000, true);
  timerAlarmEnable(Timer0_Cfg);

  Timer1_Cfg = timerBegin(1, 80, true);
  timerAttachInterrupt(Timer1_Cfg, &second_timer, true);
  timerAlarmWrite(Timer1_Cfg, 1000000, true);
  timerAlarmEnable(Timer1_Cfg);
}

void UpdateDisplay()
{
  uint32_t hv5222_1, hv5222_2, hv5222_3;

  bufer[5] = masshv5522[Nixie[5]];
  bufer[4] = masshv5522[Nixie[4]];
  bufer[3] = masshv5522[Nixie[3]];
  bufer[2] = masshv5522[Nixie[2]];
  bufer[1] = masshv5522[Nixie[1]];
  bufer[0] = masshv5522[Nixie[0]];

  #ifdef IN_14
  if (display == 0 && timeon) {
    if (DotTimer.isReady()) {
      if (dmooveright) dmoove++;
      if (dmooveleft) dmoove--;
      if (dmoove > 11) {
        dmoove = 11;
        dmooveleft = true;
        dmooveright = false;
      }
      if (dmoove < 0) {
        dmoove = 0;
        dmooveleft = false;
        dmooveright = true;
      }
      if (dmoove == 11) DotTimer.setInterval(76);
      if (dmoove == 10) DotTimer.setInterval(84);
      bufer[buferDot[dmoove]] |= (1 << DotMooveBufer[dmoove]);
    }
    hv5222_3 = (bufer[5] << 20);
    hv5222_2 = (bufer[2] << 26) | (bufer[3] << 14) | bufer[4];
    hv5222_1 = (bufer[0] << 20) | (bufer[1] << 8) | (bufer[2] << 6);
  }
  #endif

  hspi->beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE2));
  CS_OFF_HSPI;

  #ifdef IN_14
  hspi->transfer32(hv5222_3);
  hspi->transfer32(hv5222_2);
  hspi->transfer32(hv5222_1);
  #endif

  #ifdef IN_18
  hv5222_2 = (bufer[3] << 20) | (bufer[4] << 10) | bufer[5];
  hv5222_1 = (bufer[0] << 20) | (bufer[1] << 10) | bufer[2];
  hspi->transfer32(hv5222_2);
  hspi->transfer32(hv5222_1);
  #endif

  hspi->endTransaction();
  CS_ON_HSPI;
}

void SetNixieBufer()
{
  NixieBuffer[0] = newhour / 10;
  NixieBuffer[1] = newhour % 10;
  NixieBuffer[2] = newminute / 10;
  NixieBuffer[3] = newminute % 10;
  if (mydata.seconds_switch) {
    NixieBuffer[4] = newsecond / 10;
    NixieBuffer[5] = newsecond % 10;
  } else {
    NixieBuffer[4] = 10;
    NixieBuffer[5] = 10;
  }
}

void SetNixie()
{
  Nixie[0] = hour / 10;
  Nixie[1] = hour % 10;
  Nixie[2] = minute / 10;
  Nixie[3] = minute % 10;
  if (mydata.seconds_switch) {
    Nixie[4] = second / 10;
    Nixie[5] = second % 10;
  } else {
    Nixie[4] = 10;
    Nixie[5] = 10;
  }
}