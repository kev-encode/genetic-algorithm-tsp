#include "algorithm.h"
#include <algorithm>
#include <chrono>
#include <deque>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <utility>

std::vector<std::vector<int>> create_population(int dimension, std::mt19937& rng) {
    std::vector<std::vector<int>> population(POPULATION_SIZE, std::vector<int>(dimension));

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

int run_selection(const std::vector<long long>& fitnesses, std::mt19937& rng) {
    std::uniform_int_distribution<int> pick(0, static_cast<int>(fitnesses.size()) - 1);
    int elite = pick(rng);

    for (int i = 1; i < SELECTION_SIZE; ++i) {
        int contender = pick(rng);

        if (fitnesses[contender] < fitnesses[elite]) {
            elite = contender;
        }
    }

    return elite;
}

std::vector<int> run_crossover(const std::vector<int>& parent1, const std::vector<int>& parent2, std::mt19937& rng) {
    std::uniform_real_distribution<double> chance(0.0, 1.0);

    if (chance(rng) > CROSSOVER_RATE) {
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

    // note: in_slice[node] is true if that node was copied from parent1's slice
    std::vector<bool> in_slice(dimension, false);

    for (int position = node1; position < node2; ++position) { 
        offspring[position] = parent1[position];
        in_slice[parent1[position]] = true;
    }

    // note: allows the program to iterate through parent2 once, skipping nodes copied from the slice
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

// note: mutates in place; copying is unnecessary
void run_mutation(std::vector<int>& tour, std::mt19937& rng) {
    std::uniform_real_distribution<double> chance(0.0, 1.0);

    if (chance(rng) >= MUTATION_RATE) {
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

std::vector<int> make_offspring(const std::vector<std::vector<int>>& population, const std::vector<long long>& fitnesses, std::mt19937& rng) {
    int parent1 = run_selection(fitnesses, rng);
    int parent2 = run_selection(fitnesses, rng);

    while (parent2 == parent1) {
        parent2 = run_selection(fitnesses, rng);
    }

    std::vector<int> offspring = run_crossover(population[parent1], population[parent2], rng);
    run_mutation(offspring, rng);
    return offspring;
}

// note: reverses tour positions from..to (cyclic, inclusive), keeping position in sync
// note: reverses the complement instead when shorter; same cycle, half the work on average
void reverse_segment(std::vector<int>& tour, std::vector<int>& position, int from, int to) {
    int dimension = static_cast<int>(tour.size());
    int length = (to - from + dimension) % dimension + 1;

    if (2 * length > dimension) {
        int complement_from = (to + 1) % dimension;
        to = (from - 1 + dimension) % dimension;
        from = complement_from;
        length = dimension - length;
    }

    for (int swap = 0; swap < length / 2; ++swap) {
        int node1 = tour[from];
        int node2 = tour[to];
        tour[from] = node2;
        position[node2] = from;
        tour[to] = node1;
        position[node1] = to;
        from = (from + 1) % dimension;
        to = (to - 1 + dimension) % dimension;
    }
}

void activate_node(int node, std::deque<int>& active_nodes, std::vector<bool>& is_active) {
    if (!is_active[node]) {
        active_nodes.push_back(node);
        is_active[node] = true;
    }
}

// note: tries one improving 2-opt move involving node; true if a move was applied
// note: move removes (node, adjacent) + (candidate, candidate_adjacent), adds (node, candidate) + (adjacent, candidate_adjacent)
bool improve_node(int node, std::vector<int>& tour, std::vector<int>& position, const Instance& instance, std::deque<int>& active_nodes, std::vector<bool>& is_active) {
    int dimension = static_cast<int>(tour.size());

    // note: direction 0 = successor side, 1 = predecessor side; both needed to find every candidate move
    for (int direction = 0; direction < 2; ++direction) {
        int step = direction == 0 ? 1 : dimension - 1;
        int adjacent = tour[(position[node] + step) % dimension];
        int adjacent_distance = instance.get_distance(node, adjacent);

        for (int rank = 0; rank < instance.get_neighbor_count(); ++rank) {
            int candidate = instance.get_neighbor(node, rank);
            int candidate_distance = instance.get_distance(node, candidate);

            // note: new edge must be shorter than the edge it replaces at node; sorted list -> no later candidate qualifies
            if (candidate_distance >= adjacent_distance) {
                break;
            }

            int candidate_adjacent = tour[(position[candidate] + step) % dimension];

            // note: skip moves sharing an edge (degenerate, no change)
            if (candidate == adjacent || candidate_adjacent == node) {
                continue;
            }

            int gain = adjacent_distance + instance.get_distance(candidate, candidate_adjacent)
                     - candidate_distance - instance.get_distance(adjacent, candidate_adjacent);

            if (gain > 0) {
                if (direction == 0) {
                    reverse_segment(tour, position, position[adjacent], position[candidate]);
                }

                else {
                    reverse_segment(tour, position, position[node], position[candidate_adjacent]);
                }

                // note: edges changed at these 4 nodes; recheck them
                activate_node(node, active_nodes, is_active);
                activate_node(adjacent, active_nodes, is_active);
                activate_node(candidate, active_nodes, is_active);
                activate_node(candidate_adjacent, active_nodes, is_active);
                return true;
            }
        }
    }

    return false;
}

// note: neighbor-list 2-opt with don't-look bits; ~n * k checks instead of ~n^2 / 2 per pass
// note: active queue = nodes without don't-look bit; only nodes near changed edges get rechecked
void run_two_opt(std::vector<int>& tour, const Instance& instance) {
    int dimension = static_cast<int>(tour.size());

    // note: position[node] = index in tour; O(1) lookup of a candidate's tour neighbors
    std::vector<int> position(dimension);

    for (int index = 0; index < dimension; ++index) {
        position[tour[index]] = index;
    }

    std::deque<int> active_nodes(tour.begin(), tour.end());
    std::vector<bool> is_active(dimension, true);

    while (!active_nodes.empty()) {
        int node = active_nodes.front();
        active_nodes.pop_front();
        is_active[node] = false;
        improve_node(node, tour, position, instance, active_nodes, is_active);
    }
}

std::optional<double> compute_percent_error(long long fitness, const Instance& instance) {
    std::optional<long long> optimal_fitness = instance.get_optimal_fitness();

    if (!optimal_fitness) {
        return std::nullopt;
    }

    // note: 100.0 forces floating-point division
    return (fitness - *optimal_fitness) * 100.0 / *optimal_fitness;
}

void print_generation(int generation, double generation_time, long long elite_fitness, const Instance& instance) {
    std::optional<double> percent_error = compute_percent_error(elite_fitness, instance);
    std::cout << generation << ", " << std::fixed << std::setprecision(6) << generation_time << ", " << elite_fitness << ", ";

    if (percent_error) {
        std::cout << std::setprecision(3) << *percent_error;
    }

    else {
        std::cout << "n/a";
    }

    std::cout << '\n';
}

double get_elapsed_time(std::chrono::steady_clock::time_point start) {
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
}

long long run_genetic_algorithm(const Instance& instance, std::mt19937& rng) {
    std::chrono::steady_clock::time_point generation_start = std::chrono::steady_clock::now();
    std::vector<std::vector<int>> population = create_population(instance.get_dimension(), rng);
    std::vector<long long> fitnesses;
    fitnesses.reserve(POPULATION_SIZE);

    for (const std::vector<int>& tour : population) {
        fitnesses.push_back(compute_fitness(tour, instance));
    }

    int elite = run_elitism(fitnesses);
    std::vector<int> elite_tour = population[elite];
    long long elite_fitness = fitnesses[elite];
    print_generation(0, get_elapsed_time(generation_start), elite_fitness, instance);
    int stagnant_generations = 0;

    // note: allocated once, swapped with population each generation; no per-generation rebuild
    // note: pre-sized; each thread writes only its own slot (push_back not thread-safe)
    std::vector<std::vector<int>> next_population(POPULATION_SIZE);
    std::vector<long long> next_fitnesses(POPULATION_SIZE);
    std::vector<std::mt19937::result_type> seeds(POPULATION_SIZE);

    for (int generation = 1; generation <= MAX_GENERATIONS; ++generation) {
        generation_start = std::chrono::steady_clock::now();
        
        // note: slot 0 = elite (elitism); slots 1+ = new offspring
        next_population[0] = elite_tour;
        next_fitnesses[0] = elite_fitness;

        // note: one generator per offspring; shared mt19937 across threads = data race
        // note: seeds drawn serially before parallel loop; same seed = same run, regardless of thread count
        for (int i = 1; i < POPULATION_SIZE; ++i) {
            seeds[i] = rng();
        }

        // note: offspring independent, shared data read-only; parallel without locks
        // note: dynamic scheduling; 2-opt time varies per offspring
        #pragma omp parallel for schedule(dynamic)
        for (int i = 1; i < POPULATION_SIZE; ++i) {
            std::mt19937 offspring_rng(seeds[i]);
            next_population[i] = make_offspring(population, fitnesses, offspring_rng);
            run_two_opt(next_population[i], instance);
            next_fitnesses[i] = compute_fitness(next_population[i], instance);
        }

        population.swap(next_population);
        fitnesses.swap(next_fitnesses);
        elite = run_elitism(fitnesses);

        if (fitnesses[elite] < elite_fitness) {
            elite_tour = population[elite];
            elite_fitness = fitnesses[elite];
            stagnant_generations = 0;
        }

        else {
            ++stagnant_generations;
        }

        run_two_opt(elite_tour, instance);
        elite_fitness = compute_fitness(elite_tour, instance);
        print_generation(generation, get_elapsed_time(generation_start), elite_fitness, instance);

        // note: empty optional compares false; no known optimum -> stagnation stop only
        if (stagnant_generations >= CONVERGENCE_GENERATIONS || elite_fitness == instance.get_optimal_fitness()) {
            break;
        }
    }

    return elite_fitness;
}