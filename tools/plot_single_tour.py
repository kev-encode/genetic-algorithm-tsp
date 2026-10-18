import argparse
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
import tsplib95
from matplotlib.axes import Axes
from matplotlib.figure import Figure

from load_runs import (
    AXIS_MARGIN, BASE_DIR, PLOTS_DIR, Run, load_optimal_values, load_run,
)


INSTANCES_DIR: Path = BASE_DIR / "res" / "instances" / "euc2d"
OPTIMAL_TOURS_DIR: Path = BASE_DIR / "res" / "optimal_tours"
PNG_DPI: int = 300

# note: marker size and line width shrink with the square root of the node
# count; keeps large instances from turning into a solid blob, with floors so
# the tour stays visible
MAX_MARKER_SIZE: float = 12.0
MIN_MARKER_SIZE: float = 0.5
MAX_LINE_WIDTH: float = 1.0
MIN_LINE_WIDTH: float = 0.3
SIZE_REFERENCE_NODES: int = 100

# note: tour panels draw nodes smaller than the nodes panel; the path is the
# subject there
TOUR_MARKER_SCALE: float = 0.3

# note: figure height follows the instance's aspect ratio so equal-scaled
# axes don't leave empty bands; label width is the room the y-axis takes,
# title height the room for the title and x-axis, aspect clamped so very
# wide or tall instances stay readable
FIGURE_WIDTH: float = 8.0
LABEL_WIDTH: float = 1.0
TITLE_HEIGHT: float = 1.0
MIN_ASPECT: float = 0.4
MAX_ASPECT: float = 1.6


# note: one file pair per plot, e.g. single_tour_berlin52_run1_elite.png
def output_files(run: Run, kind: str) -> tuple[Path, Path]:
    stem: str = f"single_tour_{run.instance}_run{run.number}_{kind}"
    return PLOTS_DIR / f"{stem}.png", PLOTS_DIR / f"{stem}.svg"


# note: {TSPLIB node number: (x, y)}; run and optimal tours use the same
# 1-based node numbers
def load_coordinates(instance: str) -> dict[int, tuple[float, float]]:
    problem = tsplib95.load(INSTANCES_DIR / f"{instance}.tsp")
    return {node: (x, y) for node, (x, y) in problem.node_coords.items()}


# note: only some instances ship a .opt.tour file; None when missing
def load_optimal_tour(instance: str) -> list[int] | None:
    tour_file: Path = OPTIMAL_TOURS_DIR / f"{instance}.opt.tour"

    if not tour_file.exists():
        return None

    return tsplib95.load(tour_file).tours[0]


# note: closes the loop by repeating the first node at the end
def tour_coordinates(
    tour: list[int], coordinates: dict[int, tuple[float, float]],
) -> np.ndarray:
    return np.array([coordinates[node] for node in tour + tour[:1]])


def scaled_size(maximum: float, minimum: float, dimension: int) -> float:
    scale: float = min(1.0, np.sqrt(SIZE_REFERENCE_NODES / dimension))
    return max(minimum, maximum * scale)


# note: path None draws the nodes alone
def plot_nodes_and_path(
    points: np.ndarray,
    path: np.ndarray | None,
    dimension: int,
    color: str,
    title: str,
) -> Figure:
    spans: np.ndarray = np.ptp(points, axis=0)
    aspect: float = float(np.clip(spans[1] / spans[0], MIN_ASPECT, MAX_ASPECT))

    fig: Figure
    ax: Axes
    fig, ax = plt.subplots(
        figsize=(
            FIGURE_WIDTH,
            (FIGURE_WIDTH - LABEL_WIDTH) * aspect + TITLE_HEIGHT,
        ),
    )

    marker_size: float = scaled_size(
        MAX_MARKER_SIZE, MIN_MARKER_SIZE, dimension
    )

    if path is not None:
        ax.plot(
            path[:, 0], path[:, 1], color=color,
            linewidth=scaled_size(MAX_LINE_WIDTH, MIN_LINE_WIDTH, dimension),
        )

        marker_size = max(MIN_MARKER_SIZE, marker_size * TOUR_MARKER_SCALE)

    ax.scatter(
        points[:, 0], points[:, 1], color="black", zorder=3, s=marker_size,
    )

    ax.set_title(title)
    ax.set_aspect("equal")
    ax.margins(AXIS_MARGIN)
    ax.set_xlabel("Node X-Coordinate")
    ax.set_ylabel("Node Y-Coordinate")
    fig.tight_layout()
    return fig


# note: {kind: figure}; kind names the output file, optimal only when the
# instance has a .opt.tour file
def plot_tours(
    run: Run,
    coordinates: dict[int, tuple[float, float]],
    optimal_tour: list[int] | None,
) -> dict[str, Figure]:
    points: np.ndarray = np.array(list(coordinates.values()))
    error_text: str = (
        "" if run.percent_error is None
        else f", {run.percent_error:.2f}% Error"
    )

    figures: dict[str, Figure] = {
        "nodes": plot_nodes_and_path(
            points, None, run.dimension, "black",
            f"{run.instance} Traveling Salesman Problem Instance "
            f"({run.dimension} Nodes)",
        ),
        "elite": plot_nodes_and_path(
            points, tour_coordinates(run.tour, coordinates), run.dimension,
            "tab:blue",
            f"{run.instance} Run {run.number} Elite Genetic Algorithm Tour "
            f"({run.dimension} Nodes{error_text})",
        ),
    }

    if optimal_tour is not None:
        figures["optimal"] = plot_nodes_and_path(
            points, tour_coordinates(optimal_tour, coordinates),
            run.dimension, "tab:green",
            f"{run.instance} Optimal Tour ({run.dimension} Nodes)",
        )

    return figures


def save_plot(fig: Figure, run: Run, kind: str) -> Path:
    png_file, svg_file = output_files(run, kind)
    PLOTS_DIR.mkdir(parents=True, exist_ok=True)
    fig.savefig(png_file, dpi=PNG_DPI)
    fig.savefig(svg_file)
    return png_file


if __name__ == "__main__":
    parser: argparse.ArgumentParser = argparse.ArgumentParser(
        description="Plot the nodes, elite tour and optimal tour of a run."
    )

    parser.add_argument(
        "run_file", type=Path,
        help="path to a run file, e.g. data/runs/berlin52/run1.txt",
    )

    args: argparse.Namespace = parser.parse_args()
    run: Run = load_run(args.run_file, load_optimal_values())
    optimal_tour: list[int] | None = load_optimal_tour(run.instance)
    figures: dict[str, Figure] = plot_tours(
        run, load_coordinates(run.instance), optimal_tour
    )

    for kind, fig in figures.items():
        png_file: Path = save_plot(fig, run, kind)
        print(f"plotted {run.instance} run {run.number} {kind} -> {png_file}")

    if optimal_tour is None:
        print(f"no optimal tour file for {run.instance}; skipped")