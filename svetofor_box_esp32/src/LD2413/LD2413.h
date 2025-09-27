#pragma once
#include <Arduino.h>

//#define DEBUG_SERIAL Serial
#if CONFIG_IDF_TARGET_ESP32
#define RADAR_SERIAL Serial2
#elif CONFIG_IDF_TARGET_ESP32S3
#define RADAR_SERIAL Serial2
#else
#define RADAR_SERIAL Serial1
#endif

#define RADAR_SPEED  115200L

#define RADAR_SET_MIN_DISTANCE_CMD   0x74
#define RADAR_SET_MAX_DISTANCE_CMD   0x75
#define RADAR_SET_THRESHOLD_CMD      0x72
#define RADAR_SET_TIMEOUT_CMD        0x71
#define RADAR_GET_FIRMWARE_CMD       0x00
#define RADAR_STOP_CMD               0xff
#define RADAR_START_CMD              0xfe


class LD2413 {
   public: 
      LD2413();
      void begin();
      void begin(uint8_t _rx, uint8_t _tx);
      void init(uint16_t _min, uint16_t _max, uint16_t _tm);
	  void calibrate();
      void cmd2( uint8_t _cmd );
      void cmd4( uint8_t _cmd, uint16_t _arg );
      uint16_t wait_cmd(uint32_t _tm=1000);
      float wait_data(uint32_t _tm=1000); 
   private:
      void debug_char(uint8_t _b);

//      HardwareSerial* RadarSerial;
//      HardwareSerial* DebugSerial;   
};


