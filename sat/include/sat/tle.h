// tle.h - Two-Line Element parsing
#ifndef SAT_TLE_H
#define SAT_TLE_H

#include "sat/sgp4.h"

// Parse a TLE (optional name line + line 1 + line 2) into `s` and run
// sgp4_init(). `name` may be NULL. Returns 0 on success, -1 on a format or
// checksum error (message written to stderr).
int tle_parse(const char *name, const char *line1, const char *line2,
              sgp4_sat_t *s);

// Load a TLE from a text file (CelesTrak 3-line format, or bare 2-line).
// `select` chooses the satellite: NULL/"" = first one in the file,
// all digits = NORAD catalogue number, otherwise a case-insensitive
// substring of the name line (e.g. "ISS").
// Returns 0 on success, -1 if not found / invalid.
int tle_load_file(const char *path, const char *select, sgp4_sat_t *s);

#endif
