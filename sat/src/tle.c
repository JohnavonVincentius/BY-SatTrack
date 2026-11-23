// tle.c - Two-Line Element parsing
#include "sat/tle.h"
#include "sat/timeutil.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define DEG2RAD (M_PI / 180.0)
#define XPDOTP (1440.0 / (2.0 * M_PI)) // rev/day -> rad/min

static void rstrip(char *str) {
  size_t n = strlen(str);
  while (n > 0 && isspace((unsigned char)str[n - 1]))
    str[--n] = '\0';
}

// Copy columns [start, start+len) (1-based like the TLE spec) into buf.
static void field(const char *line, int start, int len, char *buf) {
  memcpy(buf, line + start - 1, (size_t)len);
  buf[len] = '\0';
}

static double field_d(const char *line, int start, int len) {
  char buf[32];
  field(line, start, len, buf);
  return atof(buf);
}

// TLE "assumed decimal point" exponent format, e.g. " 12345-3" = 0.12345e-3
static double field_exp(const char *line, int start) {
  char buf[16];
  field(line, start, 8, buf);
  // buf: [sign][5 digits][exp sign][exp digit]
  char mant[16];
  snprintf(mant, sizeof(mant), "%c.%.5s", buf[0] == '-' ? '-' : '+', buf + 1);
  double m = atof(mant);
  int e = atoi(buf + 6);
  return m * pow(10.0, e);
}

static int checksum_ok(const char *line) {
  int sum = 0;
  for (int i = 0; i < 68; i++) {
    char c = line[i];
    if (isdigit((unsigned char)c))
      sum += c - '0';
    else if (c == '-')
      sum += 1;
  }
  return (sum % 10) == (line[68] - '0');
}

int tle_parse(const char *name, const char *line1, const char *line2,
              sgp4_sat_t *s) {
  char l1[130], l2[130];
  snprintf(l1, sizeof(l1), "%s", line1);
  snprintf(l2, sizeof(l2), "%s", line2);
  rstrip(l1);
  rstrip(l2);

  if (strlen(l1) < 69 || strlen(l2) < 69 || l1[0] != '1' || l2[0] != '2') {
    fprintf(stderr, "tle: lines are not a valid TLE (need 69-char lines starting with 1 and 2)\n");
    return -1;
  }
  if (!checksum_ok(l1) || !checksum_ok(l2)) {
    fprintf(stderr, "tle: checksum mismatch (TLE corrupted or mis-copied)\n");
    return -1;
  }

  memset(s, 0, sizeof(*s));
  if (name) {
    snprintf(s->name, sizeof(s->name), "%s", name);
    rstrip(s->name);
    if (s->name[0] == '0' && s->name[1] == ' ') // "0 ISS (ZARYA)" style
      memmove(s->name, s->name + 2, strlen(s->name + 2) + 1);
  }

  s->norad_id = (int)field_d(l1, 3, 5);

  // Epoch: YYDDD.DDDDDDDD
  int yy = (int)field_d(l1, 19, 2);
  double doy = field_d(l1, 21, 12);
  int year = yy < 57 ? 2000 + yy : 1900 + yy;
  s->epoch_jd = jd_from_ymdhms(year, 1, 1, 0, 0, 0.0) + doy - 1.0;

  s->bstar = field_exp(l1, 54);

  s->inclo = field_d(l2, 9, 8) * DEG2RAD;
  s->nodeo = field_d(l2, 18, 8) * DEG2RAD;
  {
    char buf[16];
    char ecc[24];
    field(l2, 27, 7, buf);
    snprintf(ecc, sizeof(ecc), "0.%s", buf);
    s->ecco = atof(ecc);
  }
  s->argpo = field_d(l2, 35, 8) * DEG2RAD;
  s->mo = field_d(l2, 44, 8) * DEG2RAD;
  s->no_kozai = field_d(l2, 53, 11) / XPDOTP;

  if (s->name[0] == '\0')
    snprintf(s->name, sizeof(s->name), "NORAD %d", s->norad_id);

  if (sgp4_init(s) != 0) {
    fprintf(stderr, "tle: %s: %s\n", s->name, sgp4_strerror(s->error));
    return -1;
  }
  return 0;
}

static int is_all_digits(const char *str) {
  if (!*str)
    return 0;
  for (; *str; str++)
    if (!isdigit((unsigned char)*str))
      return 0;
  return 1;
}

static int ci_contains(const char *hay, const char *needle) {
  size_t n = strlen(needle);
  for (; *hay; hay++) {
    size_t i = 0;
    while (i < n && hay[i] &&
           tolower((unsigned char)hay[i]) == tolower((unsigned char)needle[i]))
      i++;
    if (i == n)
      return 1;
  }
  return 0;
}

int tle_load_file(const char *path, const char *select, sgp4_sat_t *s) {
  FILE *f = fopen(path, "r");
  if (!f) {
    perror(path);
    return -1;
  }
  char prev[130] = "", line[130];
  char name[130] = "";
  int result = -1;
  int by_id = select && is_all_digits(select);
  int want_id = by_id ? atoi(select) : 0;

  while (fgets(line, sizeof(line), f)) {
    rstrip(line);
    if (line[0] == '1' && line[1] == ' ') {
      char l2[130];
      if (!fgets(l2, sizeof(l2), f))
        break;
      rstrip(l2);
      // name is the previous line if it wasn't part of another TLE
      snprintf(name, sizeof(name), "%s", (prev[0] && prev[0] != '2') ? prev : "");
      int id = atoi(line + 2);
      int match = !select || !*select || (by_id && id == want_id) ||
                  (!by_id && ci_contains(name, select));
      if (match) {
        result = tle_parse(name[0] ? name : NULL, line, l2, s);
        break;
      }
      snprintf(prev, sizeof(prev), "%s", l2);
      continue;
    }
    snprintf(prev, sizeof(prev), "%s", line);
  }
  fclose(f);
  if (result != 0)
    fprintf(stderr, "tle: no usable TLE matching '%s' in %s\n",
            select && *select ? select : "(first)", path);
  return result;
}
