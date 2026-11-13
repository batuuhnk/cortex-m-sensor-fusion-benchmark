/**
 * @file workloads.c
 * @brief Synthetic navigation-filter workloads. See workloads.h.
 */
#include "workloads.h"

#include <math.h>
#include <string.h>

/* Static buffers: no heap, deterministic memory placement. Place this
 * section in TCM or regular SRAM through the linker script to compare. */
static float  s_a32[WL_MAX_N][WL_MAX_N], s_b32[WL_MAX_N][WL_MAX_N], s_c32[WL_MAX_N][WL_MAX_N];
static double s_a64[WL_MAX_N][WL_MAX_N], s_b64[WL_MAX_N][WL_MAX_N], s_c64[WL_MAX_N][WL_MAX_N];
static float  s_p32[WL_MAX_N][WL_MAX_N], s_f32[WL_MAX_N][WL_MAX_N], s_q32[WL_MAX_N];
static double s_p64[WL_MAX_N][WL_MAX_N], s_f64[WL_MAX_N][WL_MAX_N], s_q64[WL_MAX_N];
static float  s_x32[WL_MAX_N];

static uint32_t clamp_n(uint32_t n) { return (n == 0U) ? 1U : ((n > WL_MAX_N) ? WL_MAX_N : n); }
static uint32_t clamp_m(uint32_t m) { return (m == 0U) ? 1U : ((m > WL_MAX_M) ? WL_MAX_M : m); }

/* Deterministic, well-conditioned test data. */
static void fill_f32(float dst[WL_MAX_N][WL_MAX_N], uint32_t n, float seed)
{
    for (uint32_t i = 0; i < n; i++) {
        for (uint32_t j = 0; j < n; j++) {
            dst[i][j] = seed * (float)((i * 7U + j * 3U) % 11U) * 0.01f;
        }
    }
}

static void fill_f64(double dst[WL_MAX_N][WL_MAX_N], uint32_t n, double seed)
{
    for (uint32_t i = 0; i < n; i++) {
        for (uint32_t j = 0; j < n; j++) {
            dst[i][j] = seed * (double)((i * 7U + j * 3U) % 11U) * 0.01;
        }
    }
}

/* Transition matrix close to identity, like F = I + A*dt in a navigation filter. */
static void make_transition_f32(uint32_t n)
{
    for (uint32_t i = 0; i < n; i++) {
        for (uint32_t j = 0; j < n; j++) {
            s_f32[i][j] = (i == j) ? 1.0f : (((i + j) % 5U == 0U) ? 0.002f : 0.0f);
        }
        s_q32[i] = 1e-6f;
    }
}

static void make_transition_f64(uint32_t n)
{
    for (uint32_t i = 0; i < n; i++) {
        for (uint32_t j = 0; j < n; j++) {
            s_f64[i][j] = (i == j) ? 1.0 : (((i + j) % 5U == 0U) ? 0.002 : 0.0);
        }
        s_q64[i] = 1e-6;
    }
}

/* Symmetric positive definite starting covariance. */
static void make_covariance_f32(uint32_t n)
{
    for (uint32_t i = 0; i < n; i++) {
        for (uint32_t j = 0; j < n; j++) {
            s_p32[i][j] = (i == j) ? 1.0f : 0.01f / (float)(1U + ((i > j) ? i - j : j - i));
        }
    }
}

static void make_covariance_f64(uint32_t n)
{
    for (uint32_t i = 0; i < n; i++) {
        for (uint32_t j = 0; j < n; j++) {
            s_p64[i][j] = (i == j) ? 1.0 : 0.01 / (double)(1U + ((i > j) ? i - j : j - i));
        }
    }
}

float wl_scalar_f32(uint32_t iterations)
{
    volatile float a = 1.1f;
    float acc = 0.0f;
    for (uint32_t i = 0; i < iterations; i++) {
        float r = a * 2.2f + 3.3f;          /* multiply-accumulate */
        r = sqrtf(r);                        /* square root          */
        r = sinf(a) * cosf(r);               /* transcendental        */
        r = r / (a + 0.001f);                /* divide                */
        acc += r;
        a = 1.0f + 0.5f * r;
    }
    return acc;
}

double wl_scalar_f64(uint32_t iterations)
{
    volatile double a = 1.1;
    double acc = 0.0;
    for (uint32_t i = 0; i < iterations; i++) {
        double r = a * 2.2 + 3.3;
        r = sqrt(r);
        r = sin(a) * cos(r);
        r = r / (a + 0.001);
        acc += r;
        a = 1.0 + 0.5 * r;
    }
    return acc;
}

float wl_matmul_f32(uint32_t n)
{
    n = clamp_n(n);
    fill_f32(s_a32, n, 1.0f);
    fill_f32(s_b32, n, 2.0f);
    for (uint32_t i = 0; i < n; i++) {
        for (uint32_t j = 0; j < n; j++) {
            float sum = 0.0f;
            for (uint32_t k = 0; k < n; k++) {
                sum += s_a32[i][k] * s_b32[k][j];
            }
            s_c32[i][j] = sum;
        }
    }
    return s_c32[n - 1U][n - 1U];
}

double wl_matmul_f64(uint32_t n)
{
    n = clamp_n(n);
    fill_f64(s_a64, n, 1.0);
    fill_f64(s_b64, n, 2.0);
    for (uint32_t i = 0; i < n; i++) {
        for (uint32_t j = 0; j < n; j++) {
            double sum = 0.0;
            for (uint32_t k = 0; k < n; k++) {
                sum += s_a64[i][k] * s_b64[k][j];
            }
            s_c64[i][j] = sum;
        }
    }
    return s_c64[n - 1U][n - 1U];
}

float wl_ekf_predict_f32(uint32_t n)
{
    n = clamp_n(n);
    make_transition_f32(n);
    make_covariance_f32(n);

    /* T = F * P */
    for (uint32_t i = 0; i < n; i++) {
        for (uint32_t j = 0; j < n; j++) {
            float sum = 0.0f;
            for (uint32_t k = 0; k < n; k++) {
                sum += s_f32[i][k] * s_p32[k][j];
            }
            s_c32[i][j] = sum;
        }
    }
    /* P = T * F^T + Q, upper triangle computed and mirrored (P stays symmetric) */
    for (uint32_t i = 0; i < n; i++) {
        for (uint32_t j = i; j < n; j++) {
            float sum = 0.0f;
            for (uint32_t k = 0; k < n; k++) {
                sum += s_c32[i][k] * s_f32[j][k];
            }
            if (i == j) {
                sum += s_q32[i];
            }
            s_p32[i][j] = sum;
            s_p32[j][i] = sum;
        }
    }
    return s_p32[0][0];
}

double wl_ekf_predict_f64(uint32_t n)
{
    n = clamp_n(n);
    make_transition_f64(n);
    make_covariance_f64(n);

    for (uint32_t i = 0; i < n; i++) {
        for (uint32_t j = 0; j < n; j++) {
            double sum = 0.0;
            for (uint32_t k = 0; k < n; k++) {
                sum += s_f64[i][k] * s_p64[k][j];
            }
            s_c64[i][j] = sum;
        }
    }
    for (uint32_t i = 0; i < n; i++) {
        for (uint32_t j = i; j < n; j++) {
            double sum = 0.0;
            for (uint32_t k = 0; k < n; k++) {
                sum += s_c64[i][k] * s_f64[j][k];
            }
            if (i == j) {
                sum += s_q64[i];
            }
            s_p64[i][j] = sum;
            s_p64[j][i] = sum;
        }
    }
    return s_p64[0][0];
}

float wl_ekf_update_f32(uint32_t n, uint32_t m)
{
    static float h[WL_MAX_M][WL_MAX_N];
    static float pht[WL_MAX_N][WL_MAX_M];
    static float s[WL_MAX_M][WL_MAX_M];
    static float l[WL_MAX_M][WL_MAX_M];
    static float sinv[WL_MAX_M][WL_MAX_M];
    static float k[WL_MAX_N][WL_MAX_M];
    float y[WL_MAX_M];

    n = clamp_n(n);
    m = clamp_m(m);
    if (m > n) {
        m = n;
    }
    make_covariance_f32(n);

    /* H selects the first m states (typical for position/velocity aiding). */
    memset(h, 0, sizeof(h));
    for (uint32_t r = 0; r < m; r++) {
        h[r][r] = 1.0f;
        y[r] = 0.1f * (float)(r + 1U);
    }
    for (uint32_t i = 0; i < n; i++) {
        s_x32[i] = 0.0f;
    }

    /* PHt = P * H^T  (n x m) */
    for (uint32_t i = 0; i < n; i++) {
        for (uint32_t j = 0; j < m; j++) {
            float sum = 0.0f;
            for (uint32_t c = 0; c < n; c++) {
                sum += s_p32[i][c] * h[j][c];
            }
            pht[i][j] = sum;
        }
    }
    /* S = H * PHt + R  (m x m) */
    for (uint32_t i = 0; i < m; i++) {
        for (uint32_t j = 0; j < m; j++) {
            float sum = 0.0f;
            for (uint32_t c = 0; c < n; c++) {
                sum += h[i][c] * pht[c][j];
            }
            s[i][j] = sum + ((i == j) ? 0.25f : 0.0f);
        }
    }
    /* Cholesky: S = L * L^T */
    memset(l, 0, sizeof(l));
    for (uint32_t i = 0; i < m; i++) {
        for (uint32_t j = 0; j <= i; j++) {
            float sum = s[i][j];
            for (uint32_t c = 0; c < j; c++) {
                sum -= l[i][c] * l[j][c];
            }
            if (i == j) {
                l[i][i] = sqrtf((sum > 1e-12f) ? sum : 1e-12f);
            } else {
                l[i][j] = sum / l[j][j];
            }
        }
    }
    /* S^-1 column by column: solve L z = e, then L^T x = z */
    for (uint32_t col = 0; col < m; col++) {
        float z[WL_MAX_M];
        for (uint32_t i = 0; i < m; i++) {
            float sum = (i == col) ? 1.0f : 0.0f;
            for (uint32_t c = 0; c < i; c++) {
                sum -= l[i][c] * z[c];
            }
            z[i] = sum / l[i][i];
        }
        for (int32_t i = (int32_t)m - 1; i >= 0; i--) {
            float sum = z[i];
            for (uint32_t c = (uint32_t)i + 1U; c < m; c++) {
                sum -= l[c][i] * sinv[c][col];
            }
            sinv[i][col] = sum / l[i][i];
        }
    }
    /* K = PHt * S^-1  (n x m), x += K * y */
    for (uint32_t i = 0; i < n; i++) {
        float dx = 0.0f;
        for (uint32_t j = 0; j < m; j++) {
            float sum = 0.0f;
            for (uint32_t c = 0; c < m; c++) {
                sum += pht[i][c] * sinv[c][j];
            }
            k[i][j] = sum;
            dx += sum * y[j];
        }
        s_x32[i] += dx;
    }
    /* P = P - K * (H * P) = P - K * PHt^T, kept symmetric */
    for (uint32_t i = 0; i < n; i++) {
        for (uint32_t j = i; j < n; j++) {
            float sum = 0.0f;
            for (uint32_t c = 0; c < m; c++) {
                sum += k[i][c] * pht[j][c];
            }
            s_p32[i][j] -= sum;
            s_p32[j][i] = s_p32[i][j];
        }
    }
    return s_x32[0] + s_p32[0][0];
}

float wl_quat_propagate_f32(uint32_t steps)
{
    float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f;
    const float dt = 0.001f;
    for (uint32_t s = 0; s < steps; s++) {
        /* Synthetic gyro rates in rad/s, varied per step. */
        const float wx = 0.10f + 0.001f * (float)(s & 0x0FU);
        const float wy = -0.05f;
        const float wz = 0.02f * sinf(0.01f * (float)s);

        /* Rotation vector increment and exact quaternion from it. */
        const float ax = wx * dt, ay = wy * dt, az = wz * dt;
        const float angle = sqrtf(ax * ax + ay * ay + az * az);
        float dq0 = 1.0f, k = 0.5f;
        if (angle > 1e-9f) {
            dq0 = cosf(0.5f * angle);
            k = sinf(0.5f * angle) / angle;
        }
        const float dq1 = ax * k, dq2 = ay * k, dq3 = az * k;

        /* q = q * dq */
        const float n0 = q0 * dq0 - q1 * dq1 - q2 * dq2 - q3 * dq3;
        const float n1 = q0 * dq1 + q1 * dq0 + q2 * dq3 - q3 * dq2;
        const float n2 = q0 * dq2 - q1 * dq3 + q2 * dq0 + q3 * dq1;
        const float n3 = q0 * dq3 + q1 * dq2 - q2 * dq1 + q3 * dq0;

        /* Renormalize to control drift. */
        const float inv = 1.0f / sqrtf(n0 * n0 + n1 * n1 + n2 * n2 + n3 * n3);
        q0 = n0 * inv; q1 = n1 * inv; q2 = n2 * inv; q3 = n3 * inv;
    }
    return q0 + q1 + q2 + q3;
}
