# Running the benchmark on an STM32 board

Works on any STM32 with a Cortex-M3/M4/M7/M33/M55 core (DWT cycle counter). Tested to compile against the STM32H7 CMSIS headers with GCC (arm-none-eabi 14.2), for Cortex-M33 with and without FPU and Cortex-M7 with a double-precision FPU.

## 1. Add the files to a CubeIDE project

1. Create a project for your board in STM32CubeIDE, set the system clock, and enable one UART (or SWO) for the log.
2. Copy `bench/` and `port/stm32/bench_port_stm32.c` into the project and add `bench/` to the include paths.
3. In `main.c`, after the clock and UART are initialized:

   ```c
   #include "bench_runner.h"
   ...
   /* USER CODE BEGIN 2 */
   bench_run_all();
   /* USER CODE END 2 */
   ```

4. Log output: add `BENCH_UART=huart2` (your handle) to the preprocessor symbols, or retarget `printf` and leave it out.
5. Add `BENCH_OPT="O3"` (matching the optimization level) to the preprocessor symbols so the log records it.

The static buffers need about 64 KB of RAM (float and double 32 x 32 matrices). Reduce `WL_MAX_N` in `workloads.h` on smaller parts.

## 2. Run the configuration matrix

Change one setting at a time, rebuild, run, and save the terminal output as one log file per configuration:

| Log name | FPU (Project > Properties > MCU Settings) | Optimization | Cache |
| --- | --- | --- | --- |
| `O0_soft.txt` | Floating-point unit: none, ABI soft | `-O0` | off |
| `O0_fpu.txt` | FPU on, ABI hard | `-O0` | off |
| `O3_fpu.txt` | FPU on, ABI hard | `-O3` | off |
| `O3_fpu_cache.txt` | FPU on, ABI hard | `-O3` | on |

Cache on/off:
- Cortex-M7 (STM32H7/F7): `SCB_EnableICache(); SCB_EnableDCache();` before `bench_run_all()`.
- STM32U5/L5/H5: enable the ICACHE peripheral in CubeMX (or `HAL_ICACHE_Enable()`).
- STM32F4/L4: flash ART accelerator, `__HAL_FLASH_INSTRUCTION_CACHE_ENABLE()`.

The log header records clock, FPU type (`soft`, `sp`, `dp`), optimization level and cache state, for example:

```
# fusion-bench cpu_hz=160000000 fpu=sp opt=O3 cache=ICACHE reps=20
case,param,reps,min_cycles,avg_cycles,max_cycles,min_us,cpu_hz,fpu,opt,cache,checksum
...
```

## 3. Turn the logs into results

```bash
python3 tools/parse_log.py logs/O0_soft.txt logs/O0_fpu.txt logs/O3_fpu.txt logs/O3_fpu_cache.txt \
    --out results/<board> --md results/<board>/summary.md
python3 tools/project_load.py results/<board>/O3_fpu_cache.csv tools/example_taskset.toml \
    --md results/<board>/projection.md
```

Commit `results/<board>/` with a short note: board, core, clock, compiler version, and where the buffers were placed (TCM or SRAM).
