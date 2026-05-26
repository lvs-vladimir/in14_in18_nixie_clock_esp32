void setup()
{
  init_timers();
  Serial.begin(115200);

  WiFiConnect_APcreate();
  //configTime(0, 3600*7, "pool.ntp.org");
  OtaUpdate();

  pinMode(HSPI_SS, OUTPUT);
  pinMode(BL, OUTPUT);
  digitalWrite(BL, 1);

  hspi = new SPIClass(HSPI);
  hspi->begin(HSPI_SCLK, HSPI_MISO, HSPI_MOSI, HSPI_SS); // SCLK, MISO, MOSI, SS

  strip.begin();
  strip.setBrightness(0);

  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LED_OUTPUT_PIN, PWM_CHANNEL);
  ledcWrite(PWM_CHANNEL, 255);

  hour = 23;
  minute = 22;
  second = 00;
  oldminute = minute;

  // Init and get the time
 

  NTPClientUpdate();

  pricebtc = 42345;
  priceeth = 0;
  TempValue = 18;

   SwitchDisplayTimer.stop();
   veml.begin();
   veml.setLowThreshold(10000);
   veml.setHighThreshold(20000);
   veml.interruptEnable(true);
  
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