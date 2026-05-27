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
#include "Freenove_WS2812_Lib_for_ESP32.h"
#include "Adafruit_VEML7700.h"

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
};
Data mydata;
FileData fd(&LittleFS, "/setting.dat", 'B', &mydata, sizeof(mydata));
GyverPortal ui(&LittleFS);

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

//#define IN_14
#define IN_18

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
#define LEDS_PIN 16
#define CHANNEL 1

#define HSPI_MISO 12
#define HSPI_MOSI 13
#define HSPI_SCLK 14
#define HSPI_SS 15

#define num_ranges 6

Freenove_ESP32_WS2812 strip = Freenove_ESP32_WS2812(LEDS_COUNT, LEDS_PIN, CHANNEL, TYPE_GRB);
Adafruit_VEML7700 veml = Adafruit_VEML7700();

SPIClass *vspi = NULL;
SPIClass *hspi = NULL;

hw_timer_t *Timer0_Cfg = NULL;
hw_timer_t *Timer1_Cfg = NULL;

timerMinim DotTimer(84);          // 11 - 84, 12 - 76 посекундный таймер для часов
timerMinim SecondTimer(1000);     // посекундный таймер для часов
timerMinim mooveNixie(100);
timerMinim SwitchDisplayTimer(40);
timerMinim ChangeCathodeTimer(60);
timerMinim ChangeCathodeTimerEffects(20);
timerMinim WS2812(10);
timerMinim SensorSelectTimer(3000);
timerMinim NtpSyncTimer(3600000);
timerMinim OwmUpdateTimer(300000);

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
uint8_t brigh_values[num_ranges] = {60, 80, 110, 150, 200, 255};//{15, 40, 80, 150, 200, 255};
#endif
#ifdef IN_18
uint8_t brigh_values[num_ranges] = {15, 40, 80, 150, 200, 255};
#endif
static const int spiClk = 500000; // 1 MHz
int incoming;
int var1;
int var2;
int var3;
int second, minute, hour, newsecond, newminute, newhour, dayOfWeek, day, dayOfMonth, month, year, minsCount, hourCount, old_second, old_minute, old_hour, oldminute;
int Nixie[40];       // цифры, которые должны показать индикаторы (0-10)
int NixieBuffer[40]; // цифры, которые должны показать индикаторы (0-10)
int vemlvalue;



#ifdef IN_14
//                           0                1                2               3               4               5               6               7               8               9
int32_t masshv5522[] = {0b000000010000, 0b000010000000, 0b000100000000, 0b001000000000, 0b010000000000, 0b100000000000, 0b000000000001, 0b000000000010, 0b000000000100, 0b000000001000, 0b000000000000};
#endif // 

#ifdef IN_18
 //       +               -          9            0             1             2           8           3             7             6            5            4
  //0b10000000000  0b01000000000 0b1000000000 0b0100000000 0b0010000000 0b0001000000 0b0000100000 0b0000010000 0b0000001000 0b0000000100 0b00000000010 0b00000000001
//static const uint32_t masshv5522[] = {0b0100000000, 0b0010000000, 0b0001000000, 0b0000010000, 0b00000000001, 0b00000000010, 0b0000000100, 0b0000001000, 0b0000100000, 0b1000000000, 0b0000000000};

//                                     0      1     2     3     4     5     6     7     8     9     
static const uint16_t masshv5522[] = {0x100, 0x80, 0x40, 0x10, 0x01, 0x02, 0x04, 0x08, 0x20, 0x200, 0x00};
#endif //

int32_t bufer[40];

int32_t buferDot[12] = {0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5};
int32_t DotMooveBufer[12] = {6, 5, 6, 5, 6, 5, 6, 5, 6, 5, 6, 5};
int32_t MooveNixieBufer[12] = {11, 23, 35, 47, 6, 5, 6, 5, 6, 5, 6, 5};

int cathodeMask[] = {1, 0, 2, 9, 3, 8, 4, 7, 5, 6}; // порядок катодов in14

boolean flipIndics[6];
int startCathode[11], endCathode[11];

byte display = 0;
byte effects = 2;
byte off_effects = 0;
byte on_effects = 0;
int32_t dmoove = 0;
int32_t MN = 6;
int32_t Counter = 6;
int WS2812_j = 0;
int rand_arr[] = {0,1,2,3,4,5};
int time1 = 4020; // 4020// Длительность одного такта в микросекундах. За это время выводится старая цифра и новая, время делится между ними
int time2 = 0;    // длительность такта, умноженная на число шагов дает общее время смены информации на дисплее
int hourchange, minutecgange, secondchange;
  // задается число шагов алгоритма. За эти шаги старая цифра сменится на новую
int ct_sum = 58; // 60
int ct=0;
  // задается приращение времени на каждом такте, микросекунд
int delta = 67;

timerMinim FadeTimer1(time1);
timerMinim FadeTimer2(time2);

boolean off_left_to_right_Nixie = false; // Очистить дисплей слево на право
boolean off_right_to_left_Nixie = false; // Очистить диспей справо на лево
boolean on_left_to_right_Nixie = false;  // Включить дисплей слево на право
boolean on_right_to_left_Nixie = false;  // Включить диспей справо на лево
boolean rightMooveNixie = false;
boolean dmooveright = true;
boolean dmooveleft = false;
boolean flip;
boolean flipInit = true;
boolean flip_switch, flip_switch2 = true;
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

uint16_t prev_value = 0;
byte stable_count = 0;
const uint16_t STABLE_LIMIT = 15;

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