// Dumps TEME position/velocity for verification against reference SGP4.
#include "sat/tle.h"
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char **argv) {
  sgp4_sat_t s;
  if (argc < 3 || tle_load_file(argv[1], argv[2], &s)) return 1;
  for (int i = 3; i < argc; i++) {
    double t = atof(argv[i]), r[3], v[3];
    int e = sgp4_propagate(&s, t, r, v);
    printf("%g %d %.8f %.8f %.8f %.9f %.9f %.9f\n", t, e, r[0], r[1], r[2], v[0], v[1], v[2]);
  }
  return 0;
}
