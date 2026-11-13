/**
 * @file dummy_kf.h
 * @brief C port of the MATLAB dummy Kalman filter and optimization loads
 *        used in the original study (see ../README.md).
 *
 * KF_S sets the state count (30 or 15 in the study) and KF_REAL the
 * floating-point type (float or double), both at compile time.
 */
#ifndef DUMMY_KF_H
#define DUMMY_KF_H

#ifndef KF_S
#define KF_S 30
#endif

#ifndef KF_REAL
#define KF_REAL float
#endif

typedef KF_REAL kf_real_t;

/** One Kalman filter step on random data (includes a KF_S x KF_S inverse). */
void dummy_kf(kf_real_t xhat[KF_S], kf_real_t sigma_x[KF_S][KF_S]);

/** Gradient-descent optimization with a quadratic penalty (scalar load). */
void opt_sim(void);

#endif /* DUMMY_KF_H */
