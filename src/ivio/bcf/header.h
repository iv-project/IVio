#pragma once

#include <vector>
#include <string>

namespace ivio::bcf {

struct header {
    std::vector<std::tuple<std::string, std::string>> table;
    std::vector<std::string> genotypes;
};

}
