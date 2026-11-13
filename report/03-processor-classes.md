# 3. Processor classes and selection criteria

Raw speed is only one of the criteria. For an embedded navigation computer the decision usually comes down to the list below; the benchmark answers the first point, the rest come from the system requirements and the team.

## Criteria

| Criterion | Questions to answer |
| --- | --- |
| Compute margin | Does the projected load fit the budget with the shipping build settings? How much growth room is left (more states, higher rates)? |
| Floating point | Single or double precision in hardware? Does the filter need double for long-term numerical stability of the covariance? |
| Determinism | Tightly coupled memory for critical code and data? Cache effects on worst-case timing? Interrupt latency? |
| Interfaces | Enough UART/SPI/CAN/Ethernet channels with DMA for the sensors and outputs; hardware timestamping of sensor samples. |
| Memory | Internal flash and RAM, or external memory (cost, boot time, attack surface, certification effort)? |
| Toolchain and ecosystem | RTOS support, debugger and trace (SWO/ETM), static analysis, team experience. |
| Lifecycle and supply | Long-term availability, temperature range, second source, export restrictions. |
| Safety and security | Memory protection, secure boot, crypto acceleration, TrustZone; availability of safety documentation. |

## Processor classes

| Class | Typical examples | Strengths | Watch out for |
| --- | --- | --- | --- |
| Single-core Cortex-M33/M4 | STM32U5, STM32L4/F4 | Low power, simple, cheap reference platform | Single-precision FPU only on most parts; limited headroom for large filters at high rates |
| High-performance Cortex-M7 | STM32H7, STM32F7 | Double-precision FPU on many parts, L1 caches, TCM, high clock | Cache and memory-map discipline needed for deterministic timing |
| Dual-core MCU (M7 + M4) | STM32H745/H747/H755/H757 | One die: fusion on the M7, I/O on the M4, hardware semaphores for inter-core sync | Two firmware images, shared-memory design, debugging two cores |
| Crossover MCU | NXP i.MX RT series | Very high clock, large on-chip SRAM | External flash is required for code |
| Cortex-R real-time processors | TI AM24x/AM26x, Renesas RZ/T | Real-time focus, lockstep options for functional safety | Different toolchain and ecosystem from Cortex-M |
| Floating-point DSP | ADI SHARC+, TI C67x | Very high floating-point throughput per watt for matrix and filter math | Separate toolchain, usually paired with an MCU for system control |

Check every figure against the current datasheet of the exact part number; families differ by memory size, FPU type and maximum clock.

## Using the class table with the benchmark

1. Measure on a cheap reference board from the first row (for example a Cortex-M33 board).
2. Project onto one representative part per class with `code/tools/project_load.py`.
3. Eliminate the classes that do not fit the budget, then compare the remaining ones on the other criteria.
4. Confirm the top candidate by running the same harness on its evaluation board, and replace the estimated `relative_perf` with the measured value.
