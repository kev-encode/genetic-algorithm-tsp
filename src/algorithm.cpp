#include "algorithm.h"
#include <algorithm>
#include <numeric>

std::vector<std::vector<int>> create_population(int population_size, int dimension, std::mt19937& rng) {
    std::vector<std::vector<int>> population(population_size, std::vector<int>(dimension));

    for (std::vector<int>& tour : population) {
        std::iota(tour.begin(), tour.end(), 0);
        std::shuffle(tour.begin(), tour.end(), rng);
    }

    return population;
}

long long compute_fitness(const std::vector<int>& tour, const Instance& instance) {
    long long fitness = 0;
    int dimension = static_cast<int>(tour.size());

    for (int position = 0; position < dimension - 1; ++position) {
        fitness += instance.get_distance(tour[position], tour[position + 1]);
    }

    fitness += instance.get_distance(tour[dimension - 1], tour[0]);
    return fitness;
}

int run_elitism(const std::vector<long long>& fitnesses) {
    return static_cast<int>(std::min_element(fitnesses.begin(), fitnesses.end()) - fitnesses.begin());
}

int run_selection(const std::vector<long long>& fitnesses, int selection_size, std::mt19937& rng) {
    std::uniform_int_distribution<int> pick(0, static_cast<int>(fitnesses.size()) - 1);
    int elite = pick(rng);

    for (int i = 1; i < selection_size; ++i) {
        int contender = pick(rng);

        if (fitnesses[contender] < fitnesses[elite]) {
            elite = contender;
        }
    }

    return elite;
}

std::vector<int> run_crossover(const std::vector<int>& parent1, const std::vector<int>& parent2, double crossover_rate, std::mt19937& rng) {
    std::uniform_real_distribution<double> chance(0.0, 1.0);

    if (chance(rng) > crossover_rate) {
        return chance(rng) < 0.5 ? parent2 : parent1;
    }

    int dimension = static_cast<int>(parent1.size());
    std::uniform_int_distribution<int> pick(0, dimension - 1);
    int node1 = pick(rng);
    int node2 = pick(rng);

    while (node1 == node2) {
        node2 = pick(rng);
    }

    if (node1 > node2) {
        std::swap(node1, node2);
    }

    std::vector<int> offspring(dimension);
    std::vector<bool> in_slice(dimension, false);

    for (int position = node1; position < node2; ++position) { 
        offspring[position] = parent1[position];
        in_slice[parent1[position]] = true;
    }

    int parent2_position = 0;

    for (int offspring_position = 0; offspring_position < dimension; ++offspring_position) {
        if (offspring_position >= node1 && offspring_position < node2) {
            continue;
        }

        while (in_slice[parent2[parent2_position]]) {
            ++parent2_position;
        }

        offspring[offspring_position] = parent2[parent2_position++];
    }

    return offspring;
}

void run_mutation(std::vector<int>& tour, double mutation_rate, std::mt19937& rng) {
    std::uniform_real_distribution<double> chance(0.0, 1.0);

    if (chance(rng) >= mutation_rate) {
        return;
    }

    int dimension = static_cast<int>(tour.size());
    std::uniform_int_distribution<int> pick(0, dimension - 1);
    int position1 = pick(rng);
    int position2 = pick(rng);

    while (position1 == position2) {
        position2 = pick(rng);
    }

    std::swap(tour[position1], tour[position2]);
}