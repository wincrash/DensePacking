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


def plot(x, y, xlabel, ylabel, outpath):
    plt.figure(figsize=(8, 4.5))
    plt.plot(x, y, marker='o', linestyle='-', markersize=4)
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.grid(True)
    plt.tight_layout()
    plt.savefig(outpath)
    plt.close()
    print('Saved', outpath)


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

    # Prepare and plot each requested series, dropping rows with NaN in either column
    def plot_series(candidates, ylabel, outname):
        col = find_column(df, candidates)
        if col is None:
            print(f'{ylabel} column not found; skipping {outname}', file=sys.stderr)
            return
        sub = df[[step_col, col]].copy()
        sub[step_col] = pd.to_numeric(sub[step_col], errors='coerce')
        sub[col] = pd.to_numeric(sub[col], errors='coerce')
        sub = sub.dropna()
        if sub.empty:
            print(f'No numeric data for {col}; skipping {outname}', file=sys.stderr)
            return
        plot(sub[step_col].values, sub[col].values, 'STEP', ylabel, os.path.join(outdir, outname))

    plot_series(['OVERLAP', 'overlap'], 'Overlap', 'overlap_vs_step.png')
    plot_series(['R_SCALE_DELTA', 'r_scale_delta', 'RADIUS_SCALE_DELTA', 'radius_scale_delta'], 'R_SCALE_DELTA', 'r_scale_delta_vs_step.png')
    plot_series(['RELAX_COEFF', 'relax_coeff', 'RELAXATION_COEFFICIENT', 'relaxation_coefficient'], 'RELAX_COEFF', 'relax_coeff_vs_step.png')


if __name__ == '__main__':
    csv_path = os.path.join(os.getcwd(), 'build', 'timers.csv')
    main(csv_path)
