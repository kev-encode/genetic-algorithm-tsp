#pragma once
#include "instance.h"
#include <optional>
#include <random>
#include <vector>

// note: for d18512, 10000 seems to be a good upper-bound; many runs stop between 6000-8000 generations
const int MAX_GENERATIONS = 10000;

// note: less than 200 results in premature convergence
const int CONVERGENCE_GENERATIONS = 200;
const int POPULATION_SIZE = 200;
const int SELECTION_SIZE = 7;
const double CROSSOVER_RATE = 0.85;

// note: having guaranteed mutation resulted in better, consistent tour diversity
const double MUTATION_RATE = 1.0;

struct Generation {
    int number;
    double time;
    long long elite_fitness;
};

// note: returned instead of printed; caller decides where output goes (file write happens after timing stops)
struct RunResult {
    long long elite_fitness;
    std::vector<int> elite_tour;
    std::vector<Generation> generations;
};

std::vector<std::vector<int>> create_population(int dimension, std::mt19937& rng);
long long compute_fitness(const std::vector<int>& tour, const Instance& instance);
int run_elitism(const std::vector<long long>& fitnesses);
int run_selection(const std::vector<long long>& fitnesses, std::mt19937& rng);
std::vector<int> run_crossover(const std::vector<int>& parent1, const std::vector<int>& parent2, std::mt19937& rng);
void run_mutation(std::vector<int>& tour, std::mt19937& rng);
std::vector<int> make_offspring(const std::vector<std::vector<int>>& population, const std::vector<long long>& fitnesses, std::mt19937& rng);
void run_two_opt(std::vector<int>& tour, const Instance& instance);
std::optional<double> compute_percent_error(long long fitness, const Instance& instance);
RunResult run_genetic_algorithm(const Instance& instance, std::mt19937& rng);