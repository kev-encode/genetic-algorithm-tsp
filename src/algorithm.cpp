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
    int n = static_cast<int>(tour.size());

    for (int i = 0; i < n - 1; ++i) {
        fitness += instance.get_distance(tour[i], tour[i + 1]);
    }

    fitness += instance.get_distance(tour[n - 1], tour[0]);
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

    std::vector<int> child(dimension);
    std::vector<bool> in_slice(dimension, false);

    for (int i = node1; i < node2; ++i) { 
        child[i] = parent1[i];
        in_slice[parent1[i]] = true;
    }

    int j = 0;
    
    for (int i = 0; i < dimension; ++i) {
        if (i >= node1 && i < node2) {
            continue;
        }

        while (in_slice[parent2[j]]) {
            ++j;
        }

        child[i] = parent2[j++];
    }

    return child;
}