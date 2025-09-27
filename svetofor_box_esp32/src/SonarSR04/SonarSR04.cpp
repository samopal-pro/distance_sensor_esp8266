
#include "SonarSR04.h"

// Constructors ////////////////////////////////////////////////////////////////

SonarSR04::SonarSR04(uint8_t _echo, uint8_t _trig, uint32_t _tm1, uint32_t _tm2){
    pinEcho = _echo;
    pinTrig = _trig;
    tm1     = _tm1;
    tm2     = _tm2;
}

void SonarSR04::init(){
    pinMode(pinTrig, OUTPUT);
    pinMode(pinEcho, INPUT);
    digitalWrite(pinTrig, LOW); 
}

float SonarSR04::getDistance(){
//    pinMode(pinTrig, OUTPUT);
//    pinMode(pinEcho, INPUT);
    digitalWrite(pinTrig, LOW);
    delayMicroseconds(tm1);
    digitalWrite(pinTrig, HIGH);
    delayMicroseconds(tm2);
    digitalWrite(pinTrig, LOW);

    uint32_t _dur = pulseIn(pinEcho, HIGH, TM_ECHO);
    float _dist = (float)_dur/5.8;

//    Serial.printf("!!! SR04 %d %d %d\n",(int)pinEcho,(int)pinTrig,(int)_dist);
    return _dist;
}

