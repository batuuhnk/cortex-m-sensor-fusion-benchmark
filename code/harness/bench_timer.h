/**
 * @file bench_timer.h
 * @brief Cycle counter abstraction for the benchmark harness.
 *
 * On Cortex-M3/M4/M7/M33/M55 the DWT cycle counter is used (one tick per core
 * clock). On the host build the counter is emulated from a monotonic clock so
 * the same harness and output format can be checked on a PC.
 */
#ifndef BENCH_TIMER_H
#define BENCH_TIMER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Enable the cycle counter. Call once before any measurement. */
void bench_timer_init(void);

/** Current cycle count. Wraps at 2^32; differences are taken modulo 2^32. */
uint32_t bench_timer_now(void);

/** Core clock in Hz, used to convert cycles to time in the report. */
uint32_t bench_timer_hz(void);

#ifdef __cplusplus
}
#endif

#endif /* BENCH_TIMER_H */
