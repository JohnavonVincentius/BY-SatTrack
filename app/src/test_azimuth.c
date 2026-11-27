#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>

#include <hal/devconf.h>
#include <hal/azimuth.h>

static volatile sig_atomic_t running = 1;
static void on_sigint(int s) { (void)s; running = 0; }

static void test_count_to_deg(void) {
    int64_t quarter_dish = llround(AZIMUTH_COUNTS_PER_DISH_REV / 4.0) * AZIMUTH_DIR;
    int64_t full_dish    = llround(AZIMUTH_COUNTS_PER_DISH_REV) * AZIMUTH_DIR;

    assert(fabs(azimuth_count_to_deg(0)             -   0.0) < 0.01);
    assert(fabs(azimuth_count_to_deg(quarter_dish)  -  90.0) < 0.01);
    assert(fabs(azimuth_count_to_deg(-quarter_dish) - 270.0) < 0.01);
    assert(fabs(azimuth_count_to_deg(full_dish)     -   0.0) < 0.01);

    printf("[PASS] azimuth_count_to_deg() unit tests (%.0f counts/dish rev)\n",
           AZIMUTH_COUNTS_PER_DISH_REV);
}

int main(void) {
    signal(SIGINT, on_sigint);
    test_count_to_deg();

    if (azimuth_init(0.0) < 0) {
        fprintf(stderr, "azimuth_init() failed\n");
        return 1;
    }

    printf("Spin motor shaft 1 full turn to verify counts/rev (Ctrl+C to stop)...\n");
    struct timespec ts = { .tv_sec = 0, .tv_nsec = 50000000L }; // 50 ms

    while (running) {
        if (azimuth_update() < 0) {
            fprintf(stderr, "\nazimuth_update() failed\n");
            break;
        }

        int64_t cnt = azimuth_get_count();
        double motor_revs = (double)cnt / (double)AZIMUTH_COUNTS_PER_MOTOR_REV;
        double dish_deg   = azimuth_get_angle();

        printf("\rCount: %8lld | Motor revs: %6.2f | Dish Azimuth: %6.2f deg   ",
               (long long)cnt, motor_revs, dish_deg);
        fflush(stdout);
        nanosleep(&ts, NULL);
    }

    printf("\nDone.\n");
    return 0;
}