#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <hal/devconf.h>
#include <hal/eqep.h>

#define EQEP_CEIL_MAX 4294967295L
 
static const char *func_names[] = {
  [quadrature] = "quadrature x4",
  [pulse_dir]  = "pulse-direction",
  [increase]   = "increase",
  [decrease]   = "decrease",
};

static int eqep_write(const char *path, const char *value)
{
  FILE *f = fopen(path, "w");
  if (!f) { perror(path); return -1; }

  int ok = fputs(value, f) >= 0;
  ok &= fclose(f) == 0;           /* sysfs reports rejected values on close */
  if (!ok) { perror(path); return -1; }
  return 0;
}
 
static int eqep_read(const char *path, char *buf, int len)
{
  FILE *f = fopen(path, "r");
  if (!f) { perror(path); return -1; }

  if (!fgets(buf, len, f)) { perror(path); fclose(f); return -1; }
  fclose(f);

  buf[strcspn(buf, "\n")] = '\0';
  return 0;
}

#ifdef EQEP0_CTR0_DEV
  int eqep0_INIT(enum eqepFunction func){
    if (func < quadrature || func > decrease) return -1;
 
    eqep0_disable();
    if (eqep_write(EQEP0_CTR0_FUNCTION, func_names[func]) < 0) return -1;
    if (eqep0_setCelling(EQEP_CEIL_MAX) < 0) return -1;
    if (eqep_write(EQEP0_CTR0_COUNT, "0") < 0) return -1;
    eqep0_enable();
    return 0;
  }

  int eqep0_setCelling(long ceil){
    if (ceil < 0 || ceil > EQEP_CEIL_MAX) {
      fprintf(stderr, "eqep0: ceiling %ld out of range (0..%ld)\n", ceil, EQEP_CEIL_MAX);
      return -1;
    }
 
    char buf[16];
    snprintf(buf, sizeof buf, "%ld", ceil);
    return eqep_write(EQEP0_CTR0_CEILING, buf);
  }

  long eqep0_getCount(){
    char buf[16];
    if (eqep_read(EQEP0_CTR0_COUNT, buf, sizeof buf) < 0) return -1;
    return strtol(buf, NULL, 10);
  }

  void eqep0_disable(){
    eqep_write(EQEP0_CTR0_ENABLE, "0");
  }

  void eqep0_enable(){
    eqep_write(EQEP0_CTR0_ENABLE, "1");
  }
#endif