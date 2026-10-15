#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

int main() {
    std::ifstream inFile("../res/instances/tsp/a280.tsp");

    if (!inFile.is_open()) {
        std::cout << "error: could not open file" << '\n';
        return 1;
    }

    std::string line;
    while (std::getline(inFile, line)) {
        std::stringstream ss(line);
        std::string word;
        while (ss >> word) {
            std::cout << word;
        }
    }

    return 0;
}