#ifndef _STEPGEN_H_
#define _STEPGEN_H_

int stepgen_init();
void stepgen_cleanup();
void stepgen_makeRealtime();

void stepgen_setFreq(int hz);
void stepgen_genPwm(int n);
void stepgen_genPwmRev(int n);
long stepgen_getCount();

#endif
