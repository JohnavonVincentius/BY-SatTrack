// sat_test - print live satellite azimuth / elevation for the tracker.
//
// Mode 1 (standalone, from a TLE file):
//   sat_test tle --tle tle/amateur.txt  (see tle/fetch_tle.sh) --sat ISS [--lat 49.2781 --lon -122.9199 --alt 370]
//            [--interval 1] [--count 0] [--time 2026-11-23T05:00:00Z] [--passes 3]
//
// Mode 2 (GPredict does the orbit math, we act as its rotator):
//   sat_test gpredict [--port 4533]
//   then in GPredict: Edit > Preferences > Interfaces > Rotators > Add
//   host = <BeagleY IP>, port = 4533, Az type 0..360, el 0..90.
//   Open a module > menu > Antenna Control, pick the sat, press Track + Engage.
#define _POSIX_C_SOURCE 200809L
#include "sat/observer.h"
#include "sat/rotctld.h"
#include "sat/sgp4.h"
#include "sat/timeutil.h"
#include "sat/tle.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Default ground station: SFU Burnaby campus (change with --lat/--lon/--alt)
#define DEF_LAT 49.2781
#define DEF_LON -122.9199
#define DEF_ALT 370.0

static volatile sig_atomic_t g_stop = 0;
static void on_sigint(int sig) {
  (void)sig;
  g_stop = 1;
}

static void sleep_until_unix(double t) {
  struct timespec ts;
  ts.tv_sec = (time_t)t;
  ts.tv_nsec = (long)((t - (double)ts.tv_sec) * 1e9);
  while (clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &ts, NULL) != 0 && !g_stop)
    ;
}

static void usage(const char *prog) {
  fprintf(stderr,
          "usage:\n"
          "  %s tle --tle FILE [--sat NAME|NORAD] [--lat D] [--lon D] [--alt M]\n"
          "         [--interval SEC] [--count N] [--time ISO8601] [--passes N]\n"
          "  %s gpredict [--port 4533]\n",
          prog, prog);
}

static int run_tle(int argc, char **argv) {
  const char *tle_path = NULL, *select = NULL, *time_iso = NULL;
  observer_t obs = {DEF_LAT, DEF_LON, DEF_ALT};
  double interval = 1.0;
  long count = 0; // 0 = forever
  int passes = 3;

  for (int i = 2; i < argc; i++) {
    const char *a = argv[i];
    const char *v = (i + 1 < argc) ? argv[i + 1] : NULL;
    if (!v) {
      usage(argv[0]);
      return 2;
    }
    if (!strcmp(a, "--tle")) tle_path = v;
    else if (!strcmp(a, "--sat")) select = v;
    else if (!strcmp(a, "--lat")) obs.lat_deg = atof(v);
    else if (!strcmp(a, "--lon")) obs.lon_deg = atof(v);
    else if (!strcmp(a, "--alt")) obs.alt_m = atof(v);
    else if (!strcmp(a, "--interval")) interval = atof(v);
    else if (!strcmp(a, "--count")) count = atol(v);
    else if (!strcmp(a, "--time")) time_iso = v;
    else if (!strcmp(a, "--passes")) passes = atoi(v);
    else {
      usage(argv[0]);
      return 2;
    }
    i++;
  }
  if (!tle_path) {
    usage(argv[0]);
    return 2;
  }

  sgp4_sat_t sat;
  if (tle_load_file(tle_path, select, &sat) != 0)
    return 1;

  // Simulated clock (--time) runs from the given instant; otherwise wall clock
  double jd_start = jd_now();
  double jd_sim0 = jd_start;
  int simulated = 0;
  if (time_iso) {
    if (jd_from_iso(time_iso, &jd_sim0) != 0) {
      fprintf(stderr, "bad --time, use YYYY-MM-DDTHH:MM:SSZ\n");
      return 2;
    }
    simulated = 1;
  }

  char buf[32], buf2[32], buf3[32];
  jd_to_str(sat.epoch_jd, buf, sizeof(buf));
  double age = jd_sim0 - sat.epoch_jd;
  printf("Satellite : %s (NORAD %d)\n", sat.name, sat.norad_id);
  printf("TLE epoch : %s UTC  (age %.1f days%s)\n", buf, age,
         age > 7.0 ? "  ** STALE: download a fresh TLE **" : "");
  printf("Observer  : lat %.4f  lon %.4f  alt %.0f m\n", obs.lat_deg,
         obs.lon_deg, obs.alt_m);
  printf("Period    : %.1f min\n", 2.0 * 3.14159265358979 / sat.no);

  // Upcoming passes
  double tp = jd_sim0;
  for (int k = 0; k < passes; k++) {
    pass_t p;
    if (next_pass(&sat, &obs, tp, 48.0, 0.0, &p) != 0) {
      if (k == 0)
        printf("No pass in the next 48 h.\n");
      break;
    }
    jd_to_str(p.aos_jd, buf, sizeof(buf));
    jd_to_str(p.tca_jd, buf2, sizeof(buf2));
    jd_to_str(p.los_jd, buf3, sizeof(buf3));
    printf("Pass %d    : AOS %s (az %5.1f)  max el %4.1f @ %s  LOS %s (az %5.1f)\n",
           k + 1, buf, p.aos_az_deg, p.max_el_deg, buf2 + 11, buf3 + 11,
           p.los_az_deg);
    tp = p.los_jd + 60.0 / 86400.0;
  }
  printf("\n%-19s  %8s  %7s  %9s  %8s  %s\n", "UTC", "Az(deg)", "El(deg)",
         "Range(km)", "RR(km/s)", "Status");

  double t_wall0 = unix_from_jd(jd_start);
  // Align the first tick to a whole second on the wall clock
  double next_tick = (double)(long)t_wall0 + 1.0;
  for (long n = 0; (count == 0 || n < count) && !g_stop; n++) {
    double jd;
    if (simulated)
      jd = jd_sim0 + n * interval / 86400.0;
    else {
      sleep_until_unix(next_tick);
      next_tick += interval;
      jd = jd_now();
    }
    look_t l;
    int err = look_angles(&sat, &obs, jd, &l);
    if (err) {
      fprintf(stderr, "propagation error: %s\n", sgp4_strerror(err));
      return 1;
    }
    jd_to_str(jd, buf, sizeof(buf));
    printf("%-19s  %8.2f  %7.2f  %9.1f  %8.3f  %s\n", buf, l.az_deg,
           l.el_deg, l.range_km, l.range_rate,
           l.el_deg >= 0.0 ? "VISIBLE" : "below horizon");
    fflush(stdout);
    // TODO hardware: if (l.el_deg >= 0) rotator_goto(l.az_deg, l.el_deg);
  }
  return 0;
}

static int run_gpredict(int argc, char **argv) {
  int port = 4533;
  for (int i = 2; i + 1 < argc; i += 2)
    if (!strcmp(argv[i], "--port"))
      port = atoi(argv[i + 1]);

  rotctld_t rot;
  if (rotctld_open(&rot, port) != 0)
    return 1;
  printf("rotctld server listening on port %d - point GPredict's rotator at this host.\n",
         port);
  fflush(stdout);

  char buf[32];
  while (!g_stop) {
    double az, el;
    rot_event_t ev = rotctld_poll(&rot, 200, &az, &el);
    jd_to_str(jd_now(), buf, sizeof(buf));
    switch (ev) {
    case ROT_EV_CONNECT:
      printf("%s  GPredict connected\n", buf);
      break;
    case ROT_EV_DISCONNECT:
      printf("%s  GPredict disconnected\n", buf);
      break;
    case ROT_EV_STOP:
      printf("%s  STOP\n", buf);
      break;
    case ROT_EV_SET_POS: {
      double az360 = az < 0.0 ? az + 360.0 : az; // GPredict may use -180..180
      printf("%s  target  Az %7.2f  El %6.2f\n", buf, az360, el);
      // No hardware yet: report that we "arrived" instantly.
      // TODO hardware: drive motors, then set rot.cur_* from encoder/ADC.
      rot.cur_az = az;
      rot.cur_el = el;
      break;
    }
    default:
      break;
    }
    fflush(stdout);
  }
  rotctld_close(&rot);
  return 0;
}

int main(int argc, char **argv) {
  signal(SIGINT, on_sigint);
  signal(SIGTERM, on_sigint);
  signal(SIGPIPE, SIG_IGN); // GPredict closing the socket mustn't kill us
  if (argc < 2) {
    usage(argv[0]);
    return 2;
  }
  if (!strcmp(argv[1], "tle"))
    return run_tle(argc, argv);
  if (!strcmp(argv[1], "gpredict"))
    return run_gpredict(argc, argv);
  usage(argv[0]);
  return 2;
}
