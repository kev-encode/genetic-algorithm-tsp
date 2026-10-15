#include <iostream>
#include <fstream>
#include <string>

int main() {
    std::ifstream inFile("../res/instances/tsp/a280.tsp");

    if (!inFile.is_open()) {
        std::cout << "error: could not open file" << std::endl;
        return 1;
    }

    std::string line;
    while (std::getline(inFile, line)) {
        std::cout << line << std::endl;
    }

    return 0;
}