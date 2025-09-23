#pragma once
#include <Arduino.h>

#define TM_ECHO                250000        //Таймаут pulseIn в мкс


class SonarSR04
{
  public:
 

   
    uint8_t last_status;

    SonarSR04(uint8_t _echo, uint8_t _trig, uint32_t _tm1 = 2, uint32_t _tm2 = 10);
    uint16_t getDistance();
  
    void init();


  private:
     uint8_t pinTrig;
     uint8_t pinEcho;
     uint32_t tm1;
     uint32_t tm2;
};
