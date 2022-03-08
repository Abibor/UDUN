
#include <UIPEthernet.h> // Used for Ethernet

// **** ETHERNET SETTING ****
byte mac[] = { 0x54, 0x34, 0x41, 0x30, 0x30, 0x31 };
// IP-адрес, назначаемый Ethernet shield:
byte ip[] = {192, 168, 0, 33};
// IP-адрес, dns сервера:
byte myDns[] = { 192, 168, 0, 1 };
// адрес шлюза:
byte gateway[] = { 192, 168, 0, 1 };
// маска:
byte subnet[] = { 255, 255, 255, 0 };

EthernetServer server(80);

// Определяем, к какому выводу Arduino подключен выход LM35:
#define sensorPin A0

/*
  int ledPin3 = 3; // указываем что светодиод будет управляться через 2 Pin
  int ledPin4 = 4;
  int ledPin5 = 5;
*/

String readString = String(40); //строка для получения данных с адреса длиной 30 символов

//для переключения режимов работы МКК использую логическую переменную
boolean MOD = false; // автоматический режим работы по температуре по умолчанию

//тут или в коде нужно реализовать чтение статуса логической переменной из энергонезависимой памяти EEEPROM
boolean CON1 = false; //изначальный статус КОНДИЦИОНЕРА - выключен
boolean CON2 = false;
boolean HEAT = false; //изначальный статус ПЕЧКИ - выключен

float temperature;
float preTemp;

EthernetClient client;

int seconds; // - колличество секунд;
int minutes; // - колличество минут;
int hours; // - колличество часов;
int days; // - колличество часов;
uint32_t timer;

uint32_t myTimer1; // переменная хранения времени (unsigned long)
uint32_t myTimer;

int tempUp = 27;
int tempDown = 22;
int tempUpHeat = 17;
int tempDownHeat = 10;

void setup() {
  analogReference(INTERNAL);        // включаем внутрений источник опорного 1,1 вольт

  Serial.begin(9600);

  // запуск соединения Ethernet и сервера:
  Ethernet.begin(mac, ip, myDns, gateway, subnet); // инициализация контроллера
  
  if (Ethernet.begin(mac) == 0) {
    // ошибка получения IP-адреса
  }

  server.begin();

  //Serial.print("IP Address: "); // выводим в монитор порта IP МКК
  Serial.println(Ethernet.localIP());
  //Serial.print("SubnetMask: ");
  Serial.println(Ethernet.subnetMask());
  //Serial.print("GatewayIP: ");
  Serial.println(Ethernet.gatewayIP());
  /*
    Serial.print("DnsServerIP: ");
    Serial.println(Ethernet.dnsServerIP());
  */

  //устанавливаем pin'ы на выход
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
}

void loop() {
  // Получить показания датчика температуры:
  int reading = analogRead(sensorPin);

  if (millis() - myTimer >= 1000) {
    myTimer = millis();
    preTemp = reading / 9.31;

    Serial.print("T - ");
    Serial.println(preTemp);            // отправляем в монитор порта
    //Serial.println(" C");
  }


  if (millis() - myTimer1 >= 10000) {   // ищем разницу (500 мс)
    myTimer1 = millis();              // сброс таймера
    // выполнить действие

    // Преобразование показаний в напряжение:
    //float voltage = reading * (5000 / 1024.0);

    // Преобразование напряжения в температуру в градусах Цельсия:
    //temperature = voltage / 10;

    temperature = reading / 9.31;          // переводим в цельсии


    Serial.println(temperature);            // отправляем в монитор порта
    //Serial.println(" C");

    //Serial.print("Time - ");
    //Serial.println((float)millis() / 3600000); // вывод в порт времени с начала работы мкк

  }


  // Преобразование показаний в напряжение:
  //float voltage = reading * (5000 / 1024.0);

  // Преобразование напряжения в температуру в градусах Цельсия:
  //temperature = voltage / 10;

  // listen for incoming clients
  client = server.available();

  if (MOD == 0) {
    //client.println(F("<p><font size=\"’5′\">Статус: <font size=\"’5′\">ВКЛ авто режим"));
    avto();
  }

  if (client) {
    //Если сервер доступен то в монитор порта выводим сообщение
    Serial.println("-> New Connection");

    //http-запрос заканчивается пустой строкой
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        // читаем char c от char HTTP запроса
        if (readString.length() < 40) {
          //сохранить символы в строку
          readString.concat(c);
        }
        //вывод символов в последовательный порт
        Serial.print(c);
        //если HTTP запрос заканчивается на
        if (c == '\n') {

          // Формируем HTML
          client.println(F("HTTP/1.1 200 OK"));
          client.println(F("Content-Type: text/html"));
          client.println(F("Connection: close"));     // закрыть сессию после ответа
          //client.println(F("Refresh: 10"));         // обновить страницу автоматически
          client.println();                 // пустая строка отделяет тело сообщения

          client.println(F("<head> <meta http-equiv='Content-Type' content='text/html; charset=utf-8' /><title> Arduino :: Управление климатом БС V0.1</title></head>"));
          //client.println(F("<meta http-equiv='Content-Type' content='text/html; charset=utf-8' /> "));
          //client.println(F("<title> Arduino :: Управление климатом БС V0.1</title>"));
          //client.println(F("</head> "));

          //client.println(F("<body"));

          client.print(F("<body<p><span style=\"font-size:20px\"><strong>Температура в помещении: </strong></span>"));
          client.print(temperature);
          client.print(F(" \xC2\xB0")); // показывает символ градуса
          client.print(F("C"));
          client.println(F("<hr />"));

          //Нажимание кнопок
          client.println(F("<p><form method=\"get\" name=\"MODE\"><p><input type=\"submit\" name=\"Level\" value=\"AUTO\">   Автоматический режим работы для поддержания температуры 21°С</p><p><input type=\"submit\" name=\"Level\" value=\"MANUAL\"> Ручной режим управления СКВ и нагревом</p>"));

          //client.println(F("<p><input type=\"submit\" name=\"Level\" value=\"AUTO\">   Автоматический режим работы для поддержания температуры 21°С</p>"));
          //client.println(F("<p><input type=\"submit\" name=\"Level\" value=\"MANUAL\"> Ручной режим управления СКВ и нагревом</p>"));

          if (readString.indexOf("Level=MANUAL") >= 0) {
            MOD = true;
            manual();
          }

          if (readString.indexOf("KON1=ON") >= 0) {
            digitalWrite(3, HIGH);
            CON1 = true;
            manual();
          }
          if (readString.indexOf("KON1=OFF") >= 0) {
            digitalWrite(3, LOW);
            CON1 = false;
            manual();
          }

          if (readString.indexOf("KON2=ON") >= 0) {
            digitalWrite(4, HIGH);
            CON2 = true;
            manual();
          }
          if (readString.indexOf("KON2=OFF") >= 0) {
            digitalWrite(4, LOW);
            CON2 = false;
            manual();
          }

          if (readString.indexOf("HEAT=ON") >= 0) {
            digitalWrite(5, HIGH);
            HEAT = true;
            manual();
          }
          if (readString.indexOf("HEAT=OFF") >= 0) {
            digitalWrite(5, LOW);
            HEAT = false;
            manual();
          }

          if (readString.indexOf("Level=AUTO") >= 0) {


            /*
              if (temperature > tempUp) {
              client.println(F("<p><table align=\"left\" border=\"1\" cellpadding=\"1\" cellspacing=\"1\" style=\"width:500px\"><tbody><tr><td style=\"text-align:center\"><span style=\"color:#3498db\">Кондиционер 1</span></td><td style=\"text-align:center\"><span style=\"color:#3498db\">Кондиционер 2</span></td><td style=\"text-align:center\"><span style=\"color:#e74c3c\">Обогреватель</span></td></tr><tr><td style=\"text-align:center\">Статус: ВКЛ (при +31° до 21°)</td><td style=\"text-align:center\">Статус: ВКЛ (при +31° до 21°)</td><td style=\"text-align:center\">Статус: ВЫКЛ</td></tr></tbody></table><p>&nbsp;</p><p>&nbsp;</p>"));
              //client.println(F("<p>&nbsp;</p>"));
              //client.println(F("<p>&nbsp;</p>"));
              }

              if (temperature < tempDown) {
              client.println(F("<p><table align=\"left\" border=\"1\" cellpadding=\"1\" cellspacing=\"1\" style=\"width:500px\"><tbody><tr><td style=\"text-align:center\"><span style=\"color:#3498db\">Кондиционер 1</span></td><td style=\"text-align:center\"><span style=\"color:#3498db\">Кондиционер 2</span></td><td style=\"text-align:center\"><span style=\"color:#e74c3c\">Обогреватель</span></td></tr><tr><td style=\"text-align:center\">Статус: ВЫКЛ</td><td style=\"text-align:center\">Статус: ВЫКЛ</td><td style=\"text-align:center\">Статус: ВКЛ (при +10 до 18°)</td></tr></tbody></table><p>&nbsp;</p><p>&nbsp;</p>"));
              //client.println(F("<p>&nbsp;</p>"));
              //client.println(F("<p>&nbsp;</p>"));
              }
            */
            /*
              client.println(F("<hr />"));
              if (CON1 == 1) {
              client.println(F("<p>Кондиционер 1 - Включен</p>"));
              } else {
              client.println(F("<p>Кондиционер 1 - Выключен</p>"));
              }

              if (CON2 == 1) {
              client.println(F("<p>Кондиционер 2 - Включен</p>"));
              } else {
              client.println(F("<p>Кондиционер 2 - Выключен</p>"));
              }

              if (HEAT == 1) {
              client.println(F("<p>Обогреватель - Включен</p>"));
              } else {
              client.println(F("<p>Обогреватель - Выключен</p>"));
              }
            */
            MOD = false;
            avto();
          }

          if (readString.indexOf("uptemp") > 0) {
            String uptemp = (readString.substring(readString.indexOf("uptemp") + 7, readString.indexOf("&downtemp")));
            tempUp = uptemp.toInt();
            //Serial.print("Верхний порог температуры: ");
            Serial.println(tempUp);
          }

          if (readString.indexOf("downtemp") > 0) {
            String downtemp = (readString.substring(readString.indexOf("downtemp") + 9, readString.indexOf("&Le")));
            tempDown = downtemp.toInt();
            //Serial.print("Нижний порог температуры: ");
            Serial.println(tempDown);
          }


          if (readString.indexOf("upheat") > 0) {
            String upHeatTemp = (readString.substring(readString.indexOf("upheat") + 7, readString.indexOf("&downheat")));
            tempUpHeat = upHeatTemp.toInt();
            //Serial.print("Верхний порог температуры HEAT: ");
            Serial.println(tempUpHeat);
          }

          if (readString.indexOf("downheat") > 0) {
            String upHeatTemp = (readString.substring(readString.indexOf("downheat") + 9, readString.indexOf("&Le")));
            tempDownHeat = upHeatTemp.toInt();
            //Serial.print("Верхний порог температуры HEAT: ");
            Serial.println(tempDownHeat);
          }

          //Горизонатльная черта на всю страницу
          client.println(F("<hr />"));

          if (MOD == 0) {
            client.println(F("<span style=\"font-size:20px\"><strong>ТЕКУЩИЙ РЕЖИМ РАБОТЫ МКК: </strong></span><font size=\"’5′\">АВТО режим"));
            //client.println(F("<font size=\"’5′\">АВТО режим"));
            client.print(F("<p>Верхний порог темрературы СКВ = "));
            client.print(tempUp);
            client.print(F("<p>Нижний порог темрературы СКВ = "));
            client.print(tempDown);
            client.print(F("<p>Верхний порог темрературы HEAT = "));
            client.print(tempUpHeat);
            client.print(F("<p>Нижний порог темрературы HEAT = "));
            client.print(tempDownHeat);



          } else {
            client.println(F("<span style=\"font-size:20px\"><strong>ТЕКУЩИЙ РЕЖИМ РАБОТЫ МКК: </strong></span><font size=\"’5′\">РУЧНОЙ режим"));
            //client.println(F("<font size=\"’5′\">РУЧНОЙ режим"));
          }

          client.println(F("<hr />"));
          if (CON1 == 1) {
            client.println(F("<p>Кондиционер 1 - Включен</p>"));
          } else {
            client.println(F("<p>Кондиционер 1 - Выключен</p>"));
          }

          if (CON2 == 1) {
            client.println(F("<p>Кондиционер 2 - Включен</p>"));
          } else {
            client.println(F("<p>Кондиционер 2 - Выключен</p>"));
          }

          if (HEAT == 1) {
            client.println(F("<p>Обогреватель - Включен</p>"));
          } else {
            client.println(F("<p>Обогреватель - Выключен</p>"));
          }
          client.println(F("<hr />"));

          client.println(F("<p><input type=\"submit\" name=\"Level\" value=\"MODE\"> Ввод пороговых значений температуры СКВ и HEAT</p>"));

          if (readString.indexOf("Level=MODE") >= 0) {

            client.println(F("<p><form action=\"http://192.168.0.33\" method=\"get\" name=\"form\"><br>Верхний порог температуры СКВ: <input name=\"uptemp\" type=\"number\" value=""><br><br>Нижний порог температуры СКВ: <input name=\"downtemp\" type=\"number\" value=""><br> <br>Верхний порог температуры HEAT: <input name=\"upheat\" type=\"number\" value=""><br><br>Нижний порог температуры HEAT: <input name=\"downheat\" type=\"number\" value=""><br> <br><input type=\"submit\" name=\"Level\" value=\"SEND\"></form></p>"));
            //client.println(F("<br>Верхний порог СКВ: <input name=\"T_high\" type=\"number\" value=""><br>"));
            //client.println(F("<br>Нижний порог СКВ: <input name=\"T_low\" type=\"number\" value=""><br>"));
            //client.println(F("<br><input type=\"submit\" name=\"Level\" value=\"SEND\"></form></p>"));

          }

          client.print(F("<p>Время работы контроллера: "));
          client.print (millis() / 86400000);
          client.print (F(" дн., "));
          client.print ((millis() / 3600000) % 24);
          client.print (F(" час., "));
          client.print ((millis() / 60000) % 60);
          client.print (F(" мин., "));
          client.print ((millis() / 1000) % 60);
          client.println (F(" сек."));

          client.println(F("</body></html>"));
          //clearing string for next read
          readString = "";

          // give the web browser time to receive the data
          //delay(1);

          //останавливаем web-client
          client.stop();

          //Выаод значений текущего режима и включенных реле
          /*
              Serial.print("MOD - ");
              Serial.println(MOD);
              Serial.print("CON1 - ");
              Serial.println(CON1);
              Serial.print("CON2 - ");
              Serial.println(CON2);
              Serial.print("HEAT - ");
              Serial.println(HEAT);
          */

          Serial.println("Disconnected\n");
        }
      }
    }
  }
}

void avto() {
  if (temperature > tempUp) {
    digitalWrite(3, HIGH); // set the LED on
    digitalWrite(4, HIGH); // set the LED on
    CON1 = true;
    CON2 = true;
  } else if (temperature < tempDown) {
    digitalWrite(3, LOW);
    digitalWrite(4, LOW);
    CON1 = false;
    CON2 = false;
  }

  if (temperature < tempDownHeat) {
    digitalWrite(5, HIGH);
    HEAT = true;
  } else if (temperature > tempUpHeat) {
    digitalWrite(5, LOW);
    HEAT = false;
  }
}

void manual() {
  client.println(F("<p><table align=\"left\" border=\"1\" cellpadding=\"1\" cellspacing=\"1\" style=\"width:500px\"><tbody><tr><td style=\"text-align:center\"><span style=\"color:#3498db\">Кондиционер 1</span></td><td style=\"text-align:center\"><span style=\"color:#3498db\">Кондиционер 2</span></td><td style=\"text-align:center\"><span style=\"color:#e74c3c\">Обогреватель</span></td></tr><tr><td style=\"text-align:center\"><form method=\"get\" name=\"KON1\"> <p style=\"text-align:center\"><input type=\"submit\" name=\"KON1\" value = ON></p><p style=\"text-align:center\"><input type=\"submit\" name=\"KON1\" value=OFF></p></form></td><td style=\"text-align:center\"><p></p><form method = get name = KON2><p style=\"text-align:center\"><input type=\"submit\" name=\"KON2\" value = ON></p><p style=\"text-align:center\"><input type=\"submit\" name=\"KON2\" value=OFF></p></form></td><td style=\"text-align:center\"><p></p><form method = get name = HEAT><p style=\"text-align:center\"><input type=\"submit\" name=\"HEAT\" value = ON></p><p style=\"text-align:center\"><input type=\"submit\" name=\"HEAT\" value=OFF></p></form></td></tr></tbody></table><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p><p>&nbsp;</p>"));
  //client.println(F("<p>&nbsp;</p>"));
  //client.println(F("<p>&nbsp;</p>"));
  //client.println(F("<p>&nbsp;</p>"));
  //client.println(F("<p>&nbsp;</p>"));

  /*
  if (CON1 == 1) {
    client.println(F("<p>Кондиционер 1 - Включен</p>"));
  } else {
    client.println(F("<p>Кондиционер 1 - Выключен</p>"));
  }

  if (CON2 == 1) {
    client.println(F("<p>Кондиционер 2 - Включен</p>"));
  } else {
    client.println(F("<p>Кондиционер 2 - Выключен</p>"));
  }

  if (HEAT == 1) {
    client.println(F("<p>Обогреватель - Включен</p>"));
  } else {
    client.println(F("<p>Обогреватель - Выключен</p>"));
  }
  */
}
