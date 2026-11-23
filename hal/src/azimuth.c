#include <math.h>
#include <stdint.h>

#include <hal/devconf.h>
#include <hal/eqep.h>
#include <hal/azimuth.h>

static long    last_raw;
static int64_t total_count;

double azimuth_count_to_deg(int64_t count) {
    double deg = ((double)(count * AZIMUTH_DIR) / AZIMUTH_COUNTS_PER_DISH_REV) * 360.0;
    deg = fmod(deg, 360.0);
    if (deg < 0.0)    deg += 360.0;
    if (deg >= 360.0) deg = 0.0;
    return deg;
}

int azimuth_init(double start_deg) {
    if (eqep0_INIT(quadrature) < 0) return -1;

    azimuth_set_angle(start_deg);
    last_raw = eqep0_getCount();
    return (last_raw < 0) ? -1 : 0;
}

int azimuth_update(void) {
    long raw = eqep0_getCount();
    if (raw < 0) return -1;

    int32_t delta = (int32_t)((uint32_t)raw - (uint32_t)last_raw);
    last_raw = raw;
    if (delta == 0) return 0;

    total_count += delta;
    return 1;
}

double azimuth_get_angle(void) {
    return azimuth_count_to_deg(total_count);
}

int64_t azimuth_get_count(void) {
    return total_count;
}

void azimuth_set_angle(double deg) {
    double norm = fmod(deg, 360.0);
    if (norm < 0.0) norm += 360.0;
    total_count = llround((norm / 360.0) * AZIMUTH_COUNTS_PER_DISH_REV) * AZIMUTH_DIR;
}