#!/usr/bin/env python3
"""
Turn benchmark logs (captured from UART/SWO/RTT) into CSV files and a
comparison table.

Each log holds one run of bench_run_all(): a "# fusion-bench ..." header,
a CSV header line and one line per case. Lines from other firmware output are
ignored, so a raw terminal capture works.

Usage:
    python3 parse_log.py logs/O0_nofpu.txt logs/O3_fpu.txt logs/O3_fpu_cache.txt \
        --out results/ --md results/summary.md

The first log is the baseline: the table shows every run's minimum cycle count
and its speed-up over the baseline for the same case.
"""
import argparse
import csv
import os
import sys

FIELDS = ["case", "param", "reps", "min_cycles", "avg_cycles", "max_cycles",
          "min_us", "cpu_hz", "fpu", "opt", "cache", "checksum"]


def parse(path):
    """Return the list of result rows (dicts) found in one log file."""
    rows = []
    with open(path, encoding="utf-8", errors="replace") as f:
        in_table = False
        for raw in f:
            line = raw.strip()
            if line.startswith("case,param,"):
                in_table = True
                continue
            if not in_table or not line or line.startswith("#"):
                continue
            parts = line.split(",")
            if len(parts) != len(FIELDS):
                continue  # a corrupted or interleaved line
            row = dict(zip(FIELDS, parts))
            for k in ("reps", "min_cycles", "avg_cycles", "max_cycles", "cpu_hz"):
                row[k] = int(row[k])
            row["min_us"] = float(row["min_us"])
            rows.append(row)
    if not rows:
        sys.exit(f"{path}: no benchmark lines found")
    return rows


def label(rows, path):
    """Short run label from the build tags, falling back to the file name."""
    r = rows[0]
    mhz = r["cpu_hz"] / 1e6
    return f"{mhz:.0f} MHz, FPU {r['fpu']}, {r['opt']}, cache {r['cache']}" if r["opt"] != "unknown" \
        else os.path.splitext(os.path.basename(path))[0]


def write_csv(rows, out_dir, path):
    os.makedirs(out_dir, exist_ok=True)
    name = os.path.splitext(os.path.basename(path))[0] + ".csv"
    dst = os.path.join(out_dir, name)
    with open(dst, "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=FIELDS)
        w.writeheader()
        w.writerows(rows)
    return dst


def markdown(runs):
    """Comparison table: rows = cases, columns = runs (min cycles and speed-up)."""
    base_label, base_rows = runs[0]
    base = {(r["case"], r["param"]): r for r in base_rows}
    keys = [(r["case"], r["param"]) for r in base_rows]

    head = "| Case | " + " | ".join(lbl for lbl, _ in runs) + " |"
    sep = "| --- |" + " ---: |" * len(runs)
    lines = [head, sep]
    for key in keys:
        cells = []
        for i, (_, rows) in enumerate(runs):
            row = next((r for r in rows if (r["case"], r["param"]) == key), None)
            if row is None:
                cells.append("")
                continue
            us = row["min_us"]
            txt = f"{row['min_cycles']:,} cyc ({us:,.1f} us)"
            if i > 0 and row["min_cycles"] > 0:
                txt += f", {base[key]['min_cycles'] / row['min_cycles']:.1f}x"
            cells.append(txt)
        lines.append(f"| {key[0]} ({key[1]}) | " + " | ".join(cells) + " |")
    note = f"\nMinimum cycles over the repetitions; speed-up relative to the first run ({base_label})."
    return "\n".join(lines) + "\n" + note + "\n"


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("logs", nargs="+", help="captured log files, baseline first")
    ap.add_argument("--out", default="results", help="folder for the per-run CSV files")
    ap.add_argument("--md", help="write the comparison table to this Markdown file")
    args = ap.parse_args()

    runs = []
    for path in args.logs:
        rows = parse(path)
        dst = write_csv(rows, args.out, path)
        runs.append((label(rows, path), rows))
        print(f"{path}: {len(rows)} cases -> {dst}")

    table = markdown(runs)
    if args.md:
        os.makedirs(os.path.dirname(args.md) or ".", exist_ok=True)
        with open(args.md, "w", encoding="utf-8") as f:
            f.write(table)
        print(f"table -> {args.md}")
    else:
        print()
        print(table)


if __name__ == "__main__":
    main()
