// -----------------------------------------------------------------------------------------------------
// Copyright (c) 2006-2022, Knut Reinert & Freie Universität Berlin
// Copyright (c) 2016-2022, Knut Reinert & MPI für molekulare Genetik
// This file may be used, modified and/or redistributed under the terms of the 3-clause BSD-License
// shipped with this file and also available at: https://github.com/seqan/seqan3/blob/master/LICENSE.md
// -----------------------------------------------------------------------------------------------------

#include <benchmark/benchmark.h>

#include <seqan3/io/sequence_file/input.hpp>
#include <seqan3/io/sequence_file/output.hpp>
#include <seqan3/test/performance/sequence_generator.hpp>
#include <seqan3/test/performance/units.hpp>
#include <seqan3/test/seqan2.hpp>
#include <seqan3/test/tmp_filename.hpp>

#include <seqan/seq_io.h>

constexpr size_t default_seed{4242u};
static inline std::string fastq_id{"some_id"};
constexpr size_t total_size{1ULL << 24};

std::string generate_fastq_string(size_t const sequence_length)
{
    std::ostringstream stream_buffer{};
    seqan3::sequence_file_output fastq_ostream{stream_buffer, seqan3::format_fasta{}};

    for (size_t i = 0, seed = default_seed; i < total_size / sequence_length; ++i, ++seed)
        fastq_ostream.emplace_back(seqan3::test::generate_sequence<seqan3::dna5>(sequence_length, 0, seed), fastq_id);

    stream_buffer.flush();
    return stream_buffer.str();
}

[[nodiscard]] seqan3::test::tmp_filename create_fasta_file_for(std::string const & fastq_string)
{
    seqan3::test::tmp_filename fasta_file{"format_fasta_benchmark_test_file.fasta"};
    auto fasta_file_path = fasta_file.get_path();

    std::ofstream ostream{fasta_file_path};
    ostream << fastq_string;
    ostream.close();
    return fasta_file;
}

void fasta_read_from_stream_seqan3(benchmark::State & state)
{
    size_t const sequence_length = state.range(0);
    std::string fasta_file = generate_fastq_string(sequence_length);
    std::istringstream istream{fasta_file};

    for (auto _ : state)
    {
        istream.clear();
        istream.seekg(0, std::ios::beg);
        seqan3::sequence_file_input fin{istream, seqan3::format_fasta{}};

        for (auto it = fin.begin(); it != fin.end(); ++it)
            benchmark::DoNotOptimize(it);
    }

    size_t bytes_per_run = fasta_file.size();
    state.counters["bytes_per_run"] = bytes_per_run;
    state.counters["bytes_per_second"] = seqan3::test::bytes_per_second(bytes_per_run);
}

void fasta_read_from_disk_seqan3(benchmark::State & state)
{
    size_t const sequence_length = state.range(0);
    std::string fasta_file = generate_fastq_string(sequence_length);
    seqan3::test::tmp_filename file_name = create_fasta_file_for(fasta_file);

    for (auto _ : state)
    {
        seqan3::sequence_file_input fin{file_name.get_path()};
        for (auto it = fin.begin(); it != fin.end(); ++it)
            benchmark::DoNotOptimize(it);
    }

    size_t bytes_per_run = fasta_file.size();
    state.counters["bytes_per_run"] = bytes_per_run;
    state.counters["bytes_per_second"] = seqan3::test::bytes_per_second(bytes_per_run);
}

// seqan3::format_fasta needs to have read_id and read_seq set as public
void fasta_read_from_disk_stream_seqan3(benchmark::State & state)
{
    seqan3::sequence_file_input_options<seqan3::dna15> options{};

    size_t const sequence_length = state.range(0);
    std::string fasta_file = generate_fastq_string(sequence_length);
    seqan3::test::tmp_filename file_name = create_fasta_file_for(fasta_file);

    for (auto _ : state)
    {
        std::ifstream fin{file_name.get_path(), std::ios_base::in | std::ios::binary};
        auto stream_view = seqan3::detail::istreambuf(fin);
        seqan3::format_fasta reader{};
        std::string id{};
        std::vector<seqan3::dna5> sequence{};
        while (!((std::istreambuf_iterator<char>{fin} == std::istreambuf_iterator<char>{})))
        {
            reader.read_id(stream_view, options, id);
            reader.read_seq(stream_view, options, sequence);
            benchmark::ClobberMemory();
            id.clear();
            sequence.clear();
        }
    }

    size_t bytes_per_run = fasta_file.size();
    state.counters["bytes_per_run"] = bytes_per_run;
    state.counters["bytes_per_second"] = seqan3::test::bytes_per_second(bytes_per_run);
}

void fasta_read_from_stream_seqan2(benchmark::State & state)
{
    size_t const sequence_length = state.range(0);
    std::string fasta_file = generate_fastq_string(sequence_length);
    std::istringstream istream{fasta_file};

    seqan::CharString id{};
    seqan::Dna5String seq{};
    seqan::CharString qual{};

    for (auto _ : state)
    {
        istream.clear();
        istream.seekg(0, std::ios::beg);
        auto it = seqan::Iter<std::istringstream, seqan::StreamIterator<seqan::Input>>(istream);

        while (!seqan::atEnd(it))
        {
            readRecord(id, seq, qual, it, seqan::Fasta{});
            benchmark::ClobberMemory();
            clear(id);
            clear(seq);
            clear(qual);
        }
    }

    size_t bytes_per_run = fasta_file.size();
    state.counters["bytes_per_run"] = bytes_per_run;
    state.counters["bytes_per_second"] = seqan3::test::bytes_per_second(bytes_per_run);
}

void fasta_read_from_disk_seqan2(benchmark::State & state)
{
    size_t const sequence_length = state.range(0);
    std::string fasta_file = generate_fastq_string(sequence_length);
    seqan3::test::tmp_filename file_name = create_fasta_file_for(fasta_file);

    seqan::CharString id{};
    seqan::Dna5String seq{};
    seqan::CharString qual{};

    for (auto _ : state)
    {
        seqan::SeqFileIn seqFileIn(file_name.get_path().c_str());
        while (!seqan::atEnd(seqFileIn))
        {
            readRecord(id, seq, qual, seqFileIn);
            benchmark::ClobberMemory();
            clear(id);
            clear(seq);
            clear(qual);
        }
    }

    size_t bytes_per_run = fasta_file.size();
    state.counters["bytes_per_run"] = bytes_per_run;
    state.counters["bytes_per_second"] = seqan3::test::bytes_per_second(bytes_per_run);
}

BENCHMARK(fasta_read_from_stream_seqan3)->Arg(50)->Arg(1000);
BENCHMARK(fasta_read_from_disk_seqan3)->Arg(50)->Arg(1000);
BENCHMARK(fasta_read_from_stream_seqan2)->Arg(50)->Arg(1000);
BENCHMARK(fasta_read_from_disk_seqan2)->Arg(50)->Arg(1000);

BENCHMARK(fasta_read_from_disk_stream_seqan3)->Arg(50)->Arg(1000);
BENCHMARK_MAIN();
