#include <string>
#include <unordered_set>
#include <variant>
#include <iostream>
#include <memory>

#include <chrono>
#include <optional>

#include "../includes/Core/Types/BaseDataObject.h"


int main() {
std::cout << sizeof(daikon::core::types::BaseDataObject) << " " << sizeof(std::unique_ptr<int>);
    return 0;
}