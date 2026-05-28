#include <lang.h>

void build() {
  GP.BUILD_BEGIN(500);
  GP.THEME(GP_DARK);
GP.TAG_RAW(F("style"), F("label,input,select,button{font-size:16px!important;}.thinText{font-size:16px!important;}"));
GP.THEME(GP_DARK);
  GP.TITLE(DEVICE_NAME[mydata.lng]);
  GP.PAGE_TITLE(PAGE_TITLE[mydata.lng]);
  GP.HR();
  GP.UPDATE("btc,eth,usdrub,lux,optemp,ophum,oppres,sens0,sens1,sens2,sens3");
  if (ui.uri("/logout")) {
    webAuthOk = false;
  }

  if (!webAuthOk) {
    GP.BLOCK_THIN_BEGIN();
    GP.FORM_BEGIN("/login");
    M_BOX(GP_CENTER, GP.LABEL(WEB_LOGIN_TITLE[mydata.lng]););
    GP.HR();
    M_BOX(GP_LEFT, GP.LABEL(WEB_LOGIN_PASSWORD[mydata.lng]); M_BOX(GP_RIGHT, GP.PASS("login_pass", "Password", "", "100%", 31, "", false, true);););
    GP.BREAK();
    M_BOX(GP_CENTER, GP.SUBMIT_MINI(WEB_LOGIN_BUTTON[mydata.lng], GP_BLUE););
    GP.FORM_END();
    GP.BLOCK_END();
    GP.BUILD_END();
    return;
  }

  GP.NAV_TABS_LINKS("/,/setting,/info,/firmware,/log,/readme", TAB_LINKS_NAMES[mydata.lng], GP_BLUE);

  if (ui.uri("/setting")) {
    GP.FORM_BEGIN("/setting");

    GP.BLOCK_THIN_BEGIN();
    M_BOX(GP_CENTER, GP.LABEL(SETTING_TIME_NAME[mydata.lng]););
    GP.HR();
    M_BOX(GP_LEFT, GP.LABEL(SETTING_NTP_NAME[mydata.lng]); M_BOX(GP_RIGHT, GP.TEXT("ntp", "Ntp", mydata.NTPserver, "100%");););
    M_BOX(GP_LEFT, GP.LABEL(SETTING_TIMEZONE_NAME[mydata.lng]); M_BOX(GP_RIGHT, GP.SELECT("timeZone", SETTING_TIMEZONE_GMT, mydata.GMT + 12, 0);););
    M_BOX(GP_CENTER, GP.BUTTON_MINI("sntp_btn", SETTING_NTP_SYNC_BTN[mydata.lng], "", GP_BLUE, "", 0, 1););
    
  M_BOX(GP_LEFT, GP.LABEL(SETTING_NTP_PERIODIC[mydata.lng]); M_BOX(GP_RIGHT, GP.SWITCH("ntp_sync_enable", mydata.ntp_sync_enable, GP_BLUE);););
  M_BOX(GP_LEFT, GP.LABEL(SETTING_NTP_PERIODIC_LABEL[mydata.lng]); M_BOX(GP_RIGHT, GP.SPINNER("ntp_sync_interval", mydata.ntp_sync_interval, 10, 1440, 10, 0, GP_BLUE, "50px", 0);););
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
  GP.FORM_END();

  GP.FORM_BEGIN("/change_pass");
  GP.BLOCK_THIN_BEGIN();
  M_BOX(GP_CENTER, GP.LABEL(SETTING_WEB_AUTH_NAME[mydata.lng]););
  GP.HR();
  M_BOX(GP_LEFT, GP.LABEL(SETTING_WEB_AUTH_OLD_PASSWORD[mydata.lng]); M_BOX(GP_RIGHT, GP.PASS("web_old_pass", "Old password", "", "100%", 31, "", false, false);););
  M_BOX(GP_LEFT, GP.LABEL(SETTING_WEB_AUTH_PASSWORD[mydata.lng]); M_BOX(GP_RIGHT, GP.PASS("web_new_pass", "New password", "", "100%", 31, "", false, false);););
  GP.BREAK();
  M_BOX(GP_CENTER, GP.SUBMIT(SETTING_WEB_AUTH_SAVE_BTN[mydata.lng], GP_BLUE););
  GP.BLOCK_END();
  GP.FORM_END();

  GP.FORM_BEGIN("/setting");
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
    GP.BLOCK_THIN_BEGIN();
    GP.SYSTEM_INFO("1.0");
    GP.BLOCK_END();

  } else if (ui.uri("/firmware")) {
    GP.BLOCK_THIN_BEGIN();
    GP.OTA_FIRMWARE("OTA firmware", GP_BLUE);
    GP.OTA_FILESYSTEM("OTA filesystem", GP_BLUE);
    GP.FILE_UPLOAD("file_upload", "Upload file");
    GP.BLOCK_END();

  } else if (ui.uri("/log")) {
    GP.JS_BEGIN();
    *_GPP += F("setInterval(function(){location.reload()},3000)");
    GP.JS_END();
    GP.BLOCK_THIN_BEGIN();
    M_BOX(GP_CENTER, GP.LABEL("System Log"););
    GP.HR();
    byte idx = (log_count < LOG_ENTRIES) ? 0 : log_write_idx;
    byte cnt = log_count;
    for (byte i = 0; i < cnt; i++) {
      byte n = (idx + i) % LOG_ENTRIES;
      char line[LOG_LINE_LEN + 16];
      snprintf(line, sizeof(line), "[%5lu] %c: %s", log_entries[n].time, log_entries[n].level, log_entries[n].msg);
      GP.LABEL(line, "", GP_DEFAULT, 10, false, true);
      GP.BREAK();
    }
    GP.BLOCK_END();

  } else if (ui.uri("/readme")) {
    GP.BLOCK_THIN_BEGIN();
    M_BOX(GP_CENTER, GP.LABEL("README.md"););
    GP.HR();
    if (WiFi.status() == WL_CONNECTED && (readmeCache.length() == 0 || millis() - readmeFetchTime > 300000)) {
      readmeCache = httpGETRequest("https://raw.githubusercontent.com/lvs-vladimir/in14_in18_nixie_clock_esp32/main/README.md");
      if (readmeCache.length() == 0) readmeCache = "Failed to fetch README from GitHub";
      else readmeFetchTime = millis();
    }
    GP.LABEL(readmeCache, "", GP_DEFAULT, 12, false, true);
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
        GP.SELECT(buf, SensorsAutoShowSelect2, mydata.autoshow_select[i + 1], 0, 0, 1);
        M_BOX(GP_RIGHT,
          snprintf(buf, sizeof(buf), "se%d", i);
          GP.SPINNER(buf, mydata.autoshow_select_sec[i + 1], 0, 30, 1, 0, GP_BLUE, "50px", 0);
          snprintf(buf, sizeof(buf), "rm%d", i);
          GP.BUTTON_MINI(buf, "X", "", GP_RED, "20px", 0, 1);
        );
      );
      GP.BREAK();
    }
    if (mydata.autoshow_slots < 5) {
      GP.BUTTON_MINI("add_slot", "+ Add", "", GP_BLUE, "80px", 0, 1);
      GP.BREAK();
    }
    GP.BREAK();
M_BOX(GP_LEFT, GP.LABEL(SETTING_ANIM_NAME[mydata.lng]); M_BOX(GP_RIGHT, GP.SELECT("anim_change", SETTING_ANIM_ARRAY[mydata.lng], mydata.anim_change, 0, 0, 1);););
GP.BREAK();
M_BOX(GP_LEFT, GP.LABEL(SETTING_ANIM_SEC_NAME[mydata.lng]); M_BOX(GP_RIGHT, GP.SPINNER("anim_change_sec", mydata.anim_change_sec, 3, 300, 1, 0, GP_BLUE, "60px", 0);););
GP.BREAK();
M_BOX(GP_LEFT, GP.LABEL(DISPLAY_MODE_ANIM_NAME[mydata.lng]); M_BOX(GP_RIGHT, GP.SELECT("animdots", DISPLAY_MODE_ANIM_ARRAY[mydata.lng], mydata.animdots, 0, 0, 1);););
GP.HR();
M_BOX(GP_LEFT, GP.LABEL(DISPLAY_SECONDS_SWITCH[mydata.lng]); M_BOX(GP_RIGHT, GP.SWITCH("seconds_switch", mydata.seconds_switch, GP_BLUE);););
    GP.HR();
    GP.BLOCK_END();

  GP.BLOCK_THIN_BEGIN();
  M_BOX(GP_CENTER, GP.LABEL(WS2812_LABEL[mydata.lng]););
  GP.HR();
  M_BOX(GP_LEFT, GP.LABEL(WS2812_ENABLE[mydata.lng]); M_BOX(GP_RIGHT, GP.SWITCH("ws2812_enable", mydata.ws2812_enable, GP_BLUE);););
  M_BOX(GP_LEFT, GP.LABEL(WS2812_ANIM[mydata.lng]); M_BOX(GP_RIGHT, GP.SELECT("ws2812_anim", WS2812_ANIM_ARRAY[mydata.lng], mydata.ws2812_anim, 0, 0, 1);););
  M_BOX(GP_LEFT, GP.LABEL(WS2812_RANDOM_SEC[mydata.lng]); M_BOX(GP_RIGHT, GP.SPINNER("ws2812_random_sec", mydata.ws2812_random_sec, 1, 255, 1, 0, GP_BLUE, "60px", 0);););
  M_BOX(GP_LEFT, GP.LABEL(WS2812_BRIGHT[mydata.lng]); M_BOX(GP_RIGHT, GP.SLIDER("ws2812_bright", mydata.ws2812_brightness, 0, 255, 1, 0, GP_BLUE);););
  GP.HR();
  GP.BLOCK_END();

      GP.BLOCK_THIN_BEGIN();
  M_BOX(GP_CENTER, GP.LABEL(VEML_LABEL[mydata.lng]););
  GP.HR();
  M_BOX(GP_LEFT, GP.LABEL(VEML_ENABLE[mydata.lng]); M_BOX(GP_RIGHT, GP.SWITCH("veml_enable", mydata.veml_enable, GP_BLUE);););
  M_BOX(GP_CENTER, GP.LABEL(VEML_DESC[mydata.lng]););
    *_GPP += F("<div style='white-space:nowrap'>"); GP.SPINNER("veml_lm0", mydata.ws2812_lux_min[0], 0, 15000, 1, 0, GP_BLUE, "40px", 0); GP.SPINNER("veml_lx0", mydata.ws2812_lux_max[0], 0, 15000, 1, 0, GP_BLUE, "40px", 0); GP.SPINNER("veml_bv0", mydata.ws2812_bright_val[0], 0, 255, 1, 0, GP_BLUE, "40px", 0); *_GPP += F("</div>");
    *_GPP += F("<div style='white-space:nowrap'>"); GP.SPINNER("veml_lm1", mydata.ws2812_lux_min[1], 0, 15000, 1, 0, GP_BLUE, "40px", 0); GP.SPINNER("veml_lx1", mydata.ws2812_lux_max[1], 0, 15000, 1, 0, GP_BLUE, "40px", 0); GP.SPINNER("veml_bv1", mydata.ws2812_bright_val[1], 0, 255, 1, 0, GP_BLUE, "40px", 0); *_GPP += F("</div>");
    *_GPP += F("<div style='white-space:nowrap'>"); GP.SPINNER("veml_lm2", mydata.ws2812_lux_min[2], 0, 15000, 1, 0, GP_BLUE, "40px", 0); GP.SPINNER("veml_lx2", mydata.ws2812_lux_max[2], 0, 15000, 1, 0, GP_BLUE, "40px", 0); GP.SPINNER("veml_bv2", mydata.ws2812_bright_val[2], 0, 255, 1, 0, GP_BLUE, "40px", 0); *_GPP += F("</div>");
    *_GPP += F("<div style='white-space:nowrap'>"); GP.SPINNER("veml_lm3", mydata.ws2812_lux_min[3], 0, 15000, 1, 0, GP_BLUE, "40px", 0); GP.SPINNER("veml_lx3", mydata.ws2812_lux_max[3], 0, 15000, 1, 0, GP_BLUE, "40px", 0); GP.SPINNER("veml_bv3", mydata.ws2812_bright_val[3], 0, 255, 1, 0, GP_BLUE, "40px", 0); *_GPP += F("</div>");
  GP.BLOCK_END();
  GP.BLOCK_THIN_BEGIN();
  M_BOX(GP_CENTER, GP.LABEL(VEML_NIXIE[mydata.lng]););
  GP.HR();
  M_BOX(GP_CENTER, GP.LABEL(VEML_NIXIE_DESC[mydata.lng]););
    *_GPP += F("<div style='white-space:nowrap'>"); GP.SPINNER("vnlm0", mydata.nixie_lux_min[0], 0, 15000, 1, 0, GP_BLUE, "40px", 0); GP.SPINNER("vnlx0", mydata.nixie_lux_max[0], 0, 15000, 1, 0, GP_BLUE, "40px", 0); GP.SPINNER("vnbv0", mydata.nixie_bright_val[0], 0, 255, 1, 0, GP_BLUE, "40px", 0); *_GPP += F("</div>");
  *_GPP += F("<style>.spinner{display:inline-flex!important;margin:0!important;padding:0!important;border:none!important;background:none!important;align-items:center!important}.spinner input[type=number]{margin:0!important;padding:0!important;font-size:12px!important;width:40px!important;border:1px solid #aaa!important;border-radius:3px!important;text-align:center!important;vertical-align:middle!important}.spinner button{display:inline-flex!important;align-items:center!important;justify-content:center!important;font-size:14px!important;width:22px!important;height:24px!important;padding:0!important;margin:0!important;border:1px solid #aaa!important;border-radius:3px!important;background:#f0f0f0!important;cursor:pointer!important;line-height:1!important}</style>");

    *_GPP += F("<div style='white-space:nowrap'>"); GP.SPINNER("vnlm1", mydata.nixie_lux_min[1], 0, 15000, 1, 0, GP_BLUE, "40px", 0); GP.SPINNER("vnlx1", mydata.nixie_lux_max[1], 0, 15000, 1, 0, GP_BLUE, "40px", 0); GP.SPINNER("vnbv1", mydata.nixie_bright_val[1], 0, 255, 1, 0, GP_BLUE, "40px", 0); *_GPP += F("</div>");
    *_GPP += F("<div style='white-space:nowrap'>"); GP.SPINNER("vnlm2", mydata.nixie_lux_min[2], 0, 15000, 1, 0, GP_BLUE, "40px", 0); GP.SPINNER("vnlx2", mydata.nixie_lux_max[2], 0, 15000, 1, 0, GP_BLUE, "40px", 0); GP.SPINNER("vnbv2", mydata.nixie_bright_val[2], 0, 255, 1, 0, GP_BLUE, "40px", 0); *_GPP += F("</div>");
    *_GPP += F("<div style='white-space:nowrap'>"); GP.SPINNER("vnlm3", mydata.nixie_lux_min[3], 0, 15000, 1, 0, GP_BLUE, "40px", 0); GP.SPINNER("vnlx3", mydata.nixie_lux_max[3], 0, 15000, 1, 0, GP_BLUE, "40px", 0); GP.SPINNER("vnbv3", mydata.nixie_bright_val[3], 0, 255, 1, 0, GP_BLUE, "40px", 0); *_GPP += F("</div>");
    *_GPP += F("<div style='white-space:nowrap'>"); GP.SPINNER("vnlm4", mydata.nixie_lux_min[4], 0, 15000, 1, 0, GP_BLUE, "40px", 0); GP.SPINNER("vnlx4", mydata.nixie_lux_max[4], 0, 15000, 1, 0, GP_BLUE, "40px", 0); GP.SPINNER("vnbv4", mydata.nixie_bright_val[4], 0, 255, 1, 0, GP_BLUE, "40px", 0); *_GPP += F("</div>");
    *_GPP += F("<div style='white-space:nowrap'>"); GP.SPINNER("vnlm5", mydata.nixie_lux_min[5], 0, 15000, 1, 0, GP_BLUE, "40px", 0); GP.SPINNER("vnlx5", mydata.nixie_lux_max[5], 0, 15000, 1, 0, GP_BLUE, "40px", 0); GP.SPINNER("vnbv5", mydata.nixie_bright_val[5], 0, 255, 1, 0, GP_BLUE, "40px", 0); *_GPP += F("</div>");
  GP.BLOCK_END();


  }
  M_BOX(GP_CENTER, GP.BUTTON_MINI_LINK("/logout", WEB_LOGOUT_BUTTON[mydata.lng], GP_RED, "100px"););
  GP.BUILD_END();
}

void action(GyverPortal& ui) {
  if (ui.form("/login")) {
    String pass = ui.arg("login_pass");
    pass.trim();
    if (pass == String(webPass)) {
      webAuthOk = true;
      log_add('I', "Web login OK with pass: %s (expected: %s)", pass.c_str(), webPass);
    } else {
      log_add('W', "Web login rejected: got=%s exp=%s", pass.c_str(), webPass);
    }
    return;
  }

  if (!webAuthOk) return;
  
  if (ui.form("/change_pass")) {
    String oldPass = ui.getString("web_old_pass");
    String newPass = ui.getString("web_new_pass");
    oldPass.trim();
    newPass.trim();
    if (oldPass == String(webPass) && newPass.length() > 0 && newPass.length() <= 31) {
      newPass.toCharArray(webPass, sizeof(webPass));
      strcpy(mydata.webPass, webPass);
      fd.updateNow();
      log_add('I', "Web password changed to: %s", webPass);
    } else {
      log_add('W', "Web password change rejected: old=%s exp=%s newlen=%d", 
              oldPass.c_str(), webPass, newPass.length());
    }
    return;
  }
  
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
    int8_t new_gmt = ui.getInt("timeZone") - 12;
    hour += new_gmt - mydata.GMT;
    mydata.GMT = new_gmt;
    timeClient.setTimeOffset(3600 * mydata.GMT);
  }
    if (ui.click("lng")) {
      mydata.lng = ui.getInt("lng");
    }
    if (ui.clickInt("seconds_switch", mydata.seconds_switch)) {};
    if (ui.clickInt("auto_show_switch", mydata.autoshow_switch));
    if (ui.clickInt("ws2812_enable", mydata.ws2812_enable));
    if (ui.clickInt("ws2812_anim", mydata.ws2812_anim));
    if (ui.clickInt("ws2812_bright", mydata.ws2812_brightness));
    if (ui.clickInt("ws2812_random_sec", mydata.ws2812_random_sec));
    if (ui.clickInt("veml_enable", mydata.veml_enable));
    if (ui.clickInt("veml_lm0", mydata.ws2812_lux_min[0]));
    if (ui.clickInt("veml_lx0", mydata.ws2812_lux_max[0]));
    if (ui.clickInt("veml_bv0", mydata.ws2812_bright_val[0]));
    if (ui.clickInt("veml_lm1", mydata.ws2812_lux_min[1]));
    if (ui.clickInt("veml_lx1", mydata.ws2812_lux_max[1]));
    if (ui.clickInt("veml_bv1", mydata.ws2812_bright_val[1]));
    if (ui.clickInt("veml_lm2", mydata.ws2812_lux_min[2]));
    if (ui.clickInt("veml_lx2", mydata.ws2812_lux_max[2]));
    if (ui.clickInt("veml_bv2", mydata.ws2812_bright_val[2]));
    if (ui.clickInt("veml_lm3", mydata.ws2812_lux_min[3]));
    if (ui.clickInt("veml_lx3", mydata.ws2812_lux_max[3]));
    if (ui.clickInt("veml_bv3", mydata.ws2812_bright_val[3]));
  if (ui.clickInt("ntp_sync_enable", mydata.ntp_sync_enable));
  if (ui.clickInt("ntp_sync_interval", mydata.ntp_sync_interval));

  if (ui.clickInt("vnlm0", mydata.nixie_lux_min[0]));
  if (ui.clickInt("vnlx0", mydata.nixie_lux_max[0]));
  if (ui.clickInt("vnbv0", mydata.nixie_bright_val[0]));
  if (ui.clickInt("vnlm1", mydata.nixie_lux_min[1]));
  if (ui.clickInt("vnlx1", mydata.nixie_lux_max[1]));
  if (ui.clickInt("vnbv1", mydata.nixie_bright_val[1]));
  if (ui.clickInt("vnlm2", mydata.nixie_lux_min[2]));
  if (ui.clickInt("vnlx2", mydata.nixie_lux_max[2]));
  if (ui.clickInt("vnbv2", mydata.nixie_bright_val[2]));
  if (ui.clickInt("vnlm3", mydata.nixie_lux_min[3]));
  if (ui.clickInt("vnlx3", mydata.nixie_lux_max[3]));
  if (ui.clickInt("vnbv3", mydata.nixie_bright_val[3]));
  if (ui.clickInt("vnlm4", mydata.nixie_lux_min[4]));
  if (ui.clickInt("vnlx4", mydata.nixie_lux_max[4]));
  if (ui.clickInt("vnbv4", mydata.nixie_bright_val[4]));
  if (ui.clickInt("vnlm5", mydata.nixie_lux_min[5]));
  if (ui.clickInt("vnlx5", mydata.nixie_lux_max[5]));
  if (ui.clickInt("vnbv5", mydata.nixie_bright_val[5]));
    if (ui.clickInt("anim_change", mydata.anim_change));
  if (ui.clickInt("anim_change_sec", mydata.anim_change_sec));
  if (ui.clickInt("animdots", mydata.animdots));
    if (ui.clickInt("autoshow_sec", mydata.autoshow_min));
    for (byte i = 0; i < mydata.autoshow_slots; i++) {
      char buf[16];
      snprintf(buf, sizeof(buf), "sa%d", i);
      if (ui.click(buf)) mydata.autoshow_select[i + 1] = ui.getInt(buf);
      snprintf(buf, sizeof(buf), "se%d", i);
      if (ui.click(buf)) mydata.autoshow_select_sec[i + 1] = ui.getInt(buf);
      snprintf(buf, sizeof(buf), "rm%d", i);
      if (ui.click(buf)) {
        byte slot = i + 1;
        for (byte j = slot; j < mydata.autoshow_slots; j++) {
          mydata.autoshow_select[j] = mydata.autoshow_select[j + 1];
          mydata.autoshow_select_sec[j] = mydata.autoshow_select_sec[j + 1];
        }
        mydata.autoshow_select[mydata.autoshow_slots] = 0;
        mydata.autoshow_select_sec[mydata.autoshow_slots] = 10;
        mydata.autoshow_slots--;
      }
    }
  if (ui.click("add_slot") && mydata.autoshow_slots < 5) {
    mydata.autoshow_slots++;
    mydata.autoshow_select[mydata.autoshow_slots] = 0;
    mydata.autoshow_select_sec[mydata.autoshow_slots] = 10;
  }
    fd.updateNow();
  }
  if (ui.update()) {
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
