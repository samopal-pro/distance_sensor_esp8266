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

bool is_update     = false;

void WiFi_test(){
   uint32_t _ms = millis();
// Если режим точки доступа
    if( w_stat2 == EWS_AP_MODE ){   
       return;
    }
// WiFi не сконфигурен  
   if ( strcmp(EA_Config.AP_SSID, "none")==0 && w_stat2 != EWS_AP_MODE) {
      if( w_stat2 != EWS_NOT_CONFIG ){
         Serial.println(F("??? WiFi is not config"));
         w_stat2 = EWS_NOT_CONFIG;
         ledSetWiFiMode(LED_WIFI_OFF);
      }         
      return;
   }   
// Пытаемся подключиться к WiFi   
   if( w_stat2 == EWS_OFF || w_stat2 == EWS_NOT_CONFIG ){
      WiFi_ScanNetwork();
      WiFi.mode(WIFI_STA);
      if( EA_Config.isDHCP == false )WiFi.config(EA_Config.IP,EA_Config.GW,EA_Config.MASK,EA_Config.DNS);
      WiFi.begin(EA_Config.AP_SSID, EA_Config.AP_PASS);
      w_stat2 = EWS_WAIT;
      msSTA = _ms;
      ledSetWiFiMode(LED_WIFI_WAIT);
      Serial.print(F("!!! WiFi connect wait "));
      Serial.println(EA_Config.AP_SSID);
      return;
   }
// Проверяем что есть подключение к WiFi   
   if(  WiFi.status() == WL_CONNECTED && w_stat2 == EWS_WAIT ){
      Serial.print(F("!!! WiFi connect "));
      Serial.print(WiFi.localIP());
      Serial.print(" MASK: ");
      Serial.print(WiFi.subnetMask());
      Serial.print(" GW: ");
      Serial.print(WiFi.gatewayIP());
      Serial.print(" DNS: ");
      Serial.println(WiFi.dnsIP());
      ledSetWiFiMode(LED_WIFI_ON);
      w_stat2 = EWS_ON;
      
      return;
   }
// Нет соединения с WiFi
   if( WiFi.status() != WL_CONNECTED ){
// Проверяем что время соединения вышло    
      if( w_stat2 == EWS_WAIT && ( msSTA > _ms || ( _ms - msSTA ) > TM_WIFI_CONNECT ) ){
         WiFi_stop("??? WiFi Connect Timeout");     
      }
      if( w_stat2 == EWS_ON ){
        WiFi_stop("??? WiFi Connect Lost");             
      }
   }

}

void WiFi_startAP(){
   WiFi_ScanNetwork();
   WiFi.mode(WIFI_AP);
   WiFi.softAP(EA_Config.ESP_NAME);
   Serial.printf("!!! Start AP %s\n",EA_Config.ESP_NAME);
   Serial.println(F("Open http://192.168.4.1 in your browser"));
   HTTP_begin();
   msAP = millis();
   w_stat2 = EWS_AP_MODE;
   if( EA_Config.isWiFiAlways)ledSetWiFiMode(LED_WIFI_AP1);
   else ledSetWiFiMode(LED_WIFI_AP);
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
   ledSetWiFiMode(LED_WIFI_OFF);
 }


/**
 * Старт WEB сервера
 */
void HTTP_begin(void){
   
 // Поднимаем WEB-сервер  
   server.on ( "/", HTTP_handleRoot );
//   server.on ( "/config", HTTP_handleConfig );
//   server.on ( "/login", HTTP_handleLogin );
   server.on ( "/logo.png", HTTP_handleLogo );
   server.on ( "/stat1.png", HTTP_handlePngStat1 );
   server.on ( "/stat2.png", HTTP_handlePngStat2 );
   server.on ( "/stat3.png", HTTP_handlePngStat3 );
   server.on ( "/wifi1.png", HTTP_handlePngWiFi1 );
   server.on ( "/wifi2.png", HTTP_handlePngWiFi2 );
   server.on ( "/type1.png", HTTP_handlePngType1 );
   server.on ( "/type2.png", HTTP_handlePngType2 );
   server.on ( "/type3.png", HTTP_handlePngType3 );
   server.onNotFound ( HTTP_handleRoot );
  //here the list of headers to be recorded
   const char * headerkeys[] = {"User-Agent","Cookie"} ;
   size_t headerkeyssize = sizeof(headerkeys)/sizeof(char*);
  //ask server to track these headers
   server.collectHeaders(headerkeys, headerkeyssize );
   httpUpdater.setup(&server,"/update");


   
   server.begin();
//   WiFi_ScanNetwork();
   Serial.printf( "!!! HTTP server started ...\n" );
  
}

void HTTP_handleLogo() {  server.send_P(200, PSTR("image/png"), logo, sizeof(logo));}
void HTTP_handlePngStat1() {  server.send_P(200, PSTR("image/png"), stat1_png, sizeof(stat1_png));}
void HTTP_handlePngStat2() {  server.send_P(200, PSTR("image/png"), stat2_png, sizeof(stat2_png));}
void HTTP_handlePngStat3() {  server.send_P(200, PSTR("image/png"), stat3_png, sizeof(stat3_png));}
void HTTP_handlePngWiFi1() {  server.send_P(200, PSTR("image/png"), wifi1_png, sizeof(wifi1_png));}
void HTTP_handlePngWiFi2() {  server.send_P(200, PSTR("image/png"), wifi2_png, sizeof(wifi2_png));}
void HTTP_handlePngType1() {  server.send_P(200, PSTR("image/png"), type1_png, sizeof(type1_png));}
void HTTP_handlePngType2() {  server.send_P(200, PSTR("image/png"), type2_png, sizeof(type2_png));}
void HTTP_handlePngType3() {  server.send_P(200, PSTR("image/png"), type3_png, sizeof(type3_png));}

/**
 * Обработчик событий WEB-сервера
 */
void HTTP_loop(void){
//  WiFi_ScanNetwork();
  server.handleClient();
  dnsServer.processNextRequest();
}

/**
 * Формирование CSS стилей WEB-сервера
 * 
 * @param out - строковый буфер
 */
void HTTP_printCSS(String &out){
    out += "<style>\n";
    out += " body { background-color:#ffffff;padding:20px; }\n";
    out += " .main {width:600;background:#cccccc;color:#000088;padding:10px;border-radius:8px;float:left;}\n";
    out += " .main label {display:block;float:left;width:360;;font-size:12pt}\n";
    out += " .main input {background:#ffffff;color:#000088;border:0;font-size:12pt}\n";
    out += " .main select {background:#ffffff;color:#000088;border:0;font-size:12pt;width:299px;}\n";
    out += " .main fieldset {border-color:#000088; font-size:14pt;}\n";
    out += " .main h2 {font-size:14pt;}\n";
    out += " .main h3 {font-size:12pt;}\n";
    out += " .t1 {font-size:10pt;}\n";
    out += " .main legend {font-size:20px;font-weight:bold;}\n";
    out += " .td1 {color:#000088;align:center;}\n";
    out += " .lab1 label {float:left;width:262;font-size:12pt;}\n";
    out += " .lab2 label {display:block;float:left;width:262;font-size:12pt;}\n";
    out += " .lab2 select {background:#ffffff;color:#000088;font-size:12pt;width:299px;}\n";
    out += " .a1 {color:#000088;font-size:12pt;}\n";

//    out += " .col1 {width:50%;float:left;}\n";
//    out += " .col2 {width:45%;float:left;}\n";
//    out += " .text {width:100%;float:left;}\n";
//    out += " .tail {position:absolute;bottom:20;}\n";
//    out += " .imp2 {margin-top:3px;float:left;}\n";
    out += " .btn {font-size: 1.2em;color:#000088;}\n";
    out += " hr {border-top:1px solid #000088;}\n";
    out += " input[type=file]::file-selector-button {border: 2px solid #000088;background:#fceade;color:#c55a11;width:30%;}\n";
    out += " input[type=file]::file-selector-button:hover {background:#000088;}\n";
  //  out += " input[type=submit] {font-size: 1.2em;color:#000088;}\n";
    out += "</style>\n";  

}



/**
 * Выаод заголовка файла HTML
 */
void HTTP_printHeader(String &out,const char *title, uint16_t refresh){
  msAP = millis();
  HTTP_isAuth();
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
  HTTP_printCSS(out);
  out += "<body>\n";
  out += " <div class=\"main\" id=\"main\">\n  "; 
  out += "<h2>ДАТЧИК ПРИСУТСВИЯ АВТОМОБИЛЯ</h2>";
  out += "<p><img src=/logo.png></p>\n";

  out += "<p><br>Сенсор: ";
  out += EA_Config.ESP_NAME;
  out += " ";
  out += _VERSION;
  out += "\n";
  out += "<p>";
  out += HTTP_User;

  char s[20]; 
  out += "<p>Время: ";
  if( CheckTime(now()) )sprintf(s,"%02d.%02d.%04d %02d:%02d:%02d ",day(),month(),year(),hour(),minute(),second());   
  else sprintf(s,"30.05.2077 12:00:00"); 
  out += s; 
 
}   
 
/**
 * Выаод окнчания файла HTML
 */
void HTTP_printTail(String &out){
  out += "<br><hr align=\"left\" width=\"500\">Copyright (C) Miller-Ti, A.Shikharbeev, 2024";
  out += "</div>";
  out += "</body>\n</html>\n";
}

/**
* Проверка авторизации
*/
bool HTTP_login(String &out){
// Выход из авторизации
  if (server.hasArg("Logout")){
      Serial.println("!!! HTTP logout");
      String header = "HTTP/1.1 301 OK\r\nSet-Cookie: ESP_PASS=\r\nLocation: /\r\nCache-Control: no-cache\r\n\r\n";
      server.sendContent(header);
      return true;
  }
// Проверка введенного пароля   
  if ( server.hasArg("Login") ){
    String pass = server.arg("Password");
    
    if ( HTTP_checkAuth(pass.c_str()) >= 0 ){
      String header = "HTTP/1.1 301 OK\r\nSet-Cookie: ESP_PASS="+pass+"\r\nLocation: /\r\nCache-Control: no-cache\r\n\r\n";
      server.sendContent(header);
      Serial.println("!!! HTTP Login Success");
      return true;
    }
    else {
      Serial.println("!!! HTTP Login fail");
      return false;
    }
  }
// Проерка пароля из куков и выдача форма авторизации при необходимости
  if( HTTP_isAuth() < 0 ){
     out += "<form action='/' method='PUT'>\n";
     out += " <fieldset>\n";
     out += "  <legend>Пароль для доступа в настройки</legend>\n";
    HTTP_printInput1(out,"Пароль:","Password","",16,32,HT_PASSWORD);
     
     out += " <p><input type='submit' name='Login' value='Ввод' class='btn'>"; 
     out += " </fieldset>\n</form>\n";
  }
//  else {
//    out += "<a href=\"/?Logout=yes\">Выход</a>\n";
//  }
  return false;
}


/*
 * Оработчик главной страницы сервера
 */
void HTTP_handleRoot(void) {
    if( HTTP_redirect() )return;
    if( HTTP_checkArgs() )return;
  char str[50];
//  int gid = HTTP_isAuth();  

   String out = "";
  HTTP_printHeader(out,"Главная",0);

  out += "<form action='/' method='PUT'>\n";
// Блок №1
  out += " <fieldset>\n";
//     out += "  <legend>Пароль для доступа в настройки</legend>\n";
//     HTTP_printInput1(out,"Пароль:","Password","",16,32,HT_PASSWORD);
//   HTTP_printInput(out,"Расстояние от датчика до препятствия сейчас(мм):","xx",str,16,32,false);
  if( isnan(Distance) )strcpy(str,"NAN");
  else sprintf(str,"%d", (int)Distance );
  out += "<h3>Расстояние от датчика до препятствия сейчас (мм): ";
  out += str;
  out += "</h3>";

  if( !isnan(Temp)){
     sprintf(str,"%d", (int)Temp );
     out += "<h3>Температура (С): ";
     out += str;
     out += "</h3>";
  }
  if( !isnan(Hum)){
     sprintf(str,"%d", (int)Hum );
     out += "<h3>Влажность воздуха (%): ";
     out += str;
     out += "</h3>";
  }
  out += " <input type='submit' value='Обновить' class='btn'>"; 
  out += " <p class='t1'> Обновляйте страницу и в поле выше вы увидете расстояние от датчика до препятствия.</p>";
  
  out += " </fieldset>\n</form>\n";
 
   if( HTTP_login(out) )return;

   server.setContentLength(CONTENT_LENGTH_UNKNOWN);
//   Serial.println(out);
   Serial.printf("!!! HTTP Fragment 1 %d\n", out.length());  
   server.send ( 200, "text/html", out );
   out = "";
   if( UID >= 0 ){
     HTTP_printConfig();
     out += "*Обязательная настройка для работы без онлайн отправки данных.<br>\n";
     out += "**Обязательная настройка для отправки данных на сайт www.crm.moscow.<br>\n";
     out += "<p><a class='a1' href=/update>Обновление прошивки</a>\n";
     out += "<p><a class='a1' href=/?Default=1>Сброс до заводских настроек</a>\n";
     out += "<p><a class='a1' href=/?Reboot=1>Перезагрузка</a>\n";
     out += "<p><a class='a1' href=/?Logout=1>Выход</a>\n";
  }

   HTTP_printTail(out);
   Serial.printf("!!! HTTP Fragment 4 %d\n", out.length());  
   server.sendContent(out);

//   Serial.printf("!!! HTTP size page %d\n", out.length());  
//   Serial.println(out);
//   server.send ( 200, "text/html", out );
}


void HTTP_printConfig(){
  String out = "";
 char s[32];
  sprintf(s,"%d",EA_Config.GroundLevel);
  out += "<form action='/' method='PUT'>\n";
// Блок №2
  out += " <fieldset>\n";
  out += "  <legend>Конфигурация WI-FI</legend>\n";
  out += "<table>";
  out += "<tr><td align='center' width=50%><img src='/wifi1.png'></td><td width=50% align='center'><img src='/wifi2.png'></td></tr>";
  out += "<tr><td align='center'><input type='radio' name='WiFiMode' value='1'";
  if( !EA_Config.isWiFiAlways )out += " checked";
  out += "></td><td align='center'><input type='radio' name='WiFiMode' value='2'";
  if( EA_Config.isWiFiAlways )out += " checked";
  out += "></td></tr>";
  out += "<tr><td align='center' class='td1'>(По умолчанию) раздает WiFi до перезапуска</label>\n</td><td align='center' class='td1'>Бесконечный доступ к настройкам</td></tr></table>";
  out += "<p class='t1'>Если вам нужен online мониторинг через сайт www.crm.moscow оставьте галочку \"по умолчанию\", веберите ниже сеть WI-FI с доступом ";
  out += "в интернет, введите пароль к ней, номер бокса и ID личного кабинета.";
  out += "ID вы можете получить в технической поддержке.";
  out += "<p><input type='submit' name='Save' value='Сохранить' class='btn'>"; 
  out += " </fieldset>\n";

// Блок №2.5 
  out += " <fieldset>\n";
  out += "  <legend>Автокалибровка</legend>\n";
  out += "<p class='t1'>Автоматическая калибровка делает паузу в 5 секунд, поле нескольких замеров, пока светится желтый цвет, выбирает максимально точное расстояние.";
  out += "<p class='t1'>Для ручной настройки высоты срабатывания перепешите в поле \"* Высота датчика без автомобиля (мм):\".</br>";
  out += "Если расстояние NAN то сенсор не видит расстояние или поврежден.";
//  out += " </fieldset>\n";

// Блок №3 
//  out += " <fieldset>\n";
//  out += "  <legend>Параметры автокалибровки</legend>\n";
  sprintf(s,"%d",EA_Config.TM_BEGIN_CALIBRATE);
  HTTP_printInput1(out,"Задержка начала калибровки (сек):","TMCalibr",s,16,32,HT_NUMBER);
  sprintf(s,"%d",EA_Config.SAMPLES_CLIBRATE);
  HTTP_printInput1(out,"Количество тестовых замеров для калибровки:","NumCalibr",s,16,32,HT_NUMBER);
  out += "<p><input type='submit' name='Calibrate' value='Автоматическая калибровка' class='btn'>"; 
  out += " </fieldset>\n";

// Блок №4 
  out += " <fieldset>\n";
  out += "  <legend>Параметры переключения и цикла опроса</legend>\n";
  sprintf(s,"%d",EA_Config.TM_ON);
  HTTP_printInput1(out,"Задержка переключения на &quot;занято&quot; (сек):","TMOn",s,16,32,HT_NUMBER);
  sprintf(s,"%d",EA_Config.TM_OFF);
  HTTP_printInput1(out,"Задержка переключения на &quot;свободно&quot; (сек):","TMOff",s,16,32,HT_NUMBER);
  sprintf(s,"%d",EA_Config.TM_LOOP_SENSOR);
  HTTP_printInput1(out,"Задержка между циклами опроса сенсора (сек):","TMLoop",s,16,32,HT_NUMBER);
  out += "<p><input type='submit' name='Save' value='Сохранить' class='btn'>"; 
  out += " </fieldset>\n";


// Блок №5
  out += " <fieldset>\n";
  out += "  <legend>Режим определения препятствия</legend>\n";
  out += "<table>";
  out += "<tr><td><img src='/stat2.png'></td><td valign='middle'><input type='radio' name='NoneMode' value='1'";
  if( EA_Config.NanValueFlag  == NAN_VALUE_IGNORE )out += " checked";
  out += "></td><td valign='middle' class='td1'>Если не видит расстояние не перключается. (в этот момент мигает фиолетовым)</td></tr>";
  out += "<tr><td><img src='/stat3.png'></td><td valign='middle'><input type='radio' name='NoneMode' value='2'";
  if( EA_Config.NanValueFlag  == NAN_VALUE_BUSY )out += " checked";
  out += "></td><td valign='middle' class='td1'>Если не видит расстояние, переключается в &quot;занято&quot;</td></tr>";
  out += "<tr><td><img src='/stat1.png'></td><td valign='middle'><input type='radio' name='NoneMode' value='3'";
  if( EA_Config.NanValueFlag  == NAN_VALUE_FREE )out += " checked";
  out += "></td><td valign='middle' class='td1'>Если не видит расстояние, переключается в &quot;свободно&quot;</td></tr></table>\n";
  out += "<p><input type='submit' name='Save' value='Сохранить' class='btn'>"; 
  out += " </fieldset>\n";

   

// Блок №6
  out += " <fieldset>\n";
  out += "  <legend>Режимы определения препятсвия</legend>\n"; 
  out += "<table><tr><td><img src='/type1.png'><br>&nbsp;</td><td valign='middle'><input type='radio' name='MeasureType' value='1'";
  if( EA_Config.MeasureType  == MEASURE_TYPE_NORMAL )out += " checked";
  out += "></td><td valign='middle' class='td1'>Срабатывает при превышение погога высоты</td></tr></table>";
  sprintf(s,"%d",EA_Config.GroundLevel);
  HTTP_printInput1(out,"* Высота датчика без автомобиля (мм):","GroundLevel",s,20,32,HT_TEXT);
  sprintf(s,"%d",EA_Config.LimitDistance);
  HTTP_printInput1(out,"Высота на срабатывание датчика (мм):","LimitDistance",s,16,32,HT_NUMBER);

  out += "<table><tr><td><img src='/type2.png'><br>&nbsp;</td><td valign='middle'><input type='radio' name='MeasureType' value='2'";
  if( EA_Config.MeasureType  == MEASURE_TYPE_OUTSIDE )out += " checked";
  out += "></td><td valign='middle' class='td1'>Срабатывает если вне диапазона границ диапазона</td></tr></table>";
  sprintf(s,"%d",EA_Config.MinDistance1);
  HTTP_printInput1(out,"Минимальное расстояние срабатывание датчика (мм):","MinDistance1",s,16,32,HT_NUMBER);
  sprintf(s,"%d",EA_Config.MaxDistance1);
  HTTP_printInput1(out,"Максимальное расстояние срабатывание датчика (мм):","MaxDistance1",s,16,32,HT_NUMBER);
 
  out += "<table><tr><td><img src='/type3.png'></td><td valign='middle'><input type='radio' name='MeasureType' value='3'";
  if( EA_Config.MeasureType  == MEASURE_TYPE_INSIDE )out += " checked";
  out += "></td><td valign='middle' class='td1'>Срабатывает если внутри диапазона границ диапазона</td></tr></table>\n";
  sprintf(s,"%d",EA_Config.MinDistance2);
  HTTP_printInput1(out,"Минимальное расстояние срабатывание датчика (мм):","MinDistance2",s,16,32,HT_NUMBER);
  sprintf(s,"%d",EA_Config.MaxDistance2);
  HTTP_printInput1(out,"Максимальное расстояние срабатывание датчика (мм):","MaxDistance2",s,16,32,HT_NUMBER);


Serial.printf("!!! HTTP Fragment 2 %d\n", out.length());  
   server.sendContent(out);
   out = "";

  out += "<p><input type='submit' name='Save' value='Сохранить' class='btn'>"; 
  out += " </fieldset>\n";


// Блок №7
  out += " <fieldset>\n";
  out += "  <legend>Параметры переключения к онлайн мониторингу</legend>\n";
  out += "    <label>Посылать информацию на удаленный сервер</label>\n";
  out += "    <input type=\"checkbox\" value=\"send\" name=\"SEND_HTTP\"";
  if( EA_Config.isSendCrmMoscow  )out += " checked>\n";
  else out += ">\n";
  HTTP_printNetworks1(out,"WiFiName");
  HTTP_printInput1(out,"**Введите пароль от вашей WI-FI сети","WiFiPassword",EA_Config.AP_PASS,20,32,HT_PASSWORD);

  HTTP_printInput1(out,"**Номер договора, идентификатор мойки:","Dogovor",EA_Config.DOGOVOR_ID,20,16,HT_TEXT);
  HTTP_printInput1(out,"**Номер бокса:","Box",EA_Config.BOX_ID,20,16,HT_TEXT);
  out += "<p class='t1'>Ниже идут дополнительные настройки. Посоветуйтесь с технической поддержкой прежде чем их менять.";
  HTTP_printInput1(out,"Сервер:","Server",EA_Config.SERVER,20,32,HT_TEXT);
  sprintf(s,"%d",EA_Config.PORT);
  HTTP_printInput1(out,"Порт:","Port",s,16,32,HT_NUMBER);
  out += "<p><input type='submit' name='Save' value='Сохранить' class='btn'>"; 
  out += " </fieldset>\n";  

// Блок №8
  out += "   <fieldset>\n";
  out += "  <legend>Параметры DHCP</legend>\n";
  out += "    <label>Статический IP:</label>\n";
  out += "    <input type=\"checkbox\" value=\"static\" name=\"STATIC_IP\"";
  if( !EA_Config.isDHCP  ){
     out += " checked>\n";
     out += " <input type='hidden' name='StaticIP' value='0'>\n";
  }
  else { 
     out += ">\n";
     out += " <input type='hidden' name='StaticIP' value='1'>\n";
  }  
  sprintf(s,"%d.%d.%d.%d",EA_Config.IP[0],EA_Config.IP[1],EA_Config.IP[2],EA_Config.IP[3]);
  HTTP_printInput1(out,"Адрес:","IPAddr",s,16,32,HT_IP);
  sprintf(s,"%d.%d.%d.%d",EA_Config.MASK[0],EA_Config.MASK[1],EA_Config.MASK[2],EA_Config.MASK[3]);
  HTTP_printInput1(out,"Маска:","IPMask",s,16,32,HT_IP);
  sprintf(s,"%d.%d.%d.%d",EA_Config.GW[0],EA_Config.GW[1],EA_Config.GW[2],EA_Config.GW[3]);
  HTTP_printInput1(out,"Шлюз:","IPGate",s,16,32,HT_IP);
  sprintf(s,"%d.%d.%d.%d",EA_Config.DNS[0],EA_Config.DNS[1],EA_Config.DNS[2],EA_Config.DNS[3]);
  HTTP_printInput1(out,"DNS:",    "IPDns",s,16,32,HT_IP);
 
  out += "<p><input type='submit' name='Save' value='Сохранить' class='btn'>"; 
  out += " </fieldset>\n";  

  out += "   <fieldset>\n";
  out += "  <legend>Параметры доступа к контроллеру</legend>\n";

  HTTP_printInput1(out,"Пароль для входа в настройки устройства:","PasswordUser",EA_Config.ESP_OPER_PASS,20,32,HT_PASSWORD);
  if( UID == 0 ){
     HTTP_printInput1(out,"Пароль для входа с правами администратора:","PasswordAdmin",EA_Config.ESP_ADMIN_PASS,20,32,HT_PASSWORD);
     HTTP_printInput1(out,"Наименование устройства","NameESP",EA_Config.ESP_NAME,32,32,HT_TEXT,"lab1");
  }
  out += "<p><input type='submit' name='Save' value='Сохранить' class='btn'>"; 
  out += " </fieldset>\n</form>\n";  


   Serial.printf("!!! HTTP Fragment 3 %d\n", out.length());  
   server.sendContent(out);

}

bool HTTP_checkArgs(){
   if( UID < 0 )return false;
   bool _save = false;
// Если нажата кнопка "Калибровка"   
   if ( server.hasArg("Calibrate")  ){  
       ProcessingCalibrate();
   }
   else if( server.hasArg("Default") ){ 
       EA_default_config();
       EA_clear_arh();
       EA_save_config();
       EA_read_config();
       HTTP_goto("/", 5000, "Загрузка заводских параметров ...");
       return true;
   }
   else if( server.hasArg("Reboot") ){ 
       HTTP_goto("/", 20000, "Перезагрузка ...");
       delay(2000);
       ESP.reset();  
       return true;
   }
// Если нажата кнопка "Сохранить"   
   else if ( server.hasArg("Save") && UID >= 0){
      EA_Config.isDHCP = true;
      EA_Config.isSendCrmMoscow = false;
      if(server.hasArg("GroundLevel"))EA_Config.GroundLevel = server.arg("GroundLevel").toInt();
      if(server.hasArg("WiFiMode"))
         switch(server.arg("WiFiMode").toInt()){
             case 1: EA_Config.isWiFiAlways = false; break;
             case 2: EA_Config.isWiFiAlways = true; break;
         }
      if(server.hasArg("NoneMode"))
         switch(server.arg("NoneMode").toInt()){
             case 1: EA_Config.NanValueFlag  = NAN_VALUE_IGNORE; break;
             case 2: EA_Config.NanValueFlag  = NAN_VALUE_BUSY; break;
             case 3: EA_Config.NanValueFlag  = NAN_VALUE_FREE; break;
         }
      if(server.hasArg("MeasureType"))
         switch(server.arg("MeasureType").toInt()){
             case 1: EA_Config.MeasureType  = MEASURE_TYPE_NORMAL; break;
             case 2: EA_Config.MeasureType  = MEASURE_TYPE_OUTSIDE; break;
             case 3: EA_Config.MeasureType  = MEASURE_TYPE_INSIDE; break;
         }
      if(server.hasArg("LimitDistance"))EA_Config.LimitDistance      = server.arg("LimitDistance").toInt();
      if(server.hasArg("MinDistance1")  )EA_Config.MinDistance1      = server.arg("MinDistance1").toInt();
      if(server.hasArg("MaxDistance1")  )EA_Config.MaxDistance1      = server.arg("MaxDistance1").toInt();
      if(server.hasArg("MinDistance2")  )EA_Config.MinDistance2      = server.arg("MinDistance2").toInt();
      if(server.hasArg("MaxDistance2")  )EA_Config.MaxDistance2      = server.arg("MaxDistance2").toInt();
      if(server.hasArg("TMOn")         )EA_Config.TM_ON              = server.arg("TMOn").toInt();
      if(server.hasArg("TMOff")        )EA_Config.TM_OFF             = server.arg("TMOff").toInt();
      if(server.hasArg("TMLoop")       )EA_Config.TM_LOOP_SENSOR     = server.arg("TMLoop").toInt();
      if(server.hasArg("TMCalibr")     )EA_Config.TM_BEGIN_CALIBRATE = server.arg("TMCalibr").toInt();
      if(server.hasArg("NumCalibr")    )EA_Config.SAMPLES_CLIBRATE   = server.arg("NumCalibr").toInt();
      if(server.hasArg("PasswordAdmin") && UID == 0 )strcpy(EA_Config.ESP_ADMIN_PASS,server.arg("PasswordAdmin").c_str());
      if(server.hasArg("PasswordOper") )strcpy(EA_Config.ESP_OPER_PASS,server.arg("PasswordOper").c_str());
      if(server.hasArg("NameESP")      && UID == 0 )strcpy(EA_Config.ESP_NAME,      server.arg("NameESP").c_str());
      if(server.hasArg("WiFiName")     )strcpy(EA_Config.AP_SSID,       server.arg("WiFiName").c_str());
      if(server.hasArg("WiFiPassword") )strcpy(EA_Config.AP_PASS,       server.arg("WiFiPassword").c_str());
      if( server.hasArg("SEND_HTTP"))EA_Config.isSendCrmMoscow = true;

      if(server.hasArg("Dogovor")      )strcpy(EA_Config.DOGOVOR_ID,    server.arg("Dogovor").c_str());
      if(server.hasArg("Box")          )strcpy(EA_Config.BOX_ID,        server.arg("Box").c_str());
      if(server.hasArg("Server")       )strcpy(EA_Config.SERVER,        server.arg("Server").c_str());
      if(server.hasArg("Port")         )EA_Config.PORT              = server.arg("Port").toInt();
      if( server.hasArg("STATIC_IP"))EA_Config.isDHCP = false;
      if( server.hasArg("SEND_HTTP"))EA_Config.isSendCrmMoscow = true;
      

//      if(server.hasArg("StaticIP"))
//         switch(server.arg("StaticIP").toInt()){
//             case 0: EA_Config.isDHCP  = false; break;
//             case 1: EA_Config.isDHCP  = true;  break;
//         }
      if(server.hasArg("IPAddr"))EA_Config.IP.fromString(server.arg("IPAddr").c_str());
      if(server.hasArg("IPMask"))EA_Config.MASK.fromString(server.arg("IPMask").c_str());
      if(server.hasArg("IPGate"))EA_Config.GW.fromString(server.arg("IPGate").c_str());
      if(server.hasArg("IPDns") )EA_Config.DNS.fromString(server.arg("IPDns").c_str());
   
      _save = true;
   }
   if( _save ){
      EA_save_config(); 
      EA_read_config();      
      HTTP_goto("/", 1000, "Сохранение параметров ...");
      if( EA_Config.isWiFiAlways)ledSetWiFiMode(LED_WIFI_AP1);
      else ledSetWiFiMode(LED_WIFI_AP);
      ledSetBaseMode(LED_BASE_SAVE,true);
      delay(300);
      ledRestoreMode();
      return true;
   }
   return false;
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
//   Serial.printf("!!! Check auth pass=%s len=%d ",pass,strlen(pass));
   if( strlen(pass) == 0 ){
       UID = -1;
       HTTP_User = "Минимальный права доступа";
   }

   else if( strncmp(pass,EA_Config.ESP_ADMIN_PASS,32 ) ==0 ){
       UID = 0;
       HTTP_User = "Права администратора";
   }
   else if( strncmp(pass,EA_Config.ESP_OPER_PASS,32) == 0 ){
       UID = 1;
       HTTP_User = "Права оператора";
   }
   else {
       UID = -1;
       HTTP_User = "Минимальный права доступа";
   }
//   UID = 0;
//   HTTP_User = "Администратор";
   Serial.printf("Auth is %d\n",UID);
   return UID;
}

/**
 * Формирование одного элемента INPUT в форме
 * 
 * @param out - строковый буфер
 * @param label - метка HTTP
 * @param name - имя отображаемое на форме
 * @param value - ткущее значение 
 * @param size - ширина текстового поля
 * @param len - максимальное количество символов
 * @param type - тип поля ввода: HT_TEXT,HT_PASSWORD,HT_NUMBER,HT_IP
 */
void HTTP_printInput1(String &out,const char *label, const char *name, const char *value, int size, int len, HTTP_input_type_t htype, const char *style, const char *add_text){
   char str[10];
   if( style == NULL )out += "  <p><label>";
   else {
       out += "  <div class=\"";
       out += style;
       out += "\"><label>";
   }
   out += label;
   out += "</label><input name ='";
  out += name;
   out += "' value='";
   out += value;
   out += "' size=";
   sprintf(str,"%d",size);  
   out += str;
   out += " maxlength=";    
   sprintf(str,"%d",len);  
   out += str;
   if( htype == HT_PASSWORD )out += " type='password'";
   if( htype == HT_NUMBER )out += " type='number'";
   if( htype == HT_IP ){
      out += " placeholder=\"xxx.xxx.xxx.xxx\"";
      out += " pattern=\"^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$\"";
   }
   out += ">";
   if( add_text != NULL )out += add_text;
   if( style == NULL )out += "</p>\n";
    else out += "</div>\n";
     
}



void WiFi_ScanNetwork(){
   Serial.println("Scan networks...");
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

/**
 * Выдача доступных WiFi сете в форму
 * 
 * @param out - текстовый буфер
 * @name name - имя метки HTTP поля
 */
void HTTP_printNetworks1(String &out, const char *name){
   int n = n_ssid.size();
   if( n <= 0 ){
      HTTP_printInput1(out,"**Введите имя вашей WI-FI сети:",name,EA_Config.AP_SSID,16,32,HT_TEXT);
   }
   else {
      out += "  <div class='lab2'><p><label>**Введите имя вашей WI-FI сети:</label>\n";
      out += "<select name=\"";
      out += name;
      out += "\">\n";
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
      out += "</select>\n</p></div>\n";   
   }

}


/** 
* Редирект с любого домена на главную страницу сервера
* 
* @return - true если сработал редирект
* @return false - редирект не сработал
*/
bool HTTP_redirect() {
//  if( !isAP )return true;

  String serverLoc =  server.client().localIP().toString();
  
  if (serverLoc != server.hostHeader() ) {
    Serial.print(F("HTTPD: redirect "));
    Serial.print(server.hostHeader());
    Serial.print(F(" to "));
    Serial.println(serverLoc);  
    server.sendHeader(F("Location"), (String)F("http://") + serverLoc, true); // @HTTPHEAD send redirect
    server.send ( 302, "text/plain", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
    server.client().stop(); // Stop is needed because we sent no content length
    return true;
  }
  return false;
}

 
/**
 * Выдать сообщение и переадресоваться на заданную чтраницу через N миллисекунд
 */
void HTTP_goto(const char *url, uint32_t tm, const char *msg){

   String content;
   HTTP_printHeader(content,msg,0);
   content += "<h2>";
   content += msg;  
   content += "</h2>";
   content += "<script type=\"text/javascript\">\n";
   content += " setTimeout(function(){location=\"";
   content += url;
   content += "\";}, ";
   content += String(tm);
   content +=");\n"; 
   content += " </script>\n"; 
   HTTP_printTail(content);
   server.send(200, "text/html", content);
}
 