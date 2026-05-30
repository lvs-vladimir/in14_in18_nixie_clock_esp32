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
#include "FastLED.h"
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
};
Data mydata;
FileData fd(&LittleFS, "/setting.dat", 'B', &mydata, sizeof(mydata));
GyverPortal ui(&LittleFS);
char webLogin[16] = "";
char webPass[32] = "admin";
bool webAuthOk = false;

#define LOG_ENTRIES 200
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

SPIClass *vspi = NULL;
SPIClass *hspi = NULL;

hw_timer_t *Timer0_Cfg = NULL;
hw_timer_t *Timer1_Cfg = NULL;

timerMinim DotTimer(84);          // 11 - 84, 12 - 76 посекундный таймер для часов
timerMinim mooveNixie(100);
timerMinim SwitchDisplayTimer(40);
timerMinim ChangeCathodeTimer(60);
timerMinim SensorSelectTimer(3000);
timerMinim NtpSyncTimer(3600000);
timerMinim OwmUpdateTimer(300000);
timerMinim CoinUpdateTimer(600000);

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
boolean timer1 = false;

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

uint16_t vemllux;
uint8_t prev_brigh_value=255;

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