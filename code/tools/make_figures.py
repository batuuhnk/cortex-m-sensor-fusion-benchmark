#!/usr/bin/env python3
"""
Regenerate the report figures from the CSV files in results/.

    python3 code/tools/make_figures.py

Writes SVG files to report/figures/. Needs matplotlib.
"""
import csv
import os

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
RES = os.path.join(ROOT, "results", "cortex-m33-study")
OUT = os.path.join(ROOT, "report", "figures")

# Palette (validated: lightness band, chroma, CVD separation)
SURFACE = "#fcfcfb"
TEXT = "#0b0b0b"
TEXT_2 = "#52514e"
GRID = "#e4e3df"
BLUE, ORANGE, AQUA = "#2a78d6", "#eb6834", "#1baf7a"
MUTED = "#b9b7b0"

plt.rcParams.update({
    "font.family": "DejaVu Sans",
    "font.size": 10,
    "axes.edgecolor": GRID,
    "axes.labelcolor": TEXT_2,
    "xtick.color": TEXT_2,
    "ytick.color": TEXT_2,
    "svg.fonttype": "none",
})


def read(name):
    with open(os.path.join(RES, name), newline="", encoding="utf-8") as f:
        return list(csv.DictReader(f))


def style(ax):
    ax.set_facecolor(SURFACE)
    for s in ("top", "right", "left"):
        ax.spines[s].set_visible(False)
    ax.grid(axis="x", color=GRID, linewidth=0.8)
    ax.set_axisbelow(True)
    ax.tick_params(length=0)


def title(fig, text, sub):
    fig.text(0.02, 0.97, text, ha="left", va="top", fontsize=13, color=TEXT, weight="bold")
    fig.text(0.02, 0.905, sub, ha="left", va="top", fontsize=10, color=TEXT_2)


def fig_type_fpu():
    """Small multiples: one panel per load, bars = data type / FPU setting."""
    rows = read("systemview_captures.csv")
    loads = [("dummy_kf", "30", "Kalman step, 30 states"),
             ("dummy_kf", "15", "Kalman step, 15 states"),
             ("opt_sim", "-", "Optimization loop")]
    configs = [("double", "sp (double in software)", "double (in software)", ORANGE),
               ("float", "disabled", "float, FPU off", BLUE),
               ("float", "sp", "float, FPU on", AQUA)]

    fig, axes = plt.subplots(1, 3, figsize=(10, 3.4), facecolor=SURFACE)
    for ax, (load, states, label) in zip(axes, loads):
        vals = []
        for typ, fpu, _, _ in configs:
            r = next(x for x in rows if x["load"] == load and x["states"] == states
                     and x["type"] == typ and x["fpu"] == fpu)
            vals.append(float(r["min_run_ms"]))
        y = list(range(len(configs)))[::-1]
        ax.barh(y, vals, height=0.62, color=[c[3] for c in configs], edgecolor=SURFACE, linewidth=2)
        for yi, v in zip(y, vals):
            ax.text(v + max(vals) * 0.02, yi, f"{v:,.1f} ms", va="center", fontsize=9, color=TEXT)
        ax.set_yticks(y)
        ax.set_yticklabels([c[2] for c in configs] if ax is axes[0] else [""] * len(configs))
        ax.set_xlim(0, max(vals) * 1.32)
        ax.set_title(label, fontsize=10, color=TEXT, loc="left")
        style(ax)
    axes[1].set_xlabel("minimum run time per call (ms)")
    title(fig, "Data type and FPU on a Cortex-M33 (160 MHz)",
          "double runs in software on a single-precision FPU and is the slowest option")
    fig.subplots_adjust(left=0.17, right=0.98, top=0.74, bottom=0.17, wspace=0.12)
    fig.savefig(os.path.join(OUT, "type_fpu.svg"), facecolor=SURFACE)
    plt.close(fig)


def fig_optimization():
    """One series, ranked; the shipping combination highlighted."""
    rows = read("optimization_kf30.csv")
    rows.sort(key=lambda r: float(r["run_ms"]))
    labels = [r["configuration"].replace(" (code in flash; no FPU; default optimization)", ", baseline")
              .replace("RAM (code in RAM)", "RAM") for r in rows]
    vals = [float(r["run_ms"]) for r in rows]
    best = min(vals)
    colors = [BLUE if v == best else MUTED for v in vals]

    fig, ax = plt.subplots(figsize=(10, 4.4), facecolor=SURFACE)
    y = list(range(len(rows)))[::-1]
    ax.barh(y, vals, height=0.66, color=colors, edgecolor=SURFACE, linewidth=2)
    base = max(vals)
    for yi, v in zip(y, vals):
        ax.text(v + 2, yi, f"{v:,.0f} ms  ({base / v:.1f}x)" if v != base else f"{v:,.0f} ms  (baseline)",
                va="center", fontsize=9, color=TEXT)
    ax.set_yticks(y)
    ax.set_yticklabels(labels)
    ax.set_xlim(0, base * 1.22)
    ax.set_xlabel("run time per Kalman step, 30 states (ms)")
    style(ax)
    title(fig, "FPU, -O3 and cache together: 189 ms to 10 ms (18.9x)",
          "Each setting alone gives 1.3x to 2.1x; only the combination keeps the FPU busy")
    fig.subplots_adjust(left=0.27, right=0.97, top=0.82, bottom=0.12)
    fig.savefig(os.path.join(OUT, "optimization_kf30.svg"), facecolor=SURFACE)
    plt.close(fig)


def fig_projection():
    """Projected load per candidate against the 70 % budget."""
    data = [("STM32U585, Cortex-M33\n160 MHz (measured)", 141.7),
            ("STM32H757, Cortex-M7\n480 MHz (estimate)", 24.1),
            ("ADSP-21569, SHARC+\n1 GHz (estimate)", 8.2)]
    budget = 70.0
    fig, ax = plt.subplots(figsize=(10, 3.2), facecolor=SURFACE)
    y = list(range(len(data)))[::-1]
    vals = [d[1] for d in data]
    colors = [ORANGE if v > budget else BLUE for v in vals]
    ax.barh(y, vals, height=0.6, color=colors, edgecolor=SURFACE, linewidth=2)
    ax.axvline(budget, color=TEXT_2, linewidth=1.2, linestyle=(0, (4, 3)))
    ax.text(budget + 3, len(data) - 0.45, "budget 70 %", fontsize=9, color=TEXT_2, va="bottom")
    ax.axvline(100, color=GRID, linewidth=1, zorder=0)
    for yi, v in zip(y, vals):
        ax.text(max(v, budget) + 4, yi, f"{v:.0f} %  {'does not fit' if v > budget else 'fits'}",
                va="center", fontsize=9, color=TEXT)
    ax.set_yticks(y)
    ax.set_yticklabels([d[0] for d in data])
    ax.set_xlim(0, 200)
    ax.set_xlabel("projected CPU load of the task set (% of one core)")
    style(ax)
    title(fig, "Projected load of the task set per processor",
          "Kalman 30 states at 100 Hz + Kalman 15 states at 10 Hz + optimization loop at 25 Hz")
    fig.subplots_adjust(left=0.22, right=0.97, top=0.76, bottom=0.17)
    fig.savefig(os.path.join(OUT, "projection.svg"), facecolor=SURFACE)
    plt.close(fig)


if __name__ == "__main__":
    os.makedirs(OUT, exist_ok=True)
    fig_type_fpu()
    fig_optimization()
    fig_projection()
    print(f"figures -> {OUT}")
