#pragma once

#include <vector>
#include <string>

namespace ivio::vcf {

struct header {
    std::vector<std::tuple<std::string, std::string>> table;
    std::vector<std::string> genotypes;
};

}
