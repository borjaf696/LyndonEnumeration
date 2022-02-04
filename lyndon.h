// Borja :)
// 03/02/2022
#include <sdsl/wavelet_trees.hpp>
#include <stdlib.h>
#include <iostream>
#include <cmath>
#include <fstream>
#include <string>
#include <unordered_set>
#include <unordered_map>

std::unordered_set<char> get_seq_chars()
{
    std::unordered_set<char> solution;
    std::string seq;
    std::ifstream myfile; myfile.open("example/simulation.txt");
    if (myfile.is_open())
        myfile >> seq;
    for (auto c: seq)
        solution.emplace(c);
    return solution;
}

// Support functions

int num_remaining_chars(sdsl::wt_huff<sdsl::rrr_vector<63>> wt, char character, int position)
{
    int full_length = wt.size();
    return wt.rank(full_length, character) - wt.rank(position, character);
}

std::unordered_map<char, int> get_remaining_chars(sdsl::wt_huff<sdsl::rrr_vector<63>> wt, int position, std::unordered_set<char> char_set)
{
    std::unordered_map<char, int> return_map;
    int full_length = wt.size();
    for (auto c:char_set){
        int rank_c = wt.rank(position, c), r = wt.rank(full_length, c) - wt.rank(position, c);
        if (r > 0)
            return_map[c] = wt.select(rank_c + 1, c);
    }
    return return_map;
}


