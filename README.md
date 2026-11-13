# cortex-m-sensor-fusion-benchmark

Choosing a processor for a high-rate sensor fusion system (IMU + GNSS navigation, attitude estimation) before the hardware exists.

The method: write **synthetic ("dummy") workloads** that do the same arithmetic as each real-time task of the filter, time each one **in isolation** on a cheap reference Cortex-M board under different FPU, cache and compiler settings, then **project the total load** onto candidate processors with a budget for margin.

This is how I approach processor selection for navigation systems. [`study/`](study/README.md) shows the first application of the method on a Cortex-M33, where FPU, compiler and cache settings together cut the run time of a 30-state Kalman filter load by 18.9x and the measurements decided the processor class. `bench/` is a cleaner, reusable harness written afterwards.

```
 synthetic workloads          measure on a reference board         project onto candidates
 ------------------           ----------------------------         -----------------------
 quaternion integration  -->  cycles per call, for each        -->  load = sum(rate x cycles)
 EKF covariance predict       FPU / cache / -O setting               / (clock x relative perf)
 EKF measurement update       (DWT cycle counter)                   compare with a budget (70%)
 scalar navigation math
```

## What is in the repository

| Path | Content |
| --- | --- |
| `bench/` | Portable C11 harness and workloads (no heap, no OS). Prints one CSV line per case. |
| `port/stm32/` | STM32 port: DWT cycle counter, UART or `printf` output, cache detection. [How to run it](port/stm32/README.md). |
| `host/` | PC build of the same harness, to check the code and the output format. |
| `tools/parse_log.py` | Turns captured logs into CSV files and a comparison table (speed-up per setting). |
| `tools/project_load.py` | Projects a task set onto candidate processors. Example: [`example_taskset.toml`](tools/example_taskset.toml). |
| `docs/` | [Method](docs/01-method.md), [processor classes and selection criteria](docs/02-processor-classes.md), [dual-processor architecture patterns](docs/03-dual-processor-pattern.md). |
| `study/` | [Original study](study/README.md): the dummy Kalman filter loads (MATLAB and C), SystemView measurements on a Cortex-M33, optimization results and the processor projection. |
| `results/` | Measurements of the `bench/` harness on my own boards, added per board. |

## Workloads

| Case | What it models | Parameter |
| --- | --- | --- |
| `quat_propagate` | Attitude quaternion integration from gyro increments, with renormalization | integration steps |
| `ekf_predict_f32` / `_f64` | Covariance prediction `P = F P F^T + Q`, usually the largest cost of an EKF step | state count n (9, 15, 21, 30) |
| `ekf_update_f32` | Measurement update with Cholesky inversion of `S` | n states, m measurements |
| `matmul_f32` / `_f64` | Plain n x n matrix product | n |
| `scalar_f32` / `_f64` | Multiply-accumulate, divide, sqrt, sin/cos mix | iterations |

The update and prediction math is checked against NumPy (same inputs, results equal to 7 significant digits in single precision). Each workload returns a checksum so the compiler cannot remove it.

## Quick start

On a PC (checks the build and the output format, timings are not representative):

```bash
cd host && make run
```

On a board: see [port/stm32/README.md](port/stm32/README.md). Capture one log per configuration, then:

```bash
python3 tools/parse_log.py logs/O0_soft.txt logs/O3_fpu.txt logs/O3_fpu_cache.txt --md results/<board>/summary.md
python3 tools/project_load.py results/<board>/O3_fpu_cache.csv tools/example_taskset.toml
```

## What the method shows, and what it does not

It answers early, cheaply and with real numbers: how many cycles each task costs, how much the FPU, caches and compiler settings matter, and whether a candidate processor fits the rate budget with margin.

It does not replace measuring the final firmware: interrupt load, DMA and bus contention, memory placement of the real code and communication stacks all add to the projection. That is what the budget margin is for, and the projection should be repeated on the chosen processor as soon as a board exists.

## Author

Batuhan Kılıçlı, embedded software engineer (defense and avionics).
[LinkedIn](https://linkedin.com/in/batuhankilicli) | [GitHub](https://github.com/batuuhnk)

MIT License.
