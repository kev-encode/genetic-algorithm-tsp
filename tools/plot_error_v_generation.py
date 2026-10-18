from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
from matplotlib.axes import Axes
from matplotlib.cm import ScalarMappable
from matplotlib.colors import LogNorm
from matplotlib.figure import Figure

from load_runs import AXIS_MARGIN, PLOTS_DIR, Run, load_runs


PNG_FILE: Path = PLOTS_DIR / "error_v_generation.png"
SVG_FILE: Path = PLOTS_DIR / "error_v_generation.svg"
PNG_DPI: int = 300

# note: load_runs drops generation 0, so curves start at generation 1
FIRST_GENERATION: int = 1

# note: errors below this are drawn on a linear scale; lets runs that reach
# the optimum (0 %) stay on the plot
SYMLOG_LINEAR_THRESHOLD: float = 0.1


# note: a run that stopped early keeps its final error for the remaining
# generations; it no longer changes once stopped
def pad_errors(errors: list[float], length: int) -> np.ndarray:
    return np.pad(np.array(errors), (0, length - len(errors)), mode="edge")


# note: instances without a known optimum have no percent error; skipped
# note: one curve per run; each keeps its own length, so a run that stopped
# early ends where it stopped
def collect_curves(
    runs_by_instance: dict[str, list[Run]],
) -> tuple[list[int], list[np.ndarray]]:
    dimensions: list[int] = []
    curves: list[np.ndarray] = []

    for runs in runs_by_instance.values():
        if runs[0].optimal is None:
            continue

        for run in runs:
            dimensions.append(run.dimension)
            curves.append(np.array(
                [generation.percent_error for generation in run.generations]
            ))

    return dimensions, curves


# note: runs are padded to a common length before taking the median; a run
# that stopped early holds its final error
def median_of_curves(curves: list[np.ndarray]) -> np.ndarray:
    length: int = max(len(curve) for curve in curves)
    return np.median(
        [pad_errors(list(curve), length) for curve in curves], axis=0
    )


def plot_curves(dimensions: list[int], curves: list[np.ndarray]) -> Figure:
    fig: Figure
    ax: Axes
    fig, ax = plt.subplots(figsize=(8, 5))

    # note: colour = number of nodes on a log scale; matches
    # plot_error_v_time.py
    norm: LogNorm = LogNorm(vmin=min(dimensions), vmax=max(dimensions))
    colormap = plt.get_cmap("viridis")

    for dimension, curve in zip(dimensions, curves):
        generations: np.ndarray = np.arange(
            FIRST_GENERATION, FIRST_GENERATION + len(curve)
        )

        ax.plot(
            generations, curve, linewidth=0.5, alpha=0.3,
            color=colormap(norm(dimension)),
        )

    # note: legend-only entry for the run lines; neutral grey implies no
    # particular size, as in plot_error_v_time.py
    ax.plot([], [], linewidth=1, color="tab:gray", label="Individual run")

    overall: np.ndarray = median_of_curves(curves)
    ax.plot(
        np.arange(FIRST_GENERATION, FIRST_GENERATION + len(overall)),
        overall, linewidth=2, color="black", label="Median of all runs",
    )

    fig.colorbar(
        ScalarMappable(norm=norm, cmap=colormap), ax=ax,
        label="Number of nodes",
    )

    # note: log x-axis; small instances converge in a few generations, the
    # largest run for thousands
    ax.set_xscale("log")
    ax.set_yscale("symlog", linthresh=SYMLOG_LINEAR_THRESHOLD)
    ax.margins(AXIS_MARGIN)
    ax.set_xlabel("Generation")
    ax.set_ylabel("Percent error (%)")
    ax.set_title("Percent Error vs. Generation")
    ax.grid(True, which="major", alpha=0.3)
    ax.legend(loc="upper right")
    fig.tight_layout()
    return fig


def save_plot(fig: Figure) -> None:
    PLOTS_DIR.mkdir(parents=True, exist_ok=True)
    fig.savefig(PNG_FILE, dpi=PNG_DPI)
    fig.savefig(SVG_FILE)


if __name__ == "__main__":
    dimensions, curves = collect_curves(load_runs())
    save_plot(plot_curves(dimensions, curves))
    print(f"plotted {len(dimensions)} runs -> {PLOTS_DIR}")