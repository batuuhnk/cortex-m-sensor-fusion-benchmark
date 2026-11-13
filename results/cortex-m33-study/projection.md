Reference core: 160 MHz (measured)

| Task | Rate (Hz) | Cycles per run | Time per run (us) | Load on reference core |
| --- | ---: | ---: | ---: | ---: |
| Kalman step, 30 states | 100 | 1,600,000 | 10,000.0 | 100.0% |
| Kalman step, 15 states | 10 | 1,587,360 | 9,921.0 | 9.9% |
| Optimization loop | 25 | 2,036,000 | 12,725.0 | 31.8% |
| **Total** | | | | **141.7%** |

Projected load per candidate (budget 70% of the core, the rest is margin for drivers, communication and growth):

| Candidate | Clock (MHz) | Relative perf | Projected load | Fits budget |
| --- | ---: | ---: | ---: | :---: |
| STM32U585 (Cortex-M33, measured) | 160 | 1 | 141.7% | no |
| STM32H757, M7 core (estimate) | 480 | 1.96 | 24.1% | yes |
| ADSP-21569 SHARC+ (estimate) | 1000 | 2.76 | 8.2% | yes |
