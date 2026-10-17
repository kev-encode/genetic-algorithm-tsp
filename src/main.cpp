#include "algorithm.h"
#include "instance.h"
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <optional>
#include <random>
#include <string>

int main(int, char** argv) {
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    std::filesystem::path root_directory = std::filesystem::absolute(argv[0]).parent_path().parent_path();
    std::filesystem::path instance_file = root_directory / "res" / "tsp_instances" / "euc2d" / "a280.tsp";
    std::filesystem::path optimality_file = root_directory / "res" / "optimal_euc2d.txt";
    Instance instance = Instance::get_instance(instance_file.string(), optimality_file.string());
    std::optional<long long> optimal_fitness = instance.get_optimal_fitness();

    std::cout << "instance: " << instance_file.stem().string() << '\n';
    std::cout << "dimension: " << instance.get_dimension() << '\n';
    std::cout << "optimality: ";

    if (optimal_fitness) {
        std::cout << *optimal_fitness;
    }

    else {
        std::cout << "n/a";
    }

    std::cout << "\n\ngenerations:\n";

    std::mt19937 rng(std::random_device{}());
    long long elite_fitness = run_genetic_algorithm(instance, rng);
    double computation_time = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    std::optional<double> percent_error = compute_percent_error(elite_fitness, instance);

    std::cout << "\nelite: " << elite_fitness << '\n';
    std::cout << "percent error: ";

    if (percent_error) {
        std::cout << std::fixed << std::setprecision(3) << *percent_error;
    }

    else {
        std::cout << "n/a";
    }

    std::cout << "\ncomputation time: " << std::fixed << std::setprecision(3) << computation_time << "\n";
    return 0;
}
