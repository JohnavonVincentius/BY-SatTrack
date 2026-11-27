// timeutil.h - Julian dates and sidereal time
#ifndef SAT_TIMEUTIL_H
#define SAT_TIMEUTIL_H

#include <time.h>

// Julian date from a UTC calendar date/time (valid 1900-2100).
double jd_from_ymdhms(int year, int mon, int day, int hr, int min, double sec);

// Julian date of the current system clock (UTC). Needs a correct clock:
// run NTP / `timedatectl` on the BeagleY-AI, 1 s error ~ 7 km along-track.
double jd_now(void);

// Julian date <-> Unix time
double jd_from_unix(double unix_seconds);
double unix_from_jd(double jd);

// Parse "YYYY-MM-DDTHH:MM:SS" (UTC, optional trailing Z). Returns 0 on success.
int jd_from_iso(const char *iso, double *jd);

// Format a JD as "YYYY-MM-DD HH:MM:SS" UTC
void jd_to_str(double jd, char *buf, size_t len);

// Greenwich Mean Sidereal Time (IAU-82), radians
double gmst_from_jd(double jd_ut1);

#endif
