
#ifndef WC_TASK_h
#define WC_TASK_h

#include "MyConfig.h"
#include "WC_Config.h"
#include "WC_Sensors.h"
#include "WC_Led.h"
#include "WC_HTTP.h"
#include "WC_Event.h"
//#include "src/Slib/SButton.h"
#include "src/Slib/SBTN.h"
#include "src/DFPlayer/DFRobotDFPlayerMini.h"
//#include <WiFi.h>
#include <HTTPClient.h>

#define DEPTH_DIST_ARRAY 5
#define SAMPLE_LEN       10
#define RELIABILITY_PROC 0.15
#define FPSerial Serial1
enum ES_STAT {
  STAT_OFF,
  STAT_BT_ON,
  STAT_BT_OFF,
  STAT_WAIT_ON,
  STAT_WAIT_OFF
};

enum CALIBRATION_MODE_t {
   CM_NONE = 0,
   CM_WAIT = 1,
   CM_ON = 2
};

enum CMD_MP3_t {
   CMP3_NONE   =  0,
   CMP3_PLAY   =  1,
   CMP3_STOP   =  2,
   CMP3_VOLUME =  3
};

//extern DFRobotDFPlayerMini myDFPlayer;

extern TSaveRGB *SaveRGB1, *SaveRGB2;

void tasksStart();
void taskEvents(void *pvParameters);
void taskSensors(void *pvParameters);
void taskButton(void *pvParameters);
void taskNet(void *pvParameters);
void taskMP3(void *pvParameters);
void setVolumeMP3();
void playMP3(int dir, int num);
void stopMP3();


void handleSensor( bool _flag  );
void handleRelay1( bool _flag  );
void handleRelay2( bool _flag  );
void handleRGB1(   bool _flag  );
void handleRGB2(   bool _flag  );
void handleMP3(    bool _flag  );
void handleBusy1(  bool _flag  );
void handleBusy2(  bool _flag  );
void handleCalibrate(bool _flag);
void startCalibrate(uint32_t _delay);
void setEventRGB1(TEVENT_TYPE_t _type, uint32_t _timeOn, uint32_t _timeOff, uint32_t _color1, uint32_t _color2);
void setEventRGB2(TEVENT_TYPE_t _type, uint32_t _timeOn, uint32_t _timeOff, uint32_t _color1, uint32_t _color2);
void setEventMP3( bool _enable, uint32_t _delayOn, int _dir, int _sound, bool _loop, uint32_t _color, uint32_t _tm);
void setEventMP3( JsonObject _config, bool is_delay = true );
void setNanMode();

void handleEventWiFi(arduino_event_id_t event, arduino_event_info_t info);

bool sendHttpParam();


void checkChangeOn();
void processRelay1();
void processRelay2();
void  setRelay1( bool stat);
void  setRelay2( bool stat);
void  setRelayPin(uint8_t pin, bool stat, bool is_inverse);
void printStat(char *msg);
//bool ProcessingCalibrate(uint32_t _tm);
//float CalibrateGround();
#endif
