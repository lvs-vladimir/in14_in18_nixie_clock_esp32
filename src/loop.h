void loop()
{
  ArduinoOTA.handle(); // Обновление по воздуху

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
        //newhour = (num6 * 10) + num5;
        //newminute = (num4 * 10) + num3;
        //newsecond = (num2 * 10) + num1;

        newhour = day;
        newminute = month;
        newsecond = year;
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
      NixieBuffer[5] = 10;
      NixieBuffer[4] = 10;
      NixieBuffer[3] = newminute % 10;
      NixieBuffer[2] = newminute / 10;
      NixieBuffer[1] = 10;
      NixieBuffer[0] = 10;
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

switch_effects();

  if (timer1){
     calculateTime();
     timer1=false;
  }
 if (SecondTimer.isReady())
  {
   
   // calculateTime();
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