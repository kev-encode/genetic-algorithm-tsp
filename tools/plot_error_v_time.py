from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
from matplotlib.axes import Axes
from matplotlib.collections import PathCollection
from matplotlib.colors import LogNorm
from matplotlib.figure import Figure
from matplotlib.legend import Legend

from load_runs import AXIS_MARGIN, PLOTS_DIR, Run, load_runs


PNG_FILE: Path = PLOTS_DIR / "error_v_time.png"
SVG_FILE: Path = PLOTS_DIR / "error_v_time.svg"
PNG_DPI: int = 300


# note: instances without a known optimum have no percent error; skipped
# note: medians of each instance's runs; matches plot_error_v_dimension.py
# and plot_time_v_dimension.py
def collect_errors_and_times(
    runs_by_instance: dict[str, list[Run]],
) -> tuple[list[int], list[float], list[float]]:
    dimensions: list[int] = []
    median_times: list[float] = []
    median_errors: list[float] = []

    for runs in runs_by_instance.values():
        errors: list[float] = [
            run.percent_error for run in runs if run.percent_error is not None
        ]

        if not errors:
            continue

        dimensions.append(runs[0].dimension)
        median_times.append(
            float(np.median([run.compute_time for run in runs]))
        )

        median_errors.append(float(np.median(errors)))

    return dimensions, median_times, median_errors


def plot_errors_and_times(
    dimensions: list[int],
    median_times: list[float],
    median_errors: list[float],
) -> Figure:
    fig: Figure
    ax: Axes
    fig, ax = plt.subplots(figsize=(8, 5))

    # note: colour = number of nodes on a log scale; links this plot back to
    # the two dimension plots
    points: PathCollection = ax.scatter(
        median_times, median_errors, s=20, c=dimensions, cmap="viridis",
        norm=LogNorm(), label="Median of 10 runs",
    )

    fig.colorbar(points, ax=ax, label="Number of nodes")

    # note: log x-axis; times span 0.001 s to over 100 s
    ax.set_xscale("log")
    ax.margins(AXIS_MARGIN)
    ax.set_xlabel("Compute time (seconds)")
    ax.set_ylabel("Percent error (%)")
    ax.set_title("Percent Error vs. Compute Time")
    ax.grid(True, which="major", alpha=0.3)
    legend: Legend = ax.legend(loc="upper left")

    # note: legend marker would otherwise take the colormap's lowest colour;
    # neutral grey implies no particular size
    # note: handle keeps the colormap mapping; clear it first or the fill
    # colour is overridden
    legend_marker: PathCollection = legend.legend_handles[0]
    legend_marker.set_array(None)
    legend_marker.set_color("tab:gray")
    fig.tight_layout()
    return fig


def save_plot(fig: Figure) -> None:
    PLOTS_DIR.mkdir(parents=True, exist_ok=True)
    fig.savefig(PNG_FILE, dpi=PNG_DPI)
    fig.savefig(SVG_FILE)


if __name__ == "__main__":
    dimensions, median_times, median_errors = collect_errors_and_times(
        load_runs()
    )
    
    save_plot(plot_errors_and_times(dimensions, median_times, median_errors))
    print(f"plotted {len(dimensions)} instances -> {PLOTS_DIR}")