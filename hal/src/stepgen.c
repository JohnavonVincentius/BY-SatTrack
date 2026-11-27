#include <gpiod.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <time.h>

#include <hal/devconf.h>
#include <hal/stepgen.h>

#define NS_PER_SEC 1000000000LL
#define SPIN_NS 150000LL

static struct gpiod_chip *step_chip;
static struct gpiod_chip *dir_chip;
static struct gpiod_line_request *step_req;
static struct gpiod_line_request *dir_req;

static long count = 0;
static pthread_mutex_t count_lock = PTHREAD_MUTEX_INITIALIZER;
static long long half_period_ns = NS_PER_SEC / 1000;

static long long now_ns(){
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return t.tv_sec * NS_PER_SEC + t.tv_nsec;
}

static void sleep_until(long long ns){
  struct timespec t = { ns / NS_PER_SEC, ns % NS_PER_SEC };
  clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
}

static void wait_half_period(long long *next){
  *next += half_period_ns;
  long long now = now_ns();
  if (*next < now) *next = now;
  if (*next - now > SPIN_NS) sleep_until(*next - SPIN_NS);
  while (now_ns() < *next);
}

static struct gpiod_line_request *open_output(struct gpiod_chip *chip, unsigned int line){
  struct gpiod_line_settings *settings = gpiod_line_settings_new();
  gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);
  gpiod_line_settings_set_output_value(settings, GPIOD_LINE_VALUE_INACTIVE);

  struct gpiod_line_config *config = gpiod_line_config_new();
  gpiod_line_config_add_line_settings(config, &line, 1, settings);

  struct gpiod_line_request *req = gpiod_chip_request_lines(chip, NULL, config);

  gpiod_line_config_free(config);
  gpiod_line_settings_free(settings);
  return req;
}

int stepgen_init(){
  step_chip = gpiod_chip_open(STEPPER0_STEP_CHIP);
  dir_chip  = gpiod_chip_open(STEPPER0_DIR_CHIP);
  if (!step_chip || !dir_chip) { perror("stepgen: gpio chip"); return -1; }

  step_req = open_output(step_chip, STEPPER0_STEP_LINE);
  dir_req  = open_output(dir_chip, STEPPER0_DIR_LINE);
  if (!step_req || !dir_req) { perror("stepgen: gpio line"); return -1; }
  return 0;
}

void stepgen_cleanup(){
  gpiod_line_request_release(step_req);
  gpiod_line_request_release(dir_req);
  gpiod_chip_close(step_chip);
  gpiod_chip_close(dir_chip);
}

void stepgen_makeRealtime(){
  struct sched_param param = { .sched_priority = STEPPER0_RT_PRIORITY };
  if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &param) != 0) {
    printf("stepgen: no real-time priority (run with sudo)\n");
  }
}

void stepgen_setFreq(int hz){
  if (hz > 0) half_period_ns = NS_PER_SEC / hz / 2;
}

static void send_pulses(int n, enum gpiod_line_value dir, int change){
  gpiod_line_request_set_value(dir_req, STEPPER0_DIR_LINE, dir);

  long long next = now_ns();
  wait_half_period(&next);

  for (int i = 0; i < n; i++) {
    gpiod_line_request_set_value(step_req, STEPPER0_STEP_LINE, GPIOD_LINE_VALUE_ACTIVE);
    wait_half_period(&next);
    gpiod_line_request_set_value(step_req, STEPPER0_STEP_LINE, GPIOD_LINE_VALUE_INACTIVE);
    wait_half_period(&next);

    pthread_mutex_lock(&count_lock);
    count += change;
    pthread_mutex_unlock(&count_lock);
  }
}

void stepgen_genPwm(int n){
  send_pulses(n, GPIOD_LINE_VALUE_INACTIVE, +1);
}

void stepgen_genPwmRev(int n){
  send_pulses(n, GPIOD_LINE_VALUE_ACTIVE, -1);
}

long stepgen_getCount(){
  pthread_mutex_lock(&count_lock);
  long c = count;
  pthread_mutex_unlock(&count_lock);
  return c;
}
