#!/usr/bin/env python3
"""
Project the CPU load of a sensor fusion task set onto candidate processors,
starting from cycle counts measured on one reference processor.

    load = sum over tasks (rate_hz * cycles) / (clock_hz * relative_perf)

`relative_perf` is how many reference-core cycles one candidate cycle is worth
for this kind of code (1.0 = same core design). Take it from a benchmark that
matches the workload, ideally by running this harness on the candidate too;
CoreMark/MHz ratios are a rough fallback for integer-heavy code only.

Usage:
    python3 project_load.py results/O3_fpu_cache.csv tools/example_taskset.toml
    python3 project_load.py results/O3_fpu_cache.csv tools/example_taskset.toml --md results/projection.md
"""
import argparse
import csv
import sys

try:
    import tomllib  # Python 3.11+
except ModuleNotFoundError:  # pragma: no cover
    sys.exit("Python 3.11 or newer is needed (tomllib)")


def load_measurements(path):
    with open(path, newline="", encoding="utf-8") as f:
        rows = list(csv.DictReader(f))
    if not rows:
        sys.exit(f"{path}: empty")
    cycles = {(r["case"], r["param"]): int(r["min_cycles"]) for r in rows}
    ref_hz = int(rows[0]["cpu_hz"])
    return cycles, ref_hz


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("measured_csv", help="CSV written by parse_log.py for the reference processor")
    ap.add_argument("taskset", help="TOML file with tasks and candidate processors")
    ap.add_argument("--md", help="also write the result as a Markdown file")
    args = ap.parse_args()

    cycles, ref_hz = load_measurements(args.measured_csv)
    with open(args.taskset, "rb") as f:
        cfg = tomllib.load(f)

    budget = float(cfg.get("budget", 0.7))
    tasks = cfg["tasks"]

    # Cycles per second each task needs on the reference core.
    per_task = []
    for t in tasks:
        key = (t["case"], str(t["param"]))
        if key not in cycles:
            sys.exit(f"task '{t['name']}': case {key} not in {args.measured_csv}")
        c = cycles[key] * int(t.get("calls", 1))
        per_task.append((t["name"], float(t["rate_hz"]), c, c * float(t["rate_hz"])))
    demand = sum(x[3] for x in per_task)

    out = []
    out.append(f"Reference core: {ref_hz / 1e6:.0f} MHz (measured)\n")
    out.append("| Task | Rate (Hz) | Cycles per run | Time per run (us) | Load on reference core |")
    out.append("| --- | ---: | ---: | ---: | ---: |")
    for name, rate, c, cps in per_task:
        out.append(f"| {name} | {rate:g} | {c:,} | {c / ref_hz * 1e6:,.1f} | {cps / ref_hz:.1%} |")
    out.append(f"| **Total** | | | | **{demand / ref_hz:.1%}** |\n")

    out.append(f"Projected load per candidate (budget {budget:.0%} of the core, the rest is margin "
               f"for drivers, communication and growth):\n")
    out.append("| Candidate | Clock (MHz) | Relative perf | Projected load | Fits budget |")
    out.append("| --- | ---: | ---: | ---: | :---: |")
    for cand in cfg["candidates"]:
        hz = float(cand["clock_mhz"]) * 1e6
        perf = float(cand.get("relative_perf", 1.0))
        load = demand / (hz * perf)
        out.append(f"| {cand['name']} | {cand['clock_mhz']:g} | {perf:g} | {load:.1%} | "
                   f"{'yes' if load <= budget else 'no'} |")

    text = "\n".join(out) + "\n"
    print(text)
    if args.md:
        with open(args.md, "w", encoding="utf-8") as f:
            f.write(text)
        print(f"-> {args.md}")


if __name__ == "__main__":
    main()
