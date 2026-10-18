import argparse
import matplotlib.pyplot as plt
import numpy as np
from pathlib import Path
from matplotlib.axes import Axes
from matplotlib.figure import Figure
from load_runs import AXIS_MARGIN, PLOTS_DIR, Run, load_optimal_values, load_run


PNG_DPI: int = 300


# note: one file pair per run, e.g. single_convergence_berlin52_run1.png
def output_files(run: Run) -> tuple[Path, Path]:
    stem: str = f"single_convergence_{run.instance}_run{run.number}"
    return PLOTS_DIR / f"{stem}.png", PLOTS_DIR / f"{stem}.svg"


def plot_convergence(run: Run) -> Figure:
    fig: Figure
    ax: Axes
    fig, ax = plt.subplots(figsize=(8, 5))

    generations: np.ndarray = np.array([generation.number for generation in run.generations])
    fitnesses: np.ndarray = np.array([generation.elite_fitness for generation in run.generations])

    # note: steps, not a line; the elite only changes when a better tour is found
    ax.step(generations, fitnesses, where="post", linewidth=1.5, color="tab:blue", label="Elite tour fitness")

    # note: optimum line and percent error axis only when the optimum is known
    if run.optimal is not None:
        optimal: int = run.optimal
        ax.axhline(optimal, linestyle="--", linewidth=1, color="black", label=f"Optimal ({optimal})")
        secondary: Axes = ax.secondary_yaxis(
            "right",
            functions=(lambda length: (length - optimal) * 100.0 / optimal, lambda error: optimal * (1 + error / 100.0)),
        )
        
        secondary.set_ylabel("Percent error (%)")

    error_text: str = "" if run.percent_error is None else f", {run.percent_error:.2f} % error"
    ax.margins(AXIS_MARGIN)
    ax.set_xlabel("Generation")
    ax.set_ylabel("Tour fitness")
    ax.set_title(f"Convergence: {run.instance} run {run.number} ({run.dimension} nodes{error_text})")
    ax.grid(True, alpha=0.3)
    ax.legend(loc="upper right")
    fig.tight_layout()
    return fig


def save_plot(fig: Figure, run: Run) -> Path:
    png_file, svg_file = output_files(run)
    PLOTS_DIR.mkdir(parents=True, exist_ok=True)
    fig.savefig(png_file, dpi=PNG_DPI)
    fig.savefig(svg_file)
    return png_file


if __name__ == "__main__":
    parser: argparse.ArgumentParser = argparse.ArgumentParser(description="Plot the convergence of a single GA run.")
    parser.add_argument("run_file", type=Path, help="path to a run file, e.g. data/runs/berlin52/run1.txt")
    args: argparse.Namespace = parser.parse_args()

    run: Run = load_run(args.run_file, load_optimal_values())
    png_file: Path = save_plot(plot_convergence(run), run)
    print(f"plotted {run.instance} run {run.number} ({len(run.generations)} generations) -> {png_file}")