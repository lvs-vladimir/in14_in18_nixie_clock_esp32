void CrossFade()
{
  
  int time1 = 4020; // 16080 4020// Длительность одного такта в микросекундах. За это время выводится старая цифра и новая, время делится между ними
  int time2 = 0;    // длительность такта, умноженная на число шагов дает общее время смены информации на дисплее
  int hourchange, minutecgange, secondchange;
  // задается число шагов алгоритма. За эти шаги старая цифра сменится на новую
  int ct_sum = 60; // 60
  // задается приращение времени на каждом такте, микросекунд
  int delta = 67;//68
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

void switch_effects(){
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
}