#include <hal/devconf.h>
#include <hal/encoder.h>
#include <hal/led.h>
#include <hal/mcp300x.h>
#include <hal/pwm.h>
#include <hal/spi.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* --- APP FLAGS ---*/
// #define DEBUG

#define ADC_VREF 3.3
#define ENC_MAX 100

static void on_sigint(int sig) {
  (void)sig;
}

int main(void) {
#ifdef SPIDEV_0_TRF0
  spi0_0_INIT();
#endif

  mcpInit(ADC_SPIDEV0_0);
  setVref(ADC_VREF);

  if (encoder_init(ENC_MAX, 10) < 0) {
    fprintf(stderr, "encoder init failed\n");
    return 1;
  }

  signal(SIGINT, on_sigint);

  pwm_set(PWM0_GPIO12_DEV, 10, 50);
  pwm_enable(PWM0_GPIO12_DEV);
  return 0;
}