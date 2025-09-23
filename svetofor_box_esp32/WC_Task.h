
#ifndef WC_TASK_h
#define WC_TASK_h

#include "MyConfig.h"
#include "WC_Config.h"
#include "WC_Sensors.h"
#include "WC_Led.h"
#include "WC_HTTP.h"
#include "src/Slib/SButton.h"
#define DEPTH_DIST_ARRAY 5
#define SAMPLE_LEN       10
#define RELIABILITY_PROC 0.15

enum ES_STAT {
  STAT_OFF,
  STAT_BT_ON,
  STAT_BT_OFF,
  STAT_WAIT_ON,
  STAT_WAIT_OFF
};
void tasksStart();
void taskSensors(void *pvParameters);
void taskButton(void *pvParameters);
void taskNet(void *pvParameters);

void checkChangeOn();
void processRelay1();
void processRelay2();
void  setRelay1( bool stat);
void  setRelay2( bool stat);
void  setRelayPin(uint8_t pin, bool stat, bool is_inverse);
void printStat(char *msg);
bool ProcessingCalibrate(uint32_t _tm);
float CalibrateGround();
#endif
