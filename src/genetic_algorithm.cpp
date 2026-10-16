#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <vector>
#include <string>

struct Node {
    int number;
    double x_coord;
    double y_coord;
};

class Instance {
public:
    explicit Instance(std::string& instance_file);

    int get_distance(int node1, int node2) const {
        return compute_distance(node1, node2);
    }

    int get_dimension() const {
        return dimension;
    }

private:
    int compute_distance(int node1, int node2) const {
        double dx = nodes[node1].x_coord - nodes[node2].x_coord;
        double dy = nodes[node1].y_coord - nodes[node2].y_coord;
        return static_cast<int>(std::ceil(std::sqrt(dx * dx + dy * dy)));
    }

    std::string name;
    std::vector<Node> nodes;
    int dimension = 0;
};

std::vector<Node> load_instance(const std::string& instance_file) {
    std::vector<Node> nodes;
    std::ifstream file(instance_file);

    if (!file.is_open()) {
        // thought: insert better error handling
        std::cout << "error: could not open file" << '\n';
        return nodes;
    }

    std::string line;

    while (std::getline(file, line)) {
        if (line.find("NODE_COORD_SECTION") != std::string::npos) {
            Node node;
            while (file >> node.number >> node.x_coord >> node.y_coord) {
                nodes.push_back(node);
            }
            
            break;
        }

        if (line.find("EOF") != std::string::npos) {
            break;
        }
    }

    file.close();
    return nodes;
}

int main() {
    const std::string instance_file = "../res/instances/tsp/euc2d/a280.tsp";
    std::vector<Node> instance_nodes = load_instance(instance_file);
    return 0;
}