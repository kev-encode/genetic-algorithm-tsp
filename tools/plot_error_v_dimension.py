import matplotlib.pyplot as plt
import numpy as np
from load_runs import PLOTS_DIR, load_runs


PNG_FILE = PLOTS_DIR / "error_v_dimension.png"
SVG_FILE = PLOTS_DIR / "error_v_dimension.svg"
PNG_DPI = 300


# note: instances without a known optimum have no percent error; skipped
# note: median instead of mean; one or two bad runs (fl3795) don't drag the summary
# note: spread between runs is left to plot_variability_v_dimension.py
def collect_errors(runs_by_instance):
    dimensions = []
    medians = []

    for runs in runs_by_instance.values():
        errors = [run.percent_error for run in runs if run.percent_error is not None]

        if not errors:
            continue

        dimensions.append(runs[0].dimension)
        medians.append(np.median(errors))

    return dimensions, medians


def plot_errors(dimensions, medians):
    fig, ax = plt.subplots(figsize=(8, 5))
    ax.scatter(dimensions, medians, s=20, color="tab:blue", label="Median of 10 runs")

    # note: log x-axis; dimensions span 51 to 18512, linear axis would crowd most instances on the left
    ax.set_xscale("log")
    ax.set_ylim(bottom=0)
    ax.set_xlabel("Number of nodes")
    ax.set_ylabel("Percent error (%)")
    ax.set_title("Percent Error vs. Number of Nodes")
    ax.grid(True, which="major", alpha=0.3)
    ax.legend(loc="upper left")
    fig.tight_layout()
    return fig


def save_plot(fig):
    PLOTS_DIR.mkdir(parents=True, exist_ok=True)
    fig.savefig(PNG_FILE, dpi=PNG_DPI)
    fig.savefig(SVG_FILE)


if __name__ == "__main__":
    dimensions, medians = collect_errors(load_runs())
    save_plot(plot_errors(dimensions, medians))
    print(f"plotted {len(dimensions)} instances -> {PLOTS_DIR}")