// observer.c - Ground station and look angles
#include "sat/observer.h"
#include "sat/timeutil.h"

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define DEG2RAD (M_PI / 180.0)
#define RAD2DEG (180.0 / M_PI)

// WGS-84 ellipsoid for the ground station
#define A_WGS84 6378.137
#define F_WGS84 (1.0 / 298.257223563)
#define OMEGA_E 7.292115146706979e-5 // earth rotation, rad/s

static void observer_ecef(const observer_t *o, double r[3]) {
  double lat = o->lat_deg * DEG2RAD, lon = o->lon_deg * DEG2RAD;
  double h = o->alt_m / 1000.0;
  double e2 = F_WGS84 * (2.0 - F_WGS84);
  double sl = sin(lat);
  double n = A_WGS84 / sqrt(1.0 - e2 * sl * sl);
  r[0] = (n + h) * cos(lat) * cos(lon);
  r[1] = (n + h) * cos(lat) * sin(lon);
  r[2] = (n * (1.0 - e2) + h) * sl;
}

static void ecef_to_geodetic(const double r[3], double *lat, double *lon,
                             double *alt) {
  double e2 = F_WGS84 * (2.0 - F_WGS84);
  double p = sqrt(r[0] * r[0] + r[1] * r[1]);
  double la = atan2(r[2], p * (1.0 - e2));
  double n = A_WGS84;
  for (int i = 0; i < 6; i++) {
    double sl = sin(la);
    n = A_WGS84 / sqrt(1.0 - e2 * sl * sl);
    la = atan2(r[2] + n * e2 * sl, p);
  }
  *lat = la * RAD2DEG;
  *lon = atan2(r[1], r[0]) * RAD2DEG;
  *alt = p / cos(la) - n;
}

int look_angles(sgp4_sat_t *s, const observer_t *obs, double jd, look_t *out) {
  double r_teme[3], v_teme[3];
  double tsince = (jd - s->epoch_jd) * 1440.0;
  int err = sgp4_propagate(s, tsince, r_teme, v_teme);
  if (err)
    return err;

  // TEME -> ECEF (pseudo earth-fixed; polar motion ignored, < 0.01 deg)
  double g = gmst_from_jd(jd);
  double cg = cos(g), sg = sin(g);
  double r[3] = {cg * r_teme[0] + sg * r_teme[1],
                 -sg * r_teme[0] + cg * r_teme[1], r_teme[2]};
  double v[3] = {cg * v_teme[0] + sg * v_teme[1] + OMEGA_E * r[1],
                 -sg * v_teme[0] + cg * v_teme[1] - OMEGA_E * r[0], v_teme[2]};

  double ro[3];
  observer_ecef(obs, ro);
  double d[3] = {r[0] - ro[0], r[1] - ro[1], r[2] - ro[2]};

  // ECEF -> topocentric South-East-Zenith
  double lat = obs->lat_deg * DEG2RAD, lon = obs->lon_deg * DEG2RAD;
  double sla = sin(lat), cla = cos(lat), slo = sin(lon), clo = cos(lon);
  double south = sla * clo * d[0] + sla * slo * d[1] - cla * d[2];
  double east = -slo * d[0] + clo * d[1];
  double zen = cla * clo * d[0] + cla * slo * d[1] + sla * d[2];

  double range = sqrt(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]);
  double az = atan2(east, -south) * RAD2DEG;
  if (az < 0.0)
    az += 360.0;

  out->jd = jd;
  out->az_deg = az;
  out->el_deg = asin(zen / range) * RAD2DEG;
  out->range_km = range;
  out->range_rate = (d[0] * v[0] + d[1] * v[1] + d[2] * v[2]) / range;
  ecef_to_geodetic(r, &out->lat_deg, &out->lon_deg, &out->alt_km);
  return 0;
}

static double elev_at(sgp4_sat_t *s, const observer_t *o, double jd) {
  look_t l;
  if (look_angles(s, o, jd, &l))
    return -90.0;
  return l.el_deg;
}

// Bisect for the horizon crossing between t0 (el sign a) and t1.
static double crossing(sgp4_sat_t *s, const observer_t *o, double t0, double t1) {
  double e0 = elev_at(s, o, t0);
  for (int i = 0; i < 30; i++) { // 30 halvings of 30 s -> sub-ms
    double tm = 0.5 * (t0 + t1);
    double em = elev_at(s, o, tm);
    if ((em >= 0.0) == (e0 >= 0.0)) {
      t0 = tm;
      e0 = em;
    } else {
      t1 = tm;
    }
  }
  return 0.5 * (t0 + t1);
}

int next_pass(sgp4_sat_t *s, const observer_t *o, double jd0, double hours,
              double min_el, pass_t *p) {
  const double step = 30.0 / 86400.0; // 30 s
  double jd_end = jd0 + hours / 24.0;
  double t = jd0;

  // If currently up, walk back to find the rise time
  if (elev_at(s, o, t) >= 0.0) {
    double tb = t;
    while (elev_at(s, o, tb) >= 0.0 && t - tb < 0.1)
      tb -= step;
    t = tb;
  }

  while (t < jd_end) {
    // find rise
    double prev = elev_at(s, o, t);
    while (t < jd_end) {
      double e = elev_at(s, o, t + step);
      if (prev < 0.0 && e >= 0.0)
        break;
      prev = e;
      t += step;
    }
    if (t >= jd_end)
      return -1;
    double aos = crossing(s, o, t, t + step);

    // walk through pass, track max, find set
    double tmax = aos, emax = 0.0, tt = aos;
    while (1) {
      tt += step;
      double e = elev_at(s, o, tt);
      if (e > emax) {
        emax = e;
        tmax = tt;
      }
      if (e < 0.0)
        break;
      if (tt - aos > 0.1) // > 2.4 h above horizon: not a LEO pass
        break;
    }
    double los = crossing(s, o, tt - step, tt);

    // refine TCA with golden-section search
    double a = tmax - step, b = tmax + step;
    const double gr = 0.6180339887498949;
    for (int i = 0; i < 40; i++) {
      double c = b - gr * (b - a), d = a + gr * (b - a);
      if (elev_at(s, o, c) > elev_at(s, o, d))
        b = d;
      else
        a = c;
    }
    tmax = 0.5 * (a + b);
    emax = elev_at(s, o, tmax);

    if (emax >= min_el) {
      look_t la, ll;
      look_angles(s, o, aos, &la);
      look_angles(s, o, los, &ll);
      p->aos_jd = aos;
      p->los_jd = los;
      p->tca_jd = tmax;
      p->max_el_deg = emax;
      p->aos_az_deg = la.az_deg;
      p->los_az_deg = ll.az_deg;
      return 0;
    }
    t = los + step;
  }
  return -1;
}
