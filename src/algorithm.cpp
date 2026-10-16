#include "algorithm.h"
#include <algorithm>
#include <numeric>

std::vector<int> make_tour(int dimension, std::mt19937& rng) {
    std::vector<int> tour(dimension);
    std::iota(tour.begin(), tour.end(), 0);
    std::shuffle(tour.begin(), tour.end(), rng);
    return tour;
}