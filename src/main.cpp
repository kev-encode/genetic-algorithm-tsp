#include "instance.h"

#include <string>

int main() {
    const std::string instance_file = "../res/instances/tsp/euc2d/a280.tsp";
    Instance instance = Instance::get_instance(instance_file);
    return 0;
}