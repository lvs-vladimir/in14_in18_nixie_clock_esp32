void updateCryptoRates()
{
  if (WiFi.status() != WL_CONNECTED) return;

  unsigned long deadline = millis() + 6000;
  String payload;

  payload = httpGETRequest("https://api.coingecko.com/api/v3/simple/price?ids=bitcoin,ethereum&vs_currencies=usd");
  JSONVar json = JSON.parse(payload);
  if (JSON.typeof(json) != "undefined") {
    if (JSON.typeof(json["bitcoin"]) != "undefined" && JSON.typeof(json["bitcoin"]["usd"]) != "undefined")
      pricebtc = (int)json["bitcoin"]["usd"];
    if (JSON.typeof(json["ethereum"]) != "undefined" && JSON.typeof(json["ethereum"]["usd"]) != "undefined")
      priceeth = (int)json["ethereum"]["usd"];
  }

  if (millis() < deadline) {
    payload = httpGETRequest("https://www.cbr-xml-daily.ru/daily_json.js");
    json = JSON.parse(payload);
    if (JSON.typeof(json) != "undefined" && JSON.typeof(json["Valute"]) != "undefined" && JSON.typeof(json["Valute"]["USD"]) != "undefined" && JSON.typeof(json["Valute"]["USD"]["Value"]) != "undefined") {
      usdRubRate = (int)((double)json["Valute"]["USD"]["Value"] + 0.5);
    }
  }

  log_add('I', "Rates update BTC=%d ETH=%d USD/RUB=%d", pricebtc, priceeth, usdRubRate);
}

static timerMinim narodmonTimer(300000);
String NarodmonSensorNames;
int nrd_values[5];
String nrd_names[5];

void narodmonUpdate()
{
  if (WiFi.status() != WL_CONNECTED) return;
  static bool firstRun = true;
  if (!firstRun && !narodmonTimer.isReady()) return;
  firstRun = false;
  if (strlen(mydata.NarodmoonID) == 0 || strlen(mydata.NarodmoonApi) == 0) return;

  md5(mydata.NarodmoonApi);

  String body = "{\"cmd\":\"sensorsOnDevice\",\"devices\":[";
  body += mydata.NarodmoonID;
  body += "],\"uuid\":\"";
  body += mydata.NarodmoonApiMD5;
  body += "\",\"lang\":\"ru\"}";

  WiFiClient client;
  HTTPClient http;
  http.begin(client, "http://api.narodmon.ru");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Narodmon-Api-Key", mydata.NarodmoonApi);
  http.addHeader("User-Agent", "NixieClock");
  http.setTimeout(5000);

  unsigned long t = millis();
  int httpCode = http.POST(body);
  String payload = "{}";
  if (httpCode == HTTP_CODE_OK) {
    payload = http.getString();
  } else {
    log_add('W', "Narodmon HTTP %d", httpCode);
  }
  http.end();
  log_add('D', "Narodmon %dms", millis() - t);

  JSONVar json = JSON.parse(payload);
  if (JSON.typeof(json) == "undefined") {
    log_add('W', "Narodmon JSON parse failed, raw: %.80s", payload.c_str());
    narodmonTimer.reset();
    return;
  }

  if (JSON.typeof(json["devices"]) == "undefined" || json["devices"].length() == 0) {
    log_add('W', "Narodmon no devices in response");
    narodmonTimer.reset();
    return;
  }

  JSONVar sensors = json["devices"][0]["sensors"];
  if (JSON.typeof(sensors) == "undefined") {
    log_add('W', "Narodmon no sensors in device");
    narodmonTimer.reset();
    return;
  }

  for (byte i = 0; i < mydata.nrd_sens_count; i++) {
    byte idx = mydata.nrd_sens[i];
    if (idx >= sensors.length()) continue;
    JSONVar s = sensors[idx];
    if (JSON.typeof(s) == "undefined") continue;
    if (JSON.typeof(s["value"]) == "undefined") continue;

    int val = (int)s["value"];
    nrd_values[i] = val;
    const char* n = (const char*)s["name"];
    nrd_names[i] = n ? String(n) : "";
    const char* u = (const char*)s["unit"];
    String unit = u ? String(u) : "";
    if (unit == "°") unit = "*";
    else if (unit == "mmHg") unit = "mHg";

    SensorsAutoShow[i] = "," + String(val) + unit;
    SensorsDisplay[i] = String(val) + unit;
  }

  NarodmonSensorNames = "";
  for (byte i = 0; i < sensors.length(); i++) {
    JSONVar s = sensors[i];
    if (JSON.typeof(s) == "undefined") continue;
    int val = (int)s["value"];
    const char* n = (const char*)s["name"];
    const char* u = (const char*)s["unit"];
    String unit = u ? String(u) : "";
    if (unit == "°") unit = "*";
    else if (unit == "mmHg") unit = "mHg";
    String name = n ? String(n) : String(i);
    if (i > 0) NarodmonSensorNames += ",";
    NarodmonSensorNames += String(i) + ": " + String(val) + unit + " " + name;
  }
  log_add('I', "Narodmon sensors: %s", NarodmonSensorNames.c_str());
  log_add('I', "Narodmon update OK");
  narodmonTimer.reset();
}

void getTemp2(byte i)
{
  if (i == 0) {
    String serverPath = "https://api.openweathermap.org/data/2.5/weather?q=" + (String)mydata.owCity + "&APPID=" + (String)mydata.owMapApiKey + "&units=metric";
    String jsonBuffer = httpGETRequest(serverPath.c_str());
    JSONVar myObject = JSON.parse(jsonBuffer);
    if (JSON.typeof(myObject) == "undefined") {
      log_add('W', "OWM JSON parse failed");
      return;
    }
    if (JSON.typeof(myObject["main"]) == "undefined") {
      log_add('W', "OWM response missing 'main', raw: %.80s", jsonBuffer.c_str());
      return;
    }
    if (JSON.typeof(myObject["main"]["temp"]) != "undefined") {
      optemperature = (int)myObject["main"]["temp"];
      TempValue = optemperature;
    }
    if (JSON.typeof(myObject["main"]["pressure"]) != "undefined") oppressure = (int)myObject["main"]["pressure"];
    if (JSON.typeof(myObject["main"]["humidity"]) != "undefined") ophumidity = (int)myObject["main"]["humidity"];
    log_add('I', "OWM update opt=%d press=%d hum=%d", optemperature, oppressure, ophumidity);

    SensorsAutoShow[4] = "," + String(optemperature) + "*";
    SensorsAutoShow[5] = "," + String(oppressure) + "mHg";
    SensorsAutoShow[6] = "," + String(ophumidity) + "%";

    SensorsDisplay[4] = String(optemperature) + "*";
    SensorsDisplay[5] = String(oppressure) + "mHg";
    SensorsDisplay[6] = String(ophumidity) + "%";
  } else if (i == 1) {
    narodmonUpdate();
  }
}

void rebuildSensorsAutoShowSelect()
{
  SensorsAutoShowSelect2 = ",";
  bool hasItem = false;
  for (byte k = 4; k <= 6; k++) {
    String item = SensorsAutoShow[k];
    while (item.startsWith(",")) item.remove(0, 1);
    if (item.length() == 0) continue;
    if (hasItem) SensorsAutoShowSelect2 += ",";
    SensorsAutoShowSelect2 += item;
    hasItem = true;
  }

  String extra[4];
  extra[0] = String(pricebtc) + "$ BTC";
  extra[1] = String(priceeth) + "$ ETH";
  extra[2] = String(usdRubRate) + " RUB/USD";
  extra[3] = String(dayOfMonth) + "." + String(month + 1) + "." + String(year);
  for (byte k = 0; k < 4; k++) {
    if (hasItem) SensorsAutoShowSelect2 += ",";
    SensorsAutoShowSelect2 += extra[k];
    hasItem = true;
  }

  for (byte k = 0; k < mydata.nrd_sens_count; k++) {
    String item = SensorsAutoShow[k];
    while (item.startsWith(",")) item.remove(0, 1);
    if (item.length() == 0) item = "s" + String(k + 1);
    if (hasItem) SensorsAutoShowSelect2 += ",";
    SensorsAutoShowSelect2 += item;
    hasItem = true;
  }
  log_add('D', "SELECTLIST len=%d sel1=%d sel2=%d list=%s",
          SensorsAutoShowSelect2.length(), mydata.autoshow_select[1], mydata.autoshow_select[2],
          SensorsAutoShowSelect2.c_str());
}
