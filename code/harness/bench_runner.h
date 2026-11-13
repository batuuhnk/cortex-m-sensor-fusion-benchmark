/**
 * @file bench_runner.h
 * @brief Runs every workload case, repeats it, and prints one CSV line per case.
 *
 * Output format (one header line, then one line per case):
 *   case,param,reps,min_cycles,avg_cycles,max_cycles,min_us,cpu_hz,fpu,opt,cache,checksum
 *
 * The output goes through bench_print(), which the port implements
 * (UART, ITM/SWO, RTT or stdout on the host).
 */
#ifndef BENCH_RUNNER_H
#define BENCH_RUNNER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Number of times each case is executed. The first run warms caches and is discarded. */
#ifndef BENCH_REPS
#define BENCH_REPS 20U
#endif

/** Port hook: write a zero-terminated string to the log channel. */
void bench_print(const char *s);

/**
 * Port hook: short tag describing the cache configuration of this run,
 * e.g. "I+D", "I", "off". The default implementation returns "n/a".
 */
const char *bench_cache_tag(void);

/** Run all cases and print the CSV report. */
void bench_run_all(void);

#ifdef __cplusplus
}
#endif

#endif /* BENCH_RUNNER_H */
