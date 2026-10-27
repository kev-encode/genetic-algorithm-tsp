from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
from matplotlib.axes import Axes
from matplotlib.figure import Figure

from load_runs import AXIS_MARGIN, PLOTS_DIR, load_runs
from plot_time_v_dimension import collect_times


PNG_FILE: Path = PLOTS_DIR / "time_fit_v_dimension.png"
SVG_FILE: Path = PLOTS_DIR / "time_fit_v_dimension.svg"
PNG_DPI: int = 300


# note: least-squares line through (ln n, ln time); returns (exponent,
# coefficient) of time = coefficient * n^exponent
def fit_power_law(
    dimensions: np.ndarray, medians: np.ndarray,
) -> tuple[float, float]:
    exponent, intercept = np.polyfit(np.log(dimensions), np.log(medians), 1)
    return float(exponent), float(np.exp(intercept))


def plot_times_fit(dimensions: list[int], medians: list[float]) -> Figure:
    dimensions_array: np.ndarray = np.array(dimensions)
    medians_array: np.ndarray = np.array(medians)
    exponent, coefficient = fit_power_law(dimensions_array, medians_array)

    fig: Figure
    ax: Axes
    fig, ax = plt.subplots(figsize=(8, 5))
    ax.scatter(
        dimensions_array, medians_array, s=20, color="tab:blue",
        label="Median of 10 runs",
    )

    fit_dimensions: np.ndarray = np.geomspace(
        dimensions_array.min(), dimensions_array.max(), 200
    )
    ax.plot(
        fit_dimensions, coefficient * fit_dimensions ** exponent,
        color="black", linestyle="-", linewidth=1.5,
        label=f"Power-law fit: time ∝ n^{exponent:.2f}",
    )

    # note: log-log axes; a power law appears as a straight line whose slope
    # is the exponent
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
    save_plot(plot_times_fit(dimensions, medians))
    print(f"plotted {len(dimensions)} instances -> {PLOTS_DIR}")
