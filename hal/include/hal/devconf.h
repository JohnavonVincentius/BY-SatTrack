/*
Device configuration file for BeagleY-AI SBC.
*/
/* ---------- DEVICE LED CONFIGURATION ----------*/

// LED Trigger File
#define ACT_LED_TRIGGER "/sys/class/leds/ACT/trigger"
#define PWR_LED_TRIGGER "/sys/class/leds/PWR/trigger"

// LED Brightness File
#define ACT_LED_BRIGHTNESS "/sys/class/leds/ACT/brightness"
#define PWR_LED_BRIGHTNESS "/sys/class/leds/PWR/brightness"

// LED Delay Files
#define ACT_LED_DELAY_ON "/sys/class/leds/ACT/delay_on"
#define ACT_LED_DELAY_OFF "/sys/class/leds/ACT/delay_off"

#define PWR_LED_DELAY_ON "/sys/class/leds/ACT/delay_on"
#define PWR_LED_DELAY_OFF "/sys/class/leds/ACT/delay_off"

/* ---------- SPI DEVICE CONFIGURATION ----------*/
/*
SPIDEV0_0:
  MOSI: GPIO10 / SPI0 MOSI
  MISO: GPIO09 / SPI0 MISO
  SCK : GPIO11 / SPI0 SCLK
  CS0 : GPIO08 / SPI0 CE0
  CS1 : GPIO07 / SPI0 CE1
*/
#define SPIDEV_0_TRF0            "/dev/spidev0.0"
#define SPIDEV_0_TRF_CLK_SPEED   250000
#define SPIDEV_0_TRF_BUFFER_SIZE 127
#define SPIDEV_0_TRF_BITS        8
#define SPIDEV_0_TRF_MODE        0

/*
SPIDEV0_1:
  MOSI: GPIO20 / SPI1 MOSI
  MISO: GPIO19 / SPI1 MISO
  SCK : GPIO21 / SPI1 SCLK
  CS0 : GPIO12 / SPI1 CE0
  CS1 : GPIO17 / SPI1 CE1
*/
// #define SPIDEV_1_TRF1          "/dev/spidev0.1"
// #define SPIDEV_1_TRF_CLK_SPEED   250000
// #define SPIDEV_1_TRF_BUFFER_SIZE 256
// #define SPIDEV_1_TRF_BITS        8
// #define SPIDEV_1_TRF_MODE        0

/* ---------- PWM DEVICE CONFIGURATION ----------*/
/*
EPWM0:
  PWM0: GPIO12 / MCU-C20
*/
#define PWM0_GPIO12_DEV        "/dev/hat/pwm/GPIO12"

/* ---------- EQEP DEVICE CONFIGURATION ----------*/
/*
EQEP0:
  EQEP0_A:  MCU-A25 / GPIO16
  EQEP0_B:  MCU-A26 / GPIO17  
  EQEP0_I:  MCU-F23 / GPIO20
  EQEP0_S:  MCU-B25 / GPIO21
*/
#define EQEP0_CTR0_DEV          "/sys/bus/counter/devices/counter0/count0"
#define EQEP0_CTR0_FUNCTIONS    EQEP0_CTR0_DEV "/function_available"
#define EQEP0_CTR0_FUNCTION     EQEP0_CTR0_DEV "/function"
#define EQEP0_CTR0_CEILING      EQEP0_CTR0_DEV "/ceiling"
#define EQEP0_CTR0_ENABLE       EQEP0_CTR0_DEV "/enable"
#define EQEP0_CTR0_COUNT        EQEP0_CTR0_DEV "/count"
