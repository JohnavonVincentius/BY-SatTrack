/*
 * wire the STEP pin to GPIO16 (EQEP0_A, HAT-36), encoder unplugged.
 * the eqep counts every rising edge in hardware, so it checks the real pulses.
 */
#include <stdio.h>
#include <time.h>

#include <hal/devconf.h>
#include <hal/eqep.h>
#include <hal/stepgen.h>

static double now_ms(){
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return t.tv_sec * 1000.0 + t.tv_nsec / 1e6;
}

static int run_test(int n, int hz, int reverse){
  stepgen_setFreq(hz);
  long pulses_before = eqep0_getCount();
  long count_before  = stepgen_getCount();

  double start = now_ms();
  if (reverse) stepgen_genPwmRev(n);
  else         stepgen_genPwm(n);
  double time_taken = now_ms() - start;

  long edges = eqep0_getCount() - pulses_before;
  long pulses = edges / 2;
  long change = stepgen_getCount() - count_before;
  long expected_change = reverse ? -n : n;

  int pass = (edges == 2 * n) && (change == expected_change);
  printf("%s  %s  n=%-5d f=%-5d  pulses=%-5ld  count change=%-6ld  time=%.1fms (expect %.1f)\n",
         pass ? "pass" : "FAIL", reverse ? "rev" : "fwd", n, hz, pulses, change,
         time_taken, n * 1000.0 / hz);
  return pass;
}

int main(){
  int tests[][2] = {
    // { pulses, frequency }
    { 1, 200 },
    { 10, 200 },
    { 200, 500 },
    { 1000, 1000 },
    { 2000, 2000 },
    { 4000, 5000 },
  };
  int num_tests = sizeof(tests) / sizeof(tests[0]);

  if (eqep0_INIT(increase) < 0) return 1;
  if (stepgen_init() < 0) return 1;
  stepgen_makeRealtime();

  int failures = 0;
  for (int i = 0; i < num_tests; i++) {
    if (!run_test(tests[i][0], tests[i][1], 0)) failures++;
    if (!run_test(tests[i][0], tests[i][1], 1)) failures++;
  }

  printf("final count = %ld (should be 0)\n", stepgen_getCount());
  if (stepgen_getCount() != 0) failures++;
  printf("%d failure(s)\n", failures);

  stepgen_cleanup();
  return failures;
}
