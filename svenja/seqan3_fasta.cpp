#include <seqan3/io/sequence_file/all.hpp>

int main(int, char ** argv)
{
    seqan3::sequence_file_input in{argv[1]};

    int sum{0};
    for (auto && rec : in)
        sum += std::ranges::size(rec.sequence());

    std::cout << sum << std::endl;
}
