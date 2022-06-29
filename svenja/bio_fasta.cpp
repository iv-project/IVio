#include <bio/seq_io/reader.hpp>

int main(int, char ** argv)
{
    bio::seq_io::reader in{argv[1]};

    int sum{0};
    for (auto && rec : in)
        sum += std::ranges::size(rec.sequence());

    std::cout << sum << std::endl;
}
