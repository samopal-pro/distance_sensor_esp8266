/**
* Проект контроллера автомоек. Версия 4 от 2020
* Copyright (C) 2020 Алексей Шихарбеев
* http://samopal.pro
*/

#include "WC_HTTP.h"
#include "WC_Proc.h"
#include "crmlogo.h"


ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
DNSServer dnsServer;

ES_WIFI_STAT w_stat2 = EWS_OFF;
bool isAP = false;
uint32_t msAP = 0, msSTA = 0;
String authPass = "";
String HTTP_User = "";
int    UID       = -1;

uint32_t msScan = 0;
std::vector <String> n_ssid;
std::vector <int> n_rssi;



void WiFi_test(){
   uint32_t _ms = millis();
// Если режим точки доступа
//    if( w_stat2 == EWS_AP_MODE ){   
//       if( msAP > _ms || ( _ms - msAP ) > TM_AP_MODE ){
//          WiFi_stop("Stop AP Timeout");     
//       }
//       return;
//    }
// WiFi не сконфигурен  
   if ( strcmp(EA_Config.AP_SSID, "none")==0 ) {
      if( w_stat2 != EWS_NOT_CONFIG ){
         Serial.println(F("WiFi is not config"));
         w_stat2 = EWS_NOT_CONFIG;
         WS_set(255,0,0,true);
      }         
      return;
   }   
// Пытаемся подключиться к WiFi   
   if( w_stat2 == EWS_OFF || w_stat2 == EWS_NOT_CONFIG ){
      WiFi.mode(WIFI_STA);
      if( EA_Config.isDHCP == false )WiFi.config(EA_Config.IP,EA_Config.GW,EA_Config.MASK);
      WiFi.begin(EA_Config.AP_SSID, EA_Config.AP_PASS);
      w_stat2 = EWS_WAIT;
      msSTA = _ms;
      WS_set(255,255,0,true);
      Serial.print(F("WiFi connect wait "));
      Serial.println(EA_Config.AP_SSID);
      return;
   }
// Проверяем что есть подключение к WiFi   
   if(  WiFi.status() == WL_CONNECTED && w_stat2 == EWS_WAIT ){
      Serial.print(F("WiFi connect "));
      Serial.println(WiFi.localIP());
      WS_set(0,255,0,true);
      w_stat2 = EWS_ON;
      return;
   }
// Нет соединения с WiFi
   if( WiFi.status() != WL_CONNECTED ){
// Проверяем что время соединения вышло    
      if( w_stat2 == EWS_WAIT && ( msSTA > _ms || ( _ms - msSTA ) > TM_WIFI_CONNECT ) ){
         WiFi_stop("WiFi Connect Timeout");     
      }
      if( w_stat2 == EWS_ON ){
        WiFi_stop("WiFi Connect Lost");             
      }
   }

}

void WiFi_startAP(){
   WiFi_ScanNetwork();
   WiFi.mode(WIFI_AP);
   WiFi.softAP(EA_Config.ESP_NAME);
   Serial.printf("Start AP %s\n",EA_Config.ESP_NAME);
   Serial.println(F("Open http://192.168.4.1 in your browser"));
   HTTP_begin();
   msAP = millis();
   w_stat2 = EWS_AP_MODE;
   WS_set(255,255,255,true);
   dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
   dnsServer.start(53, "*", WiFi.softAPIP());
}

void WiFi_stop(const char *msg){
   dnsServer.stop();
   WiFi.disconnect(); //  this alone is not enough to stop the autoconnecter
   WiFi.mode(WIFI_OFF);
   Serial.println(msg);
   server.stop();
   w_stat2 = EWS_OFF;
   WS_set(255,0,0,true);
 }


/**
 * Старт WEB сервера
 */
void HTTP_begin(void){
   
 // Поднимаем WEB-сервер  
   server.on ( "/", HTTP_handleRoot );
   server.on ( "/config", HTTP_handleConfig );
   server.on ( "/login", HTTP_handleLogin );
   server.on ( "/logo.png", HTTP_handleLogo );
   server.on ( "/reboot", HTTP_handleReboot );
   server.onNotFound ( HTTP_handleRoot );
  //here the list of headers to be recorded
   const char * headerkeys[] = {"User-Agent","Cookie"} ;
   size_t headerkeyssize = sizeof(headerkeys)/sizeof(char*);
  //ask server to track these headers
   server.collectHeaders(headerkeys, headerkeyssize );
   httpUpdater.setup(&server,"/update");


   
   server.begin();
//   WiFi_ScanNetwork();
   Serial.printf( "HTTP server started ...\n" );
  
}

void HTTP_handleLogo() {
  server.send_P(200, PSTR("image/png"), logo, sizeof(logo));
}


/**
 * Вывод в буфер одного поля формы
 */
void HTTP_printInput(String &out,const char *label, const char *name, const char *value, int size, int len, bool is_pass){
   char str[10];
   if( strlen( label ) > 0 ){
      out += "<td>";
      out += label;
      out += "</td>\n";
   }
   out += "<td><input name ='";
   out += name;
   out += "' value='";
   out += value;
   out += "' size=";
   sprintf(str,"%d",size);  
   out += str;
   out += " length=";    
   sprintf(str,"%d",len);  
   out += str;
   if( is_pass )out += " type='password'";
   out += "></td>\n";  
}

/**
 * Вывод в буфер одного поля формы в две строчки
 */
void HTTP_printInput2(String &out,const char *label, const char *name, const char *value, int size, int len, bool is_pass){
   char str[10];
   if( strlen( label ) > 0 ){
      out += "<tr><td>";
      out += label;
      out += "</td></tr>\n";
   }
   out += "<tr><td><input name ='";
   out += name;
   out += "' value='";
   out += value;
   out += "' size=";
   sprintf(str,"%d",size);  
   out += str;
   out += " length=";    
   sprintf(str,"%d",len);  
   out += str;
   if( is_pass )out += " type='password'";
   out += "></td></tr>\n";  
}

/**
 * Выаод заголовка файла HTML
 */
void HTTP_printHeader(String &out,const char *title, uint16_t refresh){
  msAP = millis();
  out += "<html>\n<head>\n<meta charset=\"utf-8\" />\n";
  if( refresh ){
     char str[10];
     sprintf(str,"%d",refresh);
     out += "<meta http-equiv='refresh' content='";
     out +=str;
     out +="'/>\n"; 
  }
  out += "<title>";
  out += title;
  out += "</title>\n";
  out += "<style>body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }</style>\n</head>\n";
  out += "<body>\n";
  out += "<h2>ДАТЧИК ПРИСУТСВИЯ АВТОМОБИЛЯ</h2>";
  out += "<img src=/logo.png>\n";
  out += "<br><br>Сенсор: ";
  out += SensorID;
  out += " ";
  out += _VERSION;
  out += "<br>Пользователь: ";
  out += HTTP_User;
  out +=" <a href=\"/login?DISCONNECT=YES\">Выход</a>\n";

  char s[20]; 
// out += "<br><br>Температура: ";
// sprintf(s,"%d",Temp);
//  out += s;
//  out += "<br>Влажность(%): ";
//  sprintf(s,"%d",Hum);
//  out += s;
  out += "<br>Время: ";
  DateTime dt1 = DateTime(Time);
  int s1 = dt1.second();
  int m1 = dt1.minute();
  int h1 = dt1.hour();
  int d2 = dt1.day();
  int m2 = dt1.month();
  int y2 = dt1.year();
  sprintf(s,"%02d.%02d.%02d %02d:%02d:%02d ",d2,m2,y2,h1,m1,s1);    
  out += s; 
out += "<br><h3><br>Расстояние от датчика до препятствия сейчас(мм): ";
sprintf(s,"%d", Distance );
out += s;
out += "</h3>";
    out += "<br> <button onClick=\"window.location.href=window.location.href\">Обновить</button>";  
  out += "<h1>";
  out += title;
  out += "</h1>\n";
  out += "<br> Обновляйте страницу и в поле выше вы увидете расстояние от датчика до препятствия.<br>";
  out += "<br> Перепишите в строку *Высота датчика без автомобиля (мм): и сохраните .<br>";
  out += "<br> Если расстояние 11111 или -1 то сенсор не видит отражения или поврежден.<br>";
   out +="Перепешите эту высоту в поле которое отмеченно знаком * и сохраните.<br>";
  out += "<br>Калибровочное установленное значение от пола(мм): ";
  sprintf(s,"%d",abs(EA_Config.GroundLevel));
  out += s;
  out += "<br>Это растояние соответсвует полю ввода с знаком *<br>";
  out += "<br>Препятствие от калибровочного значения сейчас (мм): ";
  sprintf(s,"%d",abs(EA_Config.GroundLevel - Distance ));
  out += s;

}   
 
/**
 * Выаод окнчания файла HTML
 */
void HTTP_printTail(String &out){
  out += "<br><hr align=\"left\" width=\"500\">Copyright (C) Miller-Ti, A.Shikharbeev, 2023";
  out += "</body>\n</html>\n";
}

/**
 * Ввод имени и пароля
 */
void HTTP_handleLogin(){
  String msg;
// Считываем куки  
  if (server.hasHeader("Cookie")){   
//    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
//    Serial.println(cookie);
  }
  if (server.hasArg("DISCONNECT")){
    Serial.println("Disconnect");
    String header = "HTTP/1.1 301 OK\r\nSet-Cookie: ESP_PASS=\r\nLocation: /login\r\nCache-Control: no-cache\r\n\r\n";
    server.sendContent(header);
    return;
  }
  if ( server.hasArg("PASSWORD") ){
    String pass = server.arg("PASSWORD");
    
    if ( HTTP_checkAuth(pass.c_str()) >=0 ){
      String header = "HTTP/1.1 301 OK\r\nSet-Cookie: ESP_PASS="+pass+"\r\nLocation: /\r\nCache-Control: no-cache\r\n\r\n";
      server.sendContent(header);
      Serial.println("Login Success");
      return;
    }
  msg = "Неправильное имя или пароль";
  Serial.println("Login Failed");
  }
  String out = "";
  HTTP_printHeader(out,"Авторизация");
  out += "<form action='/login' method='POST'>\
    <table border=0 width='600'>\
      <tr>\
        <td width='200'>Введите пароль:</td>\
        <td width='400'><input type='password' name='PASSWORD' placeholder='password' size='32' length='32'></td>\
      </tr>\
      <tr>\
        <td width='200'><input type='submit' name='SUBMIT' value='Ввод'></td>\
        <td width='400'>&nbsp</td>\
      </tr>\
    </table>\
    </form><b>" +msg +"</b><br>";
  HTTP_printTail(out);  
  server.send(200, "text/html", out);
}


/**
 * Обработчик событий WEB-сервера
 */
void HTTP_loop(void){
//  WiFi_ScanNetwork();
  server.handleClient();
  dnsServer.processNextRequest();
}

/**
 * Перейти на страничку с авторизацией
 */
void HTTP_gotoLogin(){
  String header = "HTTP/1.1 301 OK\r\nLocation: /login\r\nCache-Control: no-cache\r\n\r\n";
  server.sendContent(header);
}


/*
 * Оработчик главной страницы сервера
 */
void HTTP_handleRoot(void) {
  char str[50];
// Проверка авторизации  
  int gid = HTTP_isAuth();
  if ( gid < 0 ){
    HTTP_gotoLogin();
    return;
  } 

  
  String out = "";
   HTTP_printHeader(out,"Главная",5);
   
   if( isAP ){
      out += "<p>Режим точки доступа: ";
      out += EA_Config.ESP_NAME;
   }
   else {
      out += "<p>Подключено к ";
      out += EA_Config.AP_SSID;
   }   

//   sprintf(str,"<p>%02d.%02d.%02d %02d:%02d",day(Time),month(Time),year(Time),hour(Time),minute(Time));
//   out += str;

   
    
  
   if( gid >= 0 )out += "<p><a href=\"/config\">Настройка</a>";
   HTTP_printTail(out);
     
   server.send ( 200, "text/html", out );
}


/*
 * Оработчик страницы настройки сервера
 */
void HTTP_handleConfig(void) {
// Проверка прав администратора  
  if ( HTTP_isAuth() < 0 ){
    HTTP_gotoLogin();
    return;
  } 

// Сохранение контроллера
  if ( server.hasArg("AP_SSID") ){
//     EA_Config.isAP = false;
     EA_Config.isDHCP = false;
     if( server.hasArg("ESP_NAME")     )strcpy(EA_Config.ESP_NAME,server.arg("ESP_NAME").c_str());
//     if( server.hasArg("ESP_PASS")     )strcpy(EA_Config.ESP_PASS,server.arg("ESP_PASS").c_str());
     if( server.hasArg("AP_SSID")      )strcpy(EA_Config.AP_SSID,server.arg("AP_SSID").c_str());
     if( server.hasArg("AP_PASS")      )strcpy(EA_Config.AP_PASS,server.arg("AP_PASS").c_str());
     if( server.hasArg("IP1")          )EA_Config.IP[0] = atoi(server.arg("IP1").c_str());
     if( server.hasArg("IP2")          )EA_Config.IP[1] = atoi(server.arg("IP2").c_str());
     if( server.hasArg("IP3")          )EA_Config.IP[2] = atoi(server.arg("IP3").c_str());
     if( server.hasArg("IP4")          )EA_Config.IP[3] = atoi(server.arg("IP4").c_str());
     if( server.hasArg("MASK1")        )EA_Config.MASK[0] = atoi(server.arg("MASK1").c_str());
     if( server.hasArg("MASK2")        )EA_Config.MASK[1] = atoi(server.arg("MASK2").c_str());
     if( server.hasArg("MASK3")        )EA_Config.MASK[2] = atoi(server.arg("MASK3").c_str());
     if( server.hasArg("NASK4")        )EA_Config.MASK[3] = atoi(server.arg("MASK4").c_str());
     if( server.hasArg("GW1")          )EA_Config.GW[0] = atoi(server.arg("GW1").c_str());
     if( server.hasArg("GW2")          )EA_Config.GW[1] = atoi(server.arg("GW2").c_str());
     if( server.hasArg("GW3")          )EA_Config.GW[2] = atoi(server.arg("GW3").c_str());
     if( server.hasArg("GW4")          )EA_Config.GW[3] = atoi(server.arg("GW4").c_str());
     if( server.hasArg("ADMIN_PASS")   )strcpy(EA_Config.ESP_ADMIN_PASS,server.arg("ADMIN_PASS").c_str());
     if( server.hasArg("OPER_PASS")   )strcpy(EA_Config.ESP_OPER_PASS,server.arg("OPER_PASS").c_str());
//     if( server.hasArg("OPER_PASS")    )strcpy(EA_Config.ESP_OPER_PASS,server.arg("OPER_PASS").c_str());
     if( server.hasArg("DOGOVOR")   )strcpy(EA_Config.DOGOVOR_ID,server.arg("DOGOVOR").c_str());
     if( server.hasArg("BOX")   )strcpy(EA_Config.BOX_ID,server.arg("BOX").c_str());
     if( server.hasArg("SERVER")   )strcpy(EA_Config.SERVER,server.arg("SERVER").c_str());
     if( server.hasArg("PORT")          )EA_Config.PORT = atoi(server.arg("PORT").c_str());
     if( server.hasArg("GROUND_LEVEL")  )EA_Config.GroundLevel = atoi(server.arg("GROUND_LEVEL").c_str());
     if( server.hasArg("LIMIT_DISTANCE")  )EA_Config.LimitDistance = atoi(server.arg("LIMIT_DISTANCE").c_str());
     if( server.hasArg("LIMIT_DISTANCE_UP")  )EA_Config.LimitDistanceUp = atoi(server.arg("LIMIT_DISTANCE_UP").c_str());
     if( server.hasArg("ZERO_DISTANCE")  )EA_Config.ZeroDistance = atoi(server.arg("ZERO_DISTANCE").c_str());
     if( server.hasArg("TM_ON")   )EA_Config.TM_ON  = atoi(server.arg("TM_ON").c_str());
     if( server.hasArg("TM_OFF")  )EA_Config.TM_OFF = atoi(server.arg("TM_OFF").c_str());
     if( server.hasArg("IS_DHCP"))EA_Config.isDHCP = true;
/*     
     if( server.hasArg("IS_AP")  ){
         EA_Config.isAP = true;
//         int x = atoi(server.arg("IS_AP").c_str());
//         Serial.print("is_ap ");
//         Serial.println(server.arg("IS_AP").c_str());
     //    EA_Config.GroundLevel = atoi(server.arg("GROUND_LEVEL").c_str());
     }
*/     
// Отключаем режим точки доступа     
     EA_save_config();
     
//   Если был режим точки доступа - перезагружаемся 
     if( isAP ){
        String header = "HTTP/1.1 301 OK\r\nLocation: /reboot\r\nCache-Control: no-cache\r\n\r\n";
        server.sendContent(header);
        delay(1000);
        ESP.reset();
     }
     else {
        String header = "HTTP/1.1 301 OK\r\nLocation: /config\r\nCache-Control: no-cache\r\n\r\n";
        server.sendContent(header);
  
     }
     return;
  }
  
  String out = "";
  char str[10];
  HTTP_printHeader(out,"Настройка контроллера");
  out += "\
  <ul>\
  <li><a href=\"/\">Главная</a>\
  <li>&nbsp\
  <li><a href=\"/update\">Обновление прошивки</a>\
  <li>&nbsp\
  <li><a href=\"/reboot\">Перезагрузка*** </a>\
  </ul>\n";


// Форма для настзройки параметров
   out += "<h2>Конфигурация</h2>";
   out += "<h3>Параметры подключения к WiFi</h3>";
      out += "<form action='/config' method='POST'><table><tr>";
      out +="<table><tr>";
   if( UID == 0 ){
      HTTP_printInput(out,"Наименование устройства:","ESP_NAME",EA_Config.ESP_NAME,32,32);
//   out += "</tr><tr>";
//   HTTP_printInput(out,"Пароль:","ESP_PASS",EA_Config.ESP_PASS,32,32,true);
      out += "</tr><tr>";
   }
/*
   out += "<td>Режим точки доступа</td><td><input name=\"IS_AP\" type=\"checkbox\" ";
   if( EA_Config.isAP == true )out += "checked=\"checked\"";
   out += "/>&nbsp;</td>";
*/

   out += "</tr><tr>";
   HTTP_printNetworks(out);
//   HTTP_printInput(out,"Введите имя вашей WiFi сети:","AP_SSID",EA_Config.AP_SSID,32,32);
   out += "</tr><tr>";
   HTTP_printInput(out,"**Введите пораль от вашей WiFi сети","AP_PASS",EA_Config.AP_PASS,32,32,false);
   out += "</tr></table>";
//   if( UID == 0 ){
      out += "<table><tr>";
      out += "<td>Автоматическая настройка IP (DHCP)</td><td span=4><input name=\"IS_DHCP\" type=\"checkbox\" ";
      if( EA_Config.isDHCP == true )out += "checked=\"checked\"";
      out += "/>&nbsp;</td></tr><tr>";
      sprintf(str,"%d",EA_Config.IP[0]); 
      HTTP_printInput(out,"Статический IP:","IP1",str,3,3);
      sprintf(str,"%d",EA_Config.IP[1]); 
      HTTP_printInput(out,".","IP2",str,3,3);
      sprintf(str,"%d",EA_Config.IP[2]); 
      HTTP_printInput(out,".","IP3",str,3,3);
      sprintf(str,"%d",EA_Config.IP[3]); 
      HTTP_printInput(out,".","IP4",str,3,3);
      out += "</tr><tr>";
      sprintf(str,"%d",EA_Config.MASK[0]); 
      HTTP_printInput(out,"Маска","MASK1",str,3,3);
      sprintf(str,"%d",EA_Config.MASK[1]); 
      HTTP_printInput(out,".","MASK2",str,3,3);
      sprintf(str,"%d",EA_Config.MASK[2]); 
      HTTP_printInput(out,".","MASK3",str,3,3);
      sprintf(str,"%d",EA_Config.MASK[3]); 
      HTTP_printInput(out,".","MASK4",str,3,3);
      out += "</tr><tr>";
      sprintf(str,"%d",EA_Config.GW[0]); 
      HTTP_printInput(out,"Шлюз","GW1",str,3,3);
      sprintf(str,"%d",EA_Config.GW[1]); 
      HTTP_printInput(out,".","GW2",str,3,3);
      sprintf(str,"%d",EA_Config.GW[2]); 
      HTTP_printInput(out,".","GW3",str,3,3);
      sprintf(str,"%d",EA_Config.GW[3]); 
      HTTP_printInput(out,".","GW4",str,3,3);
      out += "</tr><table>";
//   }
   out += "<h3>Параметры контроллера</h3>";
   out += "<table><tr>";
   if( UID == 0 ){
      HTTP_printInput(out,"Пароль администратора для входа<br>в настройки устройства:","ADMIN_PASS",EA_Config.ESP_ADMIN_PASS,16,32,true);
      out += "</tr><tr>";
      HTTP_printInput(out,"Пароль клиента для входа в настройки устройства:","OPER_PASS",EA_Config.ESP_OPER_PASS,16,32,true);
      out += "</tr><tr>";
   }
   HTTP_printInput(out,"**Номер договора, идентификатор мойки:","DOGOVOR",EA_Config.DOGOVOR_ID,16,16,false);
   out += "</tr><tr>";
   HTTP_printInput(out,"**Номер бокса:","BOX",EA_Config.BOX_ID,16,16,false);
   if( UID == 0 ){
      out += "</tr><tr>";
      HTTP_printInput(out,"Сервер:","SERVER",EA_Config.SERVER,16,32,false);
      out += "</tr><tr>";
      sprintf(str,"%u",EA_Config.PORT); 
      HTTP_printInput(out,"Порт:","PORT",str,16,16,false);
   }

out += "</tr><tr>";
sprintf(str,"%u",EA_Config.GroundLevel);
out += "<h3>";
HTTP_printInput(out,"*Высота датчика без автомобиля (мм):","GROUND_LEVEL",str,16,16,false);
out += "</h3>";
   
   out += "</tr><tr>";
   sprintf(str,"%d",EA_Config.LimitDistance); 
   HTTP_printInput(out,"Расстояние на срабатывание датчика (мм):","LIMIT_DISTANCE",str,16,16,false);
   out += "</tr><tr>";
   sprintf(str,"%d",EA_Config.LimitDistanceUp); 
   HTTP_printInput(out,"Выше какой высоты считать что занято (мм):","LIMIT_DISTANCE_UP",str,16,16,false);
   out += "</tr><tr>";
   sprintf(str,"%d",EA_Config.ZeroDistance); 
   HTTP_printInput(out,"Заданная высота если не видит расстояние :","ZERO_DISTANCE",str,16,16,false);
   out += "</tr><tr>";
   sprintf(str,"%u",EA_Config.TM_ON); 
   HTTP_printInput(out,"Задержка переключения на красный, сек:","TM_ON",str,16,16,false);
   out += "</tr><tr>";
  sprintf(str,"%u",EA_Config.TM_OFF);
   HTTP_printInput(out,"Задержка переключения на зеленый, сек:","TM_OFF",str,16,16,false);
   out += "</tr><table>";

   
   out +="<input type='submit' name='SUBMIT_CONF' value='Сохранить'></form><br>"; 
   out +="* Обязательная настройка для работы без онлайн отправки данных<br>";
   out +="** Обязательная настройка для отправки данных на сайт www.crm.moscow<br>";
   out +="*** После перезагрузки доступ к настройкам через обнуление (магнит)<br>";
   HTTP_printTail(out);
   server.send ( 200, "text/html", out );
}        

/*
 * Перезагрузка часов
 */
void HTTP_handleReboot(void) {
// Проверка прав администратора  
  if ( HTTP_isAuth() != 0 ){
    HTTP_gotoLogin();
    return;
  } 


  String out = "";

  out = 
"<html>\
  <head>\
    <meta charset='utf-8' />\
    <meta http-equiv='refresh' content='5;URL=http://www.crm.moscow'>\
    <title>ESP8266 sensor 1</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Оборудование настраивается, сейчас датчик подключится к Вашей Wi-Fi сети. </h1>\
    </body>\
</html>";
   server.send ( 200, "text/html", out );
   ESP.reset();  
  
}

/**
 * Проверка авторизации
 */
int HTTP_isAuth(){
//  Serial.print("AUTH ");
  if (server.hasHeader("Cookie")){   
//    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
//    Serial.print(cookie);
 
    if (cookie.indexOf("ESP_PASS=") != -1) {
      authPass = cookie.substring(cookie.indexOf("ESP_PASS=")+9);       
      return HTTP_checkAuth(authPass.c_str());
    }
  }
  return -1;  
}

/**
 * Функция проверки пароля
 * возвращает 0 - админ, 1 - оператор, -1 - Не авторизован
 */
int  HTTP_checkAuth(const char *pass){
   if( strncmp(pass,EA_Config.ESP_ADMIN_PASS,32 ) ==0 ){
       UID = 0;
       HTTP_User = "Администратор";
   }
   else  if( strncmp(pass,EA_Config.ESP_OPER_PASS,32) == 0 ){
       UID = 1;
       HTTP_User = "Клиент";
   }
   else {
       UID = -1;
       HTTP_User = "Анонимный пользователь";
   }
   Serial.printf("Auth is %d\n",UID);
   return UID;
}


void WiFi_ScanNetwork(){
   n_ssid.clear();
   n_rssi.clear();
   int n = WiFi.scanNetworks();
   int indices[n];
   for (int i = 0; i < n; i++)indices[i] = i;

      // RSSI SORT
   for (int i = 0; i < n; i++) {
      for (int j = i + 1; j < n; j++) {
         if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i])) {
            std::swap(indices[i], indices[j]);
         }
      }
   }
   
   for (int i = 0; i < n; ++i) {
      n_ssid.push_back(WiFi.SSID(indices[i]));
      n_rssi.push_back(WiFi.RSSI(indices[i]));
   }
   for( int i=0; i< n_ssid.size();i++ ){
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(n_ssid[i]);
      Serial.print(" (");
      Serial.print(n_rssi[i]);
      Serial.println(")");
   }
}

void HTTP_printNetworks(String &out){
   int n = n_ssid.size();
   if( n <= 0 ){
      HTTP_printInput(out,"**Введите имя вашей WiFi сети:","AP_SSID",EA_Config.AP_SSID,32,32);
   }
   else {
      out += "<td>**Выберите сеть WiFi</td><td>";
      out += "<select name=\"AP_SSID\">\n";
      for (int i=0; i<n; i++){
         out += "    <option value=\"";
         out += n_ssid[i];
         out += "\"";
 //        if( strcmp(n_ssid[i].c_str(),EA_Config.AP_SSID) == 0 )out+=" selected";
         if( n_ssid[i] == EA_Config.AP_SSID )out+=" selected";
         out += ">";
         out += n_ssid[i];
         out += " [";
         out += n_rssi[i];
         out += "dB] ";
         out += "</option>\n";
      }     
      out += "</select>\n</td>\n";   
   }

}


  
