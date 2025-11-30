#!/usr/bin/env python3
"""
Generate PNG charts from `build/timers.csv` using matplotlib in headless mode.

Creates the following files in `build/`:
- `overlap_vs_step.png`
- `r_scale_delta_vs_step.png`
- `relax_coeff_vs_step.png`

Usage: run from repository root (script uses `./build/timers.csv`).
"""
import os
import sys
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
try:
    import pandas as pd
except Exception:
    print('Error: pandas is required. Install with `pip3 install pandas`', file=sys.stderr)
    sys.exit(3)



def read_csv_with_pandas(path):
    # Try semicolon first, then comma; return a DataFrame
    try:
        df = pd.read_csv(path, sep=';')
        if df.shape[1] > 1:
            return df
    except Exception:
        pass
    # fallback to default delimiter inference
    try:
        df = pd.read_csv(path)
        return df
    except Exception as e:
        print('Failed to read CSV with pandas:', e, file=sys.stderr)
        raise


def find_column(df, candidates):
    cols = {c.lower(): c for c in df.columns}
    for cand in candidates:
        key = cand.lower()
        if key in cols:
            return cols[key]
    return None


def plot_on_axes(ax, x, y, xlabel, ylabel, ylog=False):
    ax.plot(x, y, marker='o', linestyle='-', markersize=4)
    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)
    if ylog:
        ax.set_yscale('log')
    ax.grid(True)


def main(csv_path):
    if not os.path.exists(csv_path):
        print('Error: CSV not found:', csv_path, file=sys.stderr)
        sys.exit(2)
    df = read_csv_with_pandas(csv_path)
    if df.empty:
        print('CSV has no data rows', file=sys.stderr)
        sys.exit(2)

    # Find STEP column (case-insensitive)
    step_col = find_column(df, ['STEP', 'Step', 'step'])
    if step_col is None:
        print('STEP column not found', file=sys.stderr)
        sys.exit(2)

    outdir = os.path.dirname(csv_path)

    # Prepare numeric columns for plotting
    def get_series(candidates):
        col = find_column(df, candidates)
        if col is None:
            return None, None
        sub = df[[step_col, col]].copy()
        sub[step_col] = pd.to_numeric(sub[step_col], errors='coerce')
        sub[col] = pd.to_numeric(sub[col], errors='coerce')
        sub = sub.dropna()
        if sub.empty:
            return None, None
        return sub[step_col].values, sub[col].values

    # Collect series
    overlap_x, overlap_y = get_series(['OVERLAP', 'overlap'])
    rscale_x, rscale_y = get_series(['R_SCALE_DELTA', 'r_scale_delta', 'RADIUS_SCALE_DELTA', 'radius_scale_delta'])
    relax_x, relax_y = get_series(['RELAX_COEFF', 'relax_coeff', 'RELAXATION_COEFFICIENT', 'relaxation_coefficient'])

    # Create a single 2x2 figure
    fig, axes = plt.subplots(2, 2, figsize=(12, 8))
    ax_list = axes.flatten()

    # overlap linear
    ax = ax_list[0]
    if overlap_x is not None:
        plot_on_axes(ax, overlap_x, overlap_y, 'STEP', 'Overlap (linear)', ylog=False)
    else:
        ax.text(0.5, 0.5, 'No OVERLAP data', ha='center', va='center')
        ax.set_xlabel('STEP')
        ax.set_ylabel('Overlap (linear)')

    # overlap log
    ax = ax_list[1]
    if overlap_x is not None:
        # filter positive y values for log plot
        mask = overlap_y > 0
        if mask.any():
            plot_on_axes(ax, overlap_x[mask], overlap_y[mask], 'STEP', 'Overlap (log y)', ylog=True)
        else:
            ax.text(0.5, 0.5, 'No positive OVERLAP values', ha='center', va='center')
            ax.set_xlabel('STEP')
            ax.set_ylabel('Overlap (log y)')
    else:
        ax.text(0.5, 0.5, 'No OVERLAP data', ha='center', va='center')
        ax.set_xlabel('STEP')
        ax.set_ylabel('Overlap (log y)')

    # R_SCALE_DELTA
    ax = ax_list[2]
    if rscale_x is not None:
        plot_on_axes(ax, rscale_x, rscale_y, 'STEP', 'R_SCALE_DELTA', ylog=False)
    else:
        ax.text(0.5, 0.5, 'No R_SCALE_DELTA data', ha='center', va='center')
        ax.set_xlabel('STEP')
        ax.set_ylabel('R_SCALE_DELTA')

    # RELAX_COEFF
    ax = ax_list[3]
    if relax_x is not None:
        plot_on_axes(ax, relax_x, relax_y, 'STEP', 'RELAX_COEFF', ylog=False)
    else:
        ax.text(0.5, 0.5, 'No RELAX_COEFF data', ha='center', va='center')
        ax.set_xlabel('STEP')
        ax.set_ylabel('RELAX_COEFF')

    plt.tight_layout()
    outname = os.path.join(outdir, 'timers_charts.png')
    fig.savefig(outname)
    plt.close(fig)
    print('Saved', outname)


if __name__ == '__main__':
    csv_path = os.path.join(os.getcwd(), 'build', 'timers.csv')
    main(csv_path)
