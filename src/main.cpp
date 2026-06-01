//19.12.25 Прошика часов на газорвзрядных индикаторах IN-14 и IN-18
//
#include <variables.h>
#include <setting.h>
#include <task.h>

void md5(char * apimd5) {
  unsigned char * hash = MD5::make_hash(apimd5);
  char * md5str = MD5::make_digest(hash, 16);
  sprintf_P(mydata.NarodmoonApiMD5, (PGM_P) F("%S"), md5str);
}

String httpGETRequest(const char * serverName) {
  HTTPClient http;
  String a;
  http.begin(serverName);
  int httpResponseCode = http.GET();
  String payload = "{}";
  if (httpResponseCode == HTTP_CODE_OK) {
    payload = http.getString();
  } else {
    http.end();
    return a;
  }
  http.end();
  return payload;
}

void ValueTempUpdate() {
  TempValue = optemperature;
}

#include <getTemp.h>
#include <webui.h>
#include <effects.h>
#include <loop.h>
#include <setup.h>
