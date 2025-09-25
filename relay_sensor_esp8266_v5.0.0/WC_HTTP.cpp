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
#ifdef DNS_SERVER
DNSServer dnsServer;
#endif

ES_WIFI_STAT w_stat2 = EWS_OFF;
bool isAP = false;
uint32_t msAP = 0, msSTA = 0;
String authPass = "";
String HTTP_User = "";
int    UID       = -1;

uint32_t msScan = 0;
uint32_t msLoad = 0;
std::vector <String> n_ssid;
std::vector <int> n_rssi;

bool is_update     = false;
bool is_load_page = false;

char *pages[] =  {"/", "/conf1", "/conf2" };


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
   if( EA_Config.isWiFiAlways || isWiFiAlways1 )ledSetWiFiMode(LED_WIFI_AP1);
   else ledSetWiFiMode(LED_WIFI_AP);
#ifdef DNS_SERVER
   dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
   dnsServer.start(53, "*", WiFi.softAPIP());
#endif
}

void WiFi_stop(const char *msg){
#ifdef DNS_SERVER
   dnsServer.stop();
#endif
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
   server.on ( "/conf1", HTTP_handleConfig1 );
   server.on ( "/conf2", HTTP_handleConfig2 );
   server.on ( "/distance", HTTP_handleDistance );
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
   server.on ( "/relay0.png", HTTP_handlePngRelay0 );
   server.on ( "/relay1.png", HTTP_handlePngRelay1 );
   server.on ( "/relay2.png", HTTP_handlePngRelay2 );
   server.on ( "/relay3.png", HTTP_handlePngRelay3 );
   server.on ( "/relay4.png", HTTP_handlePngRelay4 );
   server.on ( "/relay5.png", HTTP_handlePngRelay5 );
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
void HTTP_handlePngRelay0(){  server.send_P(200, PSTR("image/png"), relay0_png, sizeof(relay0_png));}
void HTTP_handlePngRelay1(){  server.send_P(200, PSTR("image/png"), relay1_png, sizeof(relay1_png));}
void HTTP_handlePngRelay2(){  server.send_P(200, PSTR("image/png"), relay2_png, sizeof(relay2_png));}
void HTTP_handlePngRelay3(){  server.send_P(200, PSTR("image/png"), relay3_png, sizeof(relay3_png));}
void HTTP_handlePngRelay4(){  server.send_P(200, PSTR("image/png"), relay4_png, sizeof(relay4_png));}
void HTTP_handlePngRelay5(){  server.send_P(200, PSTR("image/png"), relay5_png, sizeof(relay5_png));}

/**
 * Обработчик событий WEB-сервера
 */
void HTTP_loop(void){
//  WiFi_ScanNetwork();
  server.handleClient();
#ifdef DNS_SERVER
  dnsServer.processNextRequest();
#endif  
}

/**
 * Формирование CSS стилей WEB-сервера
 * 
 * @param out - строковый буфер
 */
void HTTP_printCSS(String &out){
    out += "<style>\n";
    out += " body { background-color:#ffffff;padding:20px;color:#000088; }\n";
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
//    out += " .btn {font-size: 1.2em;color:#000088;}\n";
    out += " td {valign:middle}\n";
    out += " hr {border-top:1px solid #000088;}\n";
    out += " input[type=file]::file-selector-button {border: 2px solid #000088;background:#fceade;color:#c55a11;width:30%;}\n";
    out += " input[type=file]::file-selector-button:hover {background:#000088;}\n";
    out += " input[type=submit] {font-size: 14pt;color:#000088;}\n";
    out += " input[type=submit].btn1 {background:#8888ff;}\n";
    out += " input[type=submit].btn2 {background:#88ff88;}\n";
    out += " input[type=submit].btn3 {background:#ffff88;}\n";
    out += " input[type=submit].btn4 {background:#ff8888;}\n";
    out += " input[type=submit].btn0 {background:#cccccc;}\n";
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
  out += "<h2>ДАТЧИК ПРИСУТСТВИЯ</h2>";
  out += "<p><img src=/logo.png></p>\n";

  //out += "<p><br>Сенсор: ";
  //out += EA_Config.ESP_NAME;
  //out += " ";
  out += _VERSION;
  out += "\n";
  out += "<p>";
  out += HTTP_User;

  char s[20]; 
  out += "<p>Дата: ";
  if( CheckTime(now()) )sprintf(s,"%02d.%02d.%04d %02d:%02d:%02d ",day(),month(),year(),hour(),minute(),second());   
  else sprintf(s,"30.05.2077 12:00:00"); 
  out += s; 
 
}   
 
/**
 * Выаод окнчания файла HTML
 */
void HTTP_printTail(String &out){
  out += "<br><hr align=\"left\" width=\"500\">Copyright (C) Miller-Ti, A.Shikharbeev, 2025&nbsp;&nbsp;&nbsp;&nbsp;Made from Russia";
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
 * Оработчик страницы с расстоянием
 */
void HTTP_handleDistance(void) {
  String out = "";
  char str[50];

  out += "<html>\n<head>\n<meta charset=\"utf-8\" />\n";
//  out += "<meta http-equiv='refresh' content='5'>\n";

  out += "<title>Расстояние от датчика</title>";
  out += "<style>\n";
  out += "body { background-color:#cccccc; color:#000088;}\n";
  out += "input[type=submit] {font-size: 1.2em;color:#000088;}\n";
  out += "</style>\n";    
  out += "<body>\n";
  out += "<form action='/distance' method='PUT'>\n";
  if( isnan(Distance) )strcpy(str,"NAN");
  else sprintf(str,"%d", (int)Distance );
  out += "<h3>Расстояние от датчика до препятствия сейчас (мм): ";
  out += str;
  out += "</h3>";
  out += "<input type='submit' value='Обновить' class='btn'>"; 

  out += "</form>\n";
  out += "</body>\n</html>\n";
   server.send(200, "text/html", out);
}

/*
 * Оработчик главной страницы сервера
 */
void HTTP_handleRoot(void) {
   int numPage = 0;
#ifdef DNS_SERVER
  if( HTTP_redirect() )return;
#endif
  if( HTTP_checkArgs(numPage) )return;
  if( msLoad != 0 ){
     Serial.println(F("!!! Skip HTTP root ..."));
     return;
  }
  msLoad = millis();
  char str[50];
  String out = "";
  is_load_page = true;
  HTTP_printHeader(out,"Главная",0);
  HTTP_print_menu(out, numPage);
// Блок №1
  out += "<fieldset>\n";

  out += "<iframe src='/distance' width=100% height=80 allowtransparency frameborder=0 scrolling='no'></iframe>\n"; 
  out += "<p class='t1'> расстояние NAN и датчик светится розовым, то сенсор не видит расстояние или поврежден.";
  out += "Если датчик светится МАЛИНОВЫМ - НЕ ОБНОВЛЯЙТЕ СТРАНИЦУ Сначала расположите датчик так, чтобы он стабильно замерял, видел расстояние и не светился малиновым.</p>";
  out += "</fieldset>\n";

  HTTP_login(out);
//  if( HTTP_login(out) )return;

#ifdef HTTP_FRAGMETATION
   server.setContentLength(CONTENT_LENGTH_UNKNOWN);
   Serial.printf("!!! HTTP Fragment 1 %d\n", out.length());  
   server.send ( 200, "text/html", out );
   out = "";
#endif

   if( UID >= 0 ){
      out += "<form action='";out += pages[numPage];out += "' method='PUT'>\n";
      HTTP_printConfigColor(out);
      HTTP_printConfig(out);
      out += "</form>\n";
      out += "<p><form action='/update' method='GET'><input type='submit' value='Обновление прошивки' class='btn1'></form>\n";
      out += "<p><form action='/' method='GET'><input type='submit' Name='Default' value='Сброс до заводских настроек' class='btn2'></form>\n";
      out += "<p><form action='/' method='GET'><input type='submit' Name='Reboot' value='Перезагрузка' class='btn3'></form>\n";
      out += "<p><form action='/' method='GET'><input type='submit' Name='Logout' value='Выход' class='btn4'></form>\n";
   }

   HTTP_printTail(out);
#ifdef HTTP_FRAGMETATION
   Serial.printf("!!! HTTP Fragment 4 %d\n", out.length());  
   server.sendContent(out);
#else
   Serial.printf("!!! HTTP Length %d\n", out.length());  
   server.send(200, "text/html", out);
   Serial.printf("!!! HTTP size page %d\n", out.length());  
#endif
   msLoad = 0;
   is_load_page = false;
}

/*
 * Оработчик страницы реле
 */
void HTTP_handleConfig1(void) {
   int numPage = 1;
#ifdef DNS_SERVER
  if( HTTP_redirect() )return;
#endif
  if( HTTP_checkArgs(numPage) )return;
  if( msLoad != 0 ){
     Serial.println(F("!!! Skip HTTP config ..."));
     return;
  }
  msLoad = millis();
  char str[50];
  String out = "";
  is_load_page = true;
  HTTP_printHeader(out,"Конфигурация реле",0);
  HTTP_print_menu(out, numPage);

  if( HTTP_login(out) )HTTP_goto(pages[numPage], 2, "Введите пароль");

  if( UID >= 0 ){
      out += "<h1>Настройка конфигурации реле</h1>\n";
      out += "<form action='";out += pages[numPage];out += "' method='PUT'>\n";
      HTTP_printConfigRelay(out);
      out += "</form>\n";
      out += "<p><form action='/update' method='GET'><input type='submit' value='Обновление прошивки' class='btn1'></form>\n";
      out += "<p><form action='/' method='GET'><input type='submit' Name='Default' value='Сброс до заводских настроек' class='btn2'></form>\n";
      out += "<p><form action='/' method='GET'><input type='submit' Name='Reboot' value='Перезагрузка' class='btn3'></form>\n";
      out += "<p><form action='/' method='GET'><input type='submit' Name='Logout' value='Выход' class='btn4'></form>\n";
  }

  HTTP_printTail(out);
#ifdef HTTP_FRAGMETATION
  Serial.printf("!!! HTTP Fragment 4 %d\n", out.length());  
  server.sendContent(out);
#else
  Serial.printf("!!! HTTP Length %d\n", out.length());  
  server.send(200, "text/html", out);
  Serial.printf("!!! HTTP size page %d\n", out.length());  
#endif
   msLoad = 0;
   is_load_page = false;
}

/*
 * Оработчик страницы сетевых настроек
 */
void HTTP_handleConfig2(void) {
   int numPage = 2;
#ifdef DNS_SERVER
  if( HTTP_redirect() )return;
#endif
  if( HTTP_checkArgs(numPage) )return;
  if( msLoad != 0 ){
     Serial.println(F("!!! Skip HTTP config ..."));
     return;
  }
  msLoad = millis();
  char str[50];
  String out = "";
  is_load_page = true;
  HTTP_printHeader(out,"Конфигурация доступа",0);
  HTTP_print_menu(out, numPage);


  if( HTTP_login(out) )HTTP_goto(pages[numPage], 2, "Введите пароль");

  if( UID >= 0 ){
      out += "<h1>Настройка сетевых параметров</h1>\n";
      out += "<form action='";out += pages[numPage];out += "' method='PUT'>\n";
      HTTP_printConfigNet(out);
      out += "</form>\n";
      out += "<p><form action='/update' method='GET'><input type='submit' value='Обновление прошивки' class='btn1'></form>\n";
      out += "<p><form action='/' method='GET'><input type='submit' Name='Default' value='Сброс до заводских настроек' class='btn2'></form>\n";
      out += "<p><form action='/' method='GET'><input type='submit' Name='Reboot' value='Перезагрузка' class='btn3'></form>\n";
      out += "<p><form action='/' method='GET'><input type='submit' Name='Logout' value='Выход' class='btn4'></form>\n";
  }

  HTTP_printTail(out);
#ifdef HTTP_FRAGMETATION
  Serial.printf("!!! HTTP Fragment 4 %d\n", out.length());  
  server.sendContent(out);
#else
  Serial.printf("!!! HTTP Length %d\n", out.length());  
  server.send(200, "text/html", out);
  Serial.printf("!!! HTTP size page %d\n", out.length());  
#endif
   msLoad = 0;
   is_load_page = false;
}


/**
* Настройка подсветки
**/
void HTTP_printConfigColor(String &out){
  char s[50];
// Блок №1c
  out += "<fieldset>\n";
  out += "<legend>Настройка подсветки датчика</legend>\n";
  out += "<table width=100%>";
  sprintf(s,"%d",EA_Config.Brightness);
  HTTP_printInput1(out,"Яркость 0-10","Brightness",s,16,32,HT_NUMBER);

  out += "<tr><td colspan=2>Цвет в режиме &quot;Свободно&quot;</td></tr>";
  out += "<tr>\n";
  HTTP_print_td_color(out, COLOR_FREE1, "ColorFree", 1, EA_Config.ColorFree,EA_Config.ColorFreeNum,25);
  HTTP_print_td_color(out, COLOR_FREE2, "ColorFree", 2, EA_Config.ColorFree,EA_Config.ColorFreeNum,25);
  HTTP_print_td_color(out, 0xffffff, "ColorFree", 3, EA_Config.ColorFree,EA_Config.ColorFreeNum,50,true);
  out += "</tr>\n";

  out += "<tr><td colspan=3>";
  HTTP_print_input_checkbox(out,"isFreeBlink","1",EA_Config.isColorFreeBlink);

  out += "<br>Мигание в режиме &quot;Свободно&quot; ";
  out += "</td></tr>";
  out += "<tr>\n";
  HTTP_print_td_color(out, COLOR_BLINK1, "ColorBlink", 1, EA_Config.ColorBlink,EA_Config.ColorBlinkNum,25);
  HTTP_print_td_color(out, COLOR_BLINK2, "ColorBlink", 2, EA_Config.ColorBlink,EA_Config.ColorBlinkNum,25);
  HTTP_print_td_color(out, 0xffffff, "ColorBlink", 3, EA_Config.ColorBlink,EA_Config.ColorBlinkNum,50,true);
  out += "</tr>\n";

  out += "<tr><td colspan=2><br>Цвет в режиме &quot;Занято&quot;</td></tr>";
  out += "<tr>";
  HTTP_print_td_color(out, COLOR_BUSY1, "ColorBusy", 1, EA_Config.ColorBusy,EA_Config.ColorBusyNum,25);
  HTTP_print_td_color(out, COLOR_BUSY2, "ColorBusy", 2, EA_Config.ColorBusy,EA_Config.ColorBusyNum,25);
  HTTP_print_td_color(out, 0xffffff, "ColorBusy", 3, EA_Config.ColorBusy,EA_Config.ColorBusyNum,50,true);
  out += "</tr>\n";

  out += "</table>";
  out += "<p><input type='submit' name='Save' value='Сохранить' class='btn'>"; 
  out += "</fieldset>\n";  

#ifdef HTTP_FRAGMETATION
  Serial.printf("!!! HTTP Fragment 2 %d\n", out.length());  
  server.sendContent(out);
  out = "";
#endif  
}




/**
* Парметры основных настроек
*/
void HTTP_printConfig(String &out){
  char s[32];
// Блок №2
  out += "<fieldset>\n";
  out += "<legend>Конфигурация WI-FI</legend>\n";
  out += "<table>";
  out += "<tr><td align='center' width=50%><img src='/wifi1.png'></td><td width=50% align='center'><img src='/wifi2.png'></td></tr>";
  out += "<tr><td align='center'>";
  HTTP_print_input_radio(out,"WiFiMode","1",!EA_Config.isWiFiAlways);
  out += "</td><td align='center'>";
  HTTP_print_input_radio(out,"WiFiMode","2",EA_Config.isWiFiAlways);
  out += "</td></tr>";
  out += "<tr><td align='center' class='td1'>Раздает WI-FI до перезапуска. Первый светодиод бирюзовый. </label>\n</td><td align='center' class='td1'>Всегда раздает WiFi. Первый светодиод белый.</td></tr></table>";
  out += "<p><input type='submit' name='Save' value='Сохранить' class='btn'>"; 
  out += "</fieldset>\n";

// Блок №2.5 
  out += "<fieldset>\n";
  out += "<legend>Автокалибровка</legend>\n";
  out += "<p><input type='submit' name='Calibrate' value='Автоматическая калибровка расстояния' class='btn'>"; 

// Блок №3 
  sprintf(s,"%d",EA_Config.TM_BEGIN_CALIBRATE);
  HTTP_printInput1(out,"Задержка начала калибровки (сек):","TMCalibr",s,16,32,HT_NUMBER);
  sprintf(s,"%d",EA_Config.SAMPLES_CLIBRATE);
  HTTP_printInput1(out,"Количество тестовых замеров для калибровки:","NumCalibr",s,16,32,HT_NUMBER);
  out += "<p><input type='submit' name='Save' value='Сохранить' class='btn'>"; 
  out += "</fieldset>\n";
//#ifdef HTTP_FRAGMETATION
//   Serial.printf("!!! HTTP Fragment 3a %d\n", out.length());  
//   server.sendContent(out);
 //  out = "";
//#endif

// Блок №5
  out += "<fieldset>\n";
  out += "<legend>Если датчик не видит расстояние</legend>\n";
  out += "<table>";
  HTTP_print_img_radio(out,"/stat2.png","Если не видит расстояние - не переключается. (в этот момент мигает фиолетовым)","NoneMode","1",( EA_Config.NanValueFlag  == NAN_VALUE_IGNORE ),false);
  HTTP_print_img_radio(out,"/stat3.png","Если не видит расстояние - переключается в &quot;занято&quot;","NoneMode","2",( EA_Config.NanValueFlag  == NAN_VALUE_BUSY ),false);
  HTTP_print_img_radio(out,"/stat1.png","Если не видит расстояние - переключается в &quot;свободно&quot;","NoneMode","3",( EA_Config.NanValueFlag  == NAN_VALUE_FREE ),false);

  sprintf(s,"%06lX",(uint32_t)COLOR_NAN);
  out += "<tr><td bgcolor='#"; out += s; out += "' height='50pt'>&nbsp;</td><td>";
  HTTP_print_input_checkbox(out,"isColorNan","1",EA_Config.isColorNan);
  out += "</td><td>Активация малиновой подсветки если датчик ничего не видит.";
  out += "</td></tr>";
  out += "</table>\n";
  
  sprintf(s,"%d",EA_Config.TM_LOOP_SENSOR);
  HTTP_printInput1(out,"Задержка между циклами опроса сенсора (сек):","TMLoop",s,16,32,HT_NUMBER);

  out += "<p><input type='submit' name='Save' value='Сохранить' class='btn'>"; 
  out += "</fieldset>\n";

// Блок №6
  out += "<fieldset>\n";
  out += "<legend>Режимы определения препятствия</legend>\n"; 
  HTTP_print_img_radio(out,"/type1.png","Установка датчика на потолке","MeasureType","1",( EA_Config.MeasureType  == MEASURE_TYPE_NORMAL ),true);
  sprintf(s,"%d",EA_Config.GroundLevel);
  HTTP_printInput1(out,"*Расстояние от датчика до пола (мм):","GroundLevel",s,20,32,HT_TEXT);
  sprintf(s,"%d",EA_Config.LimitDistance);
  HTTP_printInput1(out,"Минимальная высота на срабатывание (мм):","LimitDistance",s,16,32,HT_NUMBER);

  HTTP_print_img_radio(out,"/type2.png","Определяет как &quot;занято&quot; в заданном диапазоне:","MeasureType","2",( EA_Config.MeasureType  == MEASURE_TYPE_OUTSIDE ), true);
  sprintf(s,"%d",EA_Config.MinDistance1);
  HTTP_printInput1(out,"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀От (мм):","MinDistance1",s,16,32,HT_NUMBER);
  sprintf(s,"%d",EA_Config.MaxDistance1);
  HTTP_printInput1(out,"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀До (мм):","MaxDistance1",s,16,32,HT_NUMBER);
 
  HTTP_print_img_radio(out,"/type3.png","Определяет как &quot;свободно&quot; в заданном диапазоне:","MeasureType","3",( EA_Config.MeasureType  == MEASURE_TYPE_INSIDE ), true);
  sprintf(s,"%d",EA_Config.MinDistance2);
  HTTP_printInput1(out,"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀От (мм):","MinDistance2",s,16,32,HT_NUMBER);
  sprintf(s,"%d",EA_Config.MaxDistance2);
  HTTP_printInput1(out,"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀До (мм):","MaxDistance2",s,16,32,HT_NUMBER);

  out += "<p><input type='submit' name='Save' value='Сохранить' class='btn'>"; 
  out += " </fieldset>\n";


//         if( n_ssid[i] == EA_Config.AP_SSID )out+=" selected";


#ifdef HTTP_FRAGMETATION
   Serial.printf("!!! HTTP Fragment 3d %d\n", out.length());  
   server.sendContent(out);
   out = "";
#endif
}

/**
* Настройка реле
**/
void HTTP_printConfigRelay(String &out){
  char s[32];
//  sprintf(s,"%d",EA_Config.GroundLevel);
// Блок №4.1 
  out += "<fieldset>\n";
  out += "<legend>Режим работы реле №1</legend>\n";

  sprintf(s,"%d",EA_Config.TM_DelayON1);
  HTTP_printInput1(out,"Задержка переключения на &quot;занято&quot; (сек):","TMOn1",s,16,32,HT_NUMBER);
  sprintf(s,"%d",EA_Config.TM_DelayOFF1);
  HTTP_printInput1(out,"Задержка переключения на &quot;свободно&quot; (сек):","TMOff1",s,16,32,HT_NUMBER);

  HTTP_print_img_radio(out,"/relay0.png","Не используется","ModeRelay1","0",( EA_Config.ModeRelay1  == RELAY_NONE ), true);
  HTTP_print_img_radio(out,"/relay1.png","Реле по умолчанию, постоянно ВКЛ или ВЫКЛ режима Занято-свободно","ModeRelay1","1",( EA_Config.ModeRelay1  == RELAY_NORMAL ), true);
  HTTP_print_img_radio(out,"/relay2.png","Управление кнопкой на открытия или закрытие ворот","ModeRelay1","2",( EA_Config.ModeRelay1  == RELAY_PULSE ), true);
  HTTP_print_img_radio(out,"/relay4.png","Импульсный режим","ModeRelay1","4",( EA_Config.ModeRelay1  == RELAY_PWM  ), true);
  out +="<br>";
  HTTP_print_input_checkbox(out,"isInverseRelay1","1",EA_Config.isInverseRelay1);
  out += "<label><b>Инверсия занято/свободно</b></label>";
  out +="<br><br>";
  HTTP_print_img_radio(out,"/relay5.png","Управление кнопкой открытия-закрытия ворот","ModeRelay1","5",( EA_Config.ModeRelay1  == RELAY_PULSE2 ), true);
  sprintf(s,"%d",EA_Config.TM_PulseRelay1);
  HTTP_printInput1(out,"           На сколько секунд замкнуть контакты:","TM_PulseRelay1",s,16,32,HT_NUMBER);
  sprintf(s,"%d",EA_Config.TM_PauseRelay1);
  HTTP_printInput1(out,"           На сколько секунд разамкнуть контакты:","TM_PauseRelay1",s,16,32,HT_NUMBER);

  out += "<p><input type='submit' name='Save' value='Сохранить' class='btn'>"; 
  out += "</fieldset>\n";  

//#ifdef HTTP_FRAGMETATION
//   Serial.printf("!!! HTTP Fragment 3b %d\n", out.length());  
//   server.sendContent(out);
//   out = "";
//#endif
// Блок №4.2
  out += "<fieldset>\n";
  out += "<legend>Режим работы реле №2</legend>\n";

  sprintf(s,"%d",EA_Config.TM_DelayON2);
  HTTP_printInput1(out,"Задержка переключения на &quot;занято&quot; (сек):","TMOn2",s,16,32,HT_NUMBER);
  sprintf(s,"%d",EA_Config.TM_DelayOFF2);
  HTTP_printInput1(out,"Задержка переключения на &quot;свободно&quot; (сек):","TMOff2",s,16,32,HT_NUMBER);

  HTTP_print_img_radio(out,"/relay0.png","Не используется","ModeRelay2","0",( EA_Config.ModeRelay2  == RELAY_NONE ), true);
  HTTP_print_img_radio(out,"/relay1.png","Реле по умолчанию, постоянно ВКЛ или ВЫКЛ режима Занято-свободно","ModeRelay2","1",( EA_Config.ModeRelay2  == RELAY_NORMAL ), true);
  HTTP_print_img_radio(out,"/relay2.png","Управление кнопкой на открытия или закрытие ворот","ModeRelay2","2",( EA_Config.ModeRelay2  == RELAY_PULSE ), true);
  HTTP_print_img_radio(out,"/relay4.png","Импульсный режим","ModeRelay2","4",( EA_Config.ModeRelay2  == RELAY_PWM  ), true);
  out +="<br>";
  HTTP_print_input_checkbox(out,"isInverseRelay2","1",EA_Config.isInverseRelay2);
  out += "<label><b>Инверсия занято/свободно</b></label>";
  out +="<br><br>";
  HTTP_print_img_radio(out,"/relay5.png","Управление кнопкой открытия-закрытия ворот","ModeRelay2","5",( EA_Config.ModeRelay2  == RELAY_PULSE2 ), true);

  sprintf(s,"%d",EA_Config.TM_PulseRelay2);
  HTTP_printInput1(out,"           На сколько секунд замкнуть контакты:","TM_PulseRelay2",s,16,32,HT_NUMBER);

  sprintf(s,"%d",EA_Config.TM_PauseRelay2);
  HTTP_printInput1(out,"           На сколько секунд разамкнуть контакты:","TM_PauseRelay2",s,16,32,HT_NUMBER);
  out += "<p><input type='submit' name='Save' value='Сохранить' class='btn'>"; 

  out += "</fieldset>\n";

#ifdef HTTP_FRAGMETATION
   Serial.printf("!!! HTTP Fragment 3c %d\n", out.length());  
   server.sendContent(out);
   out = "";
#endif

}

/**
* Настройка Сетевых параметров
**/
void HTTP_printConfigNet(String &out){
  char s[32];

  out += "<fieldset>\n";
  out += "<legend>Тип сенсора</legend>\n"; 
  out += "<div class='lab2'><p><label>Выберите тип сенсора:</label>\n";
  out += "<select name='SensorType'>\n";
  out += "<option value='";out += String(SONAR_SR04T);out += "'";
  if( EA_Config.SensorType == SONAR_SR04T)out += " selected";
  out += ">Двойной ультразвуковой сенсор (SR04T)</option>";  
  out += "<option value='";out += String(SONAR_SR04TM2);out += "'";
  if( EA_Config.SensorType == SONAR_SR04TM2)out += " selected";
  out += ">Одинарный ультразвуковой сенсор (SR04M2)</option>";  
  out += "<option value='";out += String(SONAR_TFLUNA);out += "'";
  if( EA_Config.SensorType == SONAR_TFLUNA)out += " selected";
  out += ">Лазерный сенсор (TF-Luna)</option>";  
  out += "</select>\n";
  out += "</div>\n";
  out += "<p class='t1'>После смены типа сенсора нужно кратковременно передернуть питание.";

  out += "<p><input type='submit' name='Save' value='Сохранить' class='btn'>"; 
  out += " </fieldset>\n";

// Блок №7
  out += "<fieldset>\n";
  out += "<legend>Подключение к онлайн мониторингу CRM.MOSCOW</legend>\n";
  out += "<labelВвключить онлайн отправку данных</label>\n";
  
  HTTP_print_input_checkbox(out,"SEND_HTTP","send",EA_Config.isSendCrmMoscow);
  
  out += "<p class='t1'>Поставьте галочку. Введите все поля с ** и сохраните. После этого в первой вкладке выключите бесконечный режим раздачи WiFi активировав режим с бирюзовой иконкой.";
  out += "После сохраните и перезагрузите устройство нажав желтую кнопку внизу натроек.</p>";

  HTTP_printNetworks1(out,"WiFiName");
  HTTP_printInput1(out,"**Введите пароль от вашей WI-FI сети:","WiFiPassword",EA_Config.AP_PASS,20,32,HT_PASSWORD);
  HTTP_printInput1(out,"**Номер договора, логин личного кабинета:","Dogovor",EA_Config.DOGOVOR_ID,20,16,HT_TEXT);
  HTTP_printInput1(out,"**Номер бокса:","Box",EA_Config.BOX_ID,20,16,HT_TEXT);
  out += "<p class='t1'>Ниже идут дополнительные настройки. Посоветуйтесь с технической поддержкой прежде чем их менять.</p>";
  HTTP_printInput1(out,"Сервер:","Server",EA_Config.SERVER,20,32,HT_TEXT);
  sprintf(s,"%d",EA_Config.PORT);
  HTTP_printInput1(out,"Порт:","Port",s,16,32,HT_NUMBER);
  sprintf(s,"%d",EA_Config.TM_HTTP_SEND);
  HTTP_printInput1(out,"Связь с сервером через, сек:","TM_HTTP_SEND",s,16,32,HT_NUMBER);
  sprintf(s,"%d",EA_Config.TM_HTTP_RETRY_ERROR);
  HTTP_printInput1(out,"Повторная попытка отправки через, сек:","TM_HTTP_RETRY_ERROR",s,16,32,HT_NUMBER);
  out += "<p><input type='submit' name='Save' value='Сохранить' class='btn'>"; 
  out += "</fieldset>\n";  
//#ifdef HTTP_FRAGMETATION
//   Serial.printf("!!! HTTP Fragment 3e %d\n", out.length());  
//   server.sendContent(out);
//   out = "";
//#endif   

// Блок №8
  out += "<fieldset>\n";
  out += "<legend>Параметры DHCP</legend>\n";
  out += "<label>Статический IP:</label>\n";
  HTTP_print_input_checkbox(out,"STATIC_IP","static",!EA_Config.isDHCP);
  
  sprintf(s,"%d.%d.%d.%d",EA_Config.IP[0],EA_Config.IP[1],EA_Config.IP[2],EA_Config.IP[3]);
  HTTP_printInput1(out,"Адрес:","IPAddr",s,16,32,HT_IP);
  sprintf(s,"%d.%d.%d.%d",EA_Config.MASK[0],EA_Config.MASK[1],EA_Config.MASK[2],EA_Config.MASK[3]);
  HTTP_printInput1(out,"Маска:","IPMask",s,16,32,HT_IP);
  sprintf(s,"%d.%d.%d.%d",EA_Config.GW[0],EA_Config.GW[1],EA_Config.GW[2],EA_Config.GW[3]);
  HTTP_printInput1(out,"Шлюз:","IPGate",s,16,32,HT_IP);
  sprintf(s,"%d.%d.%d.%d",EA_Config.DNS[0],EA_Config.DNS[1],EA_Config.DNS[2],EA_Config.DNS[3]);
  HTTP_printInput1(out,"DNS:",    "IPDns",s,16,32,HT_IP);
 
  out += "<p><input type='submit' name='Save' value='Сохранить' class='btn'>"; 
  out += "</fieldset>\n";  

  out += "<fieldset>\n";
  out += "<legend>Параметры доступа к контроллеру</legend>\n";

  HTTP_printInput1(out,"Пароль для входа в правами администратора:","PasswordUser",EA_Config.ESP_OPER_PASS,20,32,HT_PASSWORD);
  if( UID == 0 ){
     HTTP_printInput1(out,"Пароль для входа с правами суперадминистратора:","PasswordAdmin",EA_Config.ESP_ADMIN_PASS,20,32,HT_PASSWORD);
     HTTP_printInput1(out,"Наименование устройства","NameESP",EA_Config.ESP_NAME,32,32,HT_TEXT,"lab1");
     out += "<p class='t1'>После изменения имени WiFi нажмите внизу желтую кнопку &quot;Перезагрузка&quot;</p>";
  }
  out += "<p><input type='submit' name='Save' value='Сохранить' class='btn'>"; 
  out += "</fieldset>\n";  

#ifdef HTTP_FRAGMETATION
   Serial.printf("!!! HTTP Fragment 3f %d\n", out.length());  
   server.sendContent(out);
   out = "";
#endif   

}





bool HTTP_checkArgs(int current){
   if( UID < 0 )return false;
   bool _save = false;
   bool _reboot = false;
// Если нажата кнопка "Калибровка"   
   if ( server.hasArg("Calibrate")  ){  
       ProcessingCalibrate(1000);
   }
   else if( server.hasArg("Default") ){ 
       char s[32];
       strncpy(s,EA_Config.ESP_NAME,32);
       EA_default_config();
       EA_clear_arh();
       strncpy(EA_Config.ESP_NAME,s,32);
       EA_save_config();
       EA_read_config();

       HTTP_goto("/", 2000, "Загрузка заводских параметров. Перезагрузка ..."); //1.12.24
       ESP.reset();  
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
      if(server.hasArg("ColorFree")){
         switch(server.arg("ColorFree").toInt()){
             case 1: EA_Config.ColorFree = COLOR_FREE1; break;
             case 2: EA_Config.ColorFree = COLOR_FREE2; break;
             default:
                EA_Config.ColorFree = HTMLtoInt(server.arg("ColorFree_Change").c_str());
//                Serial.printf("!!! Color = %lx\n",EA_Config.ColorFree);
         }
         EA_Config.ColorFreeNum = (uint8_t)server.arg("ColorFree").toInt();
      }
      if(server.hasArg("ColorBusy")){
         switch(server.arg("ColorBusy").toInt()){
             case 1: EA_Config.ColorBusy = COLOR_BUSY1; break;
             case 2: EA_Config.ColorBusy = COLOR_BUSY2; break;
             default:
                EA_Config.ColorBusy = HTMLtoInt(server.arg("ColorBusy_Change").c_str());
         }
         EA_Config.ColorBusyNum = (uint8_t)server.arg("ColorBusy").toInt();
      }
      if(server.hasArg("ColorBlink")){
         switch(server.arg("ColorBlink").toInt()){
             case 1: EA_Config.ColorBlink = COLOR_BLINK1; break;
             case 2: EA_Config.ColorBlink = COLOR_BLINK2; break;
             default:
                EA_Config.ColorBlink = HTMLtoInt(server.arg("ColorBlink_Change").c_str());
         }
         EA_Config.ColorBlinkNum = (uint8_t)server.arg("ColorBlink").toInt();
      }
      EA_Config.isColorFreeBlink = false;   
      if( server.hasArg("isFreeBlink"))EA_Config.isColorFreeBlink = true;
      EA_Config.isColorNan = false;   
      if( server.hasArg("isColorNan"))EA_Config.isColorNan = true;
      if(server.hasArg("Brightness")  )EA_Config.Brightness = server.arg("Brightness").toInt();

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
      if(server.hasArg("SensorType")){EA_Config.SensorType      = (T_SENSOR_TYPE)server.arg("SensorType").toInt();_reboot = true;}
      

      if(server.hasArg("LimitDistance"))EA_Config.LimitDistance      = server.arg("LimitDistance").toInt();
      if(server.hasArg("MinDistance1")  )EA_Config.MinDistance1      = server.arg("MinDistance1").toInt();
      if(server.hasArg("MaxDistance1")  )EA_Config.MaxDistance1      = server.arg("MaxDistance1").toInt();
      if(server.hasArg("MinDistance2")  )EA_Config.MinDistance2      = server.arg("MinDistance2").toInt();
      if(server.hasArg("MaxDistance2")  )EA_Config.MaxDistance2      = server.arg("MaxDistance2").toInt();
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
      if(server.hasArg("Port")         )EA_Config.PORT                      = server.arg("Port").toInt();
      if(server.hasArg("TM_HTTP_SEND") )EA_Config.TM_HTTP_SEND              = server.arg("TM_HTTP_SEND").toInt();
      if(server.hasArg("TM_HTTP_RETRY_ERROR"))EA_Config.TM_HTTP_RETRY_ERROR = server.arg("TM_HTTP_RETRY_ERROR").toInt();



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

      if(server.hasArg("TMOn1")         )EA_Config.TM_DelayON1              = server.arg("TMOn1").toInt();
      if(server.hasArg("TMOff1")        )EA_Config.TM_DelayOFF1             = server.arg("TMOff1").toInt();
      if(server.hasArg("ModeRelay1")    )EA_Config.ModeRelay1               = (T_RELAY_MODE)server.arg("ModeRelay1").toInt();
      if(server.hasArg("TM_PulseRelay1"))EA_Config.TM_PulseRelay1 = server.arg("TM_PulseRelay1").toInt();   
      if(server.hasArg("TM_PauseRelay1"))EA_Config.TM_PauseRelay1 = server.arg("TM_PauseRelay1").toInt();    
      if(server.hasArg("isInverseRelay1"))EA_Config.isInverseRelay1 = true;
      else EA_Config.isInverseRelay1 = false;

      if(server.hasArg("TMOn2")         )EA_Config.TM_DelayON2              = server.arg("TMOn2").toInt();
      if(server.hasArg("TMOff2")        )EA_Config.TM_DelayOFF2             = server.arg("TMOff2").toInt();
      if(server.hasArg("ModeRelay2")    )EA_Config.ModeRelay2               = (T_RELAY_MODE)server.arg("ModeRelay2").toInt();
      if(server.hasArg("TM_PulseRelay2"))EA_Config.TM_PulseRelay2 = server.arg("TM_PulseRelay2").toInt();   
      if(server.hasArg("TM_PauseRelay2"))EA_Config.TM_PauseRelay2 = server.arg("TM_PauseRelay2").toInt();   
      if(server.hasArg("isInverseRelay2"))EA_Config.isInverseRelay2 = true;
      else EA_Config.isInverseRelay2 = false;

   
      _save = true;
   }
/*   
if( _reboot ){ 
      EA_save_config(); 
      EA_read_config();      
       HTTP_goto("/", 20000, "Сохранение параметров и перезагрузка ...");
       delay(2000);
       system_restart();  
       return true;
   }
*/

   if( _save ){
      EA_save_config(); 
      EA_read_config();      
      HTTP_goto(pages[current], 1000, "Сохранение параметров ...");
      if( EA_Config.isWiFiAlways || isWiFiAlways1)ledSetWiFiMode(LED_WIFI_AP1);
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
   if( style == NULL )out += "<p><label>";
   else {
       out += "<div class=\"";
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
   if( style == NULL )out += "</p>";
    else out += "</div>";
     
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
#ifdef DNS_SERVER

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
#endif
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

void  HTTP_print_td_color(String &out, uint32_t color, char *name, uint8_t value, uint32_t color_set, uint8_t color_num, uint8_t proc, bool is_change){
  char s[64];
  sprintf(s,"%06lX",color);
  out += "<td align='center' width='";
  out += proc;
  //else out += "%' width='25%' ";
  out += "%' bgcolor='#"; out += s; out += "'>";
  sprintf(s,"%d",(int)value);  
  HTTP_print_input_radio(out,name,s,(value == color_num));
  if( is_change ){
     sprintf(s,"%06lX",color_set);
     out += " Настройка: <input type='color' value='#";
     out += s;
     out += "' name='";
     out += name;
     out += "_Change'>";
  }



  out += "</td>";
}

void HTTP_print_img_radio(String &out,char *img, char *label, char *name, char *value,bool checked, bool is_table){
  if(is_table)out += "<table>"; 
  out += "<tr><td><img src='";
  out += img;
  out += "'></td><td>";
  HTTP_print_input_radio(out,name,value,checked);
  out += "</td><td>";
  out += label;
  out += "</td></tr>";
  if(is_table)out += "</table>"; 

}



void HTTP_print_input_radio(String &out,char *name, char *value,bool checked){
  out += "<input type='radio' name='";
  out += name;
  out += "' value='";
  out += value;
  out += "'";
  if( checked )out += " checked";
  out += ">";   
}

void HTTP_print_input_checkbox(String &out,char *name, char *value,bool checked){
  out += "<input type='checkbox' name='";
  out += name;
  out += "' value='";
  out += value;
  out += "'";
  if( checked )out += " checked";
  out += ">";   
}

void HTTP_print_menu(String &out, int current){
  out += "<p><table width='100%'><tr>";
  out += "<td width = '33%' align='center'>";
  if( current == 0 )out += "<form method='GET'><input type='submit' value='Основные настройки' class='btn0'></form>\n";
  else out += "<form action='/' method='GET'><input type='submit' value='Основные настройки' class='btn2'></form>\n";
  out += "</td>";
  out += "<td width = '33%' align='center'>";
  if( current == 1 )out += "<form method='GET'><input type='submit' value='Настройки реле' class='btn0'></form>\n";
  else out += "<form action='/conf1' method='GET'><input type='submit' value='Настройки реле' class='btn2'></form>\n";
  out += "</td>";
  out += "<td width = '33%' align='center'>";
  if( current == 2 )out += "<form method='GET'><input type='submit' value='Сеть и сенсор' class='btn0'></form>\n";
  else out += "<form action='/conf2' method='GET'><input type='submit' value='Сеть и сенсор' class='btn2'></form>\n";
  out += "</td>";
  out += "</tr></table>\n";

}

uint32_t HTMLtoInt(const char *s_color){
   if(strlen(s_color) != 7)return 0L;
   if( s_color[0] != '#' )return 0L;
//   Serial.printf("!!!! color=%s\n",s_color);
   uint32_t color = 0L;
   uint32_t cur = 0;
   for(int i=1; i<7; i++){
       switch( s_color[i] ){
          case '0': cur = 0L; break;
          case '1': cur = 1L; break;
          case '2': cur = 2L; break;
          case '3': cur = 3L; break;
          case '4': cur = 4L; break;
          case '5': cur = 5L; break;
          case '6': cur = 6L; break;
          case '7': cur = 7L; break;
          case '8': cur = 8L; break;
          case '9': cur = 9L; break;
          case 'a': 
          case 'A': cur = 10L; break;
          case 'b': 
          case 'B': cur = 11L; break;
          case 'c': 
          case 'C': cur = 12L; break;
          case 'd': 
          case 'D': cur = 13L; break;
          case 'e': 
          case 'E': cur = 14L; break;
          case 'f': 
          case 'F': cur = 15L; break;
          default : cur = 0L;
       }
       switch(i){
          case 1: color += cur*0x100000; break;
          case 2: color += cur*0x10000; break;
          case 3: color += cur*0x1000; break;
          case 4: color += cur*0x100; break;
          case 5: color += cur*0x10; break;
          case 6: color += cur; break;
       }
//       Serial.printf("!!!! %d %c %lx %lx\n",i,s_color[i],cur,color);
   }
   return color;

}