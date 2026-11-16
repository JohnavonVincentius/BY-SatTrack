#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include <hal/devconf.h>

enum ledTrigger{
  NONE,
  TIMER,
  HEARTBEAT
};

int writeFile(char* file, char* write){
  FILE *pLedTriggerFile = fopen(file, "w");
  if (pLedTriggerFile == NULL) {
    perror("Error opening LED trigger file");
    return -1;
  }

  int charWritten = fprintf(pLedTriggerFile, write);

  if (charWritten <= 0) {
    perror("Error writing data to LED file");
    return -1;
  }
  fclose(pLedTriggerFile);
  return 0;
}

int ledPWRTrigger(enum ledTrigger Trig){
  int r = -1;
  switch(Trig){
    case NONE:
      r = writeFile(PWR_LED_TRIGGER,"none");
      break;

    case TIMER:
      r = writeFile(PWR_LED_TRIGGER,"timer");
      break;
    case HEARTBEAT:
      r = writeFile(PWR_LED_TRIGGER,"heartbeat");
      break;
  };
  return r;
}

int ledACTTrigger(enum ledTrigger Trig){
  int r = -1;
  switch(Trig){
    case NONE:
      r = writeFile(ACT_LED_TRIGGER,"none");
      break;

    case TIMER:
      r = writeFile(ACT_LED_TRIGGER,"timer");
      break;
    case HEARTBEAT:
      r = writeFile(ACT_LED_TRIGGER,"heartbeat");
      break;
  };
  return r;
}

void ledPWRBlink(unsigned int tOn, unsigned int tOff){
  ledPWRTrigger(TIMER);

  char CharOn[10];
  char CharOff[10];

  snprintf(CharOn, sizeof(CharOn), "%u", tOn);
  snprintf(CharOff, sizeof(CharOff), "%u", tOff);


  writeFile(PWR_LED_DELAY_ON, CharOn);
  writeFile(PWR_LED_DELAY_OFF, CharOff);
}

void ledACTlink(unsigned int tOn, unsigned int tOff){
  ledACTTrigger(TIMER);

  char CharOn[10];
  char CharOff[10];

  snprintf(CharOn, sizeof(CharOn), "%u", tOn);
  snprintf(CharOff, sizeof(CharOff), "%u", tOff);


  writeFile(ACT_LED_DELAY_ON, CharOn);
  writeFile(ACT_LED_DELAY_OFF, CharOff);
}

void ledPWR_on(){
  ledPWRTrigger(NONE);
  writeFile(PWR_LED_BRIGHTNESS, "1");
}

void ledPWR_off(){
  ledPWRTrigger(NONE);
  writeFile(PWR_LED_BRIGHTNESS, "0");
}

void ledACT_on(){
  ledPWRTrigger(NONE);
  writeFile(ACT_LED_BRIGHTNESS, "1");
}

void ledACT_off(){
  ledPWRTrigger(NONE);
  writeFile(ACT_LED_BRIGHTNESS, "0");
}

void ledPWRBeat(){
  ledPWRTrigger(HEARTBEAT);
}

void ledACTBeat(){
  ledACTTrigger(HEARTBEAT);
}