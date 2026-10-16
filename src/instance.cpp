#include "instance.h"
#include <fstream>
#include <stdexcept>
#include <string>

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