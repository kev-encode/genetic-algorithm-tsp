import matplotlib.pyplot as plt
import numpy as np
from pathlib import Path
from matplotlib.axes import Axes
from matplotlib.figure import Figure
from load_runs import AXIS_MARGIN, PLOTS_DIR, Run, load_runs


PNG_FILE: Path = PLOTS_DIR / "error_v_dimension.png"
SVG_FILE: Path = PLOTS_DIR / "error_v_dimension.svg"
PNG_DPI: int = 300


# note: instances without a known optimum have no percent error; skipped
# note: median instead of mean; one or two bad runs (fl3795) don't drag the summary
# note: spread between runs is left to plot_variability_v_dimension.py
def collect_errors(runs_by_instance: dict[str, list[Run]]) -> tuple[list[int], list[float]]:
    dimensions: list[int] = []
    medians: list[float] = []

    for runs in runs_by_instance.values():
        errors: list[float] = [run.percent_error for run in runs if run.percent_error is not None]

        if not errors:
            continue

        dimensions.append(runs[0].dimension)
        medians.append(float(np.median(errors)))

    return dimensions, medians


def plot_errors(dimensions: list[int], medians: list[float]) -> Figure:
    fig: Figure
    ax: Axes
    fig, ax = plt.subplots(figsize=(8, 5))
    ax.scatter(dimensions, medians, s=20, color="tab:blue", label="Median of 10 runs")

    # note: log x-axis; dimensions span 51 to 18512, linear axis would crowd most instances on the left
    ax.set_xscale("log")
    ax.margins(AXIS_MARGIN)
    ax.set_xlabel("Number of nodes")
    ax.set_ylabel("Percent error (%)")
    ax.set_title("Percent Error vs. Number of Nodes")
    ax.grid(True, which="major", alpha=0.3)
    ax.legend(loc="upper left")
    fig.tight_layout()
    return fig


def save_plot(fig: Figure) -> None:
    PLOTS_DIR.mkdir(parents=True, exist_ok=True)
    fig.savefig(PNG_FILE, dpi=PNG_DPI)
    fig.savefig(SVG_FILE)


if __name__ == "__main__":
    dimensions, medians = collect_errors(load_runs())
    save_plot(plot_errors(dimensions, medians))
    print(f"plotted {len(dimensions)} instances -> {PLOTS_DIR}")