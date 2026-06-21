//19.12.25 Прошика часов на газорвзрядных индикаторах IN-14 и IN-18
//
#include <variables.h>
#include <setting.h>
#include <task.h>

void md5(char * apimd5) {
  unsigned char * hash = MD5::make_hash(apimd5);
  char * md5str = MD5::make_digest(hash, 16);
  snprintf(mydata.NarodmoonApiMD5, sizeof(mydata.NarodmoonApiMD5), "%s", md5str);
  free(md5str);
  free(hash);
}

String httpGETRequest(const char * serverName) {
  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(3000);
  HTTPClient http;
  http.begin(client, serverName);
  http.setTimeout(3000);
  unsigned long t = millis();
  int httpResponseCode = http.GET();
  String payload = "{}";
  if (httpResponseCode == HTTP_CODE_OK) {
    payload = http.getString();
  } else {
    log_add('W', "HTTP %d -> %s", httpResponseCode, serverName);
  }
  http.end();
  log_add('D', "HTTP %dms %s", millis() - t, serverName);
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
