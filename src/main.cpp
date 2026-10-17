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
    // note: timer starts before loading; computation time includes distance matrix build
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    
    // note: project root = parent of executable's folder (build/); paths work from any working directory
    std::filesystem::path root_directory = std::filesystem::absolute(argv[0]).parent_path().parent_path();
    std::filesystem::path instance_file = root_directory / "res" / "instances" / "euc2d" / "a280.tsp";
    std::filesystem::path optimality_file = root_directory / "res" / "optimal_values.txt";
    Instance instance = Instance::get_instance(instance_file.string(), optimality_file.string());
    std::optional<long long> optimal_fitness = instance.get_optimal_fitness();

    // note: filename, not instance.get_name(); some NAME headers differ from filename (linhp318)
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
