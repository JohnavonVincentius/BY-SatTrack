// timeutil.c - Julian dates and sidereal time
#define _POSIX_C_SOURCE 200809L
#include "sat/timeutil.h"

#include <math.h>
#include <stdio.h>
#include <time.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

double jd_from_ymdhms(int year, int mon, int day, int hr, int min, double sec) {
  return 367.0 * year - floor((7 * (year + floor((mon + 9) / 12.0))) * 0.25) +
         floor(275 * mon / 9.0) + day + 1721013.5 +
         ((sec / 60.0 + min) / 60.0 + hr) / 24.0;
}

double jd_from_unix(double t) { return t / 86400.0 + 2440587.5; }
double unix_from_jd(double jd) { return (jd - 2440587.5) * 86400.0; }

double jd_now(void) {
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  return jd_from_unix((double)ts.tv_sec + ts.tv_nsec * 1e-9);
}

int jd_from_iso(const char *iso, double *jd) {
  int y, mo, d, h = 0, mi = 0;
  double s = 0.0;
  int n = sscanf(iso, "%d-%d-%d%*[T ]%d:%d:%lf", &y, &mo, &d, &h, &mi, &s);
  if (n < 3)
    return -1;
  *jd = jd_from_ymdhms(y, mo, d, h, mi, s);
  return 0;
}

void jd_to_str(double jd, char *buf, size_t len) {
  time_t t = (time_t)llround(unix_from_jd(jd));
  struct tm tmv;
  gmtime_r(&t, &tmv);
  strftime(buf, len, "%Y-%m-%d %H:%M:%S", &tmv);
}

double gmst_from_jd(double jd_ut1) {
  double tut1 = (jd_ut1 - 2451545.0) / 36525.0;
  double g = -6.2e-6 * tut1 * tut1 * tut1 + 0.093104 * tut1 * tut1 +
             (876600.0 * 3600.0 + 8640184.812866) * tut1 + 67310.54841; // s
  g = fmod(g * (M_PI / 180.0) / 240.0, 2.0 * M_PI);
  if (g < 0.0)
    g += 2.0 * M_PI;
  return g;
}
