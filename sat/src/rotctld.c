// rotctld.c - Minimal Hamlib rotctld-compatible TCP server (see rotctld.h)
#define _POSIX_C_SOURCE 200809L
#include "sat/rotctld.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

int rotctld_open(rotctld_t *r, int port) {
  memset(r, 0, sizeof(*r));
  r->client_fd = -1;
  r->listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (r->listen_fd < 0) {
    perror("socket");
    return -1;
  }
  int yes = 1;
  setsockopt(r->listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
  struct sockaddr_in a = {0};
  a.sin_family = AF_INET;
  a.sin_addr.s_addr = htonl(INADDR_ANY);
  a.sin_port = htons((unsigned short)port);
  if (bind(r->listen_fd, (struct sockaddr *)&a, sizeof(a)) < 0 ||
      listen(r->listen_fd, 1) < 0) {
    perror("bind/listen");
    close(r->listen_fd);
    return -1;
  }
  return 0;
}

void rotctld_close(rotctld_t *r) {
  if (r->client_fd >= 0)
    close(r->client_fd);
  if (r->listen_fd >= 0)
    close(r->listen_fd);
  r->client_fd = r->listen_fd = -1;
}

static void reply(rotctld_t *r, const char *s) {
  if (r->client_fd >= 0)
    (void)!write(r->client_fd, s, strlen(s));
}

static void drop_client(rotctld_t *r) {
  close(r->client_fd);
  r->client_fd = -1;
  r->rxlen = 0;
}

// GPredict running under a comma-decimal locale may send "P 123,45 10,00"
static double parse_num(char **p) {
  char *s = *p;
  while (*s == ' ' || *s == '\t')
    s++;
  char tmp[32];
  int i = 0;
  while (*s && *s != ' ' && *s != '\t' && i < 31) {
    tmp[i++] = (*s == ',') ? '.' : *s;
    s++;
  }
  tmp[i] = '\0';
  *p = s;
  return strtod(tmp, NULL);
}

static rot_event_t handle_line(rotctld_t *r, char *line, double *az, double *el) {
  char buf[96];
  while (*line == ' ')
    line++;

  if (line[0] == 'P' || strncmp(line, "\\set_pos", 8) == 0) {
    char *p = line + (line[0] == 'P' ? 1 : 8);
    *az = parse_num(&p);
    *el = parse_num(&p);
    reply(r, "RPRT 0\n");
    return ROT_EV_SET_POS;
  }
  if (line[0] == 'p' || strncmp(line, "\\get_pos", 8) == 0) {
    snprintf(buf, sizeof(buf), "%.2f\n%.2f\n", r->cur_az, r->cur_el);
    reply(r, buf);
    return ROT_EV_NONE;
  }
  if (line[0] == 'S' || strncmp(line, "\\stop", 5) == 0) {
    reply(r, "RPRT 0\n");
    return ROT_EV_STOP;
  }
  if (line[0] == 'q' || line[0] == 'Q') {
    drop_client(r);
    return ROT_EV_DISCONNECT;
  }
  if (line[0] == '_' || strncmp(line, "\\get_info", 9) == 0) {
    reply(r, "BeagleY-AI SatTrack\n");
    return ROT_EV_NONE;
  }
  if (line[0] == '\0')
    return ROT_EV_NONE;
  reply(r, "RPRT -4\n"); // not implemented
  return ROT_EV_NONE;
}

rot_event_t rotctld_poll(rotctld_t *r, int timeout_ms, double *az, double *el) {
  // A complete line may already be buffered from a previous read
  char *nl = memchr(r->rxbuf, '\n', (size_t)r->rxlen);
  if (!nl) {
    fd_set rf;
    FD_ZERO(&rf);
    int maxfd = r->listen_fd;
    FD_SET(r->listen_fd, &rf);
    if (r->client_fd >= 0) {
      FD_SET(r->client_fd, &rf);
      if (r->client_fd > maxfd)
        maxfd = r->client_fd;
    }
    struct timeval tv = {timeout_ms / 1000, (timeout_ms % 1000) * 1000};
    int n = select(maxfd + 1, &rf, NULL, NULL, &tv);
    if (n <= 0)
      return ROT_EV_NONE;

    if (FD_ISSET(r->listen_fd, &rf)) {
      int fd = accept(r->listen_fd, NULL, NULL);
      if (fd >= 0) {
        if (r->client_fd >= 0)
          drop_client(r); // one client at a time; newest wins
        r->client_fd = fd;
        r->rxlen = 0;
        return ROT_EV_CONNECT;
      }
    }
    if (r->client_fd >= 0 && FD_ISSET(r->client_fd, &rf)) {
      ssize_t k = read(r->client_fd, r->rxbuf + r->rxlen,
                       sizeof(r->rxbuf) - 1 - (size_t)r->rxlen);
      if (k <= 0) {
        drop_client(r);
        return ROT_EV_DISCONNECT;
      }
      r->rxlen += (int)k;
      if (r->rxlen >= (int)sizeof(r->rxbuf) - 1)
        r->rxlen = 0; // garbage, discard
    }
    nl = memchr(r->rxbuf, '\n', (size_t)r->rxlen);
    if (!nl)
      return ROT_EV_NONE;
  }

  *nl = '\0';
  if (nl > r->rxbuf && nl[-1] == '\r')
    nl[-1] = '\0';
  char line[512];
  snprintf(line, sizeof(line), "%s", r->rxbuf);
  int used = (int)(nl - r->rxbuf) + 1;
  memmove(r->rxbuf, r->rxbuf + used, (size_t)(r->rxlen - used));
  r->rxlen -= used;
  return handle_line(r, line, az, el);
}
