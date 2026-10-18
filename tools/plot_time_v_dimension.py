import matplotlib.pyplot as plt
import numpy as np
from pathlib import Path
from matplotlib.axes import Axes
from matplotlib.figure import Figure
from load_runs import AXIS_MARGIN, PLOTS_DIR, Run, load_runs


PNG_FILE: Path = PLOTS_DIR / "time_v_dimension.png"
SVG_FILE: Path = PLOTS_DIR / "time_v_dimension.svg"
PNG_DPI: int = 300


# note: median instead of mean; matches plot_error_v_dimension.py
def collect_times(runs_by_instance: dict[str, list[Run]]) -> tuple[list[int], list[float]]:
    dimensions: list[int] = []
    medians: list[float] = []

    for runs in runs_by_instance.values():
        dimensions.append(runs[0].dimension)
        medians.append(float(np.median([run.compute_time for run in runs])))

    return dimensions, medians


def plot_times(dimensions: list[int], medians: list[float]) -> Figure:
    fig: Figure
    ax: Axes
    fig, ax = plt.subplots(figsize=(8, 5))
    ax.scatter(dimensions, medians, s=20, color="tab:blue", label="Median of 10 runs")

    # note: log-log axes; times span 0.001 s to over 100 s, and a power law appears as a straight line
    ax.set_xscale("log")
    ax.set_yscale("log")
    ax.margins(AXIS_MARGIN)
    ax.set_xlabel("Number of nodes")
    ax.set_ylabel("Compute time (seconds)")
    ax.set_title("Compute Time vs. Number of Nodes")
    ax.grid(True, which="major", alpha=0.3)
    ax.legend(loc="upper left")
    fig.tight_layout()
    return fig


def save_plot(fig: Figure) -> None:
    PLOTS_DIR.mkdir(parents=True, exist_ok=True)
    fig.savefig(PNG_FILE, dpi=PNG_DPI)
    fig.savefig(SVG_FILE)


if __name__ == "__main__":
    dimensions, medians = collect_times(load_runs())
    save_plot(plot_times(dimensions, medians))
    print(f"plotted {len(dimensions)} instances -> {PLOTS_DIR}")