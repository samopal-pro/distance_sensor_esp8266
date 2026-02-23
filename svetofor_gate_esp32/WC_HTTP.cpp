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
bool is_load_page  = false;
bool is_first_root = true;
uint32_t   http_ms = 0;
char *pages[] =  {"/", "/conf"};

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
   server.on ( "/conf", HTTP_handleConfig2 );
   
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
   server.on ( "/playMP3",    HTTP_handlePlayMP3 );
   server.on ( "/sendLoRa" ,  HTTP_handleLoRa );
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

//   out += "function playMP3(dir,num){ fetch(\"/playMP3?DIR=\"+dir+\"&NUM=\"+num, {method: \"GET\" }); }\n";
   out += "function playMP3c(dir,num,color){ fetch(\"/playMP3?COLOR=\"+color+\"&DIR=\"+dir+\"&NUM=\"+num, {method: \"GET\" }); }\n";
   out += "function playMP3(dir,num){ fetch(\"/playMP3?DIR=\"+dir+\"&NUM=\"+num, {method: \"GET\" }); }\n";
   out += "function systemMP3(check,num){ fetch(\"/playMP3?CHECK=\"+check+\"&NUM=\"+num, {method: \"GET\" }); }\n";
   out += "function sendLoRa(id,cmd){ fetch(\"/sendLoRa?id=\"+id+\"&cmd=\"+cmd, {method: \"GET\" }); }\n";
//   out += "function playMP3s(dir,num){ fetch(\"/playMP3?DIR=\"+dir+\"&NUM=\"+num, {method: \"GET\" }); return true;}\n";
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
    out += " .btn {font-size: 10pt;color:#000088;}\n";
    out += " td {valign:middle;font-size: 10pt;color:#000088;}\n";
    out += " hr {border-top:1px solid #000088;}\n";
    out += " input[type=file]::file-selector-button {border: 2px solid #000088;background:#fceade;color:#c55a11;width:30%;}\n";
    out += " input[type=file]::file-selector-button:hover {background:#000088;}\n";
    out += " input[type=submit] {font-size: 14pt;color:#000088;}\n";
    out += " input[type=submit].btn10 {font-size: 10pt;color:#000088;border: 1px;}\n";
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
  out += "<h2>ШЛЮЗ</h2>";
  out += "<p><img src=/logo.png></p>\n";
  
  //out += "<p><br>Сенсор: ";
  //out += EA_Config.ESP_NAME;
  //out += " ";
  out += "ID:";
  out += strID;
  out += " S/N:";
  out += serNo;
  out += " VER:";
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
  out += "<br><hr align=\"left\" width=\"500\">Copyright (C) Miller-Ti, A.Shikharbeev, 2026&nbsp;&nbsp;&nbsp;&nbsp;Made from Russia";
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
  if( is_first_root ){
     is_first_root = false;
     systemMP3("70", 70, PRIORITY_MP3_MEDIUM);
  }
  HTTP_printHeader(out,"Главная",0);
  HTTP_print_menu(out, numPage);
// Блок №1
  
  HTTP_login(out);
  out += "<fieldset>\n";
  out += "<legend>Список подключенных сенсоров</legend>\n";

   out += "<form action='/' method='PUT'>\n";
   HTTP_InputHidden(out,"FLAG_ROOT");
//  if( HTTP_login(out) )return;
   out += "<table width=100%>\n";
   out += "<tr>\n";
   out += "<td>ID-сенсора</td>";
   out += "<td>Серийник</td>";
   out += "<td>RSSI</td>";
   out += "<td>Стат</td>";
   out += "<td>Сек.&nbsp;назад</td>";
   out += "<td>Вкл.</td>";
   out += "<td>Отправить</td>";
   out += "<td>Отправить</td>";
   out += "<td>Отправить</td>";
   out += "</tr>\n";

   JsonObject root = jsonNodes.as<JsonObject>();
   uint32_t _ms = millis();
   for (JsonPair kv : root) {
      if (kv.value().is<JsonObject>()) {
          HTTP_printNode(out, kv.key().c_str(), kv.value().as<JsonObject>());
      }
   }
   out += "</table>";

   out += "<p><input type='submit' value='Опросить все сенсоры' class='btn' onClick='sendLoRa(\"FFFFFFFFFFFF\",\"test\");'>";
   out += "<p><input type='submit' name='Save' value='Сохранить' class='btn'>"; 
   out += "</form>\n";
   out += "</fieldset>\n";
   HTTP_printBottomMenu(out);
   HTTP_printTail(out);
   Serial.printf("!!! HTTP Root Length %d\n", out.length());  
   server.send(200, "text/html", out);
   msLoad = 0;
   is_load_page = false;
}


void HTTP_printNode(String &out, const char *_id, JsonObject _obj){
   bool _ch = false;
   if( _id == nullptr || _id[0] == 0 )return;
   out += " <tr>\n";
   out += "<td>"; 
   out += _id; 
   out += "</td>";
//   Serial.printf("!!! %s ",_id);
//   nodeListPrint("HTTP");
   if( !jsonNodeList[_id].isNull() )_ch = true;
      // Проверяем, что значение является объектом
   if ( _obj != NULL) {
      int _state = _obj["State"].as<int>(); 
      out += "<td>"; out += _obj["SN"].as<const char *>(); out += "</td>";
//      out += "<td>"; out += _obj["DogovorNo"].as<const char *>(); out += "</td>";
//      out += "<td>"; out += _obj["BoxNo"].as<const char *>(); out += "</td>";
      if(_obj["Rssi"].isNull())out += "<td>&nbsp;</td>";
      else out += "<td>"; out += _obj["Rssi"].as<int>(); out += "</td>";
      out += "<td>"; out += _obj["State"].as<int>(); out += "</td>";
      if(_obj["Time"].isNull())out += "<td>-</td>";
      else out += "<td>"; out += (millis()-_obj["Time"].as<uint32_t>())/1000; out += "</td>";
      out += "<td>";
      HTTP_print_input_checkbox(out, (char *)_id, "1", _ch);
      out += "</td>";
      out += "<td><input type='submit' value='Опрос' class='btn10' onClick='sendLoRa(\"";
      out += _id;
      out += "\",\"test\");'></td>";
      out += "<td><input type='submit' value='Перезагрузка' class='btn10' onClick='sendLoRa(\"";
      out += _id;
      out += "\",\"reboot\");'></td>";
      out += "<td><input type='submit' value='Калибровка' class='btn10' onClick='sendLoRa(\"";
      out += _id;
      out += "\",\"calibrate\");'></td>";
   }
   else  out += "<td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td>";
   out += " </tr>\n";
}

/*
 * Оработчик страницы сетевых настроек
 */
void HTTP_handleConfig2(void) {
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


/**
* Настройка Сетевых параметров
**/
void HTTP_printConfigNet(String &out){
  char s[32];

  out += "<fieldset>\n";
  out += "<legend>Подключение к внешним сервисам по WiFi</legend>\n";
  HTTP_printNetworks1(out,"WiFiName");
  HTTP_printInput1(out,"**Введите пароль от вашей WI-FI сети:","WiFiPassword",jsonConfig["WIFI"]["PASS"].as<const char *>(),20,32,HT_PASSWORD);

//  HTTP_printInput1(out,"**Номер договора, логин личного кабинета:","Dogovor",jsonConfig["NET"]["DOGOVOR_ID"].as<const char *>(),20,16,HT_TEXT);
//  HTTP_printInput1(out,"**Номер бокса:","Box",jsonConfig["NET"]["BOX_ID"].as<const char *>(),20,16,HT_TEXT);
  sprintf(s,"%d",jsonConfig["NET"]["T_SEND"].as<int>());
  HTTP_printInput1(out,"Связь с сервером каждые, сек:","TM_HTTP_SEND",s,20,32,HT_NUMBER);
  sprintf(s,"%d",jsonConfig["NET"]["T_RETRY"].as<int>());
  HTTP_printInput1(out,"Повторная попытка отправки через, сек:","TM_HTTP_RETRY_ERROR",s,20,32,HT_NUMBER);
  out += "<p><input type='submit' name='Save' value='Сохранить' class='btn'>"; 
  out += "</fieldset>\n";  

/*
  out += "<fieldset>\n";
  out += "<legend>Подключение к онлайн мониторингу CRM.MOSCOW</legend>\n";
  out += "<labelВвключить онлайн отправку данных</label>\n";
  HTTP_print_input_checkbox(out,"CRM_ENABLE","send",jsonConfig["CRM_MOSCOW"]["ENABLE"].as<bool>());
  out += "<p class='t1'>Для активации онлайн мониторинга поставьте галочку которая находится над этой строкой. Введите все поля с ** и сохраните. Нужно прописать: ID мойки, номер бокса, пароль сети интернет и выбрать сеть WI-FI. После этого сохраните и в первой вкладке выключите бесконечный режим раздачи WiFi активировав режим с бирюзовой иконкой.";
  out += "После сохраните и перезагрузите устройство нажав желтую кнопку внизу натроек.</p>";
  out += "<p class='t1'>Ниже идут дополнительные настройки. Обратитесь в техническую поддержку прежде чем их менять.</p>";
  HTTP_printInput1(out,"Сервер:","CRM_SERVER",jsonConfig["CRM_MOSCOW"]["SERVER"].as<const char *>(),20,32,HT_TEXT);
  sprintf(s,"%d",jsonConfig["CRM_MOSCOW"]["PORT"].as<int>());
  HTTP_printInput1(out,"Порт:","CRM_PORT",s,20,32,HT_NUMBER);
  out += "<p><input type='submit' name='Save' value='Сохранить' class='btn'>"; 
  out += "</fieldset>\n";  
*/
  out += "<fieldset>\n";
  out += "<legend>Подключение к онлайн мониторингу TB.SVETOFORBOX.RU</legend>\n";
  out += "<p><label>Ввключить онлайн отправку данных</label>\n";
  HTTP_print_input_checkbox(out,"TB_ENABLE","send",jsonConfig["TB"]["ENABLE"].as<bool>());
//  out += "<p><label>Отправка через шлюз</label>\n";
//  HTTP_print_input_checkbox(out,"TB_GATEWAY","send",jsonConfig["TB"]["GATEWAY"].as<bool>());
  out += "<p class='t1'>Ниже идут дополнительные настройки. Обратитесь в техническую поддержку прежде чем их менять.</p>";
  HTTP_printInput1(out,"Сервер:","TB_SERVER",jsonConfig["TB"]["SERVER"].as<const char *>(),20,32,HT_TEXT);
  sprintf(s,"%d",jsonConfig["TB"]["PORT"].as<int>());
  HTTP_printInput1(out,"Порт:","TB_PORT",s,20,32,HT_NUMBER);
  HTTP_printInput1(out,"Токен доступа:","TB_TOKEN",jsonConfig["TB"]["TOKEN"].as<const char *>(),20,32,HT_TEXT);
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

  HTTP_printInput1(out,"Пароль для входа с правами опреатора:","PasswordUser",jsonConfig["SYSTEM"]["PASS1"].as<const char *>(),20,32,HT_PASSWORD);
  if( UID >= 1 ){
     HTTP_printInput1(out,"Пароль для входа с правами администратора:","PasswordAdmin",jsonConfig["SYSTEM"]["PASS0"].as<const char *>(),20,32,HT_PASSWORD);
     if( UID >= 2 )HTTP_printInput1(out,"Пароль для входа с правами суперадминистратора:","PasswordSuperAdmin",jsonConfig["SYSTEM"]["PASSS"].as<const char *>(),20,32,HT_PASSWORD);

     HTTP_printInput1(out,"Наименование устройства","NameESP",jsonConfig["SYSTEM"]["NAME"].as<const char *>(),32,32,HT_TEXT,"lab1");
     out += "<p class='t1'>Важно! Не используйте русские буквы и специальные символы в имени сети Wi-FI. После изменения имени WiFi нажмите внизу желтую кнопку &quot;Перезагрузка&quot;</p>";
  }
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
   out += "<input type='button' value='▶' class='btn' onClick='playMP3c(";
   out += MP3_BASE_DIR;
   out += ",";
   out += jsonConfig["MP3"][name]["NUM"].as<int>();
   out += ",";
   out += jsonConfig["MP3"][name]["COLOR"].as<uint32_t>();
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
if( server.hasArg("Default") ){ 

       String ss = jsonConfig["SYSTEM"]["NAME"].as<String>();
       configDefault();
       jsonConfig["SYSTEM"]["NAME"] = ss;
       configSave();
       configRead();
//       HTTP_printMessage("Загрузка заводских параметров. Перезагрузка ...");
       HTTP_goto("/", 2000, "Загрузка заводских параметров. Перезагрузка ..."); //1.12.24
       systemMP3("89",91,PRIORITY_MP3_MAXIMAL);
//       vTaskDelay(3000);
//       ESP.restart();  
       waitMP3andReboot();
       return true;
   }
   else if( server.hasArg("Reboot") ){ 
       HTTP_goto("/", 20000, "Перезагрузка ...");
       systemMP3("89",86,PRIORITY_MP3_MAXIMAL);
       waitMP3andReboot();
//Vj;tn       vTaskDelay(3000);
//       HTTP_printMessage("Перезагрузка ...");
//       ESP.restart();  
       return true;
   }
//   else if( server.hasArg( "MP3_PLAY" ) ){
//       playMP3(server.arg("MP3_DIR").toInt(), server.arg("MP3_NUM").toInt());
//   }

// Если нажата кнопка "Сохранить"   
   else if ( server.hasArg("Save") &&  server.hasArg("FLAG_ROOT")  ) {
      jsonNodeList.clear();

      JsonObject root = jsonNodes.as<JsonObject>();
      for (JsonPair kv : root) {
          if( server.hasArg(kv.key().c_str()) ){
             JsonObject obj = kv.value().as<JsonObject>();
             jsonNodeList[kv.key().c_str()]["SN"] =  obj["SN"];    
          }
      }
      nodeListSave();
      HTTP_goto(pages[current], 1000, "Сохранение списка сенсоров ...");
  }

   else if ( server.hasArg("Save") &&  server.hasArg("FLAG_CONFIG2")  ) {

      if(server.hasArg("WiFiName")     )jsonConfig["WIFI"]["NAME"]              = server.arg("WiFiName").c_str();
      if(server.hasArg("WiFiPassword") )jsonConfig["WIFI"]["PASS"]              = server.arg("WiFiPassword").c_str();

//      if(server.hasArg("Dogovor")      )jsonConfig["NET"]["DOGOVOR_ID"]  = server.arg("Dogovor").c_str();
//      if(server.hasArg("Box")          )jsonConfig["NET"]["BOX_ID"]      = server.arg("Box").c_str();
      if(server.hasArg("TM_HTTP_SEND") )jsonConfig["NET"]["T_SEND"]      = server.arg("TM_HTTP_SEND").toInt();
      if(server.hasArg("TM_HTTP_RETRY_ERROR"))jsonConfig["NET"]["T_RETRY"] = server.arg("TM_HTTP_RETRY_ERROR").toInt();

//      if( server.hasArg("CRM_ENABLE_H")){ 
//         if( server.hasArg("CRM_ENABLE"))jsonConfig["CRM_MOSCOW"]["ENABLE"] = true;
//         else jsonConfig["CRM_MOSCOW"]["ENABLE"] = false;
//      }
 //     if(server.hasArg("CRM_SERVER")    )jsonConfig["CRM_MOSCOW"]["SERVER"]      = server.arg("CRM_SERVER").c_str();
 //     if(server.hasArg("CRN_PORT")      )jsonConfig["CRM_MOSCOW"]["PORT"]        = server.arg("CRM_PORT").toInt();

      if( server.hasArg("TB_ENABLE_H")){
          if( server.hasArg("TB_ENABLE"))jsonConfig["TB"]["ENABLE"] = true;  
          else jsonConfig["TB"]["ENABLE"] = false;    
      }
//      if( server.hasArg("TB_GATEWAY_H")){
//          if( server.hasArg("TB_GATEWAY"))jsonConfig["TB"]["GATEWAY"] = true;  
//          else jsonConfig["TB"]["GATEWAY"] = false;    
//      }
      if(server.hasArg("TB_SERVER")     )jsonConfig["TB"]["SERVER"]              = server.arg("TB_SERVER").c_str();
      if(server.hasArg("TB_PORT")       )jsonConfig["TB"]["PORT"]                = server.arg("TB_PORT").toInt();
      if(server.hasArg("TB_TOKEN")      )jsonConfig["TB"]["TOKEN"]               = server.arg("TB_TOKEN").c_str();

      if( server.hasArg("LORA_ENABLE_H")){
          if( server.hasArg("LORA_ENABLE"))jsonConfig["LORA"]["ENABLE"] = true;  
          else jsonConfig["LORA"]["ENABLE"] = false;    
      }
      if(server.hasArg("LORA_GATEWAY")      )jsonConfig["LORA"]["GATEWAY"]       = strtoull(server.arg("LORA_GATEWAY").c_str(), nullptr, 16); 


      if( server.hasArg("STATIC_IP_H")){
         if( server.hasArg("STATIC_IP"))jsonConfig["WIFI"]["DHCP"] = false;
         else jsonConfig["WIFI"]["DHCP"] = true;
      }

      if(server.hasArg("IPAddr"))jsonConfig["WIFI"]["IP"]["ADDR"]           = server.arg("IPAddr").c_str();
      if(server.hasArg("IPMask"))jsonConfig["WIFI"]["IP"]["MASK"]           = server.arg("IPMask").c_str();
      if(server.hasArg("IPGate"))jsonConfig["WIFI"]["IP"]["GW"]             = server.arg("IPGate").c_str();
      if(server.hasArg("IPDns") )jsonConfig["WIFI"]["IP"]["DNS"]            = server.arg("IPDns").c_str();
      nodeListSave();
      HTTP_goto(pages[current], 1000, "Сохранение параметров ...");


   }

   if( _save ){
      configSave();
      EventRGB1->set(COLOR_SAVE,COLOR_SAVE);

      HTTP_goto(pages[current], 1000, "Сохранение параметров ...");
      systemMP3("89",83,PRIORITY_MP3_MEDIUM);
      vTaskDelay(500);
      lastSensorOn = SS_RESTORE;
      return true;
   }
   return false;
}

void HTTP_handleLoRa(void){
   int dir        =  0;
   if( server.hasArg("id") && server.hasArg("cmd") ){
      Serial.printf("!!! HTTP LoRa %s %s\n",server.arg("id").c_str(),server.arg("cmd").c_str());
      sendLoraCmd(server.arg("id").c_str(), server.arg("cmd").c_str());       
   }
   else {
      Serial.printf("??? HTTP LoRa ???\n");       

   }
   
}

void HTTP_handlePlayMP3(void){
   int dir        =  0;
   if( server.hasArg("DIR") )dir = server.arg("DIR").toInt();
   int num        =  server.arg("NUM").toInt();
   uint32_t color =  COLOR_MP3_1;
   if( server.hasArg("COLOR") )color = server.arg("COLOR").toInt();
   if( server.hasArg("CHECK") ){
      systemMP3((char *)server.arg("CHECK").c_str(),num,PRIORITY_MP3_MEDIUM);       
      Serial.printf("!!! HTTP MP3 system %d\n",num); 
   }
   else {
      playMP3(dir,num,PRIORITY_MP3_MEDIUM,color);
      Serial.printf("!!! HTTP MP3 %d %d %06lx\n",dir,num,color); 
   }
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
  out += "<input type='hidden' name='";
  out += name;
  out += "_H' value='1'>";
  out += "<input type='checkbox' name='";
  out += name;
  out += "' value='";
  out += value;
  out += "'";
  if( checked )out += " checked";
  out += ">";   
}


void HTTP_print_menu_item(String &out, bool _isCur, int _proc, char *_label, char *_url, bool _isMP3, int _dirMP3, int _numMP3){
  out += "<td width=";
  out += _proc;
  out += "% align='center'>";
  if( _isCur ){
      out += "<form method='GET'><input type='submit' value='";
      out += _label;
      out += "' class='btn0'";
  }
  else {
      out += "<form action='";
      out += _url;
      out += "' method='GET'><input type='submit' value='";
      out += _label;
      out += "' class='btn2'";
  }
  if( _isMP3 ){
     out += " onClick='playMP3(";
     out += _dirMP3;
     out += ",";
     out += _numMP3;
     out += ");'";
  }
  out += "></form></td>\n";
  Serial.printf("!!! HTTP Menu %d %s %s %d %d %d\n",(int)_isCur, _label, _url, (int)_isMP3, _dirMP3, _numMP3 );

}

void HTTP_print_menu(String &out, int current){
  bool isMP3 =  jsonConfig["MP3"]["70"]["ENABLE"].as<bool>(); 
//  isMP3 = false;
  
  out += "\n<p><table width='100%'><tr>";
  HTTP_print_menu_item(out, ( current == 0 ), 40, "Главная страница",  "/",   isMP3, MP3_ADD_DIR, 70);
  HTTP_print_menu_item(out, ( current == 1 ), 35, "Настройки",      "/conf",  isMP3, MP3_ADD_DIR, 71);
   out += "<td'>&nbsp;</td>";

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
     systemMP3("89",89,PRIORITY_MP3_MEDIUM);
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
       systemMP3("89",88,PRIORITY_MP3_MEDIUM);
#if defined(DEBUG_SERIAL)
       Serial.println(F("Обновление прошивки завершено успешно."));
#endif
 //      delay(20000);
//       ESP.restart();

     } else {
       systemMP3("89",87,true);
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
//    delay(5000);
//    ESP.restart();
    waitMP3andReboot();

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
  systemMP3("89",90,PRIORITY_MP3_MEDIUM);


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
