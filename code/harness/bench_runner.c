/**
 * @file bench_runner.c
 * @brief Benchmark harness. See bench_runner.h.
 */
#include "bench_runner.h"
#include "bench_timer.h"
#include "workloads.h"

#include <stdio.h>

/* Build tags so every result line records how it was produced. */
#ifndef BENCH_OPT
#define BENCH_OPT "unknown"   /* set with -DBENCH_OPT=\"O3\" in the build */
#endif

#if defined(__ARM_FP) && ((__ARM_FP & 0x8) != 0)
#define BENCH_FPU "dp"        /* double-precision FPU */
#elif defined(__ARM_FP)
#define BENCH_FPU "sp"        /* single-precision FPU */
#elif defined(__arm__) || defined(__thumb__)
#define BENCH_FPU "soft"      /* no FPU: software floating point */
#else
#define BENCH_FPU "host"
#endif

__attribute__((weak)) const char *bench_cache_tag(void)
{
    return "n/a";
}

/* The checksum is written here so the compiler must keep every workload. */
volatile double g_bench_sink;

typedef enum {
    K_SCALAR_F32, K_SCALAR_F64, K_MATMUL_F32, K_MATMUL_F64,
    K_PREDICT_F32, K_PREDICT_F64, K_UPDATE_F32, K_QUAT_F32
} kind_t;

typedef struct {
    const char *name;
    kind_t      kind;
    uint32_t    p1;   /* iterations, matrix size n, or steps */
    uint32_t    p2;   /* measurement count m (update only)   */
} bench_case_t;

/* State sizes cover common filter designs: 9 (attitude/velocity/position),
 * 15 (+ gyro/accel bias), 21 and 30 (+ scale factors, misalignment, lever arm). */
static const bench_case_t k_cases[] = {
    { "scalar_f32",       K_SCALAR_F32, 100U, 0U },
    { "scalar_f64",       K_SCALAR_F64, 100U, 0U },
    { "matmul_f32",       K_MATMUL_F32, 15U,  0U },
    { "matmul_f64",       K_MATMUL_F64, 15U,  0U },
    { "ekf_predict_f32",  K_PREDICT_F32, 9U,  0U },
    { "ekf_predict_f32",  K_PREDICT_F32, 15U, 0U },
    { "ekf_predict_f32",  K_PREDICT_F32, 21U, 0U },
    { "ekf_predict_f32",  K_PREDICT_F32, 30U, 0U },
    { "ekf_predict_f64",  K_PREDICT_F64, 15U, 0U },
    { "ekf_predict_f64",  K_PREDICT_F64, 30U, 0U },
    { "ekf_update_f32",   K_UPDATE_F32, 15U,  3U },
    { "ekf_update_f32",   K_UPDATE_F32, 15U,  6U },
    { "ekf_update_f32",   K_UPDATE_F32, 30U,  6U },
    { "quat_propagate",   K_QUAT_F32,   10U,  0U },
};

static double run_case(const bench_case_t *c)
{
    switch (c->kind) {
    case K_SCALAR_F32:  return (double)wl_scalar_f32(c->p1);
    case K_SCALAR_F64:  return wl_scalar_f64(c->p1);
    case K_MATMUL_F32:  return (double)wl_matmul_f32(c->p1);
    case K_MATMUL_F64:  return wl_matmul_f64(c->p1);
    case K_PREDICT_F32: return (double)wl_ekf_predict_f32(c->p1);
    case K_PREDICT_F64: return wl_ekf_predict_f64(c->p1);
    case K_UPDATE_F32:  return (double)wl_ekf_update_f32(c->p1, c->p2);
    case K_QUAT_F32:    return (double)wl_quat_propagate_f32(c->p1);
    default:            return 0.0;
    }
}

void bench_run_all(void)
{
    char line[192];
    const uint32_t hz = bench_timer_hz();

    bench_timer_init();
    snprintf(line, sizeof(line),
             "# fusion-bench cpu_hz=%lu fpu=%s opt=%s cache=%s reps=%u\n",
             (unsigned long)hz, BENCH_FPU, BENCH_OPT, bench_cache_tag(), (unsigned)BENCH_REPS);
    bench_print(line);
    bench_print("case,param,reps,min_cycles,avg_cycles,max_cycles,min_us,cpu_hz,fpu,opt,cache,checksum\n");

    for (uint32_t i = 0; i < sizeof(k_cases) / sizeof(k_cases[0]); i++) {
        const bench_case_t *c = &k_cases[i];
        uint32_t min = 0xFFFFFFFFU, max = 0U;
        uint64_t sum = 0U;
        double chk = 0.0;

        (void)run_case(c);  /* warm-up: fills caches and branch predictor */

        for (uint32_t r = 0; r < BENCH_REPS; r++) {
            const uint32_t t0 = bench_timer_now();
            chk = run_case(c);
            const uint32_t dt = bench_timer_now() - t0;  /* modulo 2^32: wrap-safe */
            if (dt < min) { min = dt; }
            if (dt > max) { max = dt; }
            sum += dt;
        }
        g_bench_sink = chk;

        char param[24];
        if (c->kind == K_UPDATE_F32) {
            snprintf(param, sizeof(param), "n%lu_m%lu", (unsigned long)c->p1, (unsigned long)c->p2);
        } else {
            snprintf(param, sizeof(param), "%lu", (unsigned long)c->p1);
        }
        /* Integer microseconds x1000 avoids printf float support on small targets. */
        const uint32_t min_ns = (hz != 0U) ? (uint32_t)(((uint64_t)min * 1000000000ULL) / hz) : 0U;

        snprintf(line, sizeof(line), "%s,%s,%u,%lu,%lu,%lu,%lu.%03lu,%lu,%s,%s,%s,%ld\n",
                 c->name, param, (unsigned)BENCH_REPS,
                 (unsigned long)min, (unsigned long)(sum / BENCH_REPS), (unsigned long)max,
                 (unsigned long)(min_ns / 1000U), (unsigned long)(min_ns % 1000U),
                 (unsigned long)hz, BENCH_FPU, BENCH_OPT, bench_cache_tag(),
                 (long)(chk * 1000.0));
        bench_print(line);
    }
    bench_print("# done\n");
}
