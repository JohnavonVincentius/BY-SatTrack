// rotctld.h - Minimal Hamlib rotctld-compatible TCP server.
//
// Lets GPredict (Radio/Rotator control -> Rotators -> Add, host = BeagleY IP,
// port 4533) drive the tracker directly. GPredict sends:
//   "P <az> <el>\n"  set position      -> we reply "RPRT 0\n"
//   "p\n"            get position      -> we reply "<az>\n<el>\n"
//   "S\n"            stop              -> "RPRT 0\n"
//   "q\n"            close connection
#ifndef SAT_ROTCTLD_H
#define SAT_ROTCTLD_H

#include <stdbool.h>

typedef enum {
  ROT_EV_NONE = 0,
  ROT_EV_SET_POS,     // new target in az/el
  ROT_EV_STOP,
  ROT_EV_CONNECT,
  ROT_EV_DISCONNECT,
} rot_event_t;

typedef struct {
  int listen_fd;
  int client_fd;
  char rxbuf[512];
  int rxlen;
  // Position reported back to GPredict on "p". Update this from the real
  // encoders / ADC once the hardware is in the loop.
  double cur_az, cur_el;
} rotctld_t;

int rotctld_open(rotctld_t *r, int port);  // 0 on success
void rotctld_close(rotctld_t *r);

// Wait up to timeout_ms for activity; handles one command and returns what
// happened. For ROT_EV_SET_POS, *az / *el hold the requested position.
rot_event_t rotctld_poll(rotctld_t *r, int timeout_ms, double *az, double *el);

#endif
