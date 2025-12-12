/**
* Проект контроллера автомоек. Версия 4 от 2020
* Copyright (C) 2020 Алексей Шихарбеев
* http://samopal.pro
*/

#include "WC_HTTP.h"
#include "WC_Task.h"
#include "crmlogo.h"



WebServer server(80);
//ESP8266HTTPUpdateServer httpUpdater;
DNSServer dnsServer;

ES_WIFI_STAT w_stat2 = EWS_OFF;
bool isAP = false, isSTA = false;
uint32_t msAP = 0, msSTA = 0;
String authPass = "";
String HTTP_User = "";
int    UID       = -1;

uint32_t msScan = 0;
uint32_t msLoad = 0;
std::vector <String> n_ssid;
std::vector <int> n_rssi;

bool isHTTP = false;

bool is_update     = false;
bool is_load_page = false;
uint32_t   http_ms = 0;
char *pages[] =  {"/", "/conf1", "/conf2", "/conf3", "/conf4" };


void WiFi_test(){
   uint32_t _ms = millis();
// Если режим точки доступа
    if( w_stat2 == EWS_AP_MODE ){   
       return;
    }
// WiFi не сконфигурен  
   if ( jsonConfig["WIFI"]["NAME"] =="" &&  w_stat2 != EWS_AP_MODE) {
      if( w_stat2 != EWS_NOT_CONFIG ){
         Serial.println(F("??? WiFi is not config"));
         w_stat2 = EWS_NOT_CONFIG;
         ledSetColorAP(COLOR_WIFI_OFF);
      }         
      return;
   }   
// Пытаемся подключиться к WiFi   
   if( w_stat2 == EWS_OFF || w_stat2 == EWS_NOT_CONFIG ){
      WiFi_ScanNetwork();
      WiFi.mode(WIFI_STA);
      if( jsonConfig["WIFI"]["DHCP"].as<bool>() == false ){
      IPAddress ip_addr,ip_mask,ip_gate,ip_dns;
         if( ip_addr.fromString( jsonConfig["WIFI"]["IP"]["ADDR"].as<String>() ) &&
          ip_mask.fromString( jsonConfig["WIFI"]["IP"]["MASK"].as<String>() ) &&
          ip_gate.fromString( jsonConfig["WIFI"]["IP"]["GW"].as<String>() ) &&
          ip_dns.fromString(  jsonConfig["WIFI"]["IP"]["DNS"].as<String>()     ) ){
             Serial.println("Config static IP address");
             WiFi.config(ip_addr,ip_gate,ip_mask,ip_dns);    
         }
      }      
      WiFi.begin(jsonConfig["WIFI"]["NAME"].as<String>(), jsonConfig["WIFI"]["PASS"].as<String>());
      w_stat2 = EWS_WAIT;
      msSTA = _ms;
      ledSetColorAP(COLOR_WIFI_WAIT);
      Serial.print(F("!!! WiFi connect wait "));
      Serial.println(jsonConfig["WIFI"]["NAME"].as<String>());
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
      ledSetColorAP(COLOR_WIFI_ON);
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
   WiFi.softAP(jsonConfig["SYSTEM"]["NAME"].as<String>());
   Serial.printf("!!! Start AP %s\n",jsonConfig["SYSTEM"]["NAME"].as<const char *>());
   Serial.println(F("Open http://192.168.4.1 in your browser"));
// Стартуем DNS сервер   
   dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
   dnsServer.start(53, "*", WiFi.softAPIP());
// Стартуем WEBсервер
   HTTP_begin();
   msAP = millis();
   w_stat2 = EWS_AP_MODE;
   if( jsonConfig["SYSTEM"]["AP_START"].as<bool>() || isWiFiAlways1 )ledSetColorAP(COLOR_WIFI_AP1);
   else ledSetColorAP(COLOR_WIFI_AP);
//   server.begin();

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
   ledSetColorAP(COLOR_WIFI_OFF);
 }

/** 
* Редирект с любого домена на главную страницу сервера
* 
* @return - true если сработал редирект
* @return false - редирект не сработал
*/
bool HTTP_redirect1() {
//  DEBUG_WM(DEBUG_DEV,"-> " + server->hostHeader());
  
//  if(!_enableCaptivePortal) return false; // skip redirections, @todo maybe allow redirection even when no cp ? might be useful
  
  String serverLoc =  server.client().localIP().toString();
  
  if (serverLoc != server.hostHeader() ) {
#if defined(DEBUG_SERIAL)
    Serial.print(F("HTTPD: redirect "));
    Serial.print(server.hostHeader());
    Serial.print(F(" to "));
    Serial.println(serverLoc);  
#endif    
    server.sendHeader(F("Location"), (String)F("http://") + serverLoc, true); // @HTTPHEAD send redirect
    server.send ( 302, "text/plain", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
    server.client().stop(); // Stop is needed because we sent no content length
    return true;
  }
  return false;
}


/**
 * Старт WEB сервера
 */
void HTTP_begin(void){
   if( isHTTP )return;
   isHTTP = true;
   server.begin();
   
 // Поднимаем WEB-сервер  
   server.on ( "/", HTTP_handleRoot );
   server.on ( "/conf1", HTTP_handleConfig1 );
   server.on ( "/conf2", HTTP_handleConfig2 );
   server.on ( "/conf3", HTTP_handleConfig3 );
   server.on ( "/conf4", HTTP_handleConfig4 );
   server.on ( "/distance", HTTP_handleDistance );
   server.on ( "/update", HTTP_handleUpload );
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
   server.on ( "/playMP3", HTTP_handlePlayMP3 );
   server.onNotFound ( HTTP_handleRoot );
  //here the list of headers to be recorded
   const char * headerkeys[] = {"User-Agent","Cookie"} ;
   size_t headerkeyssize = sizeof(headerkeys)/sizeof(char*);
  //ask server to track these headers
   server.collectHeaders(headerkeys, headerkeyssize );
   server.on("/update1",HTTP_POST,HTTP_fileUpload1,HTTP_handleUpdate);
//   server.on("/update",HTTP_GET,HTTP_handleUpdate,HTTP_fileUpload1);


   
   server.begin();
//   WiFi_ScanNetwork();
   Serial.printf( "!!! HTTP server started ...\n" );
   dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
   dnsServer.start(53, "*", WiFi.softAPIP());
  
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
   if( !isHTTP )return;
   dnsServer.processNextRequest();   
   server.handleClient(); 

}

/**
 * Формирование JS функций
 * 
 * @param out - строковый буфер
 */
void HTTP_printJS(String &out){
   out += "<script>\n";   
   out += "function setColorVal(id,color){  event.preventDefault();document.getElementById(id).value = color; }\n";

   out += "function playMP3(dir,num){ fetch(\"/playMP3?DIR=\"+dir+\"&NUM=\"+num, {method: \"GET\" }); }\n";
//  out += " .then(response => { console.log(\"Запрос MP3, статус:\", response.status);return false; })\n";
//   out += " .catch(err => console.error(\"Ошибка запроса MP3:\", err));return false; });\n";
//   out += "}";
   out += "</script>\n";     
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
    out += " table .tab1 {border: 1px dotted grey;}\n";

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
    out += " input[type=number] {width:150;}\n";
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
  HTTP_printJS(out);
  
  out += "<body>\n";
  out += " <div class=\"main\" id=\"main\">\n  "; 
  out += "<h2>ДАТЧИК ПРИСУТСТВИЯ</h2>";
  out += "<p><img src=/logo.png></p>\n";

  //out += "<p><br>Сенсор: ";
  //out += EA_Config.ESP_NAME;
  //out += " ";
  out += SOFTWARE_V;
  out += "\n";
  out += "<p>";
  out += HTTP_User;

  char s[20]; 
  out += "<p>Дата: ";
//  if( CheckTime(now()) )sprintf(s,"%02d.%02d.%04d %02d:%02d:%02d ",day(),month(),year(),hour(),minute(),second());   
  sprintf(s,"30.05.2077 12:00:00"); 
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
//  if( HTTP_redirect() )return false;   
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


void HTTP_printBottomMenu(String &out){
   out += "<p><form action='/update' method='GET'><input type='submit' value='Обновление прошивки' class='btn1'></form>\n";
   out += "<p><form action='/' method='GET'><input type='submit' Name='Default' value='Сброс до заводских настроек' class='btn2'></form>\n";
   out += "<p><form action='/' method='GET'><input type='submit' Name='Reboot' value='Перезагрузка' class='btn3'></form>\n";
   out += "<p><form action='/' method='GET'><input type='submit' Name='Logout' value='Выход' class='btn4'></form>\n";
}


/*
 * Оработчик страницы с расстоянием
 */
void HTTP_handleDistance(void) {
  String out = "";
  char str[50];
  if( server.hasArg("Refresh"))checkPlayMP3("89",84);

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
  out += "<input type='submit' value='Обновить' name='Refresh' class='btn'>"; 

  out += "</form>\n";
  out += "</body>\n</html>\n";
   server.send(200, "text/html", out);
}

/*
 * Оработчик главной страницы сервера
 */
void HTTP_handleRoot(void) {
   int numPage = 0;
  if( HTTP_redirect() )return;
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

  out += "<iframe src='/distance' width=100% height=100 allowtransparency frameborder=0 scrolling='no'></iframe>\n"; 
  out += "<p class='t1'> расстояние NAN и датчик светится розовым, то сенсор не видит расстояние или поврежден.";
  out += "Если датчик светится МАЛИНОВЫМ - НЕ ОБНОВЛЯЙТЕ СТРАНИЦУ Сначала расположите датчик так, чтобы он стабильно замерял, видел расстояние и не светился малиновым.</p>";
  out += "</fieldset>\n";

  HTTP_login(out);
//  if( HTTP_login(out) )return;


   if( UID >= 0 ){
      out += "<form action='";out += pages[numPage];out += "' method='PUT'>\n";
      HTTP_InputHidden(out,"FLAG_ROOT");
      HTTP_printConfigColor(out);
      HTTP_printConfig(out);
      out += "</form>\n";
      HTTP_printBottomMenu(out);
   }

   HTTP_printTail(out);
   Serial.printf("!!! HTTP Root Length %d\n", out.length());  
   server.send(200, "text/html", out);
   msLoad = 0;
   is_load_page = false;
}

/*
 * Оработчик страницы реле
 */
void HTTP_handleConfig1(void) {
   int numPage = 1;
  if( HTTP_redirect() )return;
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
      HTTP_InputHidden(out,"FLAG_CONFIG1");
      HTTP_printConfigRelay(out);
      out += "</form>\n";
      HTTP_printBottomMenu(out);
  }

  HTTP_printTail(out);

  Serial.printf("!!! HTTP Config1 Length %d\n", out.length());  
  server.send(200, "text/html", out);
   msLoad = 0;
   is_load_page = false;
}

/*
 * Оработчик страницы сетевых настроек
 */
void HTTP_handleConfig2(void) {
   int numPage = 2;
  if( HTTP_redirect() )return;
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
      HTTP_InputHidden(out,"FLAG_CONFIG2");
      HTTP_printConfigNet(out);
      out += "</form>\n";
      HTTP_printBottomMenu(out);
  }

  HTTP_printTail(out);
  Serial.printf("!!! HTTP Config2 Length %d\n", out.length());  
  server.send(200, "text/html", out);
   msLoad = 0;
   is_load_page = false;
}

/*
 * Оработчик страницы сетевых настроек
 */
void HTTP_handleConfig3(void) {
   int numPage = 3;
  if( HTTP_redirect() )return;
  if( HTTP_checkArgs(numPage) )return;
  if( msLoad != 0 ){
     Serial.println(F("!!! Skip HTTP config ..."));
     return;
  }
  msLoad = millis();
  char str[50];
  String out = "";
  is_load_page = true;
  HTTP_printHeader(out,"Звуковые оповещения",0);
  HTTP_print_menu(out, numPage);


  if( HTTP_login(out) )HTTP_goto(pages[numPage], 2, "Введите пароль");

  if( UID >= 0 ){
      out += "<h1>Настройка звуковых оповещений</h1>\n";
      out += "<form action='";out += pages[numPage];out += "' method='PUT'>\n";
      HTTP_InputHidden(out,"FLAG_CONFIG3");
      HTTP_printConfig2(out);
      out += "</form>\n";
      HTTP_printBottomMenu(out);
  }

  HTTP_printTail(out);
  Serial.printf("!!! HTTP Config3 Length %d\n", out.length());  
  server.send(200, "text/html", out);
   msLoad = 0;
   is_load_page = false;
}

/*
 * Оработчик стартовых
 */
void HTTP_handleConfig4(void) {
   int numPage = 4;
  if( HTTP_redirect() )return;
  if( HTTP_checkArgs(numPage) )return;
  if( msLoad != 0 ){
     Serial.println(F("!!! Skip HTTP config ..."));
     return;
  }
  msLoad = millis();
  char str[50];
  String out = "";
  is_load_page = true;
  HTTP_printHeader(out,"Стартовые настройки",0);
  HTTP_print_menu(out, numPage);


  if( HTTP_login(out) )HTTP_goto(pages[numPage], 2, "Введите пароль");

  if( UID >= 2 ){
      out += "<h1>Стартовые настройки</h1>\n";
      out += "<form action='";out += pages[numPage];out += "' method='PUT'>\n";
      HTTP_InputHidden(out,"FLAG_CONFIG4");
      HTTP_printConfig4(out);
      out += "</form>\n";
  }
  if( UID>= 0 )HTTP_printBottomMenu(out);
  HTTP_printTail(out);
  Serial.printf("!!! HTTP Config3 Length %d\n", out.length());  
  server.send(200, "text/html", out);
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
//  out += "<table width=100%>";
  HTTP_InputInt(out,"Яркость 0-10","Brightness",jsonConfig["RGB1"]["BRIGHTNESS"].as<int>(),0,10,32);

//  out += "</table>\n";
  out += "<table width=100%>";
  HTTP_print_color3(out, jsonConfig["RGB1"]["FREE"].as<uint32_t>(), "ColorFree", "Цвет в режиме \"Свободно\"", COLOR_FREE1, COLOR_FREE2);
//  out += "<tr><td colspan=4>";
//  HTTP_print_input_checkbox(out,"isFreeBlink","1",jsonConfig["RGB1"]["IS_FREE_BLINK"].as<bool>());
//  out += "</td></tr>\n";

  HTTP_print_color3(out, jsonConfig["RGB1"]["FREE_BLINK"].as<uint32_t>(), "ColorBlink", "Мигание в режиме \"Свободно\"", COLOR_BLINK1, COLOR_BLINK2,"isFreeBlink",jsonConfig["RGB1"]["IS_FREE_BLINK"].as<bool>());
  HTTP_print_color3(out, jsonConfig["RGB1"]["BUSY"].as<uint32_t>(), "ColorBusy", "Цвет в режиме \"Занято\"", COLOR_BUSY1, COLOR_BUSY2);

  out += "</table>";
  out += "<p><input type='submit' name='Save' value='Сохранить' class='btn'>"; 
  out += "</fieldset>\n";  


}




/**
* Парметры основных настроек
*/
void HTTP_printConfig(String &out){
  char s[32];
  out += "<fieldset>\n";
  out += "<legend>Конфигурация WI-FI</legend>\n";
  out += "<table>";
  out += "<tr><td align='center' width=50%><img src='/wifi1.png'></td><td width=50% align='center'><img src='/wifi2.png'></td></tr>";
  out += "<tr><td align='center'>";
  HTTP_print_input_radio(out,"WiFiMode","1",!jsonConfig["SYSTEM"]["AP_START"].as<bool>());
  out += "</td><td align='center'>";
  HTTP_print_input_radio(out,"WiFiMode","2",jsonConfig["SYSTEM"]["AP_START"].as<bool>());
  out += "</td></tr>";
  out += "<tr><td align='center' class='td1'>Раздает WI-FI до перезапуска. Первый светодиод бирюзовый. </label>\n</td><td align='center' class='td1'>Всегда раздает WiFi. Первый светодиод белый.</td></tr></table>";
  out += "<p><input type='submit' name='Save' value='Сохранить' class='btn'>"; 
  out += "</fieldset>\n";

// Блок №2.5 
  out += "<fieldset>\n";
  out += "<legend>Автокалибровка</legend>\n";
  out += "<p><input type='submit' name='Calibrate' value='Автоматическая калибровка расстояния' class='btn'>"; 

// Блок №3 
//  sprintf(s,"%d",EA_Config.TM_BEGIN_CALIBRATE);

  HTTP_InputInt(out,"Задержка начала калибровки (сек):","TMCalibr",jsonConfig["CALIBR"]["DELAY_START"].as<int>(),0,60);
//  sprintf(s,"%d",EA_Config.SAMPLES_CLIBRATE);
  HTTP_InputInt(out,"Количество тестовых замеров для калибровки:","NumCalibr",jsonConfig["CALIBR"]["NUMBER"].as<int>(),0,60);

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
  HTTP_print_img_radio(out,"/stat2.png","Если не видит расстояние - не переключается. (в этот момент мигает фиолетовым)","NoneMode","1",( jsonConfig["RGB1"]["NAN_MODE"].as<int>()  == NAN_VALUE_IGNORE ),false);
  HTTP_print_img_radio(out,"/stat3.png","Если не видит расстояние - переключается в &quot;занято&quot;","NoneMode","2",( jsonConfig["RGB1"]["NAN_MODE"].as<int>()  == NAN_VALUE_BUSY ),false);
  HTTP_print_img_radio(out,"/stat1.png","Если не видит расстояние - переключается в &quot;свободно&quot;","NoneMode","3",( jsonConfig["RGB1"]["NAN_MODE"].as<int>()  == NAN_VALUE_FREE ),false);

  sprintf(s,"%06lX",(uint32_t)COLOR_NAN);
  out += "<tr><td bgcolor='#"; out += s; out += "' height='50pt'>&nbsp;</td><td>";
  HTTP_print_input_checkbox(out,"isColorNan","1",jsonConfig["RGB1"]["IS_NAN_MODE"].as<bool>());
  out += "</td><td>Активация малиновой подсветки если датчик ничего не видит.";
  out += "</td></tr>";
  out += "</table>\n";
  
//  sprintf(s,"%d",jsonConfig["SENSOR"]["T_LOOP"].as<int>());
  HTTP_InputInt(out,"Задержка между циклами опроса сенсора (сек):","TMLoop",jsonConfig["SENSOR"]["T_LOOP"].as<int>(),1,30);

  out += "<p><input type='submit' name='Save' value='Сохранить' class='btn'>"; 
  out += "</fieldset>\n";

// Блок №6
  out += "<fieldset>\n";
  out += "<legend>Режимы определения препятствия</legend>\n"; 
  HTTP_print_img_radio(out,"/type1.png","Установка датчика на потолке","MeasureType","1",( jsonConfig["SENSOR"]["INSTALL"].as<int>()  == INSTALL_TYPE_NORMAL ),true);
//  sprintf(s,"%d",jsonConfig["SENSOR"]["DIST_GROUND"].as<int>());
  HTTP_InputInt(out,"*Расстояние от датчика до пола (мм):","GroundLevel",jsonConfig["SENSOR"]["DIST_GROUND"].as<int>(),100,10000);
//  sprintf(s,"%d",jsonConfig["SENSOR"]["DIST_LIMIT"].as<int>());
  HTTP_InputInt(out,"Минимальная высота на срабатывание (мм):","LimitDistance",jsonConfig["SENSOR"]["DIST_LIMIT"].as<int>(),100,10000);

  HTTP_print_img_radio(out,"/type2.png","Определяет как &quot;занято&quot; в заданном диапазоне:","MeasureType","2",( jsonConfig["SENSOR"]["INSTALL"].as<int>()  == INSTALL_TYPE_OUTSIDE ), true);
//  sprintf(s,"%d",jsonConfig["SENSOR"]["DIST_MIN1"].as<int>());
  HTTP_InputInt(out,"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀От (мм):","MinDistance1",jsonConfig["SENSOR"]["DIST_MIN1"].as<int>(),100,10000);
//  sprintf(s,"%d",jsonConfig["SENSOR"]["DIST_MAX1"].as<int>());
  HTTP_InputInt(out,"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀До (мм):","MaxDistance1",jsonConfig["SENSOR"]["DIST_MAX1"].as<int>(),100,10000);
 
  HTTP_print_img_radio(out,"/type3.png","Определяет как &quot;свободно&quot; в заданном диапазоне:","MeasureType","3",( jsonConfig["SENSOR"]["INSTALL"].as<int>()  == INSTALL_TYPE_INSIDE ), true);
//  sprintf(s,"%d",jsonConfig["SENSOR"]["DIST_MIN2"].as<int>());
  HTTP_InputInt(out,"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀От (мм):","MinDistance2",jsonConfig["SENSOR"]["DIST_MIN2"].as<int>(),100,10000);
//  sprintf(s,"%d",jsonConfig["SENSOR"]["DIST_MAX2"].as<int>());
  HTTP_InputInt(out,"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀До (мм):","MaxDistance2",jsonConfig["SENSOR"]["DIST_MAX2"].as<int>(),100,10000);

  out += "<p><input type='submit' name='Save' value='Сохранить' class='btn'>"; 
  out += " </fieldset>\n";


}

/**
* Настройка реле
**/
void HTTP_printConfigRelay(String &out){
  char s[32];
  out += "<fieldset>\n";
  out += "<legend>Режим работы реле №1</legend>\n";

  HTTP_InputInt(out,"Задержка переключения на &quot;занято&quot; (сек):","TMOn1",jsonConfig["RELAY1"]["DELAY_ON"].as<int>(),0,30);
  HTTP_InputInt(out,"Задержка переключения на &quot;свободно&quot; (сек):","TMOff1",jsonConfig["RELAY1"]["DELAY_OFF"].as<int>(),0,30);

  HTTP_print_img_radio(out,"/relay0.png","Не используется","ModeRelay1","0",( jsonConfig["RELAY1"]["MODE"].as<int>()  == RELAY_NONE ), true);
  HTTP_print_img_radio(out,"/relay1.png","Реле по умолчанию, постоянно ВКЛ или ВЫКЛ режима Занято-свободно","ModeRelay1","1",( jsonConfig["RELAY1"]["MODE"].as<int>()  == RELAY_NORMAL ), true);
  HTTP_print_img_radio(out,"/relay2.png","Управление кнопкой на открытия или закрытие ворот","ModeRelay1","2",( jsonConfig["RELAY1"]["MODE"].as<int>()  == RELAY_PULSE ), true);
  HTTP_print_img_radio(out,"/relay4.png","Импульсный режим","ModeRelay1","4",( jsonConfig["RELAY1"]["MODE"].as<int>()  == RELAY_PWM  ), true);
  out +="<br>";
  HTTP_print_input_checkbox(out,"isInverseRelay1","1",jsonConfig["RELAY1"]["INVERSE"].as<bool>());
  out += "<label><b>Инверсия занято/свободно</b></label>";
  out +="<br><br>";
  HTTP_print_img_radio(out,"/relay5.png","Управление кнопкой открытия-закрытия ворот","ModeRelay1","5",( jsonConfig["RELAY1"]["MODE"].as<int>() == RELAY_PULSE2 ), true);
  HTTP_InputInt(out,"           На сколько секунд замкнуть контакты:","TM_PulseRelay1",jsonConfig["RELAY1"]["T_PULSE"].as<int>(),1,30);
  HTTP_InputInt(out,"           На сколько секунд разамкнуть контакты:","TM_PauseRelay1",jsonConfig["RELAY1"]["T_PAUSE"].as<int>(),1,30);

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

  HTTP_InputInt(out,"Задержка переключения на &quot;занято&quot; (сек):","TMOn2",jsonConfig["RELAY2"]["DELAY_ON"].as<int>(),0,30);
  HTTP_InputInt(out,"Задержка переключения на &quot;свободно&quot; (сек):","TMOff2",jsonConfig["RELAY2"]["DELAY_OFF"].as<int>(),0,30);

  HTTP_print_img_radio(out,"/relay0.png","Не используется","ModeRelay2","0",( jsonConfig["RELAY2"]["MODE"].as<int>()  == RELAY_NONE ), true);
  HTTP_print_img_radio(out,"/relay1.png","Реле по умолчанию, постоянно ВКЛ или ВЫКЛ режима Занято-свободно","ModeRelay2","1",( jsonConfig["RELAY2"]["MODE"].as<int>()  == RELAY_NORMAL ), true);
  HTTP_print_img_radio(out,"/relay2.png","Управление кнопкой на открытия или закрытие ворот","ModeRelay2","2",( jsonConfig["RELAY2"]["MODE"].as<int>()  == RELAY_PULSE ), true);
  HTTP_print_img_radio(out,"/relay4.png","Импульсный режим","ModeRelay2","4",( jsonConfig["RELAY2"]["MODE"].as<int>()  == RELAY_PWM  ), true);
  out +="<br>";
  HTTP_print_input_checkbox(out,"isInverseRelay2","1",jsonConfig["RELAY2"]["INVERSE"].as<bool>());
  out += "<label><b>Инверсия занято/свободно</b></label>";
  out +="<br><br>";
  HTTP_print_img_radio(out,"/relay5.png","Управление кнопкой открытия-закрытия ворот","ModeRelay2","5",( jsonConfig["RELAY2"]["MODE"].as<int>() == RELAY_PULSE2 ), true);
  HTTP_InputInt(out,"           На сколько секунд замкнуть контакты:","TM_PulseRelay2",jsonConfig["RELAY2"]["T_PULSE"].as<int>(),1,30);
  HTTP_InputInt(out,"           На сколько секунд разамкнуть контакты:","TM_PauseRelay2",jsonConfig["RELAY2"]["T_PAUSE"].as<int>(),1,30);

  out += "<p><input type='submit' name='Save' value='Сохранить' class='btn'>"; 

  out += "</fieldset>\n";

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
  out += "<option value='";out += String(SENSOR_SR04T);out += "'";
  if( jsonConfig["SENSOR"]["TYPE"].as<int>() == SENSOR_SR04T)out += " selected";
  out += ">Двойной ультразвуковой сенсор (SR04T)</option>";  
  out += "<option value='";out += String(SENSOR_SR04TM2);out += "'";
  if( jsonConfig["SENSOR"]["TYPE"].as<int>() == SENSOR_SR04TM2)out += " selected";
  out += ">Одинарный ультразвуковой сенсор (SR04M2)</option>";  

  out += "<option value='";out += String(SENSOR_TFLUNA_I2C);out += "'";
  if( jsonConfig["SENSOR"]["TYPE"].as<int>() == SENSOR_TFLUNA_I2C)out += " selected";
  out += ">Лазерный сенсор (TF-Luna)</option>";  

  out += "<option value='";out += String(SENSOR_LD2413_UART);out += "'";
  if( jsonConfig["SENSOR"]["TYPE"].as<int>() == SENSOR_LD2413_UART)out += " selected";
  out += ">Радар LD-2413</option>";  
  out += "</select>\n";
  out += "</div>\n";
  out += "<p class='t1'>После смены типа сенсора нужно кратковременно передернуть питание.";

  out += "<p><input type='submit' name='Save' value='Сохранить' class='btn'>"; 
  out += " </fieldset>\n";

// Блок №7
  out += "<fieldset>\n";
  out += "<legend>Подключение к онлайн мониторингу CRM.MOSCOW</legend>\n";
  out += "<labelВвключить онлайн отправку данных</label>\n";
  
  HTTP_print_input_checkbox(out,"SEND_HTTP","send",jsonConfig["CRM_MOSCOW"]["ENABLE"].as<bool>());
  
  out += "<p class='t1'>Поставьте галочку. Введите все поля с ** и сохраните. После этого в первой вкладке выключите бесконечный режим раздачи WiFi активировав режим с бирюзовой иконкой.";
  out += "После сохраните и перезагрузите устройство нажав желтую кнопку внизу натроек.</p>";

  HTTP_printNetworks1(out,"WiFiName");
  HTTP_printInput1(out,"**Введите пароль от вашей WI-FI сети:","WiFiPassword",jsonConfig["WIFI"]["PASS"].as<const char *>(),20,32,HT_PASSWORD);
 
  HTTP_printInput1(out,"**Номер договора, логин личного кабинета:","Dogovor",jsonConfig["CRM_MOSCOW"]["DOGOVOR_ID"].as<const char *>(),20,16,HT_TEXT);
  HTTP_printInput1(out,"**Номер бокса:","Box",jsonConfig["CRM_MOSCOW"]["BOX_ID"].as<const char *>(),20,16,HT_TEXT);
  out += "<p class='t1'>Ниже идут дополнительные настройки. Посоветуйтесь с технической поддержкой прежде чем их менять.</p>";
  HTTP_printInput1(out,"Сервер:","Server",jsonConfig["CRM_MOSCOW"]["SERVER"].as<const char *>(),20,32,HT_TEXT);
  sprintf(s,"%d",jsonConfig["CRM_MOSCOW"]["PORT"].as<int>());
  HTTP_printInput1(out,"Порт:","Port",s,20,32,HT_NUMBER);
  sprintf(s,"%d",jsonConfig["CRM_MOSCOW"]["T_SEND"].as<int>());
  HTTP_printInput1(out,"Связь с сервером через, сек:","TM_HTTP_SEND",s,20,32,HT_NUMBER);
  sprintf(s,"%d",jsonConfig["CRM_MOSCOW"]["T_RETRY"].as<int>());
  HTTP_printInput1(out,"Повторная попытка отправки через, сек:","TM_HTTP_RETRY_ERROR",s,20,32,HT_NUMBER);

  out += "<p><input type='submit' name='Save' value='Сохранить' class='btn'>"; 
  out += "</fieldset>\n";  
// Блок №8
  out += "<fieldset>\n";
  out += "<legend>Параметры DHCP</legend>\n";
  out += "<label>Статический IP:</label>\n";
  HTTP_print_input_checkbox(out,"STATIC_IP","static",!jsonConfig["WIFI"]["DHCP"].as<bool>());
  
//  sprintf(s,"%d.%d.%d.%d",EA_Config.IP[0],EA_Config.IP[1],EA_Config.IP[2],EA_Config.IP[3]);
  HTTP_printInput1(out,"Адрес:","IPAddr",jsonConfig["WIFI"]["IP"]["ADDR"].as<const char *>(),16,32,HT_IP);
//  sprintf(s,"%d.%d.%d.%d",EA_Config.MASK[0],EA_Config.MASK[1],EA_Config.MASK[2],EA_Config.MASK[3]);
  HTTP_printInput1(out,"Маска:","IPMask",jsonConfig["WIFI"]["IP"]["MASK"].as<const char *>(),16,32,HT_IP);
//  sprintf(s,"%d.%d.%d.%d",EA_Config.GW[0],EA_Config.GW[1],EA_Config.GW[2],EA_Config.GW[3]);
  HTTP_printInput1(out,"Шлюз:","IPGate",jsonConfig["WIFI"]["IP"]["GW"].as<const char *>(),16,32,HT_IP);
//  sprintf(s,"%d.%d.%d.%d",EA_Config.DNS[0],EA_Config.DNS[1],EA_Config.DNS[2],EA_Config.DNS[3]);
  HTTP_printInput1(out,"DNS:",    "IPDns",jsonConfig["WIFI"]["IP"]["DNS"].as<const char *>(),16,32,HT_IP);
 
  out += "<p><input type='submit' name='Save' value='Сохранить' class='btn'>"; 
  out += "</fieldset>\n";  

  out += "<fieldset>\n";
  out += "<legend>Параметры доступа к контроллеру</legend>\n";

  HTTP_printInput1(out,"Пароль для входа в правами опреатора:","PasswordUser",jsonConfig["SYSTEM"]["PASS1"].as<const char *>(),20,32,HT_PASSWORD);
  if( UID >= 1 ){
     HTTP_printInput1(out,"Пароль для входа с правами администратора:","PasswordAdmin",jsonConfig["SYSTEM"]["PASS0"].as<const char *>(),20,32,HT_PASSWORD);
     if( UID >= 2 )HTTP_printInput1(out,"Пароль для входа с правами суперадминистратора:","PasswordSuperAdmin",jsonConfig["SYSTEM"]["PASSS"].as<const char *>(),20,32,HT_PASSWORD);

     HTTP_printInput1(out,"Наименование устройства","NameESP",jsonConfig["SYSTEM"]["NAME"].as<const char *>(),32,32,HT_TEXT,"lab1");
     out += "<p class='t1'>После изменения имени WiFi нажмите внизу желтую кнопку &quot;Перезагрузка&quot;</p>";
  }
  out += "<p><input type='submit' name='Save' value='Сохранить' class='btn'>"; 
  out += "</fieldset>\n";  
}

/**
* Настройка подсветки
**/
void HTTP_printConfig2(String &out){
  char s[50];
// Блок №1c
  out += "<fieldset>\n";
  out += "<legend>Настройка RGB2</legend>\n";
//  out += "<table width=100%>";
  HTTP_InputInt(out,"Яркость 0-10","Brightness2",jsonConfig["RGB2"]["BRIGHTNESS"].as<int>(),0,10,32);

//  out += "</table>\n";
  out += "<table width=100%>";
//  out += "<tr><td colspan=4>";
//  HTTP_print_input_checkbox(out,"isFreeBlink2","1",jsonConfig["RGB2"]["IS_FREE_BLINK"].as<bool>());
//  out += "</td></tr>\n";
  HTTP_print_color3(out, jsonConfig["RGB2"]["FREE"].as<uint32_t>(), "ColorFree2", "Цвет в режиме \"Свободно\"", COLOR_FREE1, COLOR_FREE2);
//  out += "<tr><td colspan=4>&nbsp;</td></tr>\n";
  HTTP_print_color3(out, jsonConfig["RGB2"]["FREE_BLINK"].as<uint32_t>(), "ColorBlink2", "Мигание в режиме \"Свободно\"", COLOR_BLINK1, COLOR_BLINK2,"isFreeBlink2",jsonConfig["RGB2"]["IS_FREE_BLINK"].as<bool>());
//  out += "<tr><td colspan=4>&nbsp;</td></tr>\n";
  HTTP_print_color3(out, jsonConfig["RGB2"]["BUSY"].as<uint32_t>(), "ColorBusy2", "Цвет в режиме \"Занято\"", COLOR_BUSY1, COLOR_BUSY2);
//  out += "<tr><td colspan=4>";
//  HTTP_print_input_checkbox(out,"isMP3","1",jsonConfig["RGB2"]["IS_MP3"].as<bool>());
//  out += "</td></tr>\n";
//  HTTP_print_color3(out, jsonConfig["RGB2"]["MP3"].as<uint32_t>(), "ColorMP3", "Мигание в режиме \"оповещение\"", COLOR_MP3_1, COLOR_MP3_2);
  out += "</table>";
  out += "<p><input type='submit' name='Save' value='Сохранить' class='btn'>"; 
  out += "</fieldset>\n";  

  out += "<fieldset>\n";
  out += "<legend>Настройка оповещения</legend>\n";
  HTTP_InputInt(out,"Громкость звука 0-30","MP3_VOLUME",jsonConfig["MP3"]["VOLUNE"].as<int>(),0,30,32);


  out += "<table border=\"1\" style=\"border-collapse: collapse; border: 1px solid black;\">\n";
//  out += "<table class='tab1'>\n";
  out += "<tr><td width='320'>Оповещение</td><td width='50'>Вкл.</td><td width='50'>Задержка</td><td width='50'>Повтор</td><td width='50'>Тест</td><td width='50'>Цвет</td><td width='50'>Длит.</td><tr>\n";

  HTTP_print_MP3_7(out,"Момент заезда автомобиля. Файл 01/001.mp3", "BUSY" );
  HTTP_print_MP3_7(out,"Датчик перестает видеть расстояние (Машина в пене). Файл 01/002.mp3", "NAN" );
  HTTP_print_MP3_7(out,"В боксе долго находится автмомбиль. Файл 01/003.mp3", "BUSY1" );
  HTTP_print_MP3_7(out,"Автомобиль слишком долго в доксе или датчик \"залип\". Файл 01/004.mp3", "BUSY2" );
  HTTP_print_MP3_7(out,"После выезда автомобиля датчик не видит расстояния. (Под датчиком на полу много пены либо ошибка калибровки). Файл 01/005.mp3", "FREE_NAN" );
  HTTP_print_MP3_7(out,"Выезд автомобиля. Бокс свободен. (Можно загрузить рекламу) Файл 01/006.mp3", "FREE" );
  out += "</table>\n";
  out += "<p><input type='submit' name='Save' value='Сохранить' class='btn'>"; 
  out += "</fieldset>\n";  

}

/**
* Настройка подсветки
**/
void HTTP_printConfig4(String &out){
int Dir = jsonConfig["MP3"]["ADD"]["DIR"].as<int>();

  out += "<fieldset>\n";
  out += "<legend>Первый запуск</legend>\n";
  out += "<table width=100%><tr>";
  out += "<td>Активировать первый запуск</td>";
  out += "<td align='right'><input type='submit' name='BOOT0' value='Активировать' class='btn'></td>"; 
  out += "</tr></table>\n";
  out += "<p class='t1'>";
  HTTP_print_input_checkbox(out,"MP3_99_ENABLE","1",jsonConfig["MP3"]["99"]["ENABLE"].as<bool>());
  out += "Включить звуковые оповещения для первого запуска";
//  out += "<table border=\"1\" style=\"border-collapse: collapse; border: 1px solid black;\">\n";
  out += "<table width=100%>\n";
  out += "<tr><td width='450'>&nbsp;</td><td width='50'>&nbsp;</td><td width='50'>&nbsp;</td><td width='50'>&nbsp;</td><tr>\n";
//  out += "<tr><td width='500'>Оповещение</td><td width='50'>Тест</td><td width='50'>Длит.</td><tr>\n";
  HTTP_print_MP3(out,"Первый запуск, приветствие. Дорожка 02/99.mp3",Dir,99,jsonConfig["MP3"]["99"]["COLOR_TM"].as<int>());
  out += "</table>\n";


  HTTP_print_input_checkbox(out,"MP3_100_ENABLE","1",jsonConfig["MP3"]["100"]["ENABLE"].as<bool>());
  out += "Включить звуковые оповещения для каждого запуска";
//  out += "<table border=\"1\" style=\"border-collapse: collapse; border: 1px solid black;\">\n";
  out += "<table width=100%>\n";
  out += "<tr><td width='450'>&nbsp;</td><td width='50'>&nbsp;</td><td width='50'>&nbsp;</td><td width='50'>&nbsp;</td><tr>\n";

  HTTP_print_MP3(out,"Запуск датчика, приветствие. Дорожка 02/100.mp3",Dir,100);
  out += "</table>\n";


  out += "<p><input type='submit' name='Save' value='Сохранить' class='btn'>"; 
  out += "</fieldset>\n";  

  out += "<fieldset>\n";
  out += "<legend>Калибровка (Полнесение магнита на 3-5 сек)</legend>\n";
  out += "<p class='t1'>";
  HTTP_print_input_checkbox(out,"MP3_97_ENABLE","1",jsonConfig["MP3"]["97"]["ENABLE"].as<bool>());
  out += "Включить звуковые оповещения для калибровки";
  out += "<table width=100%>\n";
  out += "<tr><td width='450'>&nbsp;</td><td width='50'>&nbsp;</td><td width='50'>&nbsp;</td><td width='50'>&nbsp;</td><tr>\n";
  HTTP_print_MP3(out,"Начало калибровки. Дорожка 02/97.mp3",Dir,97);
  HTTP_print_MP3(out,"Рано убрал магнит. Дорожка 02/96.mp3",Dir,96);
  HTTP_print_MP3(out,"Датчик не видит расстояние. Дорожка 02/95.mp3",Dir,95);
  HTTP_print_MP3(out,"Датчик плохо видит расстояние. Дорожка 02/94.mp3",Dir,94);
  HTTP_print_MP3(out,"Датчик успешно откалибровался. Дорожка 02/93.mp3",Dir,93);
  out += "</table>\n";
  out += "<p><input type='submit' name='Save' value='Сохранить' class='btn'>"; 
  out += "</fieldset>\n";  

  out += "<fieldset>\n";
  out += "<legend>Сброс паролей (Удержание магнита свыше 10 сек)</legend>\n";
  out += "<p class='t1'>";
  HTTP_print_input_checkbox(out,"MP3_98_ENABLE","1",jsonConfig["MP3"]["98"]["ENABLE"].as<bool>());
  out += "Включить звуковые оповещения для сброса паролей";
  out += "<table width=100%>\n";
  out += "<tr><td width='450'>&nbsp;</td><td width='50'>&nbsp;</td><td width='50'>&nbsp;</td><td width='50'>&nbsp;</td><tr>\n";
  HTTP_print_MP3(out,"Сброс паролей. Дорожка 02/98.mp3",Dir,98);
  out += "</table>\n";
  out += "<p><input type='submit' name='Save' value='Сохранить' class='btn'>"; 
  out += "</fieldset>\n";  

  out += "<fieldset>\n";
  out += "<legend>Сброс имени WiFi (Поднесение магнита 5 раз подряд)</legend>\n";
  out += "<p class='t1'>";
  HTTP_print_input_checkbox(out,"MP3_92_ENABLE","1",jsonConfig["MP3"]["92"]["ENABLE"].as<bool>());
  out += "Включить звуковые оповещения для сброса имени WiFi";
  out += "<table width=100%>\n";
  out += "<tr><td width='450'>&nbsp;</td><td width='50'>&nbsp;</td><td width='50'>&nbsp;</td><td width='50'>&nbsp;</td><tr>\n";
  HTTP_print_MP3(out,"Сброс имени WiFi. Дорожка 02/92.mp3",Dir,92);
  out += "</table>\n";
  out += "<p><input type='submit' name='Save' value='Сохранить' class='btn'>"; 
  out += "</fieldset>\n";  

  out += "<fieldset>\n";
  out += "<legend>Оповещения в WEB интерфейсе</legend>\n";
  out += "<p class='t1'>";
  HTTP_print_input_checkbox(out,"MP3_89_ENABLE","1",jsonConfig["MP3"]["89"]["ENABLE"].as<bool>());
  out += "Включить звуковые оповещения для WEB интерфейса";
  out += "<table width=100%>\n";
  out += "<tr><td width='450'>&nbsp;</td><td width='50'>&nbsp;</td><td width='50'>&nbsp;</td><td width='50'>&nbsp;</td><tr>\n";
  HTTP_print_MP3(out,"Сброс до заводских настроек. Дорожка 02/91.mp3",Dir,91);
  HTTP_print_MP3(out,"Обновление прошивки. Дорожка 02/90.mp3",Dir,90);
  HTTP_print_MP3(out,"Нажатие на кнопку \"Обновление прошивки\". Дорожка 02/89.mp3",Dir,89);
  HTTP_print_MP3(out,"Успешное обновление прошивки. Дорожка 02/88.mp3",Dir,88);
  HTTP_print_MP3(out,"Ошибка обновления прошивки. Дорожка 02/87.mp3",Dir,87);
  HTTP_print_MP3(out,"Нажатие на кнопку \"Перезагрузки\". Дорожка 02/86.mp3",Dir,86);
  HTTP_print_MP3(out,"Нажатие на кнопку \"Автокалибровки\". Дорожка 02/85.mp3",Dir,85);
  HTTP_print_MP3(out,"Нажатие на кнопку \"Обновить расстояние\". Дорожка 02/84.mp3",Dir,84);
  HTTP_print_MP3(out,"Нажатие на кнопку \"Сохранить\". Дорожка 02/83.mp3",Dir,83);
  out += "</table>\n";
  out += "<p><input type='submit' name='Save' value='Сохранить' class='btn'>"; 
  out += "</fieldset>\n";  

}


void HTTP_print_MP3_7(String &out, char *text, char *name){
   char s[32];
   out += "<tr><td>";
   out += text;
   out += "</td><td>";
   sprintf(s,"MP3_%s_ENABLE",name);
   HTTP_print_input_checkbox(out,s,"1",jsonConfig["MP3"][name]["ENABLE"].as<bool>());
   out += "</td><td>";
   sprintf(s,"MP3_%s_DELAY",name);
   HTTP_InputInt1(out,s,jsonConfig["MP3"][name]["DELAY"].as<int>(),1,3600);
   out += "</td><td>";
   sprintf(s,"MP3_%s_LOOP",name);
   HTTP_print_input_checkbox(out,s,"1",jsonConfig["MP3"][name]["LOOP"].as<bool>());
   out += "</td><td>";
   sprintf(s,"MP3_%s_PLAY",name);
//   out += "<input type='submit' name='";out += s; out += "' value='▶' class='btn'>";
   out += "<input type='button' value='▶' class='btn' onClick='playMP3(";
   out += jsonConfig["MP3"][name]["DIR"].as<int>();
   out += ",";
   out += jsonConfig["MP3"][name]["NUM"].as<int>();
   out += ");'>";
//   out += "<input type='submit' name='MP3_PLAY' value='▶' class='btn'>";
//   HTTP_InputHidden(out,"MP3_DIR",(char *)jsonConfig["MP3"][name]["DIR"].as<const char *>());
//   HTTP_InputHidden(out,"MP3_NUM",(char *)jsonConfig["MP3"][name]["NUM"].as<const char *>());
   out += "</td><td>";
   sprintf(s,"MP3_%s_COLOR",name);
   HTTP_print_color(out, jsonConfig["MP3"][name]["COLOR"].as<uint32_t>(), s);
   out += "</td><td>";
   sprintf(s,"MP3_%s_COLOR_TM",name);
   HTTP_InputInt1(out,s,jsonConfig["MP3"][name]["COLOR_TM"].as<int>(),1,100);
   out += "</td></tr>\n";
    
}

void HTTP_print_MP3(String &out, char *text,int dir, int num, int _delay){
   char s[32];
   out += "<tr>";
   if( _delay >= 0 ){
      out += "<td>";
      out += text;
      out += "</td><td>";
      out += "Длит.";
      out += "</td><td>";
      sprintf(s,"MP3_%d_COLOR_TM",num);
      HTTP_InputInt1(out,s,_delay,1,100);
   }
   else {
      out += "<td colspan=3>";
      out += text;
   }
   out += "</td><td align='right'>"; 
   sprintf(s,"%02d",num);
//   out += "<input type='submit' name='MP3_"; out += s; out += "_PLAY' value='▶' class='btn' onClick='playMP3(";
   out += "<input type='button' value='▶' class='btn' onClick='playMP3(";
   out += dir;
   out += ",";
   out += num;
   out += ");'>";
//   sprintf(s,"%02d",dir);
//   HTTP_InputHidden(out,"MP3_DIR",s);
//   sprintf(s,"%02d",num);
//   HTTP_InputHidden(out,"MP3_NUM",s);
   out += "</td></tr>\n";
}


bool HTTP_checkArgs(int current){
   if( UID < 0 )return false;
   bool _save = false;
   bool _reboot = false;
// Если нажата кнопка "Калибровка"   
   if ( server.hasArg("Calibrate")  ){  
       checkPlayMP3("89",85);
       startCalibrate(1000);
   }
   else if( server.hasArg("Default") ){ 
       checkPlayMP3("89",91);

       String ss = jsonConfig["SYSTEM"]["NAME"].as<String>();
       configDefault();
       jsonConfig["SYSTEM"]["NAME"] = ss;
       configSave();
       configRead();
//       HTTP_printMessage("Загрузка заводских параметров. Перезагрузка ...");
       HTTP_goto("/", 2000, "Загрузка заводских параметров. Перезагрузка ..."); //1.12.24
       vTaskDelay(5000);
       ESP.restart();  
       return true;
   }
   else if( server.hasArg("Reboot") ){ 
       checkPlayMP3("89",86);
       HTTP_goto("/", 20000, "Перезагрузка ...");
       vTaskDelay(5000);
//       HTTP_printMessage("Перезагрузка ...");
       ESP.restart();  
       return true;
   }
//   else if( server.hasArg( "MP3_PLAY" ) ){
//       playMP3(server.arg("MP3_DIR").toInt(), server.arg("MP3_NUM").toInt());
//   }
   else if( server.hasArg("BOOT0") ){ 
       jsonSave["BOOT_COUNT"]  = 0;
       saveSave();
       HTTP_goto("/conf4", 5000, "Активирован первый запуск датчика. Сброшен счетчик загрузок");
       vTaskDelay(5000);
       return true;
   }

   
   else if( server.hasArg( "MP3_BUSY_PLAY"     ) ){ playMP3(jsonConfig["MP3"]["BUSY"]["DIR"].as<int>(), jsonConfig["MP3"]["BUSY"]["NUM"].as<int>());         }
   else if( server.hasArg( "MP3_NAN_PLAY"      ) ){ playMP3(jsonConfig["MP3"]["NAN"]["DIR"].as<int>(), jsonConfig["MP3"]["NAN"]["NUM"].as<int>());           }
   else if( server.hasArg( "MP3_BUSY1_PLAY"    ) ){ playMP3(jsonConfig["MP3"]["BUSY1"]["DIR"].as<int>(), jsonConfig["MP3"]["BUSY1"]["NUM"].as<int>());       }
   else if( server.hasArg( "MP3_BUSY2_PLAY"    ) ){ playMP3(jsonConfig["MP3"]["BUSY2"]["DIR"].as<int>(), jsonConfig["MP3"]["BUSY2"]["NUM"].as<int>());       }
   else if( server.hasArg( "MP3_FREE_NAN_PLAY" ) ){ playMP3(jsonConfig["MP3"]["FREE_NAN"]["DIR"].as<int>(), jsonConfig["MP3"]["FREE_NAN"]["NUM"].as<int>()); }
   else if( server.hasArg( "MP3_FREE_PLAY"     ) ){ playMP3(jsonConfig["MP3"]["FREE"]["DIR"].as<int>(), jsonConfig["MP3"]["FREE"]["NUM"].as<int>());         }
   else if( server.hasArg( "MP3_99_PLAY"       ) ){ playMP3(jsonConfig["MP3"]["ADD"]["DIR"].as<int>(), 99);     }
   else if( server.hasArg( "MP3_98_PLAY"       ) ){ playMP3(jsonConfig["MP3"]["ADD"]["DIR"].as<int>(), 98);     }
   else if( server.hasArg( "MP3_97_PLAY"       ) ){ playMP3(jsonConfig["MP3"]["ADD"]["DIR"].as<int>(), 97);     }
   else if( server.hasArg( "MP3_96_PLAY"       ) ){ playMP3(jsonConfig["MP3"]["ADD"]["DIR"].as<int>(), 96);     }
   else if( server.hasArg( "MP3_95_PLAY"       ) ){ playMP3(jsonConfig["MP3"]["ADD"]["DIR"].as<int>(), 95);     }
   else if( server.hasArg( "MP3_94_PLAY"       ) ){ playMP3(jsonConfig["MP3"]["ADD"]["DIR"].as<int>(), 94);     }
   else if( server.hasArg( "MP3_93_PLAY"       ) ){ playMP3(jsonConfig["MP3"]["ADD"]["DIR"].as<int>(), 93);     }
   else if( server.hasArg( "MP3_92_PLAY"       ) ){ playMP3(jsonConfig["MP3"]["ADD"]["DIR"].as<int>(), 92);     }
   else if( server.hasArg( "MP3_91_PLAY"       ) ){ playMP3(jsonConfig["MP3"]["ADD"]["DIR"].as<int>(), 91);     }
   else if( server.hasArg( "MP3_90_PLAY"       ) ){ playMP3(jsonConfig["MP3"]["ADD"]["DIR"].as<int>(), 90);     }
   else if( server.hasArg( "MP3_89_PLAY"       ) ){ playMP3(jsonConfig["MP3"]["ADD"]["DIR"].as<int>(), 89);     }
   else if( server.hasArg( "MP3_88_PLAY"       ) ){ playMP3(jsonConfig["MP3"]["ADD"]["DIR"].as<int>(), 88);     }
   else if( server.hasArg( "MP3_87_PLAY"       ) ){ playMP3(jsonConfig["MP3"]["ADD"]["DIR"].as<int>(), 87);     }
   else if( server.hasArg( "MP3_86_PLAY"       ) ){ playMP3(jsonConfig["MP3"]["ADD"]["DIR"].as<int>(), 86);     }
   else if( server.hasArg( "MP3_85_PLAY"       ) ){ playMP3(jsonConfig["MP3"]["ADD"]["DIR"].as<int>(), 85);     }
   else if( server.hasArg( "MP3_84_PLAY"       ) ){ playMP3(jsonConfig["MP3"]["ADD"]["DIR"].as<int>(), 84);     }
   else if( server.hasArg( "MP3_83_PLAY"       ) ){ playMP3(jsonConfig["MP3"]["ADD"]["DIR"].as<int>(), 83);     }

// Если нажата кнопка "Сохранить"   
   else if ( server.hasArg("Save") && UID >= 0){
      checkPlayMP3("89",83);
// RGB1
      if(server.hasArg("ColorFree")   )jsonConfig["RGB1"]["FREE"] = HTMLtoInt(server.arg("ColorFree").c_str());
      if(server.hasArg("ColorBusy")   )jsonConfig["RGB1"]["BUSY"] = HTMLtoInt(server.arg("ColorBusy").c_str());
      if(server.hasArg("ColorBlink")  )jsonConfig["RGB1"]["FREE_BLINK"] = HTMLtoInt(server.arg("ColorBlink").c_str());

      if(server.hasArg("FLAG_ROOT")  ){
         jsonConfig["RGB1"]["IS_FREE_BLINK"] = false;   
         jsonConfig["RGB1"]["IS_NAN_MODE"] = false;   
      }
      if( server.hasArg("isFreeBlink"))jsonConfig["RGB1"]["IS_FREE_BLINK"] = true;
      if( server.hasArg("isColorNan") )jsonConfig["RGB1"]["IS_NAN_MODE"] = true;
      if(server.hasArg("Brightness")  )jsonConfig["RGB1"]["BRIGHTNESS"] = server.arg("Brightness").toInt();

/// RGB2
      if(server.hasArg("FLAG_CONFIG3")  ){
         jsonConfig["RGB2"]["IS_FREE_BLINK"] = false;   
//         jsonConfig["RGB2"]["IS_NAN_MODE"] = false;   
      }
      if(server.hasArg("ColorFree2")  )jsonConfig["RGB2"]["FREE"] = HTMLtoInt(server.arg("ColorFree2").c_str());
      if(server.hasArg("ColorBusy2")  )jsonConfig["RGB2"]["BUSY"] = HTMLtoInt(server.arg("ColorBusy2").c_str());
      if(server.hasArg("ColorBlink2") )jsonConfig["RGB2"]["FREE_BLINK"] = HTMLtoInt(server.arg("ColorBlink2").c_str());
//      if(server.hasArg("ColorMP3")    )jsonConfig["RGB2"]["MP3"] = HTMLtoInt(server.arg("ColorMP3").c_str());
      if( server.hasArg("isFreeBlink2"))jsonConfig["RGB2"]["IS_FREE_BLINK"] = true;
//      if( server.hasArg("isColorNan2") )jsonConfig["RGB2"]["IS_NAN_MODE"] = true;
      if(server.hasArg("Brightness2")  )jsonConfig["RGB2"]["BRIGHTNESS"] = server.arg("Brightness2").toInt();

// MP3
      if(server.hasArg("FLAG_CONFIG3")  ){
         HTTP_checkArgsMP3("BUSY");
         HTTP_checkArgsMP3("NAN");
         HTTP_checkArgsMP3("BUSY1");
         HTTP_checkArgsMP3("BUSY2");
         HTTP_checkArgsMP3("FREE_NAN");
         HTTP_checkArgsMP3("FREE");
      }

/// NET
      if(server.hasArg("FLAG_CONFIG2")  ){
         jsonConfig["WIFI"]["DHCP"] = true;
         jsonConfig["CRM_MOSCOW"]["ENABLE"] = false;
      }
      if(server.hasArg("GroundLevel"))jsonConfig["SENSOR"]["DIST_GROUND"] = server.arg("GroundLevel").toInt();
      if(server.hasArg("WiFiMode"))
         switch(server.arg("WiFiMode").toInt()){
             case 1: jsonConfig["SYSTEM"]["AP_START"] = false; break;
             case 2: jsonConfig["SYSTEM"]["AP_START"] = true; break;
         }
      if(server.hasArg("NoneMode"))
         switch(server.arg("NoneMode").toInt()){
             case 1: jsonConfig["RGB1"]["NAN_MODE"]  = NAN_VALUE_IGNORE; break;
             case 2: jsonConfig["RGB1"]["NAN_MODE"]  = NAN_VALUE_BUSY; break;
             case 3: jsonConfig["RGB1"]["NAN_MODE"]  = NAN_VALUE_FREE; break;
         }
      if(server.hasArg("MeasureType"))
         switch(server.arg("MeasureType").toInt()){
             case 1: jsonConfig["SENSOR"]["INSTALL"]  = INSTALL_TYPE_NORMAL; break;
             case 2: jsonConfig["SENSOR"]["INSTALL"]  = INSTALL_TYPE_OUTSIDE; break;
             case 3: jsonConfig["SENSOR"]["INSTALL"]  = INSTALL_TYPE_INSIDE; break;
         }
      if(server.hasArg("SensorType")){jsonConfig["SENSOR"]["TYPE"]      = server.arg("SensorType").toInt();_reboot = true;}
      

      if(server.hasArg("LimitDistance") )jsonConfig["SENSOR"]["DIST_LIMIT"]     = server.arg("LimitDistance").toInt();
      if(server.hasArg("MinDistance1")  )jsonConfig["SENSOR"]["DIST_MIN1"]      = server.arg("MinDistance1").toInt();
      if(server.hasArg("MaxDistance1")  )jsonConfig["SENSOR"]["DIST_MAX1"]      = server.arg("MaxDistance1").toInt();
      if(server.hasArg("MinDistance2")  )jsonConfig["SENSOR"]["DIST_MIN2"]      = server.arg("MinDistance2").toInt();
      if(server.hasArg("MaxDistance2")  )jsonConfig["SENSOR"]["DIST_MAX2"]      = server.arg("MaxDistance2").toInt();
      if(server.hasArg("TMLoop")        )jsonConfig["SENSOR"]["T_LOOP"]         = server.arg("TMLoop").toInt();
      if(server.hasArg("TMCalibr")      )jsonConfig["CALIBR"]["DELAY_START"]    = server.arg("TMCalibr").toInt();
      if(server.hasArg("NumCalibr")     )jsonConfig["CALIBR"]["NUMBER"]         = server.arg("NumCalibr").toInt();
      if(server.hasArg("PasswordAdmin") && UID >= 1 )jsonConfig["SYSTEM"]["PASS0"] = server.arg("PasswordAdmin").c_str();
      if(server.hasArg("PasswordSuperAdmin") && UID >= 2 )jsonConfig["SYSTEM"]["PASSS"] = server.arg("PasswordSuperAdmin").c_str();
      if(server.hasArg("PasswordOper") )jsonConfig["SYSTEM"]["PASS1"]           = server.arg("PasswordOper").c_str();
      if(server.hasArg("NameESP")      && UID >= 1 )jsonConfig["SYSTEM"]["NAME"]   = server.arg("NameESP").c_str();
      if(server.hasArg("WiFiName")     )jsonConfig["WIFI"]["NAME"]              = server.arg("WiFiName").c_str();
      if(server.hasArg("WiFiPassword") )jsonConfig["WIFI"]["PASS"]              = server.arg("WiFiPassword").c_str();
      if( server.hasArg("SEND_HTTP"))jsonConfig["CRM_MOSCOW"]["ENABLE"]         = true;

      if(server.hasArg("Dogovor")      )jsonConfig["CRM_MOSCOW"]["DOGOVOR_ID"]  = server.arg("Dogovor").c_str();
      if(server.hasArg("Box")          )jsonConfig["CRM_MOSCOW"]["BOX_ID"]      = server.arg("Box").c_str();
      if(server.hasArg("Server")       )jsonConfig["CRM_MOSCOW"]["SERVER"]      = server.arg("Server").c_str();
      if(server.hasArg("Port")         )jsonConfig["CRM_MOSCOW"]["PORT"]        = server.arg("Port").toInt();
      if(server.hasArg("TM_HTTP_SEND") )jsonConfig["CRM_MOSCOW"]["T_SEND"]      = server.arg("TM_HTTP_SEND").toInt();
      if(server.hasArg("TM_HTTP_RETRY_ERROR"))jsonConfig["CRM_MOSCOW"]["T_RETRY"] = server.arg("TM_HTTP_RETRY_ERROR").toInt();



      if( server.hasArg("STATIC_IP"))jsonConfig["WIFI"]["DHCP"] = false;
//      if( server.hasArg("SEND_HTTP"))jsonConfig["CRM_MOSCOW"]["ENABLE"] = true;
      

//      if(server.hasArg("StaticIP"))
//         switch(server.arg("StaticIP").toInt()){
//             case 0: EA_Config.isDHCP  = false; break;
//             case 1: EA_Config.isDHCP  = true;  break;
//         }
      if(server.hasArg("IPAddr"))jsonConfig["WIFI"]["IP"]["ADDR"]           = server.arg("IPAddr").c_str();
      if(server.hasArg("IPMask"))jsonConfig["WIFI"]["IP"]["MASK"]           = server.arg("IPMask").c_str();
      if(server.hasArg("IPGate"))jsonConfig["WIFI"]["IP"]["GW"]             = server.arg("IPGate").c_str();
      if(server.hasArg("IPDns") )jsonConfig["WIFI"]["IP"]["DNS"]            = server.arg("IPDns").c_str();


      if(server.hasArg("FLAG_CONFIG1")  ){
         jsonConfig["RELAY1"]["INVERSE"] = false;
         jsonConfig["RELAY2"]["INVERSE"] = false;
      }
      if(server.hasArg("TMOn1")         )jsonConfig["RELAY1"]["DELAY_ON"]   = server.arg("TMOn1").toInt();
      if(server.hasArg("TMOff1")        )jsonConfig["RELAY1"]["DELAY_OFF"]  = server.arg("TMOff1").toInt();
      if(server.hasArg("ModeRelay1")    )jsonConfig["RELAY1"]["MODE"]       = (T_RELAY_MODE)server.arg("ModeRelay1").toInt();
      if(server.hasArg("TM_PulseRelay1"))jsonConfig["RELAY1"]["T_PULSE"]    = server.arg("TM_PulseRelay1").toInt();   
      if(server.hasArg("TM_PauseRelay1"))jsonConfig["RELAY1"]["T_PAUSE"]    = server.arg("TM_PauseRelay1").toInt();    
      if(server.hasArg("isInverseRelay1"))jsonConfig["RELAY1"]["INVERSE"]   = true;

      if(server.hasArg("TMOn2")         )jsonConfig["RELAY2"]["DELAY_ON"]   = server.arg("TMOn2").toInt();
      if(server.hasArg("TMOff2")        )jsonConfig["RELAY2"]["DELAY_OFF"]  = server.arg("TMOff2").toInt();
      if(server.hasArg("ModeRelay2")    )jsonConfig["RELAY2"]["MODE"]       = (T_RELAY_MODE)server.arg("ModeRelay2").toInt();
      if(server.hasArg("TM_PulseRelay2"))jsonConfig["RELAY2"]["T_PULSE"]    = server.arg("TM_PulseRelay2").toInt();   
      if(server.hasArg("TM_PauseRelay2"))jsonConfig["RELAY2"]["T_PAUSE"]    = server.arg("TM_PauseRelay2").toInt();   
      if(server.hasArg("isInverseRelay2"))jsonConfig["RELAY2"]["INVERSE"]   = true;


      if(server.hasArg("FLAG_CONFIG4")  ){
         if( server.hasArg("MP3_99_COLOR_TM"))jsonConfig["MP3"]["99"]["COLOR_TM"] = server.arg("MP3_99_COLOR_TM").toInt();
         if(server.hasArg("MP3_100_ENABLE"))jsonConfig["MP3"]["100"]["ENABLE"] = true;
         else jsonConfig["MP3"]["100"]["ENABLE"] = false;
         if(server.hasArg("MP3_99_ENABLE"))jsonConfig["MP3"]["99"]["ENABLE"] = true;
         else jsonConfig["MP3"]["99"]["ENABLE"] = false;
         if(server.hasArg("MP3_98_ENABLE"))jsonConfig["MP3"]["98"]["ENABLE"] = true;
         else jsonConfig["MP3"]["98"]["ENABLE"] = false;
         if(server.hasArg("MP3_97_ENABLE"))jsonConfig["MP3"]["97"]["ENABLE"] = true;
         else jsonConfig["MP3"]["97"]["ENABLE"] = false;
         if(server.hasArg("MP3_92_ENABLE"))jsonConfig["MP3"]["92"]["ENABLE"] = true;
         else jsonConfig["MP3"]["92"]["ENABLE"] = false;
         if(server.hasArg("MP3_89_ENABLE"))jsonConfig["MP3"]["89"]["ENABLE"] = true;
         else jsonConfig["MP3"]["89"]["ENABLE"] = false;
     }

   
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
      configSave(); 
//      configRead();      
      SaveRGB1->Save(4,ET_NORMAL,0,0,COLOR_SAVE, COLOR_NONE);
      SaveRGB2->Save(4,ET_NORMAL,0,0,COLOR_SAVE, COLOR_NONE);

//      ledSetColor(COLOR_SAVE,true);
//      HTTP_printMessage("Сохранение параметров ...");

      HTTP_goto(pages[current], 1000, "Сохранение параметров ...");
//      if( EA_Config.isWiFiAlways || isWiFiAlways1)ledSetWiFiMode(LED_WIFI_AP1);
//      else ledSetWiFiMode(LED_WIFI_AP);
//      ledSetBaseMode(LED_SAVE,true);

      vTaskDelay(500);
      SaveRGB1->Restore(4);
      SaveRGB2->Restore(4);
//      ledRestoreColor();
      return true;
   }
   return false;
}


void HTTP_handlePlayMP3(void){
  int dir =  server.arg("DIR").toInt();
  int num =  server.arg("NUM").toInt();
  Serial.printf("!!! MP3 %d %d ok\n",dir,num); 
//  if( HTTP_redirect() )return;
  playMP3(dir, num);         

}


void HTTP_checkArgsMP3(char *name){
   char s[32];
   sprintf(s,"MP3_%s_DELAY",name);
   if( server.hasArg(s) )jsonConfig["MP3"][name]["DELAY"] = server.arg(s).toInt();
   sprintf(s,"MP3_%s_ENABLE",name);
   if( server.hasArg(s) )jsonConfig["MP3"][name]["ENABLE"] = true;
   else jsonConfig["MP3"][name]["ENABLE"] = false;
   sprintf(s,"MP3_%s_LOOP",name);
   if( server.hasArg(s) )jsonConfig["MP3"][name]["LOOP"] = true;
   else jsonConfig["MP3"][name]["LOOP"] = false;
   sprintf(s,"MP3_%s_COLOR",name);
   if( server.hasArg(s) )jsonConfig["MP3"][name]["COLOR"] = HTMLtoInt(server.arg(s).c_str());
   sprintf(s,"MP3_%s_COLOR_TM",name);
   if( server.hasArg(s))jsonConfig["MP3"][name]["COLOR_TM"] = server.arg(s).toInt();   
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

   else if( jsonConfig["SYSTEM"]["PASSS"].as<String>() == pass ){
       UID = 2;
       HTTP_User = "Права для настроек первого запуска";
   }
   else if( jsonConfig["SYSTEM"]["PASS0"].as<String>() == pass ){
       UID = 1;
       HTTP_User = "Права суперадминистратора";
   }
   else if( jsonConfig["SYSTEM"]["PASS1"].as<String>() == pass ){
       UID = 0;
       HTTP_User = "Права администратора";
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
   out += "</label><input name='";
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



void HTTP_InputInt(String &out,
   const char *label, // Метка поля
   const char *name,  // Имя поля HTTP 
   int value,      // Текущее значение
   int min,        // Минимальное значение
   int max,        // Максимальное значение
   int size,          // Дина поля
   const char *style) //Стиль поля
   {
   char str[10];
   if( style == NULL )out += "<p><label>";
   else {
       out += "  <div class=\"";
       out += style;
       out += "\"><label>";
   }
   out += label;
   out += "</label><input name='";
   out += name;
   out += "' type='number'";
   out += " value=";
   out += String(value);
//   out += " size=";
//   out += String(size);
//   out += " maxlength=32";
   out += " min=";
   out += String(min);
   out += " max=";
   out += String(max);

   out += ">";
   if( style == NULL )out += "</p>\n";
   else out += "</div>\n";
     
}

void HTTP_InputRange(String &out,
   const char *label, // Метка поля
   const char *name,  // Имя поля HTTP 
   int value,      // Текущее значение
   int min,        // Минимальное значение
   int max,        // Максимальное значение
   int size,          // Дина поля
   const char *style) //Стиль поля
   {
   char str[10];
   if( style == NULL )out += "<p><label>";
   else {
       out += "  <div class=\"";
       out += style;
       out += "\"><label>";
   }
   out += label;
   out += "</label><input name='";
   out += name;
   out += "' type='range'";
   out += " value=";
   out += String(value);
//   out += " size=";
//   out += String(size);
//   out += " maxlength=32";
   out += " min=";
   out += String(min);
   out += " max=";
   out += String(max);

   out += ">";
   if( style == NULL )out += "</p>\n";
   else out += "</div>\n";
     
}

void HTTP_InputInt1(String &out,
   const char *name,  // Имя поля HTTP 
   int value,      // Текущее значение
   int min,        // Минимальное значение
   int max,        // Максимальное значение
   int size)          // Дина поля
   {
   char str[10];
   out += "<input name='";
   out += name;
   out += "' type='number'";
   out += " value=";
   out += String(value);
//   out += " size=";
//   out += String(size);
//   out += " maxlength=32";
   out += " min=";
   out += String(min);
   out += " max=";
   out += String(max);

   out += " style=\"width: 3em\">";
}

void HTTP_InputHidden(String &out, char *name, char *value){
   out += "<input type=\"hidden\" name=\"";
   out += name;
   out += "\" value=\"";
   out += value;
   out += "\">\n";
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
      HTTP_printInput1(out,"**Введите имя вашей WI-FI сети:",name,jsonConfig["WIFI"]["NAME"].as<const char *>(),16,32,HT_TEXT);
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
         if( n_ssid[i] == jsonConfig["WIFI"]["NAME"].as<const char *>() )out+=" selected";
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
   Serial.printf("!!! HTTP GOTO %s\n",msg);
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

void HTTP_print_color3(String &out, uint32_t color, char *name, char *label, uint32_t color1, uint32_t color2, char *name1, bool check){
   char s[64];
   out += "<tr><td colspan=5>&nbsp;</td></tr>\n";

   out += "<tr>\n";
   if( name1 == NULL ){
      out += "<td colspan=2>";
      out += label, 
      out += "</td>";
   }
   else {
      out += "<td>";
      HTTP_print_input_checkbox(out,name1,"1",check);
      out += "</td>";
      out += "<td>";
      out += label, 
      out += "</td>";
   }
   sprintf(s,"%06lX",color1);
   out += "<td><button style=\"background-color: ";
   out += s;
   out += ";\" onclick=\"setColorVal('";
   out += name;
   out += "','#";
   out += s;
   out += "')\">&nbsp;&nbsp;&nbsp;</button></td>\n";
   sprintf(s,"%06lX",color2);
   out += "<td><button style=\"background-color: ";
   out += s;
   out += ";\" onclick=\"setColorVal('";
   out += name;
   out += "','#";
   out += s;
   out += "')\">&nbsp;&nbsp;&nbsp;</button></td>\n";
   sprintf(s,"%06lX",color);
   out += "<td>Настройка: <input type='color' value='#";
   out += s;
   out += "' id='";
   out += name;
   out += "' name='";
   out += name;
   out += "'></td>\n";
   out += "</tr>\n"; 
}

void HTTP_print_color(String &out, uint32_t color, char *name){
   char s[64];
   sprintf(s,"%06lX",color);
   out += "<input type='color' value='#";
   out += s;
   out += "' name='";
   out += name;
   out += "'>\n";
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
  out += "<td width=40% align='center'>";
  if( current == 0 )out += "<form method='GET'><input type='submit' value='Основные настройки' class='btn0'></form>\n";
  else out += "<form action='/' method='GET'><input type='submit' value='Основные настройки' class='btn2'></form>\n";
  out += "</td>";
  out += "<td width=35% align='center'>";
  if( current == 1 )out += "<form method='GET'><input type='submit' value='Настройки реле' class='btn0'></form>\n";
  else out += "<form action='/conf1' method='GET'><input type='submit' value='Настройки реле' class='btn2'></form>\n";
  out += "</td>";
  out += "<td width=25% align='center'>";
  if( current == 2 )out += "<form method='GET'><input type='submit' value='Сеть и сенсор' class='btn0'></form>\n";
  else out += "<form action='/conf2' method='GET'><input type='submit' value='Сеть и сенсор' class='btn2'></form>\n";
  out += "</tr><tr>\n";
  out += "</td>";
  out += "<td width=33% align='center'>";
  if( current == 3 )out += "<form method='GET'><input type='submit' value='Звуковые оповещения' class='btn0'></form>\n";
  else out += "<form action='/conf3' method='GET'><input type='submit' value='Звуковые оповещения' class='btn2'></form>\n";
  out += "</td>";
  if( UID >= 2 ){
      out += "<td align='center'>";
      if( current == 4 )out += "<form method='GET'><input type='submit' value='Стартовые настройки' class='btn0'></form>\n";
      else out += "<form action='/conf4' method='GET'><input type='submit' value='Стартовые настройки' class='btn2'></form>\n";
      out += "</td>";
      out += "<td>&nbsp;</td>";

  }
  else {
      out += "<td'>&nbsp;</td>";
      out += "<td>&nbsp;</td>";
  }
//  out += "<td width=33% align='center'>";
//  if( current == 4 )out += "<form method='GET'><input type='submit' value='Стартовые настройки' class='btn0'></form>\n";
//  else out += "<form action='/conf4' method='GET'><input type='submit' value='Стартовые настройки' class='btn2'></form>\n";
//  out += "</td>";


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

/*
 * Процесс обновления прошивки через WEB
 */
void HTTP_handleUpdate() {
//   Serial.println("!!! Update start ...");
//   if( is_update ){
//      String content;
//      HTTP_printHeader(content,"Update",0);
//      content += "Update firmware. Please wait about 30 sec\n";   
 //     HTTP_printTail(content);
//      webServer.send(200, "text/html", content);
 //     is_update = false;   
//   }
//  Serial.println("Update Page");
//  strcpy(_stat_setup,"Update FW...");
//  displayGive();
  http_ms = millis();
  HTTPUpload& upload = server.upload();
///  Serial.printf("!!! Update %d\n",upload.status);
  if (upload.status == UPLOAD_FILE_START) {
     checkPlayMP3("89",89);
#if defined(DEBUG_SERIAL)
     Serial.setDebugOutput(true);
     Serial.printf("Update: %s\n", upload.filename.c_str());
//     HTTP_goto("/", 500000, "Загрузка файла ...");
#endif
     if (!Update.begin()) { //start with max available size
        Update.printError(Serial);
     }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
     if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
       Update.printError(Serial);
     }
  } else if (upload.status == UPLOAD_FILE_END) {
     if (Update.end(true)) { //true to set the size to the current progress
#if defined(DEBUG_SERIAL)
       Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
#endif
//       HTTP_goto("/", 5000, "Обновление прошивки завершено успешно. Перезагрузка через 5 сек");
       checkPlayMP3("89",88);
#if defined(DEBUG_SERIAL)
       Serial.println(F("Обновление прошивки завершено успешно."));
#endif
 //      delay(20000);
//       ESP.restart();

     } else {
       checkPlayMP3("89",87);
       Update.printError(Serial);
       HTTP_printMessage("Ошибка обновление прошивки");
     }
#if defined(DEBUG_SERIAL)
     Serial.setDebugOutput(false);
#endif
   } else {
#if defined(DEBUG_SERIAL)
        Serial.printf("Update Failed Unexpectedly (likely broken connection): status=%d\n", upload.status);
#endif
        HTTP_printMessage("Ошибка обновление прошивки");
   }
}

void HTTP_fileUpload1(){ // upload a new file to the Filing system
    HTTP_goto("/", 5000, "Обновление прошивки завершено успешно. Перезагрузка через 5 сек");
    delay(5000);
    ESP.restart();

}

void HTTP_printMessage(const char *s){
  if( HTTP_redirect() )return;
  Serial.printf("!!! HTTP MSG %s\n",s);
//  server.headers();
  http_ms = millis();
  String out = "";
  HTTP_printHeader(out,"msg",5);
  out += "  <h2>";
  out += s;
  out += "  </h2>\n";
  out += "  Сейчас загрузится главная страница ...";
  HTTP_printTail(out);
  server.send(200, "text/html", out); 
}

/**
 * Выдача страницы с формой загрузки файла прошивки "Update"
 */
void HTTP_handleUpload() {
  if( HTTP_redirect() )return;
  
   is_update = true;
  String content;
  HTTP_printHeader(content,"Update",0);
//  content += "<script>\n";
//  content += "function F1(){$('#main').html(\"Begin firmware update. Please wait 30 sec\");}\n";
//  content += "</script>\n";
  checkPlayMP3("89",90);


  content += " <form method='POST' action='/update1' enctype='multipart/form-data'>\n";
  content += "  <fieldset>\n";
  content += "    <legend>Обновление прошивки</legend>\n";
  content += "<p class='t1'>Для обновление прошивки выберите файл с прошивкой нажмите \"Update\" ";
  content += "Дождитесь завершения прошивки и перезагрузки датчика.</p>";
//  content += "    <p><label class=\"file-upload\">";
  content += "    <p><input type='file' name='update' id=\"userfile\" size=\"30\">\n";
//  content += "    <span id=\"userfile-name\">Not file firmware</span>\n";
//  content += "    <script type=\"text/javascript\">\n";
//  content += "      $('#userfile').change(function(e) {\n";
//  content += "        $('#userfile-name').text(this.files[0].name);\n";
//  content += "      })\n";
//  content += "    </script>\n";
  
//  content += "    </label>\n";
//  content += "    <label for=\"file_upload\">Select file</label>\n";    
  content += "    <p><input type='submit' value='Update'>\n";  content += "  </fieldset>\n";
  content += "</form>\n";
  HTTP_printTail(content);
  server.send(200, "text/html", content);
}
