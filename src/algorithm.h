#pragma once
#include "instance.h"
#include <random>
#include <vector>

const int POPULATION_SIZE = 200;
const int SELECTION_SIZE = 7;
const double CROSSOVER_RATE = 0.85;

std::vector<std::vector<int>> create_population(int population_size, int dimension, std::mt19937& rng);
long long compute_fitness(const std::vector<int>& tour, const Instance& instance);
int run_elitism(const std::vector<long long>& fitnesses);
int run_selection(const std::vector<long long>& fitnesses, int selection_size, std::mt19937& rng);
std::vector<int> run_crossover(const std::vector<int>& parent1, const std::vector<int>& parent2, double crossover_rate, std::mt19937& rng);