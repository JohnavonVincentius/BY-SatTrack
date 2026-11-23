// observer.h - Ground station and look angles (azimuth / elevation)
#ifndef SAT_OBSERVER_H
#define SAT_OBSERVER_H

#include "sat/sgp4.h"

typedef struct {
  double lat_deg;  // geodetic latitude, +N
  double lon_deg;  // longitude, +E (west is negative)
  double alt_m;    // height above WGS-84 ellipsoid, metres
} observer_t;

typedef struct {
  double jd;          // time of the solution (UTC)
  double az_deg;      // azimuth, 0 = North, 90 = East (0..360)
  double el_deg;      // elevation above horizon (-90..90)
  double range_km;    // slant range
  double range_rate;  // km/s, + = moving away (Doppler: f*(1 - rr/c))
  double lat_deg, lon_deg, alt_km; // sub-satellite point
} look_t;

// Propagate `s` to `jd` and compute look angles from `obs`.
// Returns 0 on success, SGP4 error code otherwise.
int look_angles(sgp4_sat_t *s, const observer_t *obs, double jd, look_t *out);

typedef struct {
  double aos_jd, los_jd, tca_jd; // rise, set, time of max elevation
  double max_el_deg;
  double aos_az_deg, los_az_deg;
} pass_t;

// Find the next pass with max elevation >= min_el_deg starting at jd_start,
// searching up to `hours` ahead. If the satellite is already up, aos_jd is
// the (past) rise time. Returns 0 if found, -1 if none.
int next_pass(sgp4_sat_t *s, const observer_t *obs, double jd_start,
              double hours, double min_el_deg, pass_t *p);

#endif
