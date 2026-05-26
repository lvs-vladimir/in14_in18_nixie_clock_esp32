#include <lang.h>

void build() {
  GP.BUILD_BEGIN(500);
  GP.THEME(GP_DARK);
  GP.TITLE(DEVICE_NAME[mydata.lng]);
  GP.HR();
  GP.PAGE_TITLE(PAGE_TITLE[mydata.lng]);
  GP.HR();
  GP.UPDATE("btc,eth,usdrub,lux,optemp,ophum,oppres,sens0,sens1,sens2,sens3,lg");
  GP.NAV_TABS_LINKS("/,/setting,/info,/firmware,/log", TAB_LINKS_NAMES[mydata.lng], GP_BLUE);

  if (ui.uri("/setting")) {
    GP.FORM_BEGIN("/setting");

    GP.BLOCK_THIN_BEGIN();
    M_BOX(GP_CENTER, GP.LABEL(SETTING_TIME_NAME[mydata.lng]););
    GP.HR();
    M_BOX(GP_LEFT, GP.LABEL(SETTING_NTP_NAME[mydata.lng]); M_BOX(GP_RIGHT, GP.TEXT("ntp", "Ntp", mydata.NTPserver, "100%");););
    M_BOX(GP_LEFT, GP.LABEL(SETTING_TIMEZONE_NAME[mydata.lng]); M_BOX(GP_RIGHT, GP.SELECT("timeZone", SETTING_TIMEZONE_GMT, mydata.GMT + 12, 0);););
    M_BOX(GP_CENTER, GP.BUTTON_MINI("sntp_btn", SETTING_NTP_SYNC_BTN[mydata.lng], "", GP_BLUE, "", 0, 1););
    GP.BLOCK_END();

    GP.BLOCK_THIN_BEGIN();
    M_BOX(GP_CENTER, GP.LABEL(SETTING_NETWORK_CONNECTION[mydata.lng]););
    GP.HR();
    M_BOX(GP_LEFT, GP.BUTTON_MINI("scan_btn", SETTING_NETWORK_SCAN_BTN[mydata.lng], "", GP_BLUE, "", 0, 1); M_BOX(GP_RIGHT, GP.SELECT("WiFi_List_Select", WiFI_List, scan_list_idx, 0, 0, 0);););
    GP.HR();
    M_BOX(GP_LEFT, GP.LABEL(SETTING_NETWORK_WIFI_NAME[mydata.lng]); M_BOX(GP_RIGHT, GP.TEXT("lg", "Login", mydata.ssid, "100%");););
    M_BOX(GP_LEFT, GP.LABEL(SETTING_NETWORK_WIFI_PASSWORD[mydata.lng]); M_BOX(GP_RIGHT, GP.TEXT("ps", "Password", mydata.pass, "100%");));
    GP.BREAK();
    M_BOX(GP_CENTER, GP.BUTTON_MINI("wifi_btn", SETTING_WIFI_CONNECT_BTN[mydata.lng], "", GP_BLUE, "", 0, 1););
    GP.BLOCK_END();

    GP.BLOCK_THIN_BEGIN();
    M_BOX(GP_CENTER, GP.LABEL(F("<a href=\"https://openweathermap.org\" target=\"_blank\">OpenWeatherMap.org</a>"), "", GP_DEFAULT, 0, 1););
    GP.HR();
    M_BOX(GP_LEFT, GP.LABEL(SETTING_OP_APIKEY[mydata.lng]); M_BOX(GP_RIGHT, GP.TEXT("ap", "ApiKey", mydata.owMapApiKey, "100%");););
    M_BOX(GP_LEFT, GP.LABEL(SETTING_OP_CITY[mydata.lng]); M_BOX(GP_RIGHT, GP.TEXT("ct", "City", mydata.owCity, "100%");););
    GP.BREAK();
    M_BOX(GP_CENTER, GP.BUTTON_MINI("op_btn", SETTING_OP_SYNC_BTN[mydata.lng], "", GP_BLUE, "", 0, 1););
    GP.BLOCK_END();

    GP.BLOCK_THIN_BEGIN();
    M_BOX(GP_CENTER, GP.LABEL(SETTING_NAROD_NAME[mydata.lng]););
    GP.HR();
    M_BOX(GP_LEFT, GP.LABEL(SETTING_NAROD_APIKEY[mydata.lng]); M_BOX(GP_RIGHT, GP.TEXT("api_narod", "ApiKey", mydata.NarodmoonApi, "100%");););
    M_BOX(GP_LEFT, GP.LABEL(SETTING_NAROD_ID[mydata.lng]); M_BOX(GP_RIGHT, GP.TEXT("id_narod", "ID", mydata.NarodmoonID, "45%", 4);););
    GP.BREAK();
    M_BOX(GP_CENTER, GP.LABEL(SETTING_NAROD_SENSORS_LABEL[mydata.lng]););
    GP.HR();
    M_BOX(GP_LEFT, GP.LABEL("", "sens0"); M_BOX(GP_RIGHT, GP.SELECT("type_sensor0", SETTING_NAROD_SENSORS[mydata.lng], mydata.nrd_type_sensor[0], 0, 0, 1); GP.SPINNER("sens0_narod", mydata.nrd_sens[0], 0, 20, 1, 0, GP_BLUE, "40px", 0);););
    M_BOX(GP_LEFT, GP.LABEL("", "sens1"); M_BOX(GP_RIGHT, GP.SELECT("type_sensor1", DATA_TYPE_SENSOR, mydata.nrd_type_sensor[1], 0, 0, 1); GP.SPINNER("sens1_narod", mydata.nrd_sens[1], 0, 20, 1, 0, GP_BLUE, "40px", 0);););
    M_BOX(GP_LEFT, GP.LABEL("", "sens2"); M_BOX(GP_RIGHT, GP.SELECT("type_sensor2", DATA_TYPE_SENSOR, mydata.nrd_type_sensor[2], 0, 0, 1); GP.SPINNER("sens2_narod", mydata.nrd_sens[2], 0, 20, 1, 0, GP_BLUE, "40px", 0);););
    GP.BREAK();
    M_BOX(GP_CENTER, GP.BUTTON_MINI("narod_btn", SETTING_NAROD_SYNC_BTN[mydata.lng], "", GP_BLUE, "", 0, 1););
    GP.BLOCK_END();

    GP.BLOCK_THIN_BEGIN();
    M_BOX(GP_LEFT, GP.LABEL(SETTING_LANGUAGE[mydata.lng]); M_BOX(GP_RIGHT, GP.SELECT("lng", SETTING_LANGUAGE_ARRAY, mydata.lng, 0, 0, 1);); GP.BREAK(););
    M_BOX(GP_LEFT, GP.LABEL(DISPLAY_SECONDS_SWITCH[mydata.lng]); M_BOX(GP_RIGHT, GP.SWITCH("seconds_switch", mydata.seconds_switch, GP_BLUE);););
    GP.BLOCK_END();

    GP.BUTTON_MINI("rst", SETTING_RESTART_BTN[mydata.lng], "", GP_BLUE, "", 0, 1);
    GP.FORM_END();

  } else if (ui.uri("/info")) {
    GP.SYSTEM_INFO("1.0");

  } else if (ui.uri("/firmware")) {
    GP.BLOCK_THIN_BEGIN();
    GP.OTA_FIRMWARE("OTA firmware", GP_BLUE);
    GP.OTA_FILESYSTEM("OTA filesystem", GP_BLUE);
    GP.FILE_UPLOAD("file_upload", "Upload file");
    GP.BLOCK_END();

  } else if (ui.uri("/log")) {
    GP.LABEL(F("<script>setInterval(function(){location.reload()},3000)</script>"), "");
    GP.BLOCK_THIN_BEGIN();
    M_BOX(GP_CENTER, GP.LABEL("System Log"););
    GP.HR();
    byte idx = (log_count < LOG_ENTRIES) ? 0 : log_write_idx;
    byte cnt = log_count;
    for (byte i = 0; i < cnt; i++) {
      byte n = (idx + i) % LOG_ENTRIES;
      char line[LOG_LINE_LEN + 16];
      snprintf(line, sizeof(line), "[%5lu] %c: %s", log_entries[n].time, log_entries[n].level, log_entries[n].msg);
      GP.LABEL(line, "", GP_DEFAULT, 10);
      GP.BREAK();
    }
    GP.BLOCK_END();

  } else {
    GP.BLOCK_THIN_BEGIN();
    M_BOX(GP_CENTER, GP.LABEL("CoinGecko"););
    GP.HR();
    M_BOX(GP_LEFT, GP.LABEL("BTC:"); GP.LABEL(" ", "btc"); GP.LABEL("$"); GP.BREAK(););
    M_BOX(GP_LEFT, GP.LABEL("ETH:"); GP.LABEL(" ", "eth"); GP.LABEL("$"); GP.BREAK(););
    M_BOX(GP_LEFT, GP.LABEL("USD/RUB:"); GP.LABEL(" ", "usdrub"); GP.BREAK(););
    GP.BLOCK_END();

    GP.BLOCK_THIN_BEGIN();
    M_BOX(GP_CENTER, GP.LABEL("VEML7700"););
    GP.HR();
    M_BOX(GP_LEFT, GP.LABEL("LUX:"); GP.LABEL(" ", "lux"); GP.BREAK(););
    GP.BLOCK_END();

    if (optemperature != 0) {
      GP.BLOCK_THIN_BEGIN();
      M_BOX(GP_CENTER, GP.LABEL(String("OpenWeatherMap: ") + mydata.owCity););
      GP.HR();
      M_BOX(GP_LEFT, GP.LABEL("Temperature:"); GP.LABEL(" ", "optemp"); GP.LABEL("C"); GP.BREAK(););
      M_BOX(GP_LEFT, GP.LABEL("Humidity:"); GP.LABEL(" ", "ophum"); GP.LABEL("%"); GP.BREAK(););
      M_BOX(GP_LEFT, GP.LABEL("Pressure:"); GP.LABEL(" ", "oppres"); GP.LABEL("hPa"); GP.BREAK(););
      GP.BLOCK_END();
    }

    GP.BLOCK_THIN_BEGIN();
    M_BOX(GP_CENTER, GP.LABEL(DISPLAY_DATA_LABEL[mydata.lng]););
    GP.HR();
    M_BOX(GP_LEFT, GP.LABEL(DISPLAY_DATA_SHOW_SWITCH[mydata.lng]); M_BOX(GP_RIGHT, GP.SWITCH("auto_show_switch", mydata.autoshow_switch, GP_BLUE); GP.SPINNER("autoshow_sec", mydata.autoshow_min, 5, 255, 1, 0, GP_BLUE, "50px", 0);););
    GP.BREAK();
    for (byte i = 0; i < mydata.autoshow_slots; i++) {
      char buf[20];
      snprintf(buf, sizeof(buf), "%d:", i + 1);
      M_BOX(GP_LEFT,
        GP.LABEL(buf);
        snprintf(buf, sizeof(buf), "sa%d", i);
        GP.SELECT(buf, SensorsAutoShowSelect2, mydata.autoshow_select[i], 0, 0, 1);
        M_BOX(GP_RIGHT,
          snprintf(buf, sizeof(buf), "se%d", i);
          GP.SPINNER(buf, mydata.autoshow_select_sec[i + 1], 0, 30, 1, 0, GP_BLUE, "50px", 0);
          snprintf(buf, sizeof(buf), "rm%d", i);
          GP.BUTTON_MINI(buf, "X", "", GP_RED, "20px", 0, 1);
        );
      );
      GP.BREAK();
    }
    if (mydata.autoshow_slots < 6) {
      GP.BUTTON_MINI("add_slot", "+ Add", "", GP_BLUE, "80px", 0, 1);
      GP.BREAK();
    }
    GP.BREAK();
    M_BOX(GP_LEFT, GP.LABEL(SETTING_ANIM_NAME[mydata.lng]); M_BOX(GP_RIGHT, GP.SELECT("anim_change", SETTING_ANIM_ARRAY[mydata.lng], mydata.anim_change, 0, 0, 1);););
    GP.HR();
    M_BOX(GP_LEFT, GP.LABEL(DISPLAY_SECONDS_SWITCH[mydata.lng]); M_BOX(GP_RIGHT, GP.SWITCH("seconds_switch", mydata.seconds_switch, GP_BLUE);););
    GP.HR();
    GP.BLOCK_END();
  }
  GP.BUILD_END();
}

void action(GyverPortal & p) {
  if (ui.click()) {
    if (ui.click("rst")) ESP.restart();
    if (ui.click("sntp_btn")) {
      timeClient.setPoolServerName(mydata.NTPserver);
      timeClient.setTimeOffset(3600 * mydata.GMT);
      timeClient.update();
    }
    if (ui.click("op_btn")) {
      if (strlen(mydata.owMapApiKey) > 0 && strlen(mydata.owCity) > 0) getTemp2(0);
    }
    if (ui.click("scan_btn")) {
      WiFI_List = "";
      int n = WiFi.scanNetworks();
      for (int i = 0; i < n; ++i) {
        WiFI_List += WiFi.SSID(i);
        if (i < n - 1) WiFI_List += ",";
      }
      WiFi.scanDelete();
    }
    if (ui.click("narod_btn")) {
      if (strlen(mydata.NarodmoonApi) > 0 && strlen(mydata.NarodmoonID) > 0) getTemp2(1);
    }
    if (ui.click("wifi_btn")) {
      WiFi.softAPdisconnect();
      WiFi.mode(WIFI_STA);
      WiFi.begin(mydata.ssid, mydata.pass);
    }
    if (ui.clickStr("ntp", mydata.NTPserver));
    if (ui.clickStr("lg", mydata.ssid));
    if (ui.clickStr("ps", mydata.pass));
    if (ui.clickStr("ap", mydata.owMapApiKey));
    if (ui.clickStr("ct", mydata.owCity));
    if (ui.clickStr("api_narod", mydata.NarodmoonApi));
    if (ui.clickStr("id_narod", mydata.NarodmoonID));
    if (ui.clickInt("sens0_narod", mydata.nrd_sens[0]));
    if (ui.clickInt("sens1_narod", mydata.nrd_sens[1]));
    if (ui.clickInt("sens2_narod", mydata.nrd_sens[2]));
    if (ui.click("WiFi_List_Select")) {
      ui.copyInt("WiFi_List_Select", scan_list_idx);
      strcpy(mydata.ssid, GPlistIdx(WiFI_List, scan_list_idx).c_str());
    }
    if (ui.click("type_sensor0")) mydata.nrd_type_sensor[0] = ui.getInt("type_sensor0");
    if (ui.click("type_sensor1")) mydata.nrd_type_sensor[1] = ui.getInt("type_sensor1");
    if (ui.click("type_sensor2")) mydata.nrd_type_sensor[2] = ui.getInt("type_sensor2");
    if (ui.click("timeZone")) {
      mydata.GMT = ui.getInt("timeZone") - 12;
      timeClient.setPoolServerName(mydata.NTPserver);
      timeClient.setTimeOffset(3600 * mydata.GMT);
      timeClient.update();
    }
    if (ui.click("lng")) {
      mydata.lng = ui.getInt("lng");
    }
    if (ui.clickInt("seconds_switch", mydata.seconds_switch)) {};
    if (ui.clickInt("auto_show_switch", mydata.autoshow_switch));
    if (ui.clickInt("anim_change", mydata.anim_change));
    if (ui.clickInt("autoshow_sec", mydata.autoshow_min));
    for (byte i = 0; i < mydata.autoshow_slots; i++) {
      char buf[16];
      snprintf(buf, sizeof(buf), "sa%d", i);
      if (ui.click(buf)) mydata.autoshow_select[i] = ui.getInt(buf);
      snprintf(buf, sizeof(buf), "se%d", i);
      if (ui.click(buf)) mydata.autoshow_select_sec[i + 1] = ui.getInt(buf);
      snprintf(buf, sizeof(buf), "rm%d", i);
      if (ui.click(buf)) {
        for (byte j = i; j < mydata.autoshow_slots - 1; j++) {
          mydata.autoshow_select[j] = mydata.autoshow_select[j + 1];
          mydata.autoshow_select_sec[j + 1] = mydata.autoshow_select_sec[j + 2];
        }
        mydata.autoshow_slots--;
      }
    }
    if (ui.click("add_slot") && mydata.autoshow_slots < 6) {
      mydata.autoshow_slots++;
      mydata.autoshow_select[mydata.autoshow_slots - 1] = 0;
      mydata.autoshow_select_sec[mydata.autoshow_slots] = 10;
    }
    fd.updateNow();
  }
  if (ui.update()) {
    if (ui.update("lg")) ui.answer(mydata.ssid);
    if (ui.update("lg")) ui.answer(mydata.ssid);
    if (ui.update("btc")) ui.answer(pricebtc);
    if (ui.update("eth")) ui.answer(priceeth);
    if (ui.update("usdrub")) ui.answer(usdRubRate);
    if (ui.update("lux")) ui.answer(vemllux);
    if (ui.update("optemp")) ui.answer(optemperature);
    if (ui.update("ophum")) ui.answer(ophumidity);
    if (ui.update("oppres")) ui.answer(oppressure);
    if (ui.update("sens0")) ui.answer(SensorsDisplay[0]);
    if (ui.update("sens1")) ui.answer(SensorsDisplay[1]);
    if (ui.update("sens2")) ui.answer(SensorsDisplay[2]);
    if (ui.update("sens3")) ui.answer(SensorsDisplay[3]);
  }
}
