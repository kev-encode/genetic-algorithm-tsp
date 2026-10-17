#pragma once
#include <cmath>
#include <optional>
#include <string>
#include <utility>
#include <vector>

struct Node {
    int number;
    double x_coord;
    double y_coord;
};

class Instance {
public:
    explicit Instance(std::string name, std::vector<Node> nodes, std::optional<long long> optimal_fitness)
        : name(std::move(name)),
          nodes(std::move(nodes)),
          dimension(static_cast<int>(this->nodes.size())),
          optimal_fitness(optimal_fitness) {}

    static Instance get_instance(const std::string& instance_file);

    int get_distance(int node1, int node2) const {
        return compute_distance(node1, node2);
    }

    int get_dimension() const {
        return dimension;
    }

    const std::string& get_name() const {
        return name;
    }

    std::optional<long long> get_optimal_fitness() const {
        return optimal_fitness;
    }

private:
    int compute_distance(int node1, int node2) const {
        double dx = nodes[node1].x_coord - nodes[node2].x_coord;
        double dy = nodes[node1].y_coord - nodes[node2].y_coord;
        return static_cast<int>(std::lround(std::sqrt(dx * dx + dy * dy)));
    }

    static std::string get_header(const std::string& line);
    static std::optional<long long> find_optimal_fitness(const std::string& opt_fits_file, const std::string& instance_name);

    std::string name;
    std::vector<Node> nodes;
    int dimension = 0;
    std::optional<long long> optimal_fitness;
};