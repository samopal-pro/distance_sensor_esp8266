#ifndef SHTTP_h
#define SHTTP_h
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>

typedef enum {
  HT_TEXT     = 0,
  HT_PASSWORD = 1,
  HT_NUMBER   = 2,
  HT_IP       = 3,
  
}HTTP_input_type_t;


void HTTP_printInput1(String &out,const char *label, const char *name, const char *value, int size, int len, HTTP_input_type_t htype=HT_TEXT, const char *style=NULL, const char *add_text=NULL);
void HTTP_printTextarea(String &out,const char *label, const char *name, JsonArray value, int col, int row, const char *style=NULL);
void HTTP_InputInt(String &out,const char *label,const char *name,int value,int min,int max,int size=32,const char *style = NULL);
void HTTP_InputRange(String &out,const char *label,const char *name,int value,int min,int max,int size=32,const char *style = NULL);
void HTTP_InputInt1(String &out,const char *name,int value,int min,int max,int size=32);
void HTTP_InputHidden(String &out, char *name, char *value = "1");


void HTTP_print_td_color(String &out, uint32_t color, char *name, uint8_t value, uint32_t color_set, uint8_t color_num, uint8_t proc, bool is_change = false);
void HTTP_print_color3(String &out, uint32_t color, char *name, char *label, uint32_t color1, uint32_t color2, char *name1 = NULL, bool check = false);
void HTTP_print_color(String &out, uint32_t color, char *name);
void HTTP_print_img_radio(String &out,char *img, char *label, char *name, char *value,bool checked, bool is_table);
void HTTP_print_input_radio(String &out,char *name, char *value,bool checked);
void HTTP_print_input_checkbox(String &out,char *name, char *value,bool checked);

void HTTP_printWiFiPower(String &out, int _power);
void HTTP_printSelectOption(String &out, const char *_name, int _num, int _cur=-1);
#endif
