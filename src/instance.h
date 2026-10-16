#pragma once
#include <cmath>
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
    explicit Instance(std::string name, std::vector<Node> nodes)
        : name(std::move(name)),
          nodes(std::move(nodes)),
          dimension(static_cast<int>(this->nodes.size())) {}

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

private:
    int compute_distance(int node1, int node2) const {
        double dx = nodes[node1].x_coord - nodes[node2].x_coord;
        double dy = nodes[node1].y_coord - nodes[node2].y_coord;
        return static_cast<int>(std::lround(std::sqrt(dx * dx + dy * dy)));
    }

    static std::string get_header(const std::string& line);

    std::string name;
    std::vector<Node> nodes;
    int dimension = 0;
};