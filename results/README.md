# results/

Raw measurements and projection tables, one folder per measurement campaign.

| Folder | Board | Produced by | Files |
| --- | --- | --- | --- |
| [`cortex-m33-study/`](cortex-m33-study/) | STM32U585, Cortex-M33 at 160 MHz | Study code in `code/study/`, timed with SEGGER SystemView | `systemview_captures.csv` (data type and FPU), `optimization_kf30.csv` (FPU/-O3/cache combinations), `best_measured.csv` + `taskset_study.toml` (projection input), `projection.md` (projection output) |

Report and figures for this campaign: [report/02-cortex-m33-study.md](../report/02-cortex-m33-study.md).

## Adding a board

Runs of the `code/harness/` benchmark go in a new folder, `results/<board>/`:

- the per-configuration CSV files and `summary.md` from `code/tools/parse_log.py`
- `projection.md` from `code/tools/project_load.py`
- a short note: board, core, clock, compiler version, buffer placement (TCM or SRAM)
