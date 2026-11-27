/* ---------- 16x02 LCD I2C Interface ----------*/
#define LCD_I2CBUS i2c1

/* ---------- MOTOR CONTROL ---------- */
// Global EN - SET HIGH to enable drivers
#define ROT_EN  GPIO4

// AZ Motor CONF
#define AZ_STP  GPIO12
#define AZ_DIR  GPIO23

// AZ STEP-Angle Conversion Factor (1 step, x deg)
#define AZ_DEG_PER_STEP 1.8/96

// EL MOTOR CONF
#define EL_STP  GPIO13
#define EL_DIR  GPIO24

#define EL_STP_DEG 1.8

/* ---------- ENCODER SENSOR ---------- */
#define ENC_PRPH  eqep0

/* ---------- ADC CONFIG ---------- */
#define ADC_BUS "SPIDEV0.0"

#define AZ_POT  SINGLE_CH0

#define JOY_Y   SINGLE_CH1
#define JOY_X   SINGLE_CH2

/* ---------- GPS INTERFACE ---------- */
#define GPS_UART  UART0

/* ----------  ---------- */

