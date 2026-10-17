#include "algorithm.h"
#include "instance.h"
#include <iostream>
#include <random>
#include <string>

int main() {
    const std::string instance_file = "../res/instances/tsp/euc2d/a280.tsp";
    Instance instance = Instance::get_instance(instance_file);
    std::mt19937 rng(std::random_device{}());
    long long best = run_genetic_algorithm(instance, rng);
    std::cout << "best: " << best << '\n';
    return 0;
}