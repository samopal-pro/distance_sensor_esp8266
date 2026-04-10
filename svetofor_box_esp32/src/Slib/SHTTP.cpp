#include "SHTTP.h"

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

void HTTP_printTextarea(String &out,
   const char *label, //Метка поля 
   const char *name,  //Имя поля HTTP
   JsonArray value,   //Массив значений
   int col,           //Количество строк
   int row,           //Количество столбцоы
   const char *style  //Стиль метки (если есть)
){

   if( style == NULL )out += "<p><label>";
   else {
       out += "  <div class=\"";
       out += style;
       out += "\"><label>";
   }
   out += label;
   out += "</label>";
   out += "<textarea name=\"";
   out += name;
   out += "\" cols=";
   out += col;
   out += " rows=";
   out += row;
   out +=">";
   //out += "192.168.1.35,\n192.168.1.36,\n192.168.1.37";
   
   if( !value.isNull() ){
      for (int i = 0; i < value.size(); i++) {
         if( i>0 )out += ",\n";
         out += value[i].as<String>();
      }
   }
   out += "</textarea>\n";
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


void HTTP_printWiFiPower(String &out, int _power){
  out += "<div class='lab2'><p><label>Уровень мощности WiFi:</label>\n";
  out += "<select name='WiFiPower'>\n";
  HTTP_printSelectOption(out, "21dBm", (int)WIFI_POWER_21dBm, _power);
  HTTP_printSelectOption(out, "20dBm", (int)WIFI_POWER_20dBm, _power);
  HTTP_printSelectOption(out, "19dBm", (int)WIFI_POWER_19dBm, _power);
  HTTP_printSelectOption(out, "17dBm", (int)WIFI_POWER_17dBm, _power);
  HTTP_printSelectOption(out, "15dBm", (int)WIFI_POWER_15dBm, _power);
  HTTP_printSelectOption(out, "11dBm", (int)WIFI_POWER_11dBm, _power);
  HTTP_printSelectOption(out, "7dBm",  (int)WIFI_POWER_7dBm,  _power);
  HTTP_printSelectOption(out, "5dBm",  (int)WIFI_POWER_5dBm,  _power);
  HTTP_printSelectOption(out, "2dBm",  (int)WIFI_POWER_2dBm,  _power);
  out += "</select>\n";
  out += "</div>\n";
}


void HTTP_printSubmit(String &out,const char *name, const char *value,const char *style, const char *tag){
   HTTP_beginTag(out,tag);
   out += "<input type='submit' name='";
   out += name;
   out +="' value='";
   out += value;
   out += "'";
   HTTP_style(out,style);  
   out += ">";   
   HTTP_endTag(out,tag);

}


void HTTP_printSelectOption(String &out, const char *_name, int _num, int _cur){
    out += "<option value='";
    out += _num;
    out += "'";
    if( _num == _cur )out += " selected";
    out += ">";
    out += _name;
    out += "</option>\n";
}

void HTTP_beginTag(String &out,const char *tag,const char *style,bool is_nl){
   if( tag == NULL )return;
   out += "<";
   out += tag;
   HTTP_style(out,style);
   out += ">";
   if( is_nl )out += "\n";   
}


void HTTP_endTag(String &out,const char *tag,bool is_nl){
   if( tag == NULL )return;
   out += "</";
   out += tag;
   out += ">";
   if( is_nl )out += "\n";

}

void HTTP_style(String &out,const char *style){
  if( style == NULL )return;
  out += " class='";
  out += style;
  out += "'";
}

void HTTP_printTag(String &out,const char *tag, const char *text, const char *style){
   if( text == NULL )return;
   HTTP_beginTag(out,tag,style);
   out += text;
   HTTP_endTag(out,tag);
}