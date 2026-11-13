/**
 * @file main.c
 * @brief Host build: runs the same harness on a PC to check the code and the
 *        output format. Host timings are NOT representative of a Cortex-M target.
 */
#include <stdio.h>
#include <stdint.h>
#include <time.h>

#include "../harness/bench_runner.h"
#include "../harness/bench_timer.h"

/* Emulated 1 GHz "cycle" counter from the monotonic clock (1 tick = 1 ns). */
void bench_timer_init(void) {}

uint32_t bench_timer_now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint32_t)((uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec);
}

uint32_t bench_timer_hz(void) { return 1000000000U; }

void bench_print(const char *s) { fputs(s, stdout); }

const char *bench_cache_tag(void) { return "host"; }

int main(void)
{
    bench_run_all();
    return 0;
}
