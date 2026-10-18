from dataclasses import dataclass, field
from pathlib import Path


BASE_DIR: Path = Path(__file__).resolve().parent.parent
RUNS_DIR: Path = BASE_DIR / "data" / "runs"
PLOTS_DIR: Path = BASE_DIR / "stdout" / "plots"
OPTIMAL_VALUES_FILE: Path = BASE_DIR / "res" / "optimal_values.txt"

# note: linhp318 has a FIXED_EDGES_SECTION; its optimum is for a constrained variant, not the plain tour the GA solves
EXCLUDED_INSTANCES: set[str] = {"linhp318"}


@dataclass
class Generation:
    number: int
    time: float
    elite_fitness: int
    percent_error: float | None


@dataclass
class Run:
    instance: str
    number: int
    dimension: int
    optimal: int | None
    elite: int
    compute_time: float
    percent_error: float | None
    generations: list[Generation] = field(default_factory=list)
    tour: list[int] = field(default_factory=list)


def compute_percent_error(fitness: int, optimal: int | None) -> float | None:
    return None if optimal is None else (fitness - optimal) * 100.0 / optimal


# note: {instance name: optimal tour length}; lines formatted "name : value"
def load_optimal_values(optimal_values_file: Path = OPTIMAL_VALUES_FILE) -> dict[str, int]:
    optimal_values: dict[str, int] = {}

    for line in optimal_values_file.read_text().splitlines():
        if line.strip():
            name, value = line.split(":")
            optimal_values[name.strip()] = int(value)

    return optimal_values


# note: file layout from write_run in src/main.cpp; header, then "generations:" and "tour:" sections
# note: optimum and percent errors recomputed from optimal_values; values stored in the file may predate a correction
def load_run(run_file: Path, optimal_values: dict[str, int]) -> Run:
    header: dict[str, str] = {}
    generations: list[Generation] = []
    tour: list[int] = []
    section: str = "header"

    for line in run_file.read_text().splitlines():
        line = line.strip()

        if not line:
            continue

        if line in ("generations:", "tour:"):
            section = line[:-1]

        elif section == "header":
            key, value = line.split(": ", 1)
            header[key] = value

        elif section == "generations":
            number, time, elite_fitness, _ = line.split(", ")
            generations.append(Generation(int(number), float(time), int(elite_fitness), None))

        else:
            tour.append(int(line))

    # note: generation 0 is the random initial population, not GA output; dropped here, and its time removed from compute time
    initial_time: float = sum(generation.time for generation in generations if generation.number == 0)
    generations = [generation for generation in generations if generation.number != 0]

    optimal: int | None = optimal_values.get(header["instance"])
    elite: int = int(header["elite"])

    for generation in generations:
        generation.percent_error = compute_percent_error(generation.elite_fitness, optimal)

    return Run(
        instance=header["instance"],
        number=int(run_file.stem.removeprefix("run")),
        dimension=int(header["dimension"]),
        optimal=optimal,
        elite=elite,
        compute_time=float(header["compute time"]) - initial_time,
        percent_error=compute_percent_error(elite, optimal),
        generations=generations,
        tour=tour,
    )


# note: {instance name: [runs sorted by run number]}, instances sorted by dimension
def load_runs(runs_dir: Path = RUNS_DIR) -> dict[str, list[Run]]:
    optimal_values: dict[str, int] = load_optimal_values()
    runs: dict[str, list[Run]] = {}

    for instance_dir in runs_dir.iterdir():
        if instance_dir.is_dir() and instance_dir.name not in EXCLUDED_INSTANCES:
            instance_runs: list[Run] = [load_run(run_file, optimal_values) for run_file in instance_dir.glob("run*.txt")]

            if instance_runs:
                runs[instance_dir.name] = sorted(instance_runs, key=lambda run: run.number)

    return dict(sorted(runs.items(), key=lambda item: item[1][0].dimension))