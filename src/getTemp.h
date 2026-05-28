void updateCryptoRates()
{
  if (WiFi.status() != WL_CONNECTED) return;

  String payload = httpGETRequest("https://api.coingecko.com/api/v3/simple/price?ids=bitcoin,ethereum&vs_currencies=usd");
  JSONVar json = JSON.parse(payload);
  if (JSON.typeof(json) != "undefined") {
    if (JSON.typeof(json["bitcoin"]["usd"]) != "undefined") pricebtc = (int)json["bitcoin"]["usd"];
    if (JSON.typeof(json["ethereum"]["usd"]) != "undefined") priceeth = (int)json["ethereum"]["usd"];
  }

  payload = httpGETRequest("https://www.cbr-xml-daily.ru/daily_json.js");
  json = JSON.parse(payload);
  if (JSON.typeof(json) != "undefined" && JSON.typeof(json["Valute"]["USD"]["Value"]) != "undefined") {
    usdRubRate = (int)((double)json["Valute"]["USD"]["Value"] + 0.5);
  }

  log_add('I', "Rates update BTC=%d ETH=%d USD/RUB=%d", pricebtc, priceeth, usdRubRate);
}

//***************Получаем темперутуру с openweathermap.org и narodmon.ru**********************************
void getTemp2(byte i){

  String serverPath;
  if (i==0) serverPath = "https://api.openweathermap.org/data/2.5/weather?q=" + (String)mydata.owCity + "&APPID=" + (String)mydata.owMapApiKey +"&units=metric";
  else if (i==1) serverPath = "https://narodmon.ru/api/sensorsOnDevice?id="+(String)mydata.NarodmoonID+"&uuid="+(String)mydata.NarodmoonApiMD5+"&api_key="+(String)mydata.NarodmoonApi+"&lang=ru";
  else if (i>1) return;
  String jsonBuffer = httpGETRequest(serverPath.c_str());
  JSONVar myObject = JSON.parse(jsonBuffer);
  if (i==0) {
    byte j=4;
    while (j<=6) {
      SensorsAutoShow[j]="";//очистка
      SensorsDisplay[j]="";//очистка
      j++;
    }
        optemperature = myObject["main"]["temp"];
        TempValue = optemperature;
        log_add('I', "OWM temp update opt=%d TempValue=%d", optemperature, TempValue);
        oppressure = myObject["main"]["pressure"];
        ophumidity = myObject["main"]["humidity"];

        SensorsAutoShow[4]+=",";
        SensorsAutoShow[4]+=optemperature;
        SensorsAutoShow[4]+="*";
    
        SensorsAutoShow[5]+=",";
        SensorsAutoShow[5]+=oppressure;
        SensorsAutoShow[5]+="mHg";
  
        SensorsAutoShow[6]+=",";
        SensorsAutoShow[6]+=ophumidity;
        SensorsAutoShow[6]+="%";
  
        SensorsDisplay[4]+=",";
        SensorsDisplay[4]+=optemperature;
        SensorsDisplay[4]+="*";
        SensorsDisplay[5]+=",";
        SensorsDisplay[5]+=oppressure;
        SensorsDisplay[5]+="mHg";
        SensorsDisplay[6]+=",";
        SensorsDisplay[6]+=ophumidity;
        SensorsDisplay[6]+="%";

  }
  if (i==1) {
    byte j=0;
    while (j<=3) {
      SensorsAutoShow[j]="";//очистка
      SensorsDisplay[j]="";//очистка
      j++;
    }
    byte f=0;
    while (f<=20) {
      f++;
    }
    for(byte i=0; i<=3; i++){
     int b= myObject["sensors"][mydata.nrd_sens[i]]["value"];
     String c = myObject["sensors"][mydata.nrd_sens[i]]["unit"];

     if (c.length()!=0){
      if(c=="°") {c=""; c+="*";}
      if(c=="mmHg") {c=""; c+="mHg";}
      SensorsAutoShow[i]+=",";
      SensorsAutoShow[i]+=b;
      SensorsAutoShow[i]+=c;
      SensorsDisplay[i]+=b;
      SensorsDisplay[i]+=c;
     }
    }
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
  log_add('D', "SELECTLIST len=%d sel1=%d sel2=%d list=%s",
          SensorsAutoShowSelect2.length(), mydata.autoshow_select[1], mydata.autoshow_select[2],
          SensorsAutoShowSelect2.c_str());
}
