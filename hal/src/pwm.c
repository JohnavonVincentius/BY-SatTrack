#include <hal/pwm.h>

#include <stdio.h>

#define NS_PER_SEC 1000000000.0

static int pwm_write(const char* dev, const char* attr, const char* value) 
{
  char path[128];
  snprintf(path, sizeof path, "%s/%s", dev, attr);

  FILE* f = fopen(path, "w");
  if (!f) {
    perror(path);
    return -1;
  }

  int ok = fputs(value, f) >= 0;
  ok &= fclose(f) == 0; /* sysfs reports rejected values on close */
  if (!ok) {
    perror(path);
    return -1;
  }
  return 0;
}

static int pwm_writeLong(const char* dev, const char* attr, long value) 
{
  char buf[24];
  snprintf(buf, sizeof buf, "%ld", value);
  return pwm_write(dev, attr, buf);
}


int pwm_set(const char *dev, double hz, int duty){
  if (hz <= 0) return -1;
  if (duty < 0)   duty = 0;
  if (duty > 100) duty = 100;
 
  long period  = (long)(NS_PER_SEC / hz + 0.5);
  long duty_ns = period * duty / 100;
 
  if (pwm_writeLong(dev, "duty_cycle", 0) < 0)   return -1;
  if (pwm_writeLong(dev, "period", period) < 0)  return -1;
  return pwm_writeLong(dev, "duty_cycle", duty_ns);
}

int pwm_enable(const char* dev) { return pwm_write(dev, "enable", "1"); }

int pwm_disable(const char* dev) { return pwm_write(dev, "enable", "0"); }

int pwm_setPolarity(const char* dev, int flip) {
  return pwm_write(dev, "polarity", flip ? "inversed" : "normal");
}