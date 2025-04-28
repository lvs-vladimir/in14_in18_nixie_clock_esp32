#include <Arduino.h>
// #include <Wire.h>
#include "SPI.h"
#include "timer2Minim.h"
#include "Freenove_WS2812_Lib_for_ESP32.h"
#include "Adafruit_VEML7700.h"


#define IN_14
//#define IN_18


#define LEDS_COUNT 6
#define LEDS_PIN 16
#define CHANNEL 1

Freenove_ESP32_WS2812 strip = Freenove_ESP32_WS2812(LEDS_COUNT, LEDS_PIN, CHANNEL, TYPE_GRB);

#define VSPI_MISO MISO
#define VSPI_MOSI MOSI
#define VSPI_SCLK SCK
#define VSPI_SS SS

#define HSPI_MISO 12
#define HSPI_MOSI 13
#define HSPI_SCLK 14
#define HSPI_SS 15

// TwoWire I2CBME = TwoWire(0);
Adafruit_VEML7700 veml = Adafruit_VEML7700();

/*
#define HSPI_MISO   26
#define HSPI_MOSI   27
#define HSPI_SCLK   25
#define HSPI_SS     32
*/

SPIClass *vspi = NULL;
SPIClass *hspi = NULL;
// SPISettings settings(10000000, MSBFIRST, SPI_MODE3);
static const int spiClk = 500000; // 1 MHz
timerMinim DotTimer(84);          // 11 - 84, 12 - 76 посекундный таймер для часов
timerMinim SecondTimer(1000);     // посекундный таймер для часов
timerMinim mooveNixie(100);
timerMinim SwitchDisplayTimer(40);
timerMinim ChangeCathodeTimer(60);
timerMinim ChangeCathodeTimerEffects(20);
timerMinim WS2812(10);

int incoming;
int var1;
int var2;
int var3;
int second, minute, hour, newsecond, newminute, newhour, dayOfWeek, day, dayOfMonth, month, year, minsCount, hourCount, old_second, old_minute, old_hour, oldminute;
int Nixie[40];       // цифры, которые должны показать индикаторы (0-10)
int NixieBuffer[40]; // цифры, которые должны показать индикаторы (0-10)
int vemlvalue;

#define CS_ON (GPIO.out_w1ts = ((uint32_t)1 << 5))
#define CS_OFF (GPIO.out_w1tc = ((uint32_t)1 << 5))

#define CS_ON_HSPI (GPIO.out_w1ts = ((uint32_t)1 << 15))
#define CS_OFF_HSPI (GPIO.out_w1tc = ((uint32_t)1 << 15))

#define BL 2
#define G 16
#define LE 15

#ifdef IN_14
//                           0                1                2               3               4               5               6               7               8               9
int32_t masshv5522[] = {0b000000010000, 0b000010000000, 0b000100000000, 0b001000000000, 0b010000000000, 0b100000000000, 0b000000000001, 0b000000000010, 0b000000000100, 0b000000001000, 0b000000000000};
#endif // 

#ifdef IN_18
 //       +               -          9            0             1             2           8           3             7             6            5            4
  //0b10000000000  0b01000000000 0b1000000000 0b0100000000 0b0010000000 0b0001000000 0b0000100000 0b0000010000 0b0000001000 0b0000000100 0b00000000010 0b00000000001

//                            0           1              2             3             4               5              6            7              8             9
int32_t masshv5522[] = {0b0100000000, 0b0010000000, 0b0001000000, 0b0000010000, 0b00000000001, 0b00000000010, 0b0000000100, 0b0000001000, 0b0000100000, 0b1000000000, 0b0000000000};
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

int pricebtc, priceeth, TempValue;
//        LD               RD
// 0b000000100000, 0b000100000000,

uint16_t digit1[] = {
    0b0000000100000000, // 0
    0b0000001000000000, // 1
    0b0000010000000000, // 2
    0b0000100000000000, // 3
    0b0001000000000000, // 4
    0b0010000000000000, // 5
    0b0100000000000000, // 6
    0b1000000000000000, // 7
    0b0000000000000001, // 8
    0b0000000000000010, // 9

    // 0b00000000,
};

uint16_t digit2[] = {
    0b0000000000000100, // 0
    0b0000000000001000, // 1
    0b0000000000010000, // 2
    0b0000000000100000, // 3
    0b0000000001000000, // 4
    0b0000000010000000, // 5
    0b0000000000000000, // 6
    0b0000000000000000, // 7
    0b0000000000000000, // 8
    0b0000000000000000, // 9
};

byte digit3[] = {
    0b00000000, // 0
    0b00000000, // 1
    0b00000000, // 2
    0b00000000, // 3
    0b00000000, // 4
    0b00000000, // 5
    0b00000001, // 6
    0b00000010, // 7
    0b00000100, // 8
    0b00001000, // 9
};

int32_t numbersArrayIv13[]{
    0b11100111, // 0
    0b00000110, // 1
    0b01110011, // 2
    0b01010111, // 3
    0b10010110, // 4
    0b11010101, // 5
    0b11110101, // 6
    0b00000111, // 7
    0b11110111, // 8
    0b11010111, // 9
    0b00000000, // чисто
    0b00001000, // точка
};

#define anod1 12
#define anod2 27
#define anod3 25
#define anod4 2
#define anod5 16
#define anod6 17

const int PWM_CHANNEL = 0;    // ESP32 имеет 16 каналов, которые могут генерировать 16 независимых сигналов.
const int PWM_FREQ = 15000;//1480;    // Официальный пример ESP32 использует частоту 5000 Гц.
const int PWM_RESOLUTION = 8; // Мы будем использовать то же разрешение, что и Uno (8 бит, 0-255), но ESP32 может достигать 16 бит.

// Максимальное значение рабочего цикла, основанное на разрешении ШИМ (будет 255, если разрешение составляет 8 бит)
const int MAX_DUTY_CYCLE = (int)(pow(2, PWM_RESOLUTION) - 1);
const int LED_OUTPUT_PIN = BL;

void calculateTime()
{
  second++;
  if (second > 59)
  {
    second = 0;
    minute++;
  }
  if (minute > 59)
  {
    minute = 0;
    hour++;
    if (hour > 23)
      hour = 0;
  }
}

void loop2 (void* pvParameters) {
  while (1) {
   
  if (WS2812.isReady()){
  //for (int j = 0; j < 255; j += 2) {

    for (int i = 0; i < LEDS_COUNT; i++) {
      strip.setLedColorData(i, strip.Wheel((i * 256 / LEDS_COUNT + WS2812_j) & 255));
    }
    strip.show();
    //delay(10);

    WS2812_j+=2;

    if (WS2812_j>=255) WS2812_j=0;

  }

   
  }
}

void setup()
{
  Serial.begin(9600);

  // Wire.begin(0x10, 33, 32, 1000000);
  // I2CBME.begin(33, 32);
  // vspi = new SPIClass(VSPI);
  // vspi->begin(VSPI_SCLK, VSPI_MISO, VSPI_MOSI, VSPI_SS); //SCLK, MISO, MOSI, SS
  // vspi->setClockDivider(SPI_CLOCK_DIV4);
  // vspi->setClockDivider(SPI_CLOCK_DIV4);
  pinMode(HSPI_SS, OUTPUT);
  // pinMode(CLR, OUTPUT);
  pinMode(BL, OUTPUT);

  // digitalWrite(HSPI_SS, HIGH);
  // digitalWrite(CLR, HIGH);
  digitalWrite(BL, 1);
  hspi = new SPIClass(HSPI);
  hspi->begin(HSPI_SCLK, HSPI_MISO, HSPI_MOSI, HSPI_SS); // SCLK, MISO, MOSI, SS

  // vspi = new SPIClass(VSPI);
  // vspi->begin();
  // vspi->setDataMode(0);
  // vspi->setBitOrder(MSBFIRST);
   strip.begin();
   strip.setBrightness(10);

  // pinMode(27, OUTPUT);
  // digitalWrite(18,0);
  //  Устанавливает канал (0–15), частоту рабочего цикла ШИМ и разрешение ШИМ (1–16 бит)
  //  ledcSetup(uint8_t channel, double freq, uint8_t resolution_bits);

  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);

  // ledcAttachPin(uint8_t pin, uint8_t channel);
  ledcAttachPin(LED_OUTPUT_PIN, PWM_CHANNEL);
  ledcWrite(PWM_CHANNEL, 255);

  // pinMode(HSPI_SS, OUTPUT);
  // CS_ON;
  // pinMode(vspi->pinSS(), OUTPUT); //VSPI SS
  // pinMode(anod1, OUTPUT);
  // pinMode(anod2, OUTPUT);
  /// pinMode(anod3, OUTPUT);
  // pinMode(anod4, OUTPUT);
  // pinMode(16, OUTPUT);
  // pinMode(17, OUTPUT);
  // pinMode(2, OUTPUT);
  // digitalWrite(anod1, 1);
  // digitalWrite(anod2, 1);
  // digitalWrite(anod3, 1);
  // digitalWrite(anod4, 1);
  // digitalWrite(16, HIGH);
  // digitalWrite(17, HIGH);

  // GPIO.out_w1ts = ((uint32_t)1 << 17);
  // GPIO.out_w1ts = ((uint32_t)1 << 16);
  // Serial.begin(9600);
  // pinMode (8, OUTPUT);

  // pinMode(latchPin, OUTPUT);
  // pinMode(clockPin, OUTPUT);
  // pinMode(dataPin, OUTPUT);
  // SPI.begin();
  hour = 23;
  minute = 22;
  second = 00;
  oldminute = minute;

  pricebtc = 42345;
  priceeth = 2590;
  TempValue = 18;

   SwitchDisplayTimer.stop();
  // veml.begin();
  // veml.setLowThreshold(10000);
  // veml.setHighThreshold(20000);
  // veml.interruptEnable(true);

  
  xTaskCreatePinnedToCore (
    loop2,     // Function to implement the task
    "loop2",   // Name of the task
    10000,      // Stack size in bytes
    NULL,      // Task input parameter
    0,         // Priority of the task
    NULL,      // Task handle.
    0          // Core where the task should run
  );

}


/*
void spiCommand(SPIClass *spi, byte data) {
  //use it as you would the regular arduino SPI API
  spi->beginTransaction(SPISettings(spiClk, LSBFIRST, SPI_MODE3));
  CS_OFF;
  spi->transfer(data);
  spi->transfer(data);
  spi->transfer(data);
  spi->transfer(data);
  CS_ON; //pull ss high to signify end of data transfer
  spi->endTransaction();
}
*/


void UpdateDisplay()
{ //){

  uint32_t hv5222_1, hv5222_2, hv5222_3;

  bufer[5] = masshv5522[Nixie[5]]; //| (1 << 6);
  bufer[4] = masshv5522[Nixie[4]]; //| (1 << 6) | (1 << 5);
  bufer[3] = masshv5522[Nixie[3]];
  bufer[2] = masshv5522[Nixie[2]];
  bufer[1] = masshv5522[Nixie[1]];
  bufer[0] = masshv5522[Nixie[0]];

#ifdef IN_14
  if ((display == 0) && timeon)
  {
    if (DotTimer.isReady())
    {
      if (dmooveright)
      {
        dmoove++;
      }
      if (dmooveleft)
      {
        dmoove--;
      }
      if (dmoove >= 11)
      {
        dmoove = 11;
        dmooveleft = true;
        dmooveright = false;
      }
      if (dmoove <= 0)
      {
        dmoove = 0;
        dmooveleft = false;
        dmooveright = true;
      }
      if (dmoove == 11)
      {
        DotTimer.setInterval(76);
      }
      if (dmoove == 10)
      {
        DotTimer.setInterval(84);
      }
      // Serial.println(dmoove);
    }
    bufer[buferDot[dmoove]] |= (1 << DotMooveBufer[dmoove]);
  }
  //    C         %
  //(1 << 13); (1 << 12);
  hv5222_3 |= (bufer[5] << 20);
  hv5222_2 |= (bufer[2] << 26) | (bufer[3] << 14) | bufer[4];
  hv5222_1 |= (bufer[0] << 20) | (bufer[1] << 8) | (bufer[2] >> 6);
#endif

  // Передача 32 bit в регистр hv5222
  hspi->beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE2));
  CS_OFF_HSPI;

#ifdef IN_14
  hspi->transfer32(hv5222_3);
  hspi->transfer32(hv5222_2);
  hspi->transfer32(hv5222_1);
#endif

#ifdef IN_18
  hv5222_2 |= (bufer[3] << 20) | (bufer[4] << 10) | bufer[5];
  hv5222_1 |= (bufer[0] << 20) | (bufer[1] << 10) | bufer[2];
  hspi->transfer32(hv5222_2);
  hspi->transfer32(hv5222_1);
#endif  


  //hspi->transfer32(0b00000000000000000000000000000000);
  // hspi->transfer16(0b0000000001100000);
  // hspi->transfer(numbersArrayIv13[Nixie[5]]);
  hspi->endTransaction();
  CS_ON_HSPI;
  /*
  vspi->beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE2));
  vspi->endTransaction();

  vspi->transfer32(hv5222);
  hspi->beginTransaction(SPISettings(50000, MSBFIRST, SPI_MODE3));
  hspi->transfer32(hv5222);
  hspi->transfer32(hv5122);
  hspi->endTransaction();

  SPI.transfer(hv5222>>24);
  SPI.transfer(hv5222>>16);
  SPI.transfer(hv5222>>8);
  SPI.transfer(hv5222);
  //Передача 32 bit в регистр hv5122
  SPI.transfer(hv5122>>24);
  SPI.transfer(hv5122>>16);
  SPI.transfer(hv5122>>8);
  SPI.transfer(hv5122);
  */
}

void SetNixieBufer()
{
  NixieBuffer[0] = newhour / 10;
  NixieBuffer[1] = newhour % 10;
  NixieBuffer[2] = newminute / 10;
  NixieBuffer[3] = newminute % 10;
  NixieBuffer[4] = newsecond / 10;
  NixieBuffer[5] = newsecond % 10;
}

void SetNixie()
{
  Nixie[0] = hour / 10;
  Nixie[1] = hour % 10;
  Nixie[2] = minute / 10;
  Nixie[3] = minute % 10;
  Nixie[4] = second / 10;
  Nixie[5] = second % 10;
}




void CrossFade()
{
  
  int time1 = 4020; // 16080 4020// Длительность одного такта в микросекундах. За это время выводится старая цифра и новая, время делится между ними
  int time2 = 0;    // длительность такта, умноженная на число шагов дает общее время смены информации на дисплее
  int hourchange, minutecgange, secondchange;
  // задается число шагов алгоритма. За эти шаги старая цифра сменится на новую

  int ct_sum = 60; // 60

  // задается приращение времени на каждом такте, микросекунд

  int delta = 68;//67


  for (int ct = 0; ct < ct_sum; ct++)
  {
    Nixie[0] = old_hour / 10;
    Nixie[1] = old_hour % 10;
    Nixie[2] = old_minute / 10;
    Nixie[3] = old_minute % 10;
    Nixie[4] = old_second / 10;
    Nixie[5] = old_second % 10;
    UpdateDisplay();

    delayMicroseconds(time1);
    time1 = time1 - delta;

  
if ((display == 0) && timeon) {
     hourchange = hour;
     minutecgange = minute;
     secondchange = second;
}

else {
     hourchange = newhour;
     minutecgange = newminute;
     secondchange = newsecond;
}

    Nixie[0] = hourchange / 10;
    Nixie[1] = hourchange % 10;
    Nixie[2] = minutecgange / 10;
    Nixie[3] = minutecgange % 10;
    Nixie[4] = secondchange / 10;
    Nixie[5] = secondchange % 10;
    UpdateDisplay();

    delayMicroseconds(time2);
    time2 = time2 + delta;

  } 


if ((display == 0) && timeon) {
  old_second = second;
  old_minute = minute;
  old_hour = hour; 

}
else{
  old_second = newsecond;
  old_minute = newminute;
  old_hour = newhour;
}
//if (hourchange==newhour) Counter = -1;
}

void SwitchNumbers()
{

  if (flipInit)
  {

    flipInit = false;
    // запоминаем, какие цифры поменялись и будем менять их
    for (int i = 0; i < 6; i++)
    {
      if (Nixie[i] != NixieBuffer[i])
      {
        flipIndics[i] = true;
      }
      else
        flipIndics[i] = false;
    }
  }

  if (ChangeCathodeTimer.isReady())
  {

    int flipCounter = 0;
    for (int i = 0; i < 6; i++)
    {
      if (flipIndics[i])
      {
        Nixie[i]--;
        if (Nixie[i] < 0)
          Nixie[i] = 9;
        if (Nixie[i] == NixieBuffer[i])
          flipIndics[i] = false;
      }
      else
      {
        flipCounter++;
      }
    }
    if (flipCounter == 6)
    { // если ни одну из 4 цифр менять не нужно
      flipInit = true;

     // Counter = -1;
    }
  }
}
//Перемешать массив
  void shuffle(int* arr, int N)
{
    // инициализация генератора случайных чисел
    srand(time(NULL));
 
    // реализация алгоритма перестановки
    for (int i = N - 1; i >= 1; i--)
    {
        int j = rand() % (i + 1);
        int tmp = arr[j];
        arr[j] = arr[i];
        arr[i] = tmp;
    }

}

void loop()
{


  // Уствнавливаем значение шим на канале 0-255(8bit)

  // analogWrite(27,100);

  ledcWrite(PWM_CHANNEL,255); // 30 минимальная яркость

  if (SwitchDisplayTimer.isReady())
  {
    // Переключение режимов отображения дисплея
    display++;
    if (display > 0)
      timeon = false;
      flip=true;
    // Включаем случайную анимацию выключения
    off_effects = random(1, 6);
    //off_effects = 5;
    if (off_effects <= 2) mooveNixie.setInterval(100); // разный интервал счетчиков для разных эффектов
    if (off_effects >= 3) mooveNixie.setInterval(20);
    Serial.println(off_effects);

    SwitchDisplayTimer.setInterval(4000);

    if (display > 3)
    {
      display = 0;
      SwitchDisplayTimer.stop();
      SwitchDisplayTimer.setInterval(40);
      // flip=false;//
    }
  }

  int num, num1, num2, num3, num4, num5, num6;

  if (display == 1) num = pricebtc;
  if (display == 2) num = priceeth;
  if (display == 3) num = TempValue;

  num1 = num % 10;
  num /= 10;
  num2 = num % 10;
  num /= 10;
  num3 = num % 10;
  num /= 10;
  num4 = num % 10;
  num /= 10;
  num5 = num % 10;
  num /= 10;
  num6 = num % 10;

  newhour = (num6 * 10) + num5;
  newminute = (num2 * 10) + num1;
  newsecond = (num4 * 10) + num3;

  // Смена эффектов отображения часов
  if (oldminute != minute)
  {
    effects++;
    if (effects > 2)
      effects = 0;
    oldminute = minute;
  }

  switch (display)
  {
  case 0:

    if (second == 30) SwitchDisplayTimer.start(); // Запускаем таймер для вывода Битка и эфира

    if (timeon)
    {
      newhour = hour;
      newminute = minute;
      newsecond = second;
      SetNixieBufer();
      on_effects = 0;
      Counter = 6;
    }
    if (flip)
    {
      newhour = hour;
      newminute = minute;
      newsecond = second;
      SetNixieBufer(); // Заночим текущие значения в буфер
      flip = false;
    }
    break;

  case 1:
    // Выводим Биток
    if (pricebtc > 0)
    {
      if (flip)
      {
        newhour = (num6 * 10) + num5;
        newminute = (num4 * 10) + num3;
        newsecond = (num2 * 10) + num1;
        SetNixieBufer();
        flip = false;
      }
    }
    else
      display++;

    break;
  case 2:
    // Выводим Эфириум
    if (priceeth > 0)
    {
      if (flip)
      {
        newhour = (num6 * 10) + num5;
        newminute = (num4 * 10) + num3;
        newsecond = (num2 * 10) + num1;
        SetNixieBufer();
        flip = false;
      }
    }
    else
      display++;
    break;
  case 3:
    // Выводим Температуру
    if (flip)
    {
      NixieBuffer[5] = 0;//10;
      NixieBuffer[4] = 0;//10;
      NixieBuffer[3] = newminute % 10;
      NixieBuffer[2] = newminute / 10;
      NixieBuffer[1] = 0;//10;
      NixieBuffer[0] = 0;//10;
      flip = false;
    }
    break;
  }

  switch (effects)
  {
  case 0:
    // Без эффекта обычное переключение индикаторов
    if ((display == 0) && timeon) SetNixie();
    else{
      //Nixie[0] = NixieBuffer[0];
      //Nixie[1] = NixieBuffer[1];
      //Nixie[2] = NixieBuffer[2];
      //Nixie[3] = NixieBuffer[3];
      //Nixie[4] = NixieBuffer[4];
     // Nixie[5] = NixieBuffer[5];
    }
    // flip_switch=true;
    UpdateDisplay();
    break;
  case 1:
    // Последовательное переключение цифр в индикаторе
    if ((display == 0) && timeon) SwitchNumbers();
    UpdateDisplay();
    break;
  case 2:
    // плавная смена цифр
   // CrossFade();
    if ((display == 0) && timeon) CrossFade();
    else UpdateDisplay();
    break;
  }

//if (off_effects==4) SwitchNumbers();
//if (off_effects==5) CrossFade();

  if (mooveNixie.isReady())
  {
    // Serial.println(MN);
    // Выключение(движение) изображения справо на лево
    if (off_effects == 1)
    { 
      for (byte i = 0; i <= Counter; i++)
      {
        Nixie[i] = NixieBuffer[(5 - Counter) + (i + 1)];
      }
      Nixie[Counter] = 10; // очищаем индикатор
      Counter--;
    }
    // Выключение(движение) изображения справо на лево
    if (off_effects == 2)
    { 
      for (byte i = 1; i <= Counter; i++)
      {
        Nixie[(5 - Counter) + i] = NixieBuffer[i - 1];
      }
      Nixie[5 - Counter] = 10; // очищаем индикатор
      Counter--;
    }
    //Переключение и исчезание катодов справа на лево
    if (off_effects == 3)
    { 
      Nixie[Counter]--;
      if (Nixie[Counter] < 0)
        Nixie[Counter] = 9;
      if (Nixie[Counter] == NixieBuffer[Counter])
      {
        Nixie[Counter] = 10; // гасим текущий индикатор
        Counter--;           // переходим к следующиму индикатору
      }
    }
    //Переключение и исчезание катодов слева на право
     if (off_effects == 4)
    { 
      Nixie[5-Counter]--;
      if (Nixie[5-Counter] < 0) Nixie[5-Counter] = 9;
      if (Nixie[5-Counter] == NixieBuffer[5-Counter])
      {
        Nixie[5-Counter] = 10; // гасим текущий индикатор
        Counter--;           // переходим к следующиму индикатору
      }
    }
    //Переключение и исчезание катодов в случайном порядке
       if (off_effects == 5)
    { 
      //Перемешиваем массив
      if (flip_switch){
        flip_switch=false;
        shuffle(rand_arr, 6);
      } 

      Nixie[rand_arr[Counter]]--;
      if (Nixie[rand_arr[Counter]] < 0) Nixie[rand_arr[Counter]] = 9;
      if (Nixie[rand_arr[Counter]] == NixieBuffer[rand_arr[Counter]])
      {
        Nixie[rand_arr[Counter]] = 10; // гасим текущий индикатор
        Counter--;   // переходим к следующиму индикатору
      }
    }
    // Включение(движение) изображения справо на лево
    if (on_effects == 1)
    { 
      for (byte i = 0; i <= Counter; i++)
      {
        Nixie[(5 - Counter) + i] = NixieBuffer[i];
      }
      Counter++;
    }
    // Включение(движение) изображения слево на право
    if (on_effects == 2)
    { 
      for (byte i = 0; i <= Counter; i++)
      {
        Nixie[i] = NixieBuffer[(5 - Counter) + i];
      }
      Counter++;
    }
    if (on_effects == 3)
    { 
      Nixie[Counter]--;
      if (Nixie[Counter] < 0)
        Nixie[Counter] = 9;
      if (Nixie[Counter] == NixieBuffer[Counter])
      {
        Counter++; // переходим к следующиму индикатору
      }
    }
    if (on_effects == 4)
    { 
      Nixie[5-Counter]--;
      if (Nixie[5-Counter] < 0) Nixie[5-Counter] = 9;
      if (Nixie[5-Counter] == NixieBuffer[5-Counter])
      {
        Counter++; // переходим к следующиму индикатору
      }
    }

     //Переключение и появление катодов в случайном порядке
       if (on_effects == 5)
    { 
      //Перемешиваем массив
      if (flip_switch){
        flip_switch=false;
        shuffle(rand_arr, 6);
      } 

      Nixie[rand_arr[Counter]]--;
      if (Nixie[rand_arr[Counter]] < 0) Nixie[rand_arr[Counter]] = 9;
      if (Nixie[rand_arr[Counter]] == NixieBuffer[rand_arr[Counter]])
      {
        Counter++;   // переходим к следующиму индикатору
      }
    }

    // выключение анимации при достижении счетчика нужного значения
    if (Counter < 0)
    {
      off_effects = 0; //Выключаем анимацию очищения дисплея
      flip_switch=true;
      flip = true;               // Заносим новое значение в массив
      on_effects = random(1, 6); // включаем случайный эффект появления
      //on_effects=5;

      if (on_effects <= 2) mooveNixie.setInterval(100); // разный интервал счетчиков для разных эффектов
      if (on_effects == 3) mooveNixie.setInterval(20);

    }
    // выключение анимации при достижении счетчика нужного значение
    if (Counter == 6)
    { 
      flip_switch=true;
      on_effects = 0;
      if (on_effects == 0)
        timeon = true;
      // flip=false;
    }
  }
 if (SecondTimer.isReady())
  {

    calculateTime();
    DotTimer.reset();
    // Serial.println(second);
    // Serial.println(old_second);
    // vemlvalue = veml.readLux();
    // Serial.print(" lux: "); Serial.println(vemlvalue);
    // if (vemlvalue>1024) vemlvalue = 1024;
    /// vemlvalue = map(vemlvalue, 0, 1024, 200, 0);
    // bmelvalue = bme.readTemperature();
    // Serial.print("Temp C*: "); Serial.print(bmelvalue);
    //  Serial.print(" lux: "); Serial.println(vemlvalue);
    // Serial.print(" vemlvalue: "); Serial.println(vemlvalue);
  }

}


