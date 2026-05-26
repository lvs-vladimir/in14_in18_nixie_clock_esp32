
/*
void getTemp(){

int8_t tempGetTemp;

String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + (String)mydata.owCity + "&APPID=" + (String)mydata.owMapApiKey +"&units=metric";
String jsonBuffer = httpGETRequest(serverPath.c_str());
JSONVar myObject = JSON.parse(jsonBuffer);
int temperature = myObject["main"]["feels_like"]; 

 if (temperature>=0) {TempValue = int(temperature); minus=false;}
 else {TempValue = int(temperature*-1); minus=true;}//Если отрицательное преобразуем в положительное значение для корректного отображения.
 //Serial.println("OP: "+ TempValue);

}
 */
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
        oppressure = myObject["main"]["pressure"];
        ophumidity = myObject["main"]["humidity"];

        SensorsAutoShow[4]+=",";
       // SensorsAutoShow[4]+="OP ";
        SensorsAutoShow[4]+=optemperature;
        SensorsAutoShow[4]+="*";
    
        SensorsAutoShow[5]+=",";
      //  SensorsAutoShow[5]+="OP ";
        SensorsAutoShow[5]+=oppressure;
        SensorsAutoShow[5]+="mHg";
  
        SensorsAutoShow[6]+=",";
       // SensorsAutoShow[6]+="OP ";
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

        Serial.print("OP: ");
        Serial.println(optemperature);
        Serial.println(oppressure);
        Serial.println(ophumidity);

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
      //SensorsNarodMon[f]="";//очистка
      f++;
    }
    for(byte i=0; i<=3; i++){
    //String a = myObject["sensors"][mydata.nrd_sens[i]]["name"];
     int b= myObject["sensors"][mydata.nrd_sens[i]]["value"];
     String c = myObject["sensors"][mydata.nrd_sens[i]]["unit"];

     if (c.length()!=0){
     //sprintf_P(Sensors2[mydata.nrd_sens[i]],(PGM_P)F("%02d %S"), b,c.c_str());
//Создаем вспомогательный строковый массив для вывода в SELECT
      if(c=="°") {c=""; c+="*";}
      if(c=="mmHg") {c=""; c+="mHg";}
      SensorsAutoShow[i]+=",";
      //SensorsAutoShow[i]+="Nrd ";     
      SensorsAutoShow[i]+=b;
      SensorsAutoShow[i]+=c;
//Создаем вспомогательный строковый массив для вывода на дисплей без доп инфы
    //  SensorsDisplay[i]+=",";
      SensorsDisplay[i]+=b;
      SensorsDisplay[i]+=c;

      //Serial.print("AutoShow: "); Serial.println(SensorsAutoShow[i]);
      Serial.print("NAROD: ");
      //Serial.println(Sensors2[mydata.nrd_sens[i]]);
     }
     else {   
      Serial.print("ПУСТО: "); Serial.println(i);}
     Serial.print("Длинна: "); Serial.println(c.length()); 
     
     
    }
      //narodhumidity = myObject["sensors"][0]["value"];
     // narodpressure = myObject["sensors"][1]["value"];
      
      //Serial.println(mydata.Sensors[0]);
      //Serial.println(mydata.Sensors[1]);
      //Serial.println(a);
      //Serial.println(narodhumidity);
      //Serial.println(narodpressure);
      //Serial.println(a.c_str());

  }
  //else return 0;
  
  // int temperature; if (temperature>=0) {tempGetTemp = int(temperature); minus=false;}
  // else {tempGetTemp = int(temperature*-1); minus=true;}//Если отрицательное преобразуем в положительное значение для корректного отображения.

  }
  



