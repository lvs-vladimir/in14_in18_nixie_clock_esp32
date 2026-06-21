#include <Arduino.h>
#include <NTPClient.h>
#include "time.h"
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <LittleFS.h>
#include <FileData.h>
#include <GyverPortal.h>
#include <MD5.h>
#include <Arduino_JSON.h>
#include <stdarg.h>
#include "SPI.h"
#include "timer2Minim.h"
#include "ws2812_driver.h"
#include "Adafruit_VEML7700.h"
#include "lwip/raw.h"
#include "lwip/ip4.h"
#include "lwip/icmp.h"
#include "lwip/pbuf.h"

#define FORMAT_LITTLEFS_IF_FAILED true

struct Data {
  char ssid[40];
  char pass[20];
  char owMapApiKey[60];
  char owCity[40];
  char NTPserver[40];
  char NarodmoonApi[20];
  char NarodmoonApiMD5[40];
  char NarodmoonID[10];
  byte lng;
  byte animdots;
  int GMT;
  boolean dots_switch, seconds_switch, autoshow_switch, random_autoshow_switch;
  byte nrd_sens[5];
  byte nrd_type_sensor[6];
  byte autoshow_min, autoshow_slots, autoshow_select[6], autoshow_select_sec[7];
  byte anim_change;
  byte anim_change_sec;
  char webPass[32];
  boolean ws2812_enable;
  byte ws2812_anim;
  byte ws2812_brightness;
  byte ws2812_random_sec;
  boolean veml_enable;
  byte ws2812_br_ranges;
  int ws2812_lux_min[4];
  int ws2812_lux_max[4];
  byte ws2812_bright_val[4];
  int nixie_lux_min[6];
  int nixie_lux_max[6];
  byte nixie_bright_val[6];
  boolean ntp_sync_enable;
  int ntp_sync_interval;
  boolean anim_by_mode;
  byte anim_time_mode;
  byte anim_data_mode;
  boolean ap_mode;
  boolean offtime_enable;
  byte offtime_start_h;
  byte offtime_start_m;
  byte offtime_end_h;
  byte offtime_end_m;
  byte nrd_sens_count;
  boolean buzzer_enable;
  byte buzzer_interval;
  unsigned int buzzer_duration;
  boolean alarm_enable;
  byte alarm_hour;
  byte alarm_minute;
  byte alarm_melody_idx;
  byte alarm_duration;
  byte alarm_volume;
  boolean reboot_enable;
  byte reboot_hour;
  byte reboot_minute;
};
Data mydata;
FileData fd(&LittleFS, "/setting.dat", 'B', &mydata, sizeof(mydata));
GyverPortal ui(&LittleFS);
char webLogin[16] = "";
char webPass[32] = "admin";
bool webAuthOk = false;

#define LOG_ENTRIES 20
#define LOG_LINE_LEN 160
struct LogEntry {
  uint32_t time;
  char level;
  char msg[LOG_LINE_LEN];
};
LogEntry log_entries[LOG_ENTRIES];
byte log_write_idx = 0;
byte log_count = 0;

void log_add(char level, const char* fmt, ...) {
  char buf[LOG_LINE_LEN];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, LOG_LINE_LEN, fmt, args);
  va_end(args);
  uint32_t now = millis();
  log_entries[log_write_idx].time = now;
  log_entries[log_write_idx].level = level;
  strncpy(log_entries[log_write_idx].msg, buf, LOG_LINE_LEN - 1);
  log_entries[log_write_idx].msg[LOG_LINE_LEN - 1] = 0;
  log_write_idx = (log_write_idx + 1) % LOG_ENTRIES;
  if (log_count < LOG_ENTRIES) log_count++;
  Serial.printf("[%lu] %c: %s\n", now, level, buf);
}

#define IN_14

#ifdef IN_14
#define YOUR_HOSTNAME "IN14-NIXIE" // Имя устройства для отображение в списке подключенных устройств роутера
#endif

#ifdef IN_18
#define YOUR_HOSTNAME "IN18-NIXIE" // Имя устройства для отображение в списке подключенных устройств роутера
#endif

#define CS_ON (GPIO.out_w1ts = ((uint32_t)1 << 5))
#define CS_OFF (GPIO.out_w1tc = ((uint32_t)1 << 5))
#define CS_ON_HSPI (GPIO.out_w1ts = ((uint32_t)1 << 15))
#define CS_OFF_HSPI (GPIO.out_w1tc = ((uint32_t)1 << 15))

#define BL 2
#define G 16
#define LE 15
#define LEDS_COUNT 6

#ifdef IN_14
#define LEDS_PIN 4
#endif

#ifdef IN_18
#define LEDS_PIN 16
#endif

#define CHANNEL 1

#define HSPI_MISO 12
#define HSPI_MOSI 13
#define HSPI_SCLK 14
#define HSPI_SS 15


#define LAMP_DOT_BIT 30
#define LAMP_PLUS_BIT 31
#define LAMP_CELSIUS_BIT 30
#define LAMP_PERCENT_BIT 31
#define num_ranges 6

CRGB leds[LEDS_COUNT];
Adafruit_VEML7700 veml = Adafruit_VEML7700();
bool veml_ok = false;

SPIClass *vspi = NULL;
SPIClass *hspi = NULL;

hw_timer_t *Timer0_Cfg = NULL;
hw_timer_t *Timer1_Cfg = NULL;
hw_timer_t *ws2812Timer = NULL;
volatile bool ws2812_timer_flag = false;

timerMinim DotTimer(91);
timerMinim mooveNixie(100);
timerMinim ChangeCathodeTimer(60);
timerMinim SwitchDisplayTimer(40);
timerMinim SensorSelectTimer(3000);
timerMinim NtpSyncTimer(3600000);
timerMinim OwmUpdateTimer(300000);
timerMinim CoinUpdateTimer(600000);
timerMinim vemlRead(2000);
timerMinim buzzerTimer(150);
timerMinim alarmTimer(100);

#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568

struct MelodyNote {
  uint16_t freq;
  uint16_t dur;
};

static const MelodyNote melody_chime[] = {
  {NOTE_C5, 200}, {NOTE_E5, 200}, {NOTE_G5, 200}, {NOTE_C6, 400},
  {0, 120},
  {NOTE_B5, 150}, {NOTE_G5, 150}, {NOTE_E5, 150}, {NOTE_C5, 400},
  {0, 120},
  {NOTE_D5, 150}, {NOTE_G5, 150}, {NOTE_B5, 200},
  {NOTE_G5, 150}, {NOTE_E5, 150}, {NOTE_C5, 300},
  {0, 120},
  {NOTE_E5, 150}, {NOTE_G5, 150}, {NOTE_C6, 300},
  {NOTE_B5, 150}, {NOTE_C6, 150}, {NOTE_G5, 200},
  {NOTE_E5, 200}, {NOTE_C5, 500},
  {0, 0}
};

static const MelodyNote melody_morning[] = {
  {NOTE_E5, 150}, {NOTE_G5, 150}, {NOTE_C6, 200}, {NOTE_E6, 200},
  {NOTE_C6, 150}, {NOTE_G5, 150}, {NOTE_E5, 150}, {NOTE_C5, 300},
  {0, 100},
  {NOTE_D5, 150}, {NOTE_G5, 150}, {NOTE_B5, 200}, {NOTE_D6, 200},
  {NOTE_B5, 150}, {NOTE_G5, 150}, {NOTE_D5, 150}, {NOTE_G5, 300},
  {0, 100},
  {NOTE_C5, 150}, {NOTE_E5, 150}, {NOTE_G5, 150}, {NOTE_C6, 200},
  {NOTE_E6, 300}, {NOTE_D6, 150}, {NOTE_C6, 150},
  {NOTE_G5, 150}, {NOTE_C6, 200}, {NOTE_E5, 200}, {NOTE_C5, 400},
  {0, 0}
};

static const MelodyNote melody_classical[] = {
  {NOTE_G5, 200}, {NOTE_C6, 200}, {NOTE_E6, 200}, {NOTE_C6, 200},
  {NOTE_F5, 150}, {NOTE_A5, 150}, {NOTE_C6, 200}, {NOTE_A5, 150},
  {NOTE_F5, 150}, {NOTE_D5, 300},
  {0, 100},
  {NOTE_E5, 150}, {NOTE_G5, 150}, {NOTE_C6, 200}, {NOTE_G5, 150},
  {NOTE_E5, 150}, {NOTE_C5, 300},
  {0, 100},
  {NOTE_F5, 150}, {NOTE_A5, 150}, {NOTE_D6, 200}, {NOTE_A5, 150},
  {NOTE_F5, 150}, {NOTE_E5, 200}, {NOTE_D5, 200}, {NOTE_C5, 400},
  {0, 0}
};

static const MelodyNote melody_simple[] = {
  {NOTE_C5, 200}, {0, 100},
  {NOTE_E5, 200}, {0, 100},
  {NOTE_G5, 200}, {0, 100},
  {NOTE_C6, 400}, {0, 200},
  {NOTE_G5, 200}, {0, 100},
  {NOTE_E5, 200}, {0, 100},
  {NOTE_C5, 500},
  {0, 0}
};

static const MelodyNote melody_tetris[] = {
  {NOTE_E5, 416}, {NOTE_B4, 208}, {NOTE_C5, 208}, {NOTE_D5, 416},
  {NOTE_C5, 208}, {NOTE_B4, 208}, {NOTE_A4, 416}, {NOTE_A4, 208},
  {NOTE_C5, 208}, {NOTE_E5, 416}, {NOTE_D5, 208}, {NOTE_C5, 208},
  {NOTE_B4, 624}, {NOTE_C5, 208}, {NOTE_D5, 416}, {NOTE_E5, 416},
  {NOTE_C5, 416}, {NOTE_A4, 416}, {NOTE_A4, 416}, {0, 416},
  {0, 208}, {NOTE_D5, 416}, {NOTE_F5, 208}, {NOTE_A5, 416},
  {NOTE_G5, 208}, {NOTE_F5, 208}, {NOTE_E5, 624}, {NOTE_C5, 208},
  {NOTE_E5, 416}, {NOTE_D5, 208}, {NOTE_C5, 208}, {NOTE_B4, 416},
  {NOTE_B4, 208}, {NOTE_C5, 208}, {NOTE_D5, 416}, {NOTE_E5, 416},
  {NOTE_C5, 416}, {NOTE_A4, 416}, {NOTE_A4, 416}, {0, 416},
  {NOTE_E5, 833}, {NOTE_C5, 833}, {NOTE_D5, 833}, {NOTE_B4, 833},
  {NOTE_C5, 833}, {NOTE_A4, 833}, {NOTE_B4, 1666},
  {NOTE_E5, 833}, {NOTE_C5, 833}, {NOTE_D5, 833}, {NOTE_B4, 833},
  {NOTE_C5, 416}, {NOTE_E5, 416}, {NOTE_A5, 833}, {NOTE_GS5, 1666},
  {NOTE_E5, 416}, {NOTE_B4, 208}, {NOTE_C5, 208}, {NOTE_D5, 416},
  {NOTE_C5, 208}, {NOTE_B4, 208}, {NOTE_A4, 416}, {NOTE_A4, 208},
  {NOTE_C5, 208}, {NOTE_E5, 416}, {NOTE_D5, 208}, {NOTE_C5, 208},
  {NOTE_B4, 624}, {NOTE_C5, 208}, {NOTE_D5, 416}, {NOTE_E5, 416},
  {NOTE_C5, 416}, {NOTE_A4, 416}, {NOTE_A4, 416}, {0, 416},
  {0, 208}, {NOTE_D5, 416}, {NOTE_F5, 208}, {NOTE_A5, 416},
  {NOTE_G5, 208}, {NOTE_F5, 208}, {0, 208}, {NOTE_E5, 416},
  {NOTE_C5, 208}, {NOTE_E5, 416}, {NOTE_D5, 208}, {NOTE_C5, 208},
  {0, 208}, {NOTE_B4, 416}, {NOTE_C5, 208}, {NOTE_D5, 416},
  {NOTE_E5, 416}, {0, 208}, {NOTE_C5, 416}, {NOTE_A4, 208},
  {NOTE_A4, 416}, {0, 416},
  {0, 0}
};

static const MelodyNote melody_mario[] = {
  {NOTE_E5, 135}, {0, 15}, {NOTE_E5, 135}, {0, 15},
  {0, 150}, {NOTE_E5, 135}, {0, 15}, {0, 150},
  {NOTE_C5, 135}, {0, 15}, {NOTE_E5, 135}, {0, 15},
  {NOTE_G5, 270}, {0, 30}, {0, 300}, {NOTE_G4, 135},
  {0, 15}, {0, 300}, {NOTE_C5, 405}, {0, 45},
  {NOTE_G4, 135}, {0, 15}, {0, 300}, {NOTE_E4, 405},
  {0, 45}, {NOTE_A4, 270}, {0, 30}, {NOTE_B4, 270},
  {0, 30}, {NOTE_AS4, 135}, {0, 15}, {NOTE_A4, 270},
  {0, 30}, {NOTE_G4, 202}, {0, 23}, {NOTE_E5, 202},
  {0, 23}, {NOTE_G5, 202}, {0, 23}, {NOTE_A5, 270},
  {0, 30}, {NOTE_F5, 135}, {0, 15}, {NOTE_G5, 135},
  {0, 15}, {0, 150}, {NOTE_E5, 270}, {0, 30},
  {NOTE_C5, 135}, {0, 15}, {NOTE_D5, 135}, {0, 15},
  {NOTE_B4, 405}, {0, 45}, {NOTE_C5, 405}, {0, 45},
  {NOTE_G4, 135}, {0, 15}, {0, 300}, {NOTE_E4, 405},
  {0, 45}, {NOTE_A4, 270}, {0, 30}, {NOTE_B4, 270},
  {0, 30}, {NOTE_AS4, 135}, {0, 15}, {NOTE_A4, 270},
  {0, 30}, {NOTE_G4, 202}, {0, 23}, {NOTE_E5, 202},
  {0, 23}, {NOTE_G5, 202}, {0, 23}, {NOTE_A5, 270},
  {0, 30}, {NOTE_F5, 135}, {0, 15}, {NOTE_G5, 135},
  {0, 15}, {0, 150}, {NOTE_E5, 270}, {0, 30},
  {NOTE_C5, 135}, {0, 15}, {NOTE_D5, 135}, {0, 15},
  {NOTE_B4, 405}, {0, 45}, {0, 300}, {NOTE_G5, 135},
  {0, 15}, {NOTE_FS5, 135}, {0, 15}, {NOTE_F5, 135},
  {0, 15}, {NOTE_DS5, 270}, {0, 30}, {NOTE_E5, 135},
  {0, 15}, {0, 150}, {NOTE_GS4, 135}, {0, 15},
  {NOTE_A4, 135}, {0, 15}, {NOTE_C4, 135}, {0, 15},
  {0, 150}, {NOTE_A4, 135}, {0, 15}, {NOTE_C5, 135},
  {0, 15}, {NOTE_D5, 135}, {0, 15}, {0, 300},
  {NOTE_DS5, 270}, {0, 30}, {0, 150}, {NOTE_D5, 405},
  {0, 45}, {NOTE_C5, 540}, {0, 60}, {0, 600},
  {0, 300}, {NOTE_G5, 135}, {0, 15}, {NOTE_FS5, 135},
  {0, 15}, {NOTE_F5, 135}, {0, 15}, {NOTE_DS5, 270},
  {0, 30}, {NOTE_E5, 135}, {0, 15}, {0, 150},
  {NOTE_GS4, 135}, {0, 15}, {NOTE_A4, 135}, {0, 15},
  {NOTE_C4, 135}, {0, 15}, {0, 150}, {NOTE_A4, 135},
  {0, 15}, {NOTE_C5, 135}, {0, 15}, {NOTE_D5, 135},
  {0, 15}, {0, 300}, {NOTE_DS5, 270}, {0, 30},
  {0, 150}, {NOTE_D5, 405}, {0, 45}, {NOTE_C5, 540},
  {0, 60}, {0, 600}, {NOTE_C5, 135}, {0, 15},
  {NOTE_C5, 270}, {0, 30}, {NOTE_C5, 135}, {0, 15},
  {0, 150}, {NOTE_C5, 135}, {0, 15}, {NOTE_D5, 270},
  {0, 30}, {NOTE_E5, 135}, {0, 15}, {NOTE_C5, 270},
  {0, 30}, {NOTE_A4, 135}, {0, 15}, {NOTE_G4, 540},
  {0, 60}, {NOTE_C5, 135}, {0, 15}, {NOTE_C5, 270},
  {0, 30}, {NOTE_C5, 135}, {0, 15}, {0, 150},
  {NOTE_C5, 135}, {0, 15}, {NOTE_D5, 135}, {0, 15},
  {NOTE_E5, 135}, {0, 15}, {0, 1200}, {NOTE_C5, 135},
  {0, 15}, {NOTE_C5, 270}, {0, 30}, {NOTE_C5, 135},
  {0, 15}, {0, 150}, {NOTE_C5, 135}, {0, 15},
  {NOTE_D5, 270}, {0, 30}, {NOTE_E5, 135}, {0, 15},
  {NOTE_C5, 270}, {0, 30}, {NOTE_A4, 135}, {0, 15},
  {NOTE_G4, 540}, {0, 60}, {NOTE_E5, 135}, {0, 15},
  {NOTE_E5, 135}, {0, 15}, {0, 150}, {NOTE_E5, 135},
  {0, 15}, {0, 150}, {NOTE_C5, 135}, {0, 15},
  {NOTE_E5, 270}, {0, 30}, {NOTE_G5, 270}, {0, 30},
  {0, 300}, {NOTE_G4, 270}, {0, 30}, {0, 300},
  {NOTE_C5, 405}, {0, 45}, {NOTE_G4, 135}, {0, 15},
  {0, 300}, {NOTE_E4, 405}, {0, 45}, {NOTE_A4, 270},
  {0, 30}, {NOTE_B4, 270}, {0, 30}, {NOTE_AS4, 135},
  {0, 15}, {NOTE_A4, 270}, {0, 30}, {NOTE_G4, 202},
  {0, 23}, {NOTE_E5, 202}, {0, 23}, {NOTE_G5, 202},
  {0, 23}, {NOTE_A5, 270}, {0, 30}, {NOTE_F5, 135},
  {0, 15}, {NOTE_G5, 135}, {0, 15}, {0, 150},
  {NOTE_E5, 270}, {0, 30}, {NOTE_C5, 135}, {0, 15},
  {NOTE_D5, 135}, {0, 15}, {NOTE_B4, 405}, {0, 45},
  {NOTE_C5, 405}, {0, 45}, {NOTE_G4, 135}, {0, 15},
  {0, 300}, {NOTE_E4, 405}, {0, 45}, {NOTE_A4, 270},
  {0, 30}, {NOTE_B4, 270}, {0, 30}, {NOTE_AS4, 135},
  {0, 15}, {NOTE_A4, 270}, {0, 30}, {NOTE_G4, 202},
  {0, 23}, {NOTE_E5, 202}, {0, 23}, {NOTE_G5, 202},
  {0, 23}, {NOTE_A5, 270}, {0, 30}, {NOTE_F5, 135},
  {0, 15}, {NOTE_G5, 135}, {0, 15}, {0, 150},
  {NOTE_E5, 270}, {0, 30}, {NOTE_C5, 135}, {0, 15},
  {NOTE_D5, 135}, {0, 15}, {NOTE_B4, 405}, {0, 45},
  {NOTE_E5, 135}, {0, 15}, {NOTE_C5, 270}, {0, 30},
  {NOTE_G4, 135}, {0, 15}, {0, 300}, {NOTE_GS4, 270},
  {0, 30}, {NOTE_A4, 135}, {0, 15}, {NOTE_F5, 270},
  {0, 30}, {NOTE_F5, 135}, {0, 15}, {NOTE_A4, 540},
  {0, 60}, {NOTE_D5, 202}, {0, 23}, {NOTE_A5, 202},
  {0, 23}, {NOTE_A5, 202}, {0, 23}, {NOTE_A5, 202},
  {0, 23}, {NOTE_G5, 202}, {0, 23}, {NOTE_F5, 202},
  {0, 23}, {NOTE_E5, 135}, {0, 15}, {NOTE_C5, 270},
  {0, 30}, {NOTE_A4, 135}, {0, 15}, {NOTE_G4, 540},
  {0, 60}, {NOTE_E5, 135}, {0, 15}, {NOTE_C5, 270},
  {0, 30}, {NOTE_G4, 135}, {0, 15}, {0, 300},
  {NOTE_GS4, 270}, {0, 30}, {NOTE_A4, 135}, {0, 15},
  {NOTE_F5, 270}, {0, 30}, {NOTE_F5, 135}, {0, 15},
  {NOTE_A4, 540}, {0, 60}, {NOTE_B4, 135}, {0, 15},
  {NOTE_F5, 270}, {0, 30}, {NOTE_F5, 135}, {0, 15},
  {NOTE_F5, 202}, {0, 23}, {NOTE_E5, 202}, {0, 23},
  {NOTE_D5, 202}, {0, 23}, {NOTE_C5, 135}, {0, 15},
  {NOTE_E4, 270}, {0, 30}, {NOTE_E4, 135}, {0, 15},
  {NOTE_C4, 540}, {0, 60}, {NOTE_E5, 135}, {0, 15},
  {NOTE_C5, 270}, {0, 30}, {NOTE_G4, 135}, {0, 15},
  {0, 300}, {NOTE_GS4, 270}, {0, 30}, {NOTE_A4, 135},
  {0, 15}, {NOTE_F5, 270}, {0, 30}, {NOTE_F5, 135},
  {0, 15}, {NOTE_A4, 540}, {0, 60}, {NOTE_D5, 202},
  {0, 23}, {NOTE_A5, 202}, {0, 23}, {NOTE_A5, 202},
  {0, 23}, {NOTE_A5, 202}, {0, 23}, {NOTE_G5, 202},
  {0, 23}, {NOTE_F5, 202}, {0, 23}, {NOTE_E5, 135},
  {0, 15}, {NOTE_C5, 270}, {0, 30}, {NOTE_A4, 135},
  {0, 15}, {NOTE_G4, 540}, {0, 60}, {NOTE_E5, 135},
  {0, 15}, {NOTE_C5, 270}, {0, 30}, {NOTE_G4, 135},
  {0, 15}, {0, 300}, {NOTE_GS4, 270}, {0, 30},
  {NOTE_A4, 135}, {0, 15}, {NOTE_F5, 270}, {0, 30},
  {NOTE_F5, 135}, {0, 15}, {NOTE_A4, 540}, {0, 60},
  {NOTE_B4, 135}, {0, 15}, {NOTE_F5, 270}, {0, 30},
  {NOTE_F5, 135}, {0, 15}, {NOTE_F5, 202}, {0, 23},
  {NOTE_E5, 202}, {0, 23}, {NOTE_D5, 202}, {0, 23},
  {NOTE_C5, 135}, {0, 15}, {NOTE_E4, 270}, {0, 30},
  {NOTE_E4, 135}, {0, 15}, {NOTE_C4, 540}, {0, 60},
  {NOTE_C5, 135}, {0, 15}, {NOTE_C5, 270}, {0, 30},
  {NOTE_C5, 135}, {0, 15}, {0, 150}, {NOTE_C5, 135},
  {0, 15}, {NOTE_D5, 135}, {0, 15}, {NOTE_E5, 135},
  {0, 15}, {0, 1200}, {NOTE_C5, 135}, {0, 15},
  {NOTE_C5, 270}, {0, 30}, {NOTE_C5, 135}, {0, 15},
  {0, 150}, {NOTE_C5, 135}, {0, 15}, {NOTE_D5, 270},
  {0, 30}, {NOTE_E5, 135}, {0, 15}, {NOTE_C5, 270},
  {0, 30}, {NOTE_A4, 135}, {0, 15}, {NOTE_G4, 540},
  {0, 60}, {NOTE_E5, 135}, {0, 15}, {NOTE_E5, 135},
  {0, 15}, {0, 150}, {NOTE_E5, 135}, {0, 15},
  {0, 150}, {NOTE_C5, 135}, {0, 15}, {NOTE_E5, 270},
  {0, 30}, {NOTE_G5, 270}, {0, 30}, {0, 300},
  {NOTE_G4, 270}, {0, 30}, {0, 300}, {NOTE_E5, 135},
  {0, 15}, {NOTE_C5, 270}, {0, 30}, {NOTE_G4, 135},
  {0, 15}, {0, 300}, {NOTE_GS4, 270}, {0, 30},
  {NOTE_A4, 135}, {0, 15}, {NOTE_F5, 270}, {0, 30},
  {NOTE_F5, 135}, {0, 15}, {NOTE_A4, 540}, {0, 60},
  {NOTE_D5, 202}, {0, 23}, {NOTE_A5, 202}, {0, 23},
  {NOTE_A5, 202}, {0, 23}, {NOTE_A5, 202}, {0, 23},
  {NOTE_G5, 202}, {0, 23}, {NOTE_F5, 202}, {0, 23},
  {NOTE_E5, 135}, {0, 15}, {NOTE_C5, 270}, {0, 30},
  {NOTE_A4, 135}, {0, 15}, {NOTE_G4, 540}, {0, 60},
  {NOTE_E5, 135}, {0, 15}, {NOTE_C5, 270}, {0, 30},
  {NOTE_G4, 135}, {0, 15}, {0, 300}, {NOTE_GS4, 270},
  {0, 30}, {NOTE_A4, 135}, {0, 15}, {NOTE_F5, 270},
  {0, 30}, {NOTE_F5, 135}, {0, 15}, {NOTE_A4, 540},
  {0, 60}, {NOTE_B4, 135}, {0, 15}, {NOTE_F5, 270},
  {0, 30}, {NOTE_F5, 135}, {0, 15}, {NOTE_F5, 202},
  {0, 23}, {NOTE_E5, 202}, {0, 23}, {NOTE_D5, 202},
  {0, 23}, {NOTE_C5, 135}, {0, 15}, {NOTE_E4, 270},
  {0, 30}, {NOTE_E4, 135}, {0, 15}, {NOTE_C4, 540},
  {0, 60}, {NOTE_C5, 405}, {0, 45}, {NOTE_G4, 405},
  {0, 45}, {NOTE_E4, 270}, {0, 30}, {NOTE_A4, 202},
  {0, 23}, {NOTE_B4, 202}, {0, 23}, {NOTE_A4, 202},
  {0, 23}, {NOTE_GS4, 202}, {0, 23}, {NOTE_AS4, 202},
  {0, 23}, {NOTE_GS4, 202}, {0, 23}, {NOTE_G4, 135},
  {0, 15}, {NOTE_D4, 135}, {0, 15}, {NOTE_E4, 810},
  {0, 90},
  {0, 0}
};

static const MelodyNote melody_imperial[] = {
  {NOTE_A4, 675}, {0, 75}, {NOTE_A4, 675}, {0, 75},
  {NOTE_A4, 112}, {0, 13}, {NOTE_A4, 112}, {0, 13},
  {NOTE_A4, 112}, {0, 13}, {NOTE_A4, 112}, {0, 13},
  {NOTE_F4, 225}, {0, 25}, {0, 250}, {NOTE_A4, 675},
  {0, 75}, {NOTE_A4, 675}, {0, 75}, {NOTE_A4, 112},
  {0, 13}, {NOTE_A4, 112}, {0, 13}, {NOTE_A4, 112},
  {0, 13}, {NOTE_A4, 112}, {0, 13}, {NOTE_F4, 225},
  {0, 25}, {0, 250}, {NOTE_A4, 450}, {0, 50},
  {NOTE_A4, 450}, {0, 50}, {NOTE_A4, 450}, {0, 50},
  {NOTE_F4, 337}, {0, 38}, {NOTE_C5, 112}, {0, 13},
  {NOTE_A4, 450}, {0, 50}, {NOTE_F4, 337}, {0, 38},
  {NOTE_C5, 112}, {0, 13}, {NOTE_A4, 900}, {0, 100},
  {NOTE_E5, 450}, {0, 50}, {NOTE_E5, 450}, {0, 50},
  {NOTE_E5, 450}, {0, 50}, {NOTE_F5, 337}, {0, 38},
  {NOTE_C5, 112}, {0, 13}, {NOTE_A4, 450}, {0, 50},
  {NOTE_F4, 337}, {0, 38}, {NOTE_C5, 112}, {0, 13},
  {NOTE_A4, 900}, {0, 100}, {NOTE_A5, 450}, {0, 50},
  {NOTE_A4, 337}, {0, 38}, {NOTE_A4, 112}, {0, 13},
  {NOTE_A5, 450}, {0, 50}, {NOTE_GS5, 337}, {0, 38},
  {NOTE_G5, 112}, {0, 13}, {NOTE_DS5, 112}, {0, 13},
  {NOTE_D5, 112}, {0, 13}, {NOTE_DS5, 225}, {0, 25},
  {0, 250}, {NOTE_A4, 225}, {0, 25}, {NOTE_DS5, 450},
  {0, 50}, {NOTE_D5, 337}, {0, 38}, {NOTE_CS5, 112},
  {0, 13}, {NOTE_C5, 112}, {0, 13}, {NOTE_B4, 112},
  {0, 13}, {NOTE_C5, 112}, {0, 13}, {0, 250},
  {NOTE_F4, 225}, {0, 25}, {NOTE_GS4, 450}, {0, 50},
  {NOTE_F4, 337}, {0, 38}, {NOTE_A4, 168}, {0, 19},
  {NOTE_C5, 450}, {0, 50}, {NOTE_A4, 337}, {0, 38},
  {NOTE_C5, 112}, {0, 13}, {NOTE_E5, 900}, {0, 100},
  {NOTE_A5, 450}, {0, 50}, {NOTE_A4, 337}, {0, 38},
  {NOTE_A4, 112}, {0, 13}, {NOTE_A5, 450}, {0, 50},
  {NOTE_GS5, 337}, {0, 38}, {NOTE_G5, 112}, {0, 13},
  {NOTE_DS5, 112}, {0, 13}, {NOTE_D5, 112}, {0, 13},
  {NOTE_DS5, 225}, {0, 25}, {0, 250}, {NOTE_A4, 225},
  {0, 25}, {NOTE_DS5, 450}, {0, 50}, {NOTE_D5, 337},
  {0, 38}, {NOTE_CS5, 112}, {0, 13}, {NOTE_C5, 112},
  {0, 13}, {NOTE_B4, 112}, {0, 13}, {NOTE_C5, 112},
  {0, 13}, {0, 250}, {NOTE_F4, 225}, {0, 25},
  {NOTE_GS4, 450}, {0, 50}, {NOTE_F4, 337}, {0, 38},
  {NOTE_A4, 168}, {0, 19}, {NOTE_A4, 450}, {0, 50},
  {NOTE_F4, 337}, {0, 38}, {NOTE_C5, 112}, {0, 13},
  {NOTE_A4, 900}, {0, 100},
  {0, 0}
};

static const MelodyNote melody_takeonme[] = {
  {NOTE_FS5, 192}, {0, 22}, {NOTE_FS5, 192}, {0, 22},
  {NOTE_D5, 192}, {0, 22}, {NOTE_B4, 192}, {0, 22},
  {0, 214}, {NOTE_B4, 192}, {0, 22}, {0, 214},
  {NOTE_E5, 192}, {0, 22}, {0, 214}, {NOTE_E5, 192},
  {0, 22}, {0, 214}, {NOTE_E5, 192}, {0, 22},
  {NOTE_GS5, 192}, {0, 22}, {NOTE_GS5, 192}, {0, 22},
  {NOTE_A5, 192}, {0, 22}, {NOTE_B5, 192}, {0, 22},
  {NOTE_A5, 192}, {0, 22}, {NOTE_A5, 192}, {0, 22},
  {NOTE_A5, 192}, {0, 22}, {NOTE_E5, 192}, {0, 22},
  {0, 214}, {NOTE_D5, 192}, {0, 22}, {0, 214},
  {NOTE_FS5, 192}, {0, 22}, {0, 214}, {NOTE_FS5, 192},
  {0, 22}, {0, 214}, {NOTE_FS5, 192}, {0, 22},
  {NOTE_E5, 192}, {0, 22}, {NOTE_E5, 192}, {0, 22},
  {NOTE_FS5, 192}, {0, 22}, {NOTE_E5, 192}, {0, 22},
  {NOTE_FS5, 192}, {0, 22}, {NOTE_FS5, 192}, {0, 22},
  {NOTE_D5, 192}, {0, 22}, {NOTE_B4, 192}, {0, 22},
  {0, 214}, {NOTE_B4, 192}, {0, 22}, {0, 214},
  {NOTE_E5, 192}, {0, 22}, {0, 214}, {NOTE_E5, 192},
  {0, 22}, {0, 214}, {NOTE_E5, 192}, {0, 22},
  {NOTE_GS5, 192}, {0, 22}, {NOTE_GS5, 192}, {0, 22},
  {NOTE_A5, 192}, {0, 22}, {NOTE_B5, 192}, {0, 22},
  {NOTE_A5, 192}, {0, 22}, {NOTE_A5, 192}, {0, 22},
  {NOTE_A5, 192}, {0, 22}, {NOTE_E5, 192}, {0, 22},
  {0, 214}, {NOTE_D5, 192}, {0, 22}, {0, 214},
  {NOTE_FS5, 192}, {0, 22}, {0, 214}, {NOTE_FS5, 192},
  {0, 22}, {0, 214}, {NOTE_FS5, 192}, {0, 22},
  {NOTE_E5, 192}, {0, 22}, {NOTE_E5, 192}, {0, 22},
  {NOTE_FS5, 192}, {0, 22}, {NOTE_E5, 192}, {0, 22},
  {NOTE_FS5, 192}, {0, 22}, {NOTE_FS5, 192}, {0, 22},
  {NOTE_D5, 192}, {0, 22}, {NOTE_B4, 192}, {0, 22},
  {0, 214}, {NOTE_B4, 192}, {0, 22}, {0, 214},
  {NOTE_E5, 192}, {0, 22}, {0, 214}, {NOTE_E5, 192},
  {0, 22}, {0, 214}, {NOTE_E5, 192}, {0, 22},
  {NOTE_GS5, 192}, {0, 22}, {NOTE_GS5, 192}, {0, 22},
  {NOTE_A5, 192}, {0, 22}, {NOTE_B5, 192}, {0, 22},
  {NOTE_A5, 192}, {0, 22}, {NOTE_A5, 192}, {0, 22},
  {NOTE_A5, 192}, {0, 22}, {NOTE_E5, 192}, {0, 22},
  {0, 214}, {NOTE_D5, 192}, {0, 22}, {0, 214},
  {NOTE_FS5, 192}, {0, 22}, {0, 214}, {NOTE_FS5, 192},
  {0, 22}, {0, 214}, {NOTE_FS5, 192}, {0, 22},
  {NOTE_E5, 192}, {0, 22}, {NOTE_E5, 192}, {0, 22},
  {NOTE_FS5, 192}, {0, 22}, {NOTE_E5, 192}, {0, 22},
  {0, 0}
};

static const MelodyNote melody_nokia[] = {
  {NOTE_E6, 180}, {NOTE_D6, 180}, {NOTE_FS5, 360}, {NOTE_GS5, 360},
  {NOTE_CS6, 180}, {NOTE_B5, 180}, {NOTE_D5, 360}, {NOTE_E5, 360},
  {NOTE_B5, 180}, {NOTE_A5, 180}, {NOTE_CS5, 360}, {NOTE_E5, 360},
  {NOTE_A5, 720},
  {0, 0}
};

static const MelodyNote melody_starwars[] = {
  {NOTE_F5, 180}, {0, 40}, {NOTE_F5, 180}, {0, 40}, {NOTE_F5, 180}, {0, 80},
  {NOTE_AS5, 700}, {0, 80}, {NOTE_F6, 700}, {0, 80},
  {NOTE_DS6, 180}, {0, 40}, {NOTE_D6, 180}, {0, 40}, {NOTE_C6, 180}, {0, 80},
  {NOTE_AS5, 800}, {0, 120},
  {NOTE_F6, 350}, {0, 60},
  {NOTE_DS6, 180}, {0, 40}, {NOTE_D6, 180}, {0, 40}, {NOTE_C6, 180}, {0, 80},
  {NOTE_AS5, 800}, {0, 120},
  {NOTE_F6, 350}, {0, 60},
  {NOTE_DS6, 180}, {0, 40}, {NOTE_D6, 180}, {0, 40}, {NOTE_DS6, 180}, {0, 80},
  {NOTE_C6, 1000},
  {0, 0}
};

static const MelodyNote melody_jingle[] = {
  {NOTE_E5, 180}, {0, 40}, {NOTE_E5, 180}, {0, 40}, {NOTE_E5, 360}, {0, 120},
  {NOTE_E5, 180}, {0, 40}, {NOTE_E5, 180}, {0, 40}, {NOTE_E5, 360}, {0, 120},
  {NOTE_E5, 180}, {0, 40}, {NOTE_G5, 180}, {0, 40}, {NOTE_C5, 260}, {0, 40},
  {NOTE_D5, 260}, {0, 40}, {NOTE_E5, 720}, {0, 120},
  {NOTE_F5, 180}, {0, 40}, {NOTE_F5, 180}, {0, 40}, {NOTE_F5, 180}, {0, 40},
  {NOTE_F5, 180}, {0, 40}, {NOTE_F5, 360}, {0, 80},
  {NOTE_E5, 180}, {0, 40}, {NOTE_E5, 180}, {0, 40}, {NOTE_E5, 180}, {0, 40},
  {NOTE_E5, 180}, {0, 80}, {NOTE_D5, 180}, {0, 40}, {NOTE_D5, 180}, {0, 40},
  {NOTE_E5, 180}, {0, 40}, {NOTE_D5, 360}, {0, 80}, {NOTE_G5, 720}, {0, 160},
  {NOTE_E5, 180}, {0, 40}, {NOTE_E5, 180}, {0, 40}, {NOTE_E5, 360}, {0, 120},
  {NOTE_E5, 180}, {0, 40}, {NOTE_E5, 180}, {0, 40}, {NOTE_E5, 360}, {0, 120},
  {NOTE_E5, 180}, {0, 40}, {NOTE_G5, 180}, {0, 40}, {NOTE_C5, 260}, {0, 40},
  {NOTE_D5, 260}, {0, 40}, {NOTE_E5, 720}, {0, 120},
  {NOTE_F5, 180}, {0, 40}, {NOTE_F5, 180}, {0, 40}, {NOTE_F5, 180}, {0, 40},
  {NOTE_F5, 180}, {0, 40}, {NOTE_F5, 360}, {0, 80},
  {NOTE_E5, 180}, {0, 40}, {NOTE_E5, 180}, {0, 40}, {NOTE_E5, 180}, {0, 40},
  {NOTE_E5, 180}, {0, 80}, {NOTE_G5, 180}, {0, 40}, {NOTE_G5, 180}, {0, 40},
  {NOTE_F5, 180}, {0, 40}, {NOTE_D5, 360}, {0, 80}, {NOTE_C5, 720},
  {0, 0}
};

#define MELODY_COUNT 11

static const MelodyNote* const melodies[MELODY_COUNT] = {
  melody_chime,
  melody_morning,
  melody_classical,
  melody_simple,
  melody_tetris,
  melody_mario,
  melody_imperial,
  melody_takeonme,
  melody_nokia,
  melody_starwars,
  melody_jingle
};

enum AlarmState : byte { ALARM_IDLE, ALARM_PLAYING, ALARM_DONE };
volatile AlarmState alarm_state = ALARM_IDLE;
uint16_t alarm_note_idx = 0;
volatile unsigned long alarm_start_ms = 0;
volatile unsigned long alarm_note_start_ms = 0;

// Определение NTP-клиента для получения времени
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP,"pool.ntp.org", 7);//"pool.ntp.org"

typedef struct {
   const uint16_t min;
   const uint16_t max;
} Range;

//Диапазон значений VEML7700
Range lux_ranges[num_ranges] = {{0, 12},{15, 47},{50, 97},{100, 297},{300, 697},{700, 15000}};
//Значения яркости газоразрядных индикаторов
#ifdef IN_14
uint8_t brigh_values[num_ranges] = {60, 80, 110, 150, 200, 255};
#endif
#ifdef IN_18
uint8_t brigh_values[num_ranges] = {15, 40, 80, 150, 200, 255};
#endif
static const int spiClk = 500000; // 1 MHz
int second, minute, hour, newsecond, newminute, newhour, dayOfWeek, day, dayOfMonth, month, year, minsCount, hourCount, old_second, old_minute, old_hour, oldminute;
int Nixie[40];       // цифры, которые должны показать индикаторы (0-10)
int NixieBuffer[40]; // цифры, которые должны показать индикаторы (0-10)

#ifdef IN_14
//                           0                1                2               3               4               5               6               7               8               9
int32_t masshv5522[] = {0b000000010000, 0b000010000000, 0b000100000000, 0b001000000000, 0b010000000000, 0b100000000000, 0b000000000001, 0b000000000010, 0b000000000100, 0b000000001000, 0b000000000000};
#endif

#ifdef IN_18
 //                                     0      1     2     3     4     5     6     7     8     9     
static const uint16_t masshv5522[] = {0x100, 0x80, 0x40, 0x10, 0x01, 0x02, 0x04, 0x08, 0x20, 0x200, 0x00};
#endif 

int32_t bufer[40];

int32_t buferDot[12] = {0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5};
int32_t DotMooveBufer[12] = {6, 5, 6, 5, 6, 5, 6, 5, 6, 5, 6, 5};

boolean flipIndics[6];

byte display = 0;
byte effects = 2;
byte off_effects = 0;
byte on_effects = 0;
int32_t dmoove = 0;
int32_t Counter = 6;
int rand_arr[] = {0,1,2,3,4,5};
boolean dmooveright = true;
boolean dmooveleft = false;
boolean flip;
boolean flipInit = true;
boolean flip_switch = true;
boolean lamp_dot_hv31 = false;
boolean lamp_plus_hv32 = false;
boolean lamp_celsius_hv31 = false;
boolean lamp_percent_hv32 = false;
boolean timeon = true;
boolean timer0 = false;
volatile uint32_t second_ticks = 0;
portMUX_TYPE secondTimerMux = portMUX_INITIALIZER_UNLOCKED;

int pricebtc, priceeth, TempValue;
int sensorDisplayValue = 0;
byte sensorDisplayDigits = 0;

enum DisplayModeState : byte {
  MODE_TIME = 0,
  MODE_TIME_OFF,
  MODE_SENSOR_ON,
  MODE_SENSOR_HOLD,
  MODE_SENSOR_OFF,
  MODE_TIME_ON
};

DisplayModeState displayState = MODE_TIME;
byte nextDisplay = 0;
byte transitionFromDisplay = 0;
byte transitionToDisplay = 0;
uint32_t modeStateStarted = 0;
uint32_t modeHoldStarted = 0;
byte activeTransitionEffect = 0;

//        LD               RD
// 0b000000100000, 0b000100000000,

const int PWM_CHANNEL = 0;    // ESP32 имеет 16 каналов, которые могут генерировать 16 независимых сигналов.
const int PWM_FREQ = 15000;//1480;    // Официальный пример ESP32 использует частоту 5000 Гц.
const int PWM_RESOLUTION = 8; // Мы будем использовать то же разрешение, что и Uno (8 бит, 0-255), но ESP32 может достигать 16 бит.

// Максимальное значение рабочего цикла, основанное на разрешении ШИМ (будет 255, если разрешение составляет 8 бит)
const int MAX_DUTY_CYCLE = (int)(pow(2, PWM_RESOLUTION) - 1);
const int LED_OUTPUT_PIN = BL;

const int BUZZER_CHANNEL = 2;
const int BUZZER_PIN = 27;
const int BUZZER_FREQ = 5000;

static enum {IDLE, ACTIVE, COOLDOWN}
buzzer_state = IDLE;

uint16_t vemllux;
uint8_t prev_brigh_value=255;
bool offtime_active = false;

static inline bool time_in_range(byte h, byte m, byte sh, byte sm, byte eh, byte em) {
  int now = h * 60 + m;
  int start = sh * 60 + sm;
  int end = eh * 60 + em;
  if (start <= end) return now >= start && now < end;
  else return now >= start || now < end;
}

unsigned long previousMillis;

String SensorsAutoShow[20];
String SensorsDisplay[20];
String SensorsAutoShowSelect2;
String WiFI_List;
char textbuffer[7] = "";
char buffer[7] = "";
byte scan_list_idx;

int optemperature, oppressure, ophumidity;
int usdRubRate;
int narodtemperature, narodpressure, narodhumidity;
bool ap_show_scroll = false;
bool wifi_dc_state = false;
String readmeCache;
unsigned long readmeFetchTime = 0;