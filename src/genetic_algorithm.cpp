#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

struct Node {
    int number;
    double x_coord;
    double y_coord;
};

std::vector<Node> load_instance(const std::string& instance_file) {
    std::vector<Node> nodes;
    std::ifstream file(instance_file);

    if (!file.is_open()) {
        // thought: should insert better error handling
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