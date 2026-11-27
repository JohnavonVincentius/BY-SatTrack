// sgp4.h - SGP4 orbit propagator (near-Earth, WGS-72), plain C.
//
// Port of the Vallado/Spacetrack Report #3 (rev. 2006) SGP4 algorithm.
// Deep-space objects (orbital period >= 225 min, e.g. GEO / Molniya) are
// rejected: an antenna tracker for LEO satellites doesn't need SDP4.
#ifndef SAT_SGP4_H
#define SAT_SGP4_H

#include <stdbool.h>

typedef struct {
  // ---- From the TLE ----
  char name[32];
  int norad_id;
  double epoch_jd;      // Julian date (UTC) of TLE epoch
  double bstar;         // drag term (1/earth radii)
  double inclo;         // inclination (rad)
  double nodeo;         // RAAN (rad)
  double ecco;          // eccentricity
  double argpo;         // argument of perigee (rad)
  double mo;            // mean anomaly (rad)
  double no_kozai;      // mean motion (rad/min) as given in TLE

  // ---- Initialised by sgp4_init ----
  int isimp;
  double no;            // un-Kozai'd mean motion (rad/min)
  double ao, con41, cc1, cc4, cc5, d2, d3, d4, delmo, eta, argpdot, omgcof,
      sinmao, t2cof, t3cof, t4cof, t5cof, x1mth2, x7thm1, mdot, nodedot,
      xlcof, xmcof, nodecf, aycof;
  int error;            // 0 = OK, see sgp4_strerror()
} sgp4_sat_t;

// Initialise propagator constants. Returns 0 on success.
int sgp4_init(sgp4_sat_t *s);

// Propagate to `tsince` minutes after epoch.
// r = position (km), v = velocity (km/s) in the TEME frame.
int sgp4_propagate(sgp4_sat_t *s, double tsince, double r[3], double v[3]);

const char *sgp4_strerror(int err);

#endif
