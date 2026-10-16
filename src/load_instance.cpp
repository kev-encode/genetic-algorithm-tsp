#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <vector>
#include <string>
#include <stdexcept>
#include <utility>

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

Instance Instance::get_instance(const std::string& instance_file) {
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
        } else if (line.rfind("DIMENSION", 0) == 0) {
            expected_dimension = std::stoi(get_header(line));
        } else if (line.rfind("NODE_COORD_SECTION") != std::string::npos) {
            Node node;

            while (file >> node.number >> node.x_coord >> node.y_coord) {
                nodes.push_back(node);
            }

            break;

        } else if (line.find("EOF") != std::string::npos) {
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
    return Instance(std::move(name), std::move(nodes));
}

int main() {
    const std::string instance_file = "../res/instances/tsp/euc2d/a280.tsp";
    return 0;
}