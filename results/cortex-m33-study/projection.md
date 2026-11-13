Reference core: 160 MHz (measured)

| Task | Rate (Hz) | Cycles per run | Time per run (us) | Load on reference core |
| --- | ---: | ---: | ---: | ---: |
| Kalman step, 30 states | 200 | 1,600,000 | 10,000.0 | 200.0% |
| Kalman step, 15 states | 20 | 1,587,360 | 9,921.0 | 19.8% |
| Optimization loop | 50 | 2,036,000 | 12,725.0 | 63.6% |
| **Total** | | | | **283.5%** |

Projected load per candidate (budget 70% of the core, the rest is margin for drivers, communication and growth):

| Candidate | Clock (MHz) | Relative perf | Projected load | Fits budget |
| --- | ---: | ---: | ---: | :---: |
| STM32U585 (Cortex-M33, measured) | 160 | 1 | 283.5% | no |
| STM32H757, M7 core (estimate) | 480 | 1.96 | 48.2% | yes |
| ADSP-21569 SHARC+ (estimate) | 1000 | 2.76 | 16.4% | yes |
