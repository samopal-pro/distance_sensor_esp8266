#include "LD2413.h"

LD2413::LD2413(){
//   RadarSerial = radar_serial;
//   DebugSerial = debug_serial;   
}

void LD2413::begin(){
   RADAR_SERIAL.begin(RADAR_SPEED);  
}

void LD2413::begin(uint8_t _rx, uint8_t _tx){
   RADAR_SERIAL.begin(RADAR_SPEED,SERIAL_8N1,_rx,_tx);  
}

void LD2413::init(uint16_t _min, uint16_t _max, uint16_t _tm){
  cmd4(RADAR_STOP_CMD,1);
  wait_cmd();
  cmd4(RADAR_SET_MIN_DISTANCE_CMD,_min);
  wait_cmd();
  cmd4(RADAR_SET_MAX_DISTANCE_CMD,_max);
  wait_cmd();
  cmd4(RADAR_SET_TIMEOUT_CMD,_tm);
  wait_cmd();
  cmd2(RADAR_START_CMD);
  wait_cmd();
    
}

void LD2413::calibrate(){
  cmd4(RADAR_STOP_CMD,1);
  wait_cmd();
  cmd2(RADAR_SET_THRESHOLD_CMD);
  wait_cmd();
  cmd2(RADAR_START_CMD);
  wait_cmd();    
}


void LD2413::cmd2(uint8_t _cmd){
#if defined(DEBUG_SERIAL)
   DEBUG_SERIAL.print(">>> cmd");
   debug_char(_cmd);
   DEBUG_SERIAL.println();
#endif
   RADAR_SERIAL.flush();
   RADAR_SERIAL.write(0xfd);
   RADAR_SERIAL.write(0xfc);
   RADAR_SERIAL.write(0xfb);
   RADAR_SERIAL.write(0xfa);
   RADAR_SERIAL.write(0x02);
   RADAR_SERIAL.write(0x00);
   RADAR_SERIAL.write(_cmd);
   RADAR_SERIAL.write(0x00);
   RADAR_SERIAL.write(0x04);
   RADAR_SERIAL.write(0x03);
   RADAR_SERIAL.write(0x02);
   RADAR_SERIAL.write(0x01);  
}

void LD2413::cmd4(uint8_t _cmd, uint16_t _arg){
   uint8_t b0 = _arg&0xff;
   uint8_t b1 = (_arg>>8)&0xff;
#if defined(DEBUG_SERIAL)
   DEBUG_SERIAL.print(">>> cmd");
   debug_char(_cmd);
   DEBUG_SERIAL.print(" arg");
   debug_char(b0);
   debug_char(b1);
   DEBUG_SERIAL.println();
#endif
   RADAR_SERIAL.flush();
   RADAR_SERIAL.write(0xfd);
   RADAR_SERIAL.write(0xfc);
   RADAR_SERIAL.write(0xfb);
   RADAR_SERIAL.write(0xfa);
   RADAR_SERIAL.write(0x04);
   RADAR_SERIAL.write(0x00);
   RADAR_SERIAL.write(_cmd);
   RADAR_SERIAL.write(0x00);
   RADAR_SERIAL.write(b0);
   RADAR_SERIAL.write(b1);
   RADAR_SERIAL.write(0x04);
   RADAR_SERIAL.write(0x03);
   RADAR_SERIAL.write(0x02);
   RADAR_SERIAL.write(0x01);  
}

uint16_t LD2413::wait_cmd(uint32_t _tm){
   int n = 0;
   int len = 0;
   uint32_t ms1 = millis();
   while(true){
      uint32_t ms = millis();
      if( _tm > 0 && labs(ms-ms1) >_tm )return 0xffff;
      if( RADAR_SERIAL.available() ){
         uint8_t c = RADAR_SERIAL.read();
         if( n == 0 && c == 0xfd )n++;
         else if( n == 1 && c == 0xfc )n++;
         else if( n == 2 && c == 0xfb )n++;
         else if( n == 3 && c == 0xfa )n++;
         else if( n == 4 ){ 
            len = (int)c; 
            n++; 
#if defined(DEBUG_SERIAL)
            DEBUG_SERIAL.print("<<< cmd len=");
            DEBUG_SERIAL.print(len);
#endif
         }
         else if( n == 5 )n++;
         else if( n>5 && n < len+6 ){
#if defined(DEBUG_SERIAL)
         debug_char(c);
#endif
            n++;
         }
         else if(n >= len+6){
#if defined(DEBUG_SERIAL)
         DEBUG_SERIAL.println(); 
#endif
            return 0;
         }
         else n=0;
      }
   }
}

float LD2413::wait_data(uint32_t _tm){
   int n = 0;
   int len = 0;
//   int count = 0;
   float val1 = NAN;
   union {
       float f;
       uint8_t b[4];
   }val;
   uint32_t ms1 = millis();
//   while(RADAR_SERIAL.available())RADAR_SERIAL.read();
//   RADAR_SERIAL.flush();
   while(true){
      uint32_t ms = millis();
      if( _tm > 0 && labs(ms-ms1) >_tm )return NAN;
      if( RADAR_SERIAL.available() ){
         uint8_t c = RADAR_SERIAL.read();

         if( n == 0 && c == 0xf4 )n++;
         else if( n == 1 && c == 0xf3 )n++;
         else if( n == 2 && c == 0xf2 )n++;
         else if( n == 3 && c == 0xf1 )n++;
         else if( n == 4 ){ 
            len = (int)c; 
            n++; 
#if defined(DEBUG_SERIAL)
            DEBUG_SERIAL.print("<<< data len=");
            DEBUG_SERIAL.print(len);
#endif
         }
         else if( n == 5 )n++;
         else if( n == 6 ){
            val.b[0] = c;
#if defined(DEBUG_SERIAL)
            debug_char(c);
#endif
            n++;
         }
         else if( n == 7 ){
            val.b[1] = c;
#if defined(DEBUG_SERIAL)
            debug_char(c);
#endif
            n++;
         }
         else if( n == 8 ){
            val.b[2] = c;
#if defined(DEBUG_SERIAL)
            debug_char(c);
#endif
            n++;
         }
         else if( n == 9 ){
            val.b[3] = c;
#if defined(DEBUG_SERIAL)
            debug_char(c);
            DEBUG_SERIAL.print(" val=");
            DEBUG_SERIAL.println(val.f,0);
#endif
//            return val.f;
            val1 = val.f;
            n=0;
//            count++;
         }
         else n=0;
      }    
      else { //Енд авайбле
//         Serial.printf("!!! Count = %d ",count);
         return val1;
      }
   }
}


void LD2413::debug_char(uint8_t _b){
#if defined(DEBUG_SERIAL)	
   if( _b < 0x10 )DEBUG_SERIAL.print(" 0x0");
   else DEBUG_SERIAL.print(" 0x");
   DEBUG_SERIAL.print(_b,HEX);
#endif   
}