#ifndef _ENCODER_H_
#define _ENCODER_H_

#define ENCODER_COUNTS_PER_STEP 4   /* x4 quadrature: 4 counts per detent */

int encoder_init(long max, long start);

int encoder_update();

long encoder_getValue();
void encoder_setValue(long value);

long encoder_getMax();
void encoder_setMax(long max);

#endif