/**
* Проект контроллера автомоек. Версия 4 от 2020
* Copyright (C) 2020 Алексей Шихарбеев
* http://samopal.pro
*/

//#define WIFI_SAV

#include "Archive.h"
#include "src/RTClib.h"

const char *_VERSION = FW_VERSION;

uint16_t      EA_Count;
size_t        EA_Size;
size_t        EA_Offset;
EA_Value      EA_Buffer[EA_BUFFER_SIZE];
EA_SaveType   EA_Save;
EA_ConfigType EA_Config;

extern struct WC_Config EC_Config;

StackArray <struct EA_SaveType> EA_Ram;

/**
 * Инициализация EEPROM
 */

void EA_begin(void){
   EA_Offset = sizeof(EA_Count) + sizeof(EA_Save) + sizeof(EA_Config);
   EA_Size = EA_Offset + sizeof(EA_Value) * EA_VALUE_NUMBER;
   EEPROM.begin(EA_Size);   
   EA_get_count();
   EA_read_config();
#ifdef WIFI_SAV   
   EA_default_config();
   EA_Config.isWiFiAlways = true;
#endif   
}

/**
 * Читаем значение счетчика записей
 */
void EA_get_count(void){
   *((uint8_t*)&EA_Count + 0) = EEPROM.read(0);
   *((uint8_t*)&EA_Count + 1) = EEPROM.read(1);
   if( EA_Count > EA_VALUE_NUMBER ){
      EEPROM.write(0, 0);   
      EEPROM.write(1, 0);  
      EEPROM.commit();
      EA_Count = 0;
   }
}

/**
 * Записать одно значение в архив
 */
void EA_save(uint32_t tm,int t,int h,int d,bool bt,int check_interval){
// Проверяем, гогда было предыдущее аналогичное событие
   int n = -1;
 //  printf("->>>>>Save %d %ld\n",check_interval,tm);
   if( check_interval!= 0 && EA_Count>0){
       for( int i=EA_Count-1; i>=0; i-- ){
           struct EA_Value val1;
           EA_get(i,val1);
           if( bt == val1.Button ){
                 Serial.printf("Found the previous value %d of %d sec ago\n",
                    (int)bt,(int)(tm - val1.Time)); 
              if( (tm - val1.Time)<check_interval ){
                 EA_Count = i;
                 Serial.printf("Reducing to %d values of archive\n",EA_Count);
              }
              break;
           }
       }
   }
   
   
   if( EA_Count >= EA_VALUE_NUMBER ){
       EA_shift(1);   
   }
// Заполняем структуру   
   struct EA_Value val;
   val.Time     = tm;
   val.Temp     = t;
   val.Hum      = h;
   val.Distance = d;
   val.Button   = bt;
   val.Enable   = true;
   int offset   = EA_Offset + sizeof(EA_Value) * EA_Count;
   for( int i=0; i<sizeof(EA_Value);i++)
      EEPROM.write(offset+i, *((uint8_t*)&val + i));
   EA_Count++;         
   EEPROM.write(0, *((uint8_t*)&EA_Count + 0));
   EEPROM.write(1, *((uint8_t*)&EA_Count + 1));
   EEPROM.commit();  
   Serial.println("Save to archive");
   EA_get(EA_Count-1,val);   
   EA_print_arh();
}

/**
 * Обнуляем архив
 */
void EA_clear_arh(){
   EA_Count=0;         
   EEPROM.write(0, *((uint8_t*)&EA_Count + 0));
   EEPROM.write(1, *((uint8_t*)&EA_Count + 1));
   EEPROM.commit();  
   Serial.println("Clear archive");
   
}

/**
 * Считать n-e значение из архива в структуру val
 */
void  EA_get(int n,struct EA_Value &val){
  if( n< 0 || n >= EA_BUFFER_SIZE || n >= EA_Count )return;
//  struct EA_Value val1;
  
  for( int j=0; j<sizeof(EA_Value); j++ ){
     int offset = n*sizeof(EA_Value)+j;
     uint8_t c = EEPROM.read(EA_Offset + offset);
     *((uint8_t*)&val + j) = c; 
   }   

//   Serial.printf("EA_get: cnt=%d n=%d tm=%ld bt=%d\n",EA_Count,n,val.Time,(int)val.Button);
  
}

/**
 * Печатаем все значения из архива на экран
 */
void EA_print_arh(){
   return;
   EA_Value val;
   Serial.printf("Archive %d values\n",EA_Count);
   
   for( int i=0; i<EA_Count; i++){
      for( int j=0; j<sizeof(EA_Value); j++ ){
         int offset = i*sizeof(EA_Value)+j;
         uint8_t c = EEPROM.read(EA_Offset + offset);
         *((uint8_t*)&val + j) = c; 
      }
      DateTime dt = DateTime(val.Time);
      Serial.printf("(%d %02d:%02d:%02d %d) ",i,dt.hour(),dt.minute(),dt.second(),(int)val.Button);  
/*         
      Serial.printf("(%d %02d.%02d.%02d %02d:%02d:%02d %d) ",i,
         day(val.Time),
         month(val.Time),
         year(val.Time),
         hour(val.Time),
         minute(val.Time),
         second(val.Time),
         (int)val.Button);  
*/  
   }
   Serial.println("");
}



/**
 * Считать до 10 значений в буфер архива
 * @return - количество значений в буфере от 0 до 10
 */
int EA_read10(){
   if( EA_Count == 0 )return 0;  
   int count;
   if( EA_Count > EA_BUFFER_SIZE )count = EA_BUFFER_SIZE;
   else count = EA_Count; 
   
// Р РЋРЎвЂЎР С‘РЎвЂљРЎвЂ№Р Р†Р В°Р ВµР С� Р В·Р Р…Р В°РЎвЂЎР ВµР Р…Р С‘РЎРЏ Р Р† Р В±РЎС“РЎвЂћРЎвЂћР ВµРЎР‚
   for( int i=0; i<count; i++){
      for( int j=0; j<sizeof(EA_Value); j++ ){
         int offset = i*sizeof(EA_Value)+j;
         uint8_t c = EEPROM.read(EA_Offset + offset);
         *((uint8_t*)&EA_Buffer + offset) = c; 
//         Serial.print(offset);
//         Serial.print(" 0x");
//         Serial.println(c); 
      }   
   }
   return count;      
}


/**
 * Сдвигаем архив на заданное число записей
 */
void EA_shift( int count ){
   EA_Value val;
   if( count > EA_Count )count = EA_Count;
   

// Определяем, сколько значений нужно сдвигать
//   int n = EA_Count - count;
   int offset_shift = count*sizeof(EA_Value);
   Serial.printf("--->Shift on %d %d %d\n",count,EA_Count,offset_shift);
//   Serial.println(count);
//   Serial.println(sizeof(EA_Value));
//   Serial.println(offset_shift);
   for( int i=count; i<EA_Count; i++){
      int offset = EA_Offset+i*sizeof(EA_Value); 
// Читаем i-е значение в буфер      
      for( int j=0; j<sizeof(EA_Value); j++ ){
         uint8_t c = EEPROM.read(offset+j);
         *((uint8_t*)&val + j) = c;
//      }
//      offset = sizeof(EA_Offset)+(i-count)*sizeof(EA_Value); 
// Пишем из буфера в i-count значение 
//      for( int j=0; j<sizeof(EA_Value); j++ ){
//          uint8_t c = *((uint8_t*)&val + j);
         EEPROM.write(offset+j-offset_shift,c);

//         Serial.print(offset+j);
//         Serial.print(" ");
//         Serial.print(offset+j-offset_shift);
//         Serial.print(" 0x");
//         Serial.println(c); 
         
      }   
   }
   if( EA_Count > count )EA_Count -= count;
   else EA_Count = 0;
   EEPROM.write(0, *((uint8_t*)&EA_Count + 0));
   EEPROM.write(1, *((uint8_t*)&EA_Count + 1));
   EEPROM.commit();  
   EA_print_arh();   
}

/**
 * Сохраняем текущее значение в память
 */
void EA_save_last(uint32_t tm,uint32_t uptime,int t,int h,int d,bool bt,int flag){
   EA_Save.Time     = tm;
   EA_Save.Uptime   = uptime;
   EA_Save.Temp     = t;
   EA_Save.Hum      = h;
   EA_Save.Distance = d;
   EA_Save.Button   = bt;
   EA_Save.Flag     = flag;
   for( int i=0; i<sizeof(EA_Save);i++)
      EEPROM.write(sizeof(EA_Count)+i, *((uint8_t*)&EA_Save + i));
   EEPROM.commit();     
   Serial.printf("Save last to EEPROM: time=%ld uptime=%ld temp=%d hum=%d dist=%d butt=%d flag=%d\n",
       EA_Save.Time, EA_Save.Uptime, EA_Save.Temp, EA_Save.Hum ,
       EA_Save.Distance ,(int)EA_Save.Button ,EA_Save.Flag);
}


/**
 * Читаем текущее значение из памяти
 */
void EA_read_last(){
   for( int i=0; i<sizeof(EA_Save); i++ ){
       int offset = sizeof(EA_Count)+i;
       uint8_t c = EEPROM.read(offset);
       *((uint8_t*)&EA_Save + i) = c; 
//         Serial.print(offset);
//         Serial.print(" 0x");
//         Serial.println(c); 
    }  
   Serial.printf("Last EEPROM value: time=%ld uptime=%ld temp=%d hum=%d dist=%d butt=%d flag=%d\n",
       EA_Save.Time, EA_Save.Uptime, EA_Save.Temp, EA_Save.Hum ,
       EA_Save.Distance ,(int)EA_Save.Button ,EA_Save.Flag);
         
}

/**
 * Читаем конфигурацию из EEPROM
 */
void EA_read_config(){
   int offset = sizeof(EA_Count) + sizeof(EA_Save);
   for( int i=0; i<sizeof(EA_ConfigType); i++ ){
       uint8_t c = EEPROM.read(offset+i);
       *((uint8_t*)&EA_Config + i) = c; 
    }  
    uint16_t src = EA_SRC();
    if( EA_Config.SRC == src ){
       Serial.printf("EEPROM Config is correct\n");
       Serial.printf("EEPROM config: level ground = %d\n",EA_Config.GroundLevel);
       sprintf(SensorID,"%s_%s",EA_Config.DOGOVOR_ID,EA_Config.BOX_ID);
       LoopInterval = EA_Config.TM_LOOP_SENSOR*1000;
//       sensorType   = EA_Config.SensorType;
//       nanValueFlag = EA_Config.NanValueFlag;
    }
    else {
       Serial.printf("EEPROM SRC is not valid: %d %d\n",src,EA_Config.SRC);
       EA_default_config();
       EA_save_config();
    }            
}


/**
 * Сохраняем конфигурацию в EEPROM
 */
void EA_save_config(){
   EA_Config.SRC = EA_SRC();
   int offset = sizeof(EA_Count) + sizeof(EA_Save);
   for( int i=0; i<sizeof(EA_ConfigType); i++ ){
      EEPROM.write(offset+i, *((uint8_t*)&EA_Config + i));       

    }  
   EEPROM.commit();     
   sprintf(SensorID,"%s_%s",EA_Config.DOGOVOR_ID,EA_Config.BOX_ID);
   Serial.printf("EEPROM config write: level ground = %d\n",EA_Config.GroundLevel);
   LoopInterval = EA_Config.TM_LOOP_SENSOR*1000;

  
}


/**
 * Вычисляем контрольную сумму
 */
uint16_t EA_SRC(void){
   uint16_t src = 0;
   size_t sz1 = sizeof(EA_ConfigType);
   uint16_t src_save = EA_Config.SRC;
   EA_Config.SRC = 0;
   for( int i=0; i<sz1; i++)src +=*((uint8_t*)&EA_Config + i);
   Serial.printf("SCR=%d\n",src); 
   EA_Config.SRC = src_save;
 
   return src;  
}

/**
 * Сброс конфиг в значения "по умолчанию"
 */
void EA_default_config(void){
   Serial.println("EEPROM Config is Default");
   size_t sz1 = sizeof(EA_Config);
   memset( &EA_Config, '\0',sz1);
   strcpy(EA_Config.ESP_NAME,DEVICE_NAME);
   strcpy(EA_Config.AP_SSID, "none");
   strcpy(EA_Config.AP_PASS, "");
#ifdef WIFI_SAV    
   strcpy(EA_Config.AP_SSID, "ASUS_58_2G");
   strcpy(EA_Config.AP_PASS, "sav59vas");
#endif   
   EA_Config.IP[0]           = 192;   
   EA_Config.IP[1]           = 168;   
   EA_Config.IP[2]           = 1;     
   EA_Config.IP[3]           = 10;
   EA_Config.MASK[0]         = 255; 
   EA_Config.MASK[1]         = 255; 
   EA_Config.MASK[2]         = 255; 
   EA_Config.MASK[3]         = 0;
   EA_Config.GW[0]           = 192;   
   EA_Config.GW[1]           = 168;   
   EA_Config.GW[2]           = 1;     
   EA_Config.GW[3]           = 1;
   EA_Config.DNS[0]          = 8;   
   EA_Config.DNS[1]          = 8;   
   EA_Config.DNS[2]          = 8;     
   EA_Config.DNS[3]          = 8;
   strcpy(EA_Config.ESP_ADMIN_PASS, DEVICE_ADMIN);
   strcpy(EA_Config.ESP_OPER_PASS, DEVICE_OPER);
   EA_Config.isSendCrmMoscow = true;
   strcpy(EA_Config.DOGOVOR_ID, "0000");
   strcpy(EA_Config.BOX_ID, "1");
   strcpy(EA_Config.SERVER, "crm.moscow");
   EA_Config.PORT            = 8001;
   EA_Config.GroundLevel     = 2500;  
   EA_Config.LimitDistance   = 250;
//   EA_Config.LimitDistanceUp = -1;
   EA_Config.MinDistance1     = 1500;
   EA_Config.MaxDistance1     = 1500;
   EA_Config.MinDistance2     = 1500;
   EA_Config.MaxDistance2     = 1500;

   EA_Config.TM_ON           = 1;
   EA_Config.TM_OFF          = 1;
   EA_Config.ZeroDistance    = 11111;
//   EA_Config.isAP = true;
   EA_Config.isDHCP          = true;
   EA_Config.SensorType      = DEFAULT_SENSOR_TYPE;
   EA_Config.NanValueFlag    = DEFAULT_NAN_VALUE_FLAG;
   EA_Config.MeasureType     = DEFAULT_MEASURE_TYPE;
   EA_Config.isWiFiAlways    = false;
   EA_Config.TM_BEGIN_CALIBRATE = 5;
   EA_Config.SAMPLES_CLIBRATE   = 10;
   EA_Config.TM_LOOP_SENSOR     = 1;
}
