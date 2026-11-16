#include <stdint.h>

#include <hal/devconf.h>
#include <hal/eqep.h>
#include <hal/encoder.h>

static long last_raw;
static long accum;      
static long value;    
static long max_value;

static long clamp(long v){
  if (v < 0)         return 0;
  if (v > max_value) return max_value;
  return v;
}

int encoder_init(long max, long start){
  if (max < 0) return -1;
  if (eqep0_INIT(quadrature) < 0) return -1;

  max_value = max;
  value     = clamp(start);
  accum     = 0;

  last_raw = eqep0_getCount();
  return (last_raw < 0) ? -1 : 0;
}

int encoder_update(){
  long raw = eqep0_getCount();
  if (raw < 0) return -1;

  int32_t delta = (int32_t)((uint32_t)raw - (uint32_t)last_raw);
  last_raw = raw;
  if (delta == 0) return 0;

  accum += delta;
  long steps = accum / ENCODER_COUNTS_PER_STEP;
  accum -= steps * ENCODER_COUNTS_PER_STEP;
  if (steps == 0) return 0;

  long old = value;
  value = clamp(value + steps);
  return value != old;
}

long encoder_getValue(){
  return value; 
}

void encoder_setValue(long v){ 
  value = clamp(v); 
  accum = 0; 
}

long encoder_getMax(){
  return max_value; 
}

void encoder_setMax(long max){
  if (max >= 0){
    max_value = max; 
    value = clamp(value); 
  } 
}