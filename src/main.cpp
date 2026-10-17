#include "algorithm.h"
#include "instance.h"
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <optional>
#include <random>
#include <string>
#include <vector>

// note: filenames without .tsp, from res/instances/euc2d/; run in listed order
// note: all 78 EUC_2D instances, sorted by dimension; small ones finish first, ~88% of batch time is the last 5
const std::vector<std::string> INSTANCES_TO_RUN = {
    "eil51",   "berlin52", "st70",     "eil76",    "pr76",    "rat99",    "kroA100", "kroB100",
    "kroC100", "kroD100",  "kroE100",  "rd100",    "eil101",  "lin105",   "pr107",   "pr124",
    "bier127", "ch130",    "pr136",    "pr144",    "ch150",   "kroA150",  "kroB150", "pr152",
    "u159",    "rat195",   "d198",     "kroA200",  "kroB200", "ts225",    "tsp225",  "pr226",
    "gil262",  "pr264",    "a280",     "pr299",    "lin318",  "linhp318", "rd400",   "fl417",
    "pr439",   "pcb442",   "d493",     "u574",     "rat575",  "p654",     "d657",    "u724",
    "rat783",  "pr1002",   "u1060",    "vm1084",   "pcb1173", "d1291",    "rl1304",  "rl1323",
    "nrw1379", "fl1400",   "u1432",    "fl1577",   "d1655",   "vm1748",   "u1817",   "rl1889",
    "d2103",   "u2152",    "u2319",    "pr2392",   "pcb3038", "fl3795",   "fnl4461", "rl5915",
    "rl5934",  "rl11849",  "usa13509", "brd14051", "d15112",  "d18512",
};

const int RUNS_PER_INSTANCE = 10;

void print_instance(const std::string& instance_name, const Instance& instance, double load_time) {
    std::optional<long long> optimal_fitness = instance.get_optimal_fitness();

    // note: filename, not instance.get_name(); some NAME headers differ from filename (linhp318)
    std::cout << "instance: " << instance_name << '\n';
    std::cout << "dimension: " << instance.get_dimension() << '\n';
    std::cout << "optimality: ";

    if (optimal_fitness) {
        std::cout << *optimal_fitness;
    }

    else {
        std::cout << "n/a";
    }

    std::cout << "\nload time: " << std::fixed << std::setprecision(3) << load_time << '\n';
}

void print_run(long long elite_fitness, double computation_time, const Instance& instance) {
    std::optional<double> percent_error = compute_percent_error(elite_fitness, instance);

    std::cout << "\nelite: " << elite_fitness << '\n';
    std::cout << "percent error: ";

    if (percent_error) {
        std::cout << std::fixed << std::setprecision(3) << *percent_error;
    }

    else {
        std::cout << "n/a";
    }

    std::cout << "\ncomputation time: " << std::fixed << std::setprecision(3) << computation_time << '\n';
}

double get_seconds_since(std::chrono::steady_clock::time_point start) {
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
}

int main(int, char** argv) {
    // note: project root = parent of executable's folder (build/); paths work from any working directory
    std::filesystem::path root_directory = std::filesystem::absolute(argv[0]).parent_path().parent_path();
    std::filesystem::path instance_directory = root_directory / "res" / "instances" / "euc2d";
    std::filesystem::path optimality_file = root_directory / "res" / "optimal_values.txt";

    // note: check every file up front; a typo late in the list would otherwise stop the batch hours in
    for (const std::string& instance_name : INSTANCES_TO_RUN) {
        if (!std::filesystem::exists(instance_directory / (instance_name + ".tsp"))) {
            std::cerr << "error: instance file not found: " << instance_name << ".tsp\n";
            return 1;
        }
    }

    std::mt19937 rng(std::random_device{}());

    for (const std::string& instance_name : INSTANCES_TO_RUN) {
        // note: loaded once, shared by all runs of this instance; distance matrix build is the slow part (~3 s for d18512)
        // note: instance scoped to this iteration; its matrix is freed before the next instance loads
        std::chrono::steady_clock::time_point load_start = std::chrono::steady_clock::now();
        std::filesystem::path instance_file = instance_directory / (instance_name + ".tsp");
        Instance instance = Instance::get_instance(instance_file.string(), optimality_file.string());
        print_instance(instance_name, instance, get_seconds_since(load_start));

        for (int run = 1; run <= RUNS_PER_INSTANCE; ++run) {
            // note: computation time excludes loading; load time printed once per instance
            std::chrono::steady_clock::time_point run_start = std::chrono::steady_clock::now();
            std::cout << "\nrun: " << run << " / " << RUNS_PER_INSTANCE << "\n\ngenerations:\n";
            long long elite_fitness = run_genetic_algorithm(instance, rng);
            print_run(elite_fitness, get_seconds_since(run_start), instance);
        }

        std::cout << "\n\n";
    }

    return 0;
}
