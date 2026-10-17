#include "instance.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <new>
#include <stdexcept>
#include <string>

void Instance::build_distance_matrix() {
    std::size_t matrix_size = static_cast<std::size_t>(dimension) * dimension;

    if (static_cast<long long>(matrix_size * sizeof(int)) > MAX_MATRIX_BYTES) {
        return;
    }

    // note: allocation failure not fatal; empty matrix -> get_distance computes on the fly
    try {
        distance_matrix.resize(matrix_size);
    }

    catch (const std::bad_alloc&) {
        distance_matrix.clear();
        return;
    }

    // note: symmetric distances; compute each pair once and mirror (halves build time)
    for (int node1 = 0; node1 < dimension; ++node1) {
        distance_matrix[static_cast<std::size_t>(node1) * dimension + node1] = 0;

        for (int node2 = node1 + 1; node2 < dimension; ++node2) {
            int distance = compute_distance(node1, node2);
            distance_matrix[static_cast<std::size_t>(node1) * dimension + node2] = distance;
            distance_matrix[static_cast<std::size_t>(node2) * dimension + node1] = distance;
        }
    }
}

void Instance::build_neighbor_lists() {
    neighbor_count = std::min(NEIGHBOR_COUNT, dimension - 1);
    neighbor_lists.resize(static_cast<std::size_t>(dimension) * neighbor_count);

    // note: rows independent; parallel build (n * n distance reads, ~1 s for d18512 single-threaded)
    #pragma omp parallel
    {
        // note: one buffer per thread, reused across rows
        std::vector<std::pair<int, int>> candidates(dimension - 1);

        #pragma omp for
        for (int node1 = 0; node1 < dimension; ++node1) {
            int count = 0;

            for (int node2 = 0; node2 < dimension; ++node2) {
                if (node2 != node1) {
                    candidates[count++] = {get_distance(node1, node2), node2};
                }
            }

            // note: (distance, node) pairs; ties broken by node number -> deterministic lists
            std::partial_sort(candidates.begin(), candidates.begin() + neighbor_count, candidates.end());

            for (int rank = 0; rank < neighbor_count; ++rank) {
                neighbor_lists[static_cast<std::size_t>(node1) * neighbor_count + rank] = candidates[rank].second;
            }
        }
    }
}

std::string Instance::get_header(const std::string& line) {
    std::size_t colon = line.find(':');

    if (colon == std::string::npos) {
        return "";
    }

    std::size_t start = line.find_first_not_of(" \t", colon + 1);
    std::size_t end = line.find_last_not_of(" \t\r");

    if (start == std::string::npos) {
        return "";
    }

    return line.substr(start, end - start + 1);
}

std::optional<long long> Instance::find_optimal_fitness(const std::string& optimality_file, const std::string& instance_name) {
    std::ifstream file(optimality_file);
    std::string line;

    while (std::getline(file, line)) {
        std::size_t colon = line.find(':');

        if (colon == std::string::npos) {
            continue;
        }

        std::string key = line.substr(0, colon);
        key.erase(key.find_last_not_of(" \t") + 1);

        if (key == instance_name) {
            return std::stoll(get_header(line));
        }
    }

    return std::nullopt;
}

Instance Instance::get_instance(const std::string& instance_file, const std::string& optimality_file) {
    std::ifstream file(instance_file);

    if (!file.is_open()) {
        throw std::runtime_error("error: could not open file: " + instance_file);
    }

    std::string name;
    int expected_dimension = -1;
    std::vector<Node> nodes;
    std::string line;

    while (std::getline(file, line)) {
        if (line.rfind("NAME", 0) == 0) {
            name = get_header(line);
        }
        
        else if (line.rfind("DIMENSION", 0) == 0) {
            expected_dimension = std::stoi(get_header(line));
        }
        
        else if (line.rfind("NODE_COORD_SECTION") != std::string::npos) {
            Node node;

            while (file >> node.number >> node.x_coord >> node.y_coord) {
                nodes.push_back(node);
            }

            break;
        }
        
        else if (line.find("EOF") != std::string::npos) {
            break;
        }
    }

    if (nodes.empty()) {
        throw std::runtime_error("error: no NODE_COORD_SECTION found in " + instance_file);
    }

    if (expected_dimension != -1 && expected_dimension != static_cast<int>(nodes.size())) {
        throw std::runtime_error("error: DIMENSION does not match node count in " + instance_file);
    }

    file.close();
    
    // note: lookup by filename, not NAME header; linhp318.tsp header says lin318 (wrong optimum)
    std::string instance_name = std::filesystem::path(instance_file).stem().string();
    std::optional<long long> optimal_fitness = find_optimal_fitness(optimality_file, instance_name);
    return Instance(std::move(name), std::move(nodes), optimal_fitness);
}