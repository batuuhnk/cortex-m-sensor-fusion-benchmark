/**
 * @file dummy_kf.c
 * @brief C port of matlab/dummy_kf.m and matlab/opt_sim.m.
 *
 * Static buffers only (no heap). The data is random: these functions exist to
 * load the CPU with the operation mix of a Kalman filter step.
 *
 * Note: compared with the code that produced the published measurements, the
 * copy of A into the left half of the augmented matrix in matrix_inverse() is
 * fixed (it was missing, leaving that half uninitialized). For a non-singular
 * matrix the fixed version performs the same operations, so the run time is
 * expected to be equivalent.
 */
#include "dummy_kf.h"

#include <math.h>
#include <stdlib.h>

#define S KF_S
typedef kf_real_t real;

static real random_real(void)
{
    return (real)rand() / (real)RAND_MAX;
}

static void mat_mul(real a[S][S], real b[S][S], real c[S][S])
{
    for (int i = 0; i < S; i++) {
        for (int j = 0; j < S; j++) {
            real sum = 0;
            for (int k = 0; k < S; k++) {
                sum += a[i][k] * b[k][j];
            }
            c[i][j] = sum;
        }
    }
}

static void mat_vec(real a[S][S], const real b[S], real c[S])
{
    for (int i = 0; i < S; i++) {
        real sum = 0;
        for (int j = 0; j < S; j++) {
            sum += a[i][j] * b[j];
        }
        c[i] = sum;
    }
}

static real vec_mat_vec(const real a[S], real b[S][S], const real c[S])
{
    real tmp[S];
    real sum = 0;
    mat_vec(b, c, tmp);
    for (int i = 0; i < S; i++) {
        sum += a[i] * tmp[i];
    }
    return sum;
}

static void mat_transpose(real a[S][S], real at[S][S])
{
    for (int i = 0; i < S; i++) {
        for (int j = 0; j < S; j++) {
            at[j][i] = a[i][j];
        }
    }
}

/* Gauss-Jordan elimination with partial pivoting. Returns -1 if singular. */
static int mat_inverse(real a[S][S], real ainv[S][S])
{
    static real t[S][2 * S];

    for (int i = 0; i < S; i++) {
        for (int j = 0; j < S; j++) {
            t[i][j] = a[i][j];
            t[i][j + S] = (i == j) ? (real)1 : (real)0;
        }
    }
    for (int i = 0; i < S; i++) {
        int p = i;
        for (int k = i + 1; k < S; k++) {
            if (fabs((double)t[k][i]) > fabs((double)t[p][i])) {
                p = k;
            }
        }
        if (p != i) {
            for (int j = 0; j < 2 * S; j++) {
                real tmp = t[i][j];
                t[i][j] = t[p][j];
                t[p][j] = tmp;
            }
        }
        if (fabs((double)t[i][i]) < 1e-12) {
            return -1;
        }
        const real scale = t[i][i];
        for (int j = 0; j < 2 * S; j++) {
            t[i][j] /= scale;
        }
        for (int k = 0; k < S; k++) {
            if (k != i) {
                const real f = t[k][i];
                for (int j = 0; j < 2 * S; j++) {
                    t[k][j] -= f * t[i][j];
                }
            }
        }
    }
    for (int i = 0; i < S; i++) {
        for (int j = 0; j < S; j++) {
            ainv[i][j] = t[i][j + S];
        }
    }
    return 0;
}

void dummy_kf(real xhat[S], real sigma_x[S][S])
{
    static real ad[S][S], ad_inv[S][S], ad_t[S][S], sigma_w[S][S];
    static real m1[S][S], m2[S][S];
    static real bd[S], cd[S], v1[S], l[S];
    const real dd = 0;
    const real u = random_real();
    const real z = random_real();
    const real sigma_v = random_real();

    for (int i = 0; i < S; i++) {
        for (int j = 0; j < S; j++) {
            ad[i][j] = random_real();
            sigma_w[i][j] = random_real();
            sigma_x[i][j] = random_real();
        }
        bd[i] = random_real();
        cd[i] = random_real();
        xhat[i] = random_real();
    }

    /* Ad = inv(Ad): the O(S^3) part that dominates the run time. */
    if (mat_inverse(ad, ad_inv) != 0) {
        return;
    }

    /* 1a: xhat = Ad * xhat + Bd * u */
    mat_vec(ad_inv, xhat, v1);
    for (int i = 0; i < S; i++) {
        xhat[i] = v1[i] + bd[i] * u;
    }

    /* 1b: SigmaX = Ad * SigmaX * Ad' + SigmaW */
    mat_transpose(ad_inv, ad_t);
    mat_mul(ad_inv, sigma_x, m1);
    mat_mul(m1, ad_t, m2);
    for (int i = 0; i < S; i++) {
        for (int j = 0; j < S; j++) {
            sigma_x[i][j] = m2[i][j] + sigma_w[i][j];
        }
    }

    /* 1c: zhat = Cd * xhat + Dd * u */
    real zhat = 0;
    for (int i = 0; i < S; i++) {
        zhat += cd[i] * xhat[i];
    }
    zhat += dd * u;

    /* 2a: L = SigmaX * Cd' / (Cd * SigmaX * Cd' + SigmaV) */
    mat_vec(sigma_x, cd, v1);
    const real denom = vec_mat_vec(cd, sigma_x, cd) + sigma_v;
    for (int i = 0; i < S; i++) {
        l[i] = v1[i] / denom;
    }

    /* 2b: xhat = xhat + L * (z - zhat) */
    const real innov = z - zhat;
    for (int i = 0; i < S; i++) {
        xhat[i] += l[i] * innov;
    }

    /* 2c: SigmaX = (I - L * Cd) * SigmaX */
    for (int i = 0; i < S; i++) {
        for (int j = 0; j < S; j++) {
            m1[i][j] = ((i == j) ? (real)1 : (real)0) - l[i] * cd[j];
        }
    }
    mat_mul(m1, sigma_x, m2);
    for (int i = 0; i < S; i++) {
        for (int j = 0; j < S; j++) {
            sigma_x[i][j] = m2[i][j];
        }
    }
}

void opt_sim(void)
{
    const real edge = (real)10.0;   /* cube edge                */
    const real mu = (real)100.0;    /* penalty weight           */
    const real alpha = (real)0.1;   /* step size                */
    const int max_iter = 2000;
    const real tol = (real)1e-10;
    real r = (real)0.2;              /* initial radius           */

    for (int k = 0; k < max_iter; k++) {
        real viol = r - edge / (real)2.0;
        if (viol < 0) {
            viol = 0;
        }
        const real d_j = (real)-1.0 + (real)2.0 * mu * viol;
        real r_new = r - alpha * d_j;
        if (r_new < 0) {
            r_new = 0;
        }
        if (fabs((double)(r_new - r)) < (double)tol) {
            break;
        }
        r = r_new;
    }
}
