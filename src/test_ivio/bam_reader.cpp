// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#include <catch2/catch_all.hpp>
#include <filesystem>
#include <fstream>
#include <ivio/ivio.h>

TEST_CASE("reading bam files", "[bam][reader]") {
    auto tmp = std::filesystem::temp_directory_path() / "ivio_test";
    std::filesystem::create_directory(tmp);

    auto expected = std::vector<ivio::bam::record> {
        { .refID = 0, .pos = 0, .mapq = 6, .bin = 4681, .flag = 2147, .next_refID = 0, .next_pos = 8952, .tlen = 9053, .read_name ="D00360:64:HBAP3ADXX:2:1115:16968:20035", .cigar = {213, 3, 0, 0, 128, 2, 0, 0}, .seq = {65, 130, 18, 20, 72, 40, 24, 33, 34, 40, 24, 129, 18, 33, 40, 33, 36, 68, 20, 40}, .seq_len = 40, .qual = {40, 38, 38, 35, 38, 38, 36, 39, 36, 32, 28, 33, 35, 37, 37, 37, 37, 34, 36, 34, 36, 36, 34, 36, 29, 34, 34, 34, 33, 34, 34, 34, 34, 34, 28, 23, 15, 29, 33, 27}},
        { .refID = 0, .pos = 0, .mapq = 60, .bin = 4681, .flag = 161, .next_refID = 0, .next_pos = 10469, .tlen = 10594, .read_name ="D00360:72:C6UP4ANXX:1:1115:17362:52449", .cigar = {228, 0, 0, 0, 240, 6, 0, 0}, .seq = {17, 65, 33, 130, 18, 65, 132, 65, 130, 18, 20, 72, 40, 24, 33, 34, 40, 24, 129, 18, 33, 40, 33, 36, 68, 20, 40, 40, 34, 24, 66, 24, 136, 68, 129, 136, 136, 36, 130, 132, 68, 68, 72, 24, 66, 18, 66, 65, 129, 66, 24, 132, 36, 20, 18, 66, 132, 65, 66, 36, 65, 66, 16}, .seq_len = 125, .qual = {34, 33, 33, 33, 33, 38, 38, 38, 38, 38, 38, 38, 38, 38, 35, 29, 34, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 14, 14, 27, 27, 38, 38, 36, 37, 34, 37, 38, 38, 38, 38, 34, 36, 36, 38, 29, 37, 38, 38, 37, 37, 37, 37, 32, 33, 38, 36, 25, 16, 25, 36, 32, 38, 38, 38, 38, 36, 36, 14, 31, 35, 38, 38, 38, 38, 13, 13, 36, 38, 38, 33, 36, 35, 29, 34, 32, 32, 13, 13, 34, 29, 34, 31, 38, 33, 35, 13, 13, 24, 23, 24, 34, 13}},
        { .refID = 0, .pos = 0, .mapq = 32, .bin = 4681, .flag = 161, .next_refID = 0, .next_pos = 9578, .tlen = 9703, .read_name ="D00360:72:C6UP4ANXX:1:1304:9139:2419", .cigar = {164, 1, 0, 0, 48, 6, 0, 0}, .seq = {136, 34, 34, 136, 17, 24, 17, 65, 33, 130, 18, 65, 132, 65, 130, 18, 20, 72, 40, 24, 33, 34, 40, 24, 129, 18, 33, 40, 33, 36, 68, 20, 40, 40, 34, 24, 66, 24, 136, 68, 129, 136, 136, 36, 130, 132, 68, 68, 72, 24, 66, 18, 66, 65, 129, 66, 24, 132, 36, 20, 18, 66, 128}, .seq_len = 125, .qual = {33, 34, 33, 33, 33, 38, 38, 38, 38, 38, 38, 38, 36, 29, 31, 38, 38, 36, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 34, 37, 36, 38, 38, 36, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 25, 37, 38, 38, 38, 38, 33, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 35, 38, 38, 29, 29, 36, 37, 34, 35, 37, 31, 38, 38, 35, 38, 38, 38, 38, 38, 38, 38, 34, 30, 36, 38, 38, 38, 33, 38, 38, 35, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 35, 38, 38, 38, 38, 38}},
        { .refID = 0, .pos = 0, .mapq = 60, .bin = 4681, .flag = 161, .next_refID = 0, .next_pos = 9322, .tlen = 9420, .read_name ="D00360:71:C61DEANXX:1:2303:17550:46600", .cigar = {212, 0, 0, 0, 0, 7, 0, 0}, .seq = {20, 18, 24, 33, 36, 24, 68, 24, 33, 33, 68, 130, 129, 130, 18, 34, 129, 136, 17, 34, 18, 130, 18, 68, 65, 66, 130, 130, 33, 132, 33, 136, 132, 72, 24, 136, 130, 72, 40, 68, 68, 68, 129, 132, 33, 36, 36, 24, 20, 33, 136, 66, 65, 65, 36, 40, 68, 20, 34, 68, 20, 33, 32}, .seq_len = 125, .qual = {18, 30, 32, 32, 33, 38, 36, 38, 38, 33, 14, 36, 37, 38, 38, 38, 36, 16, 33, 27, 37, 38, 38, 37, 38, 36, 38, 38, 36, 38, 36, 37, 38, 38, 38, 38, 38, 38, 38, 36, 38, 33, 38, 37, 33, 38, 38, 27, 14, 14, 29, 34, 34, 37, 38, 38, 38, 29, 38, 29, 37, 16, 37, 38, 38, 38, 38, 38, 38, 36, 38, 38, 38, 38, 38, 37, 38, 38, 15, 16, 28, 36, 35, 26, 21, 34, 38, 33, 33, 36, 38, 38, 35, 38, 38, 38, 35, 13, 35, 35, 33, 31, 34, 31, 34, 38, 34, 34, 38, 38, 38, 38, 34, 35, 31, 38, 36, 38, 38, 38, 31, 34, 23, 35, 38}},
        { .refID = 0, .pos = 0, .mapq = 60, .bin = 4681, .flag = 73, .next_refID = 0, .next_pos = 0, .tlen = 0, .read_name ="D00360:71:C61DEANXX:1:2304:1384:69650", .cigar = {132, 0, 0, 0, 176, 6, 0, 0}, .seq = {130, 18, 65, 132, 65, 130, 18, 20, 72, 40, 24, 33, 34, 40, 24, 129, 18, 33, 40, 33, 36, 68, 20, 40, 40, 34, 24, 66, 24, 136, 68, 129, 136, 136, 36, 130, 132, 68, 68, 68, 24, 66, 18, 66, 65, 129, 66, 24, 132, 36, 20, 18, 66, 132, 65, 66, 36, 64}, .seq_len = 115, .qual = {32, 32, 33, 32, 28, 14, 34, 38, 33, 35, 37, 38, 38, 36, 38, 29, 15, 26, 28, 37, 29, 37, 33, 37, 35, 38, 38, 38, 38, 31, 37, 37, 38, 38, 36, 38, 38, 38, 38, 38, 38, 33, 34, 34, 38, 38, 38, 29, 37, 37, 37, 29, 34, 29, 16, 29, 16, 37, 16, 25, 30, 16, 34, 37, 29, 16, 36, 35, 37, 38, 38, 38, 38, 35, 16, 25, 24, 24, 30, 14, 24, 34, 13, 25, 14, 14, 23, 24, 13, 24, 34, 29, 23, 14, 22, 34, 31, 31, 35, 23, 29, 13, 13, 21, 13, 21, 13, 23, 23, 23, 13, 25, 34, 34, 21}},
        { .refID = 0, .pos = 0, .mapq = 0, .bin = 4681, .flag = 133, .next_refID = 0, .next_pos = 0, .tlen = 0, .read_name ="D00360:71:C61DEANXX:1:2304:1384:69650", .cigar = {}, .seq = {136, 24, 136, 136, 34, 136, 132, 20, 132, 136, 130, 17, 130, 68, 129, 72, 18, 129, 40, 40, 24, 132, 130, 136, 36, 136, 24, 68, 132, 18, 18, 24, 20, 40, 132, 136, 65, 72, 136, 132, 136, 129, 24, 132, 72, 65, 20, 24, 132, 129, 132, 20, 136, 129, 20, 24, 129, 72, 136, 18, 40, 129, 128}, .seq_len = 125, .qual = {18, 28, 32, 28, 33, 16, 31, 16, 16, 16, 16, 16, 16, 16, 16, 15, 28, 28, 16, 16, 28, 16, 16, 16, 31, 16, 15, 14, 14, 29, 14, 16, 16, 25, 16, 29, 16, 36, 33, 16, 36, 38, 16, 16, 36, 16, 16, 16, 16, 15, 16, 16, 29, 15, 16, 16, 16, 16, 16, 29, 16, 29, 36, 16, 25, 16, 29, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 25, 16, 16, 30, 37, 38, 36, 25, 15, 15, 15, 15, 29, 15, 28, 15, 25, 15, 15, 28, 15, 15, 25, 26, 15, 15, 15, 15, 28, 15, 15, 15, 28, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 30, 31, 15}},
        { .refID = 0, .pos = 0, .mapq = 26, .bin = 4681, .flag = 2129, .next_refID = 0, .next_pos = 8698, .tlen = 8663, .read_name ="D00360:64:HBAP3ADXX:1:1103:13477:91844", .cigar = {5, 4, 0, 0, 80, 2, 0, 0}, .seq = {65, 130, 18, 20, 72, 40, 24, 33, 34, 40, 24, 129, 18, 33, 40, 33, 36, 68, 16}, .seq_len = 37, .qual = {35, 25, 25, 25, 16, 32, 36, 33, 27, 40, 37, 34, 37, 28, 33, 32, 25, 36, 35, 31, 36, 29, 34, 32, 25, 35, 35, 34, 17, 10, 28, 25, 35, 35, 26, 28, 30}},
        { .refID = 0, .pos = 0, .mapq = 60, .bin = 4681, .flag = 2193, .next_refID = 0, .next_pos = 6093, .tlen = 6064, .read_name ="D00360:64:HBAP3ADXX:1:1111:9203:55170", .cigar = {101, 4, 0, 0, 240, 1, 0, 0}, .seq = {65, 130, 18, 20, 72, 40, 24, 33, 34, 40, 24, 129, 18, 33, 40, 32}, .seq_len = 31, .qual = {36, 36, 40, 37, 36, 37, 37, 36, 34, 27, 32, 27, 25, 36, 30, 36, 36, 36, 35, 35, 35, 35, 35, 22, 35, 32, 19, 19, 30, 23, 28}},
        { .refID = 0, .pos = 0, .mapq = 60, .bin = 4681, .flag = 147, .next_refID = 0, .next_pos = 6525, .tlen = 6478, .read_name ="D00360:64:HBAP3ADXX:1:1116:10699:77936", .cigar = {16, 3, 0, 0}, .seq = {65, 130, 18, 20, 72, 40, 24, 33, 34, 40, 24, 129, 18, 33, 40, 33, 36, 68, 20, 40, 40, 34, 24, 66, 16}, .seq_len = 49, .qual = {40, 41, 40, 38, 40, 41, 41, 41, 41, 41, 41, 41, 41, 40, 39, 38, 41, 41, 41, 41, 41, 41, 41, 40, 39, 39, 39, 34, 40, 40, 40, 39, 41, 41, 41, 41, 39, 39, 39, 39, 39, 37, 37, 37, 37, 37, 34, 34, 34}},
    };

    auto expected_header = std::string {
        "VN:1.3\tSO:coordinate\n"
        "@SQ\tSN:chrM\tLN:16571\n"
        "@SQ\tSN:chr1\tLN:249250621\n"
        "@SQ\tSN:chr2\tLN:243199373\n"
        "@SQ\tSN:chr3\tLN:198022430\n"
        "@SQ\tSN:chr4\tLN:191154276\n"
        "@SQ\tSN:chr5\tLN:180915260\n"
        "@SQ\tSN:chr6\tLN:171115067\n"
        "@SQ\tSN:chr7\tLN:159138663\n"
        "@SQ\tSN:chr8\tLN:146364022\n"
        "@SQ\tSN:chr9\tLN:141213431\n"
        "@SQ\tSN:chr10\tLN:135534747\n"
        "@SQ\tSN:chr11\tLN:135006516\n"
        "@SQ\tSN:chr12\tLN:133851895\n"
        "@SQ\tSN:chr13\tLN:115169878\n"
        "@SQ\tSN:chr14\tLN:107349540\n"
        "@SQ\tSN:chr15\tLN:102531392\n"
        "@SQ\tSN:chr16\tLN:90354753\n"
        "@SQ\tSN:chr17\tLN:81195210\n"
        "@SQ\tSN:chr18\tLN:78077248\n"
        "@SQ\tSN:chr19\tLN:59128983\n"
        "@SQ\tSN:chr20\tLN:63025520\n"
        "@SQ\tSN:chr21\tLN:48129895\n"
        "@SQ\tSN:chr22\tLN:51304566\n"
        "@SQ\tSN:chrX\tLN:155270560\n"
        "@SQ\tSN:chrY\tLN:59373566\n"
        "@PG\tID:bwa\tPN:bwa\tVN:0.7.7-r441\tCL:bwa mem -t 2 ref.fa left.fq.gz right.fq.gz\n"
        "@PG\tID:samtools\tPN:samtools\tPP:bwa\tVN:1.15.1\tCL:samtools view -s 0.01 -b HG002.mate_pair.sorted.bam\n"
        "@PG\tID:samtools.1\tPN:samtools\tPP:samtools\tVN:1.15.1\tCL:samtools view -h sampled.bam\n"
        "@PG\tID:samtools.2\tPN:samtools\tPP:samtools.1\tVN:1.18\tCL:samtools view -o file.bam file.sam\n"
    };


    auto test_data = []() -> std::string {
        auto buffer = std::vector<uint8_t> {
            0x1f,0x8b,0x08,0x04,0x00,0x00,0x00,0x00,0x00,0xff,0x06,0x00,0x42,0x43,0x02,0x00,
            0x9a,0x02,0x7d,0x93,0xcf,0x6b,0x13,0x41,0x14,0xc7,0x9f,0x8d,0xad,0xdd,0x2a,0x3d,
            0x79,0x50,0x10,0xd9,0x83,0xd7,0x2c,0xf3,0xe6,0xf7,0xc4,0x16,0x52,0x5b,0x68,0x85,
            0xb6,0x56,0x0a,0xa5,0x15,0x44,0x92,0xe6,0x47,0x53,0x12,0x53,0x37,0xa1,0x05,0x11,
            0xc1,0x8b,0x20,0xe8,0x51,0x3c,0x78,0xf1,0xe6,0x59,0xf0,0x22,0x82,0xe0,0xad,0x08,
            0x5e,0x45,0x10,0xfd,0x07,0x44,0xf0,0x16,0xf0,0x20,0x4e,0x66,0x7f,0x38,0x29,0xd6,
            0x43,0xc8,0x7c,0xe6,0xb3,0xf3,0x7d,0xfb,0xde,0xee,0x5e,0x99,0x5b,0x39,0xf1,0xbd,
            0x00,0x50,0x5e,0x5a,0x08,0x36,0x56,0x4b,0x18,0xb1,0x60,0xfd,0x5a,0x69,0xbb,0xdb,
            0x8d,0x6b,0xad,0xdb,0x95,0x7e,0x7d,0xaa,0xbc,0x7e,0x3d,0x58,0x5f,0x2d,0x6d,0xef,
            0xc4,0x2b,0xc1,0xb2,0xbd,0x40,0x0a,0x85,0xde,0x26,0x0e,0x37,0x29,0x37,0x54,0x10,
            0x49,0x7d,0x41,0x13,0xc1,0xd0,0x18,0xa6,0x98,0x27,0x98,0x8b,0x31,0x9a,0x50,0x6b,
            0x89,0x27,0x78,0x22,0x10,0x05,0xa7,0x4a,0x7a,0x42,0x38,0xa1,0x89,0x41,0x41,0xa5,
            0x7f,0x42,0x3a,0xa1,0xd0,0x1e,0x21,0x52,0x79,0x42,0x39,0x21,0x0c,0x32,0x2d,0xa5,
            0x5f,0x5c,0x3b,0xc1,0x25,0x93,0xdc,0xd6,0xf7,0x84,0x49,0x04,0x52,0x64,0xf6,0x9e,
            0xfd,0x06,0x89,0x33,0x4c,0x08,0xc6,0x15,0xf7,0x8b,0x20,0xa6,0x86,0x10,0x29,0x50,
            0xfa,0x86,0x26,0x86,0x69,0x81,0xda,0x08,0xdf,0x24,0xdd,0xa3,0xbd,0xde,0x68,0xa5,
            0x7d,0x93,0xb4,0x4f,0x14,0xe3,0x46,0x70,0xe2,0x1b,0x91,0x18,0x2a,0x18,0x32,0x43,
            0x7d,0xe3,0x06,0x60,0x08,0x13,0x5c,0x09,0xe6,0x0b,0x37,0x00,0x8d,0x68,0x04,0xc5,
            0x91,0x2c,0x37,0x00,0xa5,0x89,0x52,0x94,0x8f,0x94,0x77,0x03,0xb0,0x13,0xa3,0xda,
            0x68,0x3f,0x8a,0xba,0xfe,0x25,0xb3,0xe5,0x05,0xf5,0xa3,0xa8,0x6b,0x9f,0x6b,0xa4,
            0x66,0xb4,0x47,0xea,0xba,0x17,0xc8,0x08,0x17,0xd2,0x1f,0xcb,0x66,0xf2,0x54,0x04,
            0x55,0x44,0x8c,0x3c,0xc7,0xad,0xa4,0xb6,0x7d,0x4f,0xdc,0x81,0xb5,0xc5,0xe0,0xea,
            0x42,0xa9,0x7a,0x50,0x09,0xd6,0x56,0xdd,0x9f,0x7d,0x31,0x49,0xa4,0x22,0x55,0x8c,
            0x39,0xc7,0x60,0x7e,0x79,0xb8,0x19,0x76,0xea,0x9d,0xb0,0xd8,0x0f,0x69,0x18,0xd7,
            0x1b,0x51,0xa3,0x12,0xb6,0xeb,0x8d,0x7e,0xd4,0xb8,0x13,0x35,0xef,0x86,0x71,0xab,
            0xb9,0x93,0xae,0xb3,0xb4,0x5e,0xa5,0xd3,0xef,0x76,0xdb,0xbd,0x61,0xe4,0xdf,0xf5,
            0x5a,0x16,0x8f,0x11,0x8a,0xc8,0x45,0x67,0x32,0xdc,0x6f,0xd5,0x0f,0xc2,0x62,0x2f,
            0x24,0x11,0xc1,0xb0,0x58,0x0d,0x97,0x16,0x09,0xa1,0x51,0xc7,0x7e,0x13,0xb7,0xf6,
            0x2a,0xad,0x38,0xea,0x75,0xe3,0x7e,0xbd,0x16,0x55,0x2b,0x9d,0xa3,0x35,0x6c,0xd0,
            0x91,0x2a,0xf9,0xfa,0x7f,0xa5,0x76,0x42,0xbb,0xb1,0xd7,0x3e,0x26,0x93,0x1e,0x97,
            0x69,0xb3,0x92,0x54,0xfd,0x8f,0xcc,0x6e,0xd8,0x68,0xb5,0xeb,0xc3,0xc0,0x64,0x61,
            0xf5,0xd4,0x79,0x00,0x18,0xb7,0xbf,0xe1,0x27,0x0d,0x6f,0xcb,0x39,0x20,0xcc,0xce,
            0x7f,0x99,0x4e,0x81,0xc2,0x93,0x1f,0xf7,0x33,0x60,0x70,0xf1,0xe9,0xc7,0xd3,0x29,
            0x70,0xa8,0x1d,0xd6,0x32,0x10,0x30,0xf3,0xf8,0x70,0x2a,0x05,0x09,0x97,0xc7,0x58,
            0x06,0x0a,0x9a,0xf3,0xf7,0x82,0x14,0x34,0xec,0x6f,0xbc,0x99,0x4c,0xc1,0xc0,0xe0,
            0xdd,0xee,0xe4,0x44,0x5a,0x94,0xc0,0xf3,0x73,0x67,0x73,0x42,0xe0,0xc1,0x99,0x9c,
            0x28,0x0c,0x76,0x7f,0x9d,0xca,0x88,0xc1,0xc6,0x8d,0xaf,0x13,0x19,0x71,0xb8,0x34,
            0xd1,0xc8,0x49,0x40,0xf9,0xc1,0x85,0x9c,0x24,0xcc,0xbd,0xae,0x8e,0x67,0xa4,0xe0,
            0xc3,0xcf,0x4f,0x27,0x33,0xd2,0x50,0xbe,0xf9,0x32,0x27,0x03,0xcf,0x66,0x1e,0x16,
            0x52,0xa2,0x04,0xf6,0x5e,0xbd,0xcf,0x09,0xa1,0xd9,0xfc,0x36,0x96,0x11,0x85,0xfd,
            0xcf,0xd3,0x85,0xb4,0x85,0x4d,0x78,0x31,0x3b,0x97,0x35,0xb7,0x05,0xbf,0x07,0x8f,
            0x0a,0x7f,0x00,0x99,0xeb,0x32,0xc1,0x49,0x05,0x00,0x00,0x1f,0x8b,0x08,0x04,0x00,
            0x00,0x00,0x00,0x00,0xff,0x06,0x00,0x42,0x43,0x02,0x00,0x74,0x04,0x9d,0x95,0xcf,
            0x6f,0xe3,0x44,0x14,0xc7,0xa7,0x5d,0x58,0xaa,0x95,0x12,0x4f,0xbd,0xee,0x7a,0xdc,
            0xd8,0xb1,0x67,0x3c,0xe3,0xb8,0x22,0x48,0xfe,0x15,0xa7,0xf1,0x72,0xc0,0xa9,0x97,
            0x4d,0x0f,0xad,0xba,0x0a,0x48,0xd5,0x4a,0x20,0xc1,0x0a,0x09,0x95,0x1b,0x9c,0x57,
            0xaa,0x2d,0x23,0xf9,0x8a,0xf8,0x0b,0x90,0x90,0xb8,0x70,0xe0,0x84,0xc4,0xff,0x00,
            0x37,0x10,0xff,0x00,0x88,0xeb,0x1e,0x11,0x27,0xde,0xb8,0x6e,0x9b,0xdd,0xb6,0xcb,
            0x8f,0x17,0x27,0x99,0x37,0x3f,0xac,0xef,0xfb,0xcc,0x9b,0x79,0x3f,0xa0,0x4b,0x1b,
            0xdd,0xde,0x57,0xd7,0xd1,0x93,0x0d,0xbf,0xf3,0xff,0x64,0x08,0xbd,0xe7,0x22,0x54,
            0x04,0x41,0x9c,0x06,0x59,0x9a,0x64,0x8b,0x79,0x7e,0x14,0xe7,0xc5,0xf1,0x71,0x16,
            0x65,0x61,0x18,0x4e,0xb2,0x30,0x9d,0xa5,0xbb,0x59,0x04,0x13,0x26,0xe8,0x97,0x5b,
            0x08,0x9d,0xae,0x23,0x94,0x57,0xaa,0xb6,0xf0,0x09,0x65,0x3e,0x29,0x55,0xea,0x53,
            0x5e,0x68,0xbe,0xef,0x79,0xae,0xe7,0xf1,0x11,0x77,0x4c,0xea,0x0a,0x30,0xc6,0x19,
            0x87,0xc7,0x62,0x8c,0x51,0x26,0xcd,0xd4,0x15,0x8b,0x0e,0x0e,0x0f,0xf6,0xd0,0x41,
            0xf1,0x38,0x09,0x50,0xbe,0xdc,0xf3,0x8f,0x97,0x7b,0x68,0x99,0x3f,0x7e,0xf2,0xf1,
            0xa7,0x07,0xe3,0x30,0x9d,0x84,0xe1,0xf8,0xf5,0x71,0x1a,0x1e,0x24,0xc1,0x72,0x9c,
            0x8e,0xc3,0xfb,0xc8,0x59,0x5b,0x11,0xff,0xa6,0x14,0xff,0x15,0x7a,0xda,0xf9,0xbf,
            0x43,0x14,0x1f,0xee,0x5c,0x88,0x9f,0x46,0xd9,0x5e,0xfa,0xee,0x51,0x92,0x1f,0x82,
            0xf8,0xb0,0x13,0x3f,0x8d,0xd3,0x28,0x9b,0x44,0x49,0x32,0x43,0xbf,0xc1,0x92,0x67,
            0xb7,0x11,0xda,0xcc,0x69,0xa5,0xe6,0xf5,0xb5,0x31,0x30,0x32,0x27,0x4d,0x51,0x36,
            0x0d,0xaf,0xea,0xa2,0x58,0x90,0xb9,0x3a,0xcf,0xcb,0x39,0xa9,0xb9,0xa6,0xce,0xeb,
            0x7c,0xce,0xf3,0x39,0x66,0x14,0xcc,0x3b,0x37,0xd7,0x62,0xde,0x3f,0x58,0xbf,0x3f,
            0x18,0x00,0x18,0xc1,0x84,0xf4,0x80,0x89,0x67,0x41,0x4b,0x12,0x72,0xa8,0xc7,0x0d,
            0x6c,0x70,0x47,0x0e,0x70,0xde,0xb7,0x5d,0xd9,0xe8,0xf5,0xb8,0xe7,0x51,0x0e,0x6f,
            0x76,0x9c,0x5e,0x8f,0x59,0xcc,0xf6,0xa8,0xdb,0xeb,0x11,0x9d,0xb0,0x1e,0xc0,0x5b,
            0x03,0x78,0xd3,0xe8,0x61,0xbc,0x2b,0xf9,0x9d,0x00,0xbf,0xb7,0x87,0x2b,0x8c,0x84,
            0xf3,0x3c,0xa3,0x13,0x81,0xd0,0x1f,0xe2,0x46,0x46,0x71,0x90,0x64,0xb3,0x30,0x9e,
            0x65,0x51,0x12,0xce,0xd0,0xd7,0xf0,0xa2,0x00,0x08,0x35,0x8c,0x35,0x9b,0xe4,0xff,
            0x72,0x3a,0xa5,0xec,0x92,0x10,0xb7,0x6c,0xf8,0xb9,0x0e,0x0b,0x13,0xfc,0xea,0x88,
            0xd1,0x32,0xa2,0x97,0x1d,0x40,0xc4,0xb2,0x80,0x9d,0x2b,0xec,0xd6,0x39,0x5b,0x3a,
            0xe4,0x67,0xb3,0xdc,0xd5,0xb5,0x67,0xce,0x25,0xa1,0x28,0x95,0x84,0xde,0x07,0x42,
            0x8f,0x5e,0x96,0x45,0x27,0x1c,0xa1,0x1f,0xf9,0x25,0xa1,0x10,0x08,0x85,0xc5,0x83,
            0x8e,0x50,0x14,0x07,0x31,0x64,0xd1,0x64,0x12,0x64,0x49,0x9a,0x06,0x01,0xfa,0x59,
            0xae,0x79,0x0d,0x21,0x4d,0x25,0x94,0x93,0x82,0x50,0x5a,0x54,0x65,0xa5,0xb2,0xb2,
            0xd9,0x64,0x6a,0xa5,0x16,0xf9,0xbc,0xaa,0x68,0x4d,0x9b,0x7a,0x41,0x9a,0x6a,0xe1,
            0x17,0x45,0x51,0xd6,0x94,0x73,0xa2,0xd1,0x66,0x9e,0xe7,0xdc,0x2f,0x34,0x56,0x68,
            0xd4,0x51,0x87,0x8e,0xdc,0x7d,0x88,0xa2,0xcf,0x65,0xcc,0x1c,0xd3,0x81,0xcc,0x0a,
            0xd9,0xc5,0xbd,0xb6,0xab,0xc5,0x07,0x51,0x0a,0x08,0x74,0xd0,0xef,0xc3,0x39,0x92,
            0xbd,0x16,0x64,0x0f,0x16,0xe7,0xa3,0xd2,0xc0,0x51,0xb0,0xc9,0xdd,0xed,0x2d,0xe6,
            0x51,0xca,0xcf,0x40,0xb8,0x3d,0xd7,0xa5,0x36,0xb3,0x99,0xc7,0xda,0x04,0x65,0xae,
            0xdd,0x4e,0xb7,0x99,0xee,0xae,0x30,0x8a,0x67,0x92,0xd1,0x27,0xc0,0xe8,0x81,0xba,
            0xc2,0xc8,0x6b,0x19,0xed,0xa3,0xcf,0xd0,0xf3,0x76,0x23,0xa3,0x04,0x52,0x69,0x37,
            0xc9,0xe0,0xaa,0x98,0x04,0xa8,0x86,0x99,0xdf,0x41,0x1a,0xfd,0xcb,0xec,0x29,0xae,
            0x9c,0xb2,0xb7,0x80,0x8d,0x63,0xf6,0x21,0x1c,0x57,0xc8,0x28,0x2d,0x65,0xdb,0x14,
            0x96,0xa0,0xa2,0xdd,0x62,0x5b,0x88,0x8b,0xc4,0xa1,0x6d,0x74,0x16,0x9c,0x25,0x8b,
            0x59,0xd8,0xc2,0x02,0x1b,0x43,0xcc,0x84,0x85,0xb9,0x5c,0x09,0x18,0xb0,0x41,0xc8,
            0xb0,0x0f,0x27,0xc7,0xe8,0xf7,0x75,0xd2,0x23,0xcc,0xd2,0xfb,0xf7,0x98,0x6d,0xbb,
            0xba,0xd5,0xeb,0x6d,0xc1,0x47,0xd7,0xf5,0x9e,0xc1,0xd8,0x16,0x30,0x59,0x97,0x4c,
            0xc2,0x77,0x82,0x87,0x71,0x22,0xa9,0x7c,0x20,0xef,0xa6,0x3b,0xab,0x54,0xd0,0xbe,
            0x8a,0xd0,0xe7,0x17,0x99,0xf3,0xdf,0xa8,0x34,0xa4,0x69,0x58,0x53,0x6b,0x75,0x53,
            0x6d,0x56,0x45,0xb9,0x50,0x4b,0xdf,0x27,0x75,0xd5,0xf0,0x86,0x14,0xb5,0xaa,0x12,
            0xcd,0xaf,0x9b,0x7c,0xd1,0xd4,0x4d,0x49,0xea,0x45,0xae,0x91,0xba,0xac,0xb5,0xa6,
            0xd4,0x48,0xb9,0x68,0x54,0xbf,0x3c,0x55,0x4d,0xb8,0x57,0xb1,0x8d,0x3b,0x53,0x4c,
            0x13,0x63,0x78,0xa0,0x47,0x81,0xdc,0xe8,0x63,0x6c,0x40,0xf8,0x9c,0x62,0xee,0x61,
            0xcc,0xdb,0x19,0x18,0x5b,0x4a,0x3b,0x17,0xc0,0xf0,0x76,0x78,0xc5,0x0c,0x8c,0x87,
            0x90,0x6c,0x86,0x02,0x66,0x29,0xa6,0x02,0x0d,0x53,0x51,0x8c,0x6d,0xe9,0x9b,0xdd,
            0xb7,0xb3,0xa1,0xad,0x00,0x0d,0x24,0x69,0x7c,0xbf,0x5a,0x4a,0xb6,0x65,0x8e,0x3c,
            0xda,0x10,0x9d,0xff,0x17,0x45,0xe8,0x57,0x7a,0x7d,0x29,0x91,0xb7,0xb1,0x3c,0x47,
            0x71,0x32,0x9d,0xc2,0x7d,0xb3,0x9b,0x24,0xe8,0xd5,0x57,0x10,0x3a,0xba,0xb6,0x94,
            0x60,0xd7,0x30,0x0c,0xec,0x70,0x3a,0xf0,0x05,0x13,0x26,0x75,0x0c,0xee,0xda,0x50,
            0x46,0x1c,0xc3,0x75,0xd9,0xe6,0x1d,0x13,0xfe,0xb6,0xcd,0x61,0x57,0x46,0xe2,0xa9,
            0xdc,0x2a,0xf1,0x42,0x19,0x09,0x76,0xc7,0x6f,0x8c,0xd3,0xe4,0x20,0x9e,0x2e,0xc7,
            0x51,0x5b,0x47,0xbe,0x5d,0x51,0x7e,0x96,0xdd,0x5f,0x6c,0xd8,0x9d,0xff,0x93,0x0e,
            0xf9,0xaa,0xdf,0xac,0x3c,0x0c,0xb3,0x59,0x04,0xf2,0x27,0x93,0x70,0x1a,0xa0,0x8f,
            0x40,0xf8,0xb3,0xb5,0x2b,0xc2,0x1d,0xce,0x7d,0xc1,0x85,0xe0,0x6c,0xe0,0x0c,0x0c,
            0x3e,0xe4,0x9c,0xbb,0xd2,0xee,0xb9,0xce,0xdd,0xbb,0x43,0xdd,0x3c,0xd7,0x1b,0x4a,
            0xbd,0xf6,0x8b,0x7a,0x23,0xd0,0x3b,0x0d,0x0e,0xe2,0x10,0xca,0x5e,0x30,0x8e,0xee,
            0xa3,0x6f,0x56,0x49,0x83,0xde,0x35,0xf4,0x25,0x0a,0x3b,0xff,0xa9,0x81,0xd0,0xa1,
            0xf1,0x12,0xbd,0x69,0x16,0x06,0xe9,0x6c,0x96,0x4d,0xa7,0xb3,0x38,0x45,0xf8,0xd6,
            0x0d,0x05,0x1b,0x8e,0x21,0xf6,0x77,0x7c,0xcf,0xdf,0xe9,0xcc,0x1f,0x79,0x17,0xad,
            0xd1,0x88,0xf9,0xbe,0x3f,0x92,0xce,0x48,0x9a,0x68,0x4b,0x3a,0x63,0xe7,0xe5,0xbb,
            0xbd,0x38,0xc2,0xe3,0xe5,0x1e,0xfa,0x1b,0xf2,0x39,0xcd,0x4d,0x60,0x08,0x00,0x00,
            0x1f,0x8b,0x08,0x04,0x00,0x00,0x00,0x00,0x00,0xff,0x06,0x00,0x42,0x43,0x02,0x00,
            0x1b,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
        };
        auto r = std::string{};
        r.resize(buffer.size());
        memcpy(r.data(), buffer.data(), buffer.size());
        return r;
    }();

    SECTION("Creating Test file") {
        auto ofs = std::ofstream{tmp / "file.bam", std::ios::binary};
        ofs << test_data;
    }

    SECTION("Read from std::filesystem::path") {
        auto reader = ivio::bam::reader{{tmp / "file.bam"}};
        CHECK(reader.header().buffer.size() == expected_header.size());
        CHECK(reader.header().buffer == expected_header);
        for (size_t i{0}; i < std::min(reader.header().buffer.size(), expected_header.size()); ++i) {
            INFO("i: " << i);
            CHECK(reader.header().buffer[i] == expected_header[i]);
        }
        auto vec = std::vector(begin(reader), end(reader));
        static_assert(std::same_as<decltype(vec), decltype(expected)>, "vec and expected should have the exact same type");

        REQUIRE(vec.size() == expected.size());
        for (size_t i{0}; i < vec.size(); ++i) {


            INFO("i: " << i);
            CHECK(vec[i].refID == expected[i].refID);
            CHECK(vec[i].pos == expected[i].pos);
            CHECK(vec[i].mapq == expected[i].mapq);
            CHECK(vec[i].bin == expected[i].bin);
            CHECK(vec[i].flag == expected[i].flag);
            CHECK(vec[i].next_refID == expected[i].next_refID);
            CHECK(vec[i].next_pos == expected[i].next_pos);
            CHECK(vec[i].tlen == expected[i].tlen);
            CHECK(vec[i].read_name == expected[i].read_name);
            CHECK(vec[i].cigar == expected[i].cigar);
            CHECK(vec[i].seq == expected[i].seq);
            CHECK(vec[i].qual == expected[i].qual);

            CHECK(vec[i] == expected[i]);
        }
    }

    SECTION("Read from std::ifstream") {
        auto ifs = std::ifstream{tmp / "file.bam"};
        auto reader = ivio::bam::reader{{ifs}};
        CHECK(reader.header().buffer == expected_header);
        auto vec = std::vector(begin(reader), end(reader));
        static_assert(std::same_as<decltype(vec), decltype(expected)>, "vec and expected should have the exact same type");
        CHECK(vec == expected);
    }

    SECTION("Read from std::stringstream") {
        auto ss = std::stringstream{test_data};
        auto reader = ivio::bam::reader{{ss}};
        CHECK(reader.header().buffer == expected_header);
        auto vec = std::vector(begin(reader), end(reader));
        static_assert(std::same_as<decltype(vec), decltype(expected)>, "vec and expected should have the exact same type");
        CHECK(vec == expected);
    }

    SECTION("cleanup - deleting temp folder") {
        std::filesystem::remove_all(tmp);
    }
}