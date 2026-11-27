#ifndef HAL_AZIMUTH_H
#define HAL_AZIMUTH_H

#include <stdint.h>

int     azimuth_init(double start_deg);
int     azimuth_update(void);

// maps signed encoder count to dish azimuth in [0.0, 360.0)
double  azimuth_count_to_deg(int64_t count);

double  azimuth_get_angle(void);
int64_t azimuth_get_count(void);
void    azimuth_set_angle(double deg);

#endif