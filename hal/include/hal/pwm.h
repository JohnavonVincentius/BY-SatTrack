#ifndef _PWM_H_
#define _PWM_H_

/*
 * All functions take the PWM sysfs directory, e.g. PWM0_GPIO12_DEV
 * ("/dev/hat/pwm/GPIO12"). Return 0 on success, -1 on error.
 */

int pwm_set(const char *dev, double hz, int duty);
int pwm_enable(const char *dev);
int pwm_disable(const char *dev);
int pwm_setPolarity(const char *dev, int flip);      /* 0 = normal, 1 = inversed */

#endif