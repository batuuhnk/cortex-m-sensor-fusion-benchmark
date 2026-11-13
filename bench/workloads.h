/**
 * @file workloads.h
 * @brief Synthetic ("dummy") workloads that mirror the arithmetic of a
 *        strapdown navigation filter.
 *
 * None of these produce a meaningful navigation solution. They exist to load
 * the CPU with the same mix of operations a real filter uses (matrix products,
 * a small matrix inversion, quaternion integration, transcendental functions)
 * so each task can be timed in isolation on a candidate processor.
 *
 * Every workload returns a checksum so the compiler cannot remove the work.
 */
#ifndef WORKLOADS_H
#define WORKLOADS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Largest state dimension supported by the matrix workloads. */
#define WL_MAX_N 32
/** Largest measurement dimension supported by the update workload. */
#define WL_MAX_M 8

/** Scalar mix: multiply-accumulate, divide, sqrt, sin/cos (single precision). */
float wl_scalar_f32(uint32_t iterations);

/** Same scalar mix in double precision. */
double wl_scalar_f64(uint32_t iterations);

/** C = A * B for n x n matrices, single precision. */
float wl_matmul_f32(uint32_t n);

/** C = A * B for n x n matrices, double precision. */
double wl_matmul_f64(uint32_t n);

/**
 * Kalman covariance prediction P = F * P * F^T + Q for an n-state filter
 * (single precision). This is usually the dominant cost of an EKF step.
 */
float wl_ekf_predict_f32(uint32_t n);

/** Same covariance prediction in double precision. */
double wl_ekf_predict_f64(uint32_t n);

/**
 * Kalman measurement update with m measurements on an n-state filter:
 * S = H P H^T + R, K = P H^T S^-1, x += K y, P = (I - K H) P.
 * S is inverted with a Cholesky factorization (single precision).
 */
float wl_ekf_update_f32(uint32_t n, uint32_t m);

/**
 * Attitude quaternion integration from gyro increments, `steps` times,
 * with renormalization (single precision). Models a high-rate IMU loop.
 */
float wl_quat_propagate_f32(uint32_t steps);

#ifdef __cplusplus
}
#endif

#endif /* WORKLOADS_H */
