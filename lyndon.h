// Borja :)
// 03/02/2022
#include <sdsl/wavelet_trees.hpp>
#include <stdlib.h>
#include <iostream>
#include <cmath>
#include <fstream>
#include <string>
#include <unordered_set>
#include <set>
#include <unordered_map>
#include <map>
#include <vector>
#include <dirent.h>
#include <sstream>
#include <chrono>
#include <filesystem>

std::vector<std::string> get_all_files_dir(std::string path)
{
    std::vector<std::string> files;
    for (std::filesystem::recursive_directory_iterator i(path), end; i != end; ++i) 
        if (!std::filesystem::is_directory(i->path()))
            files.push_back(std::string(i->path()));
    return files;
}

std::pair<int, int> get_sigma_length(std::string file)
{
    std::string segment;
    int i = 0, sigma = 0, length = 0;
    std::stringstream ss_file(file);
    while(std::getline(ss_file, segment, '_'))
    {
        if (i == 1){
            sigma = stoi(segment);
        }
        if (i == 2)
            length = stoi(segment.substr(0,2));
        i++;
    }
    return {sigma, length};
}

std::unordered_set<char> get_seq_chars(std::string path)
{
    std::unordered_set<char> solution;
    std::string seq;
    std::ifstream myfile; myfile.open(path);
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

std::map<char, int> get_remaining_chars(sdsl::wt_huff<sdsl::rrr_vector<63>> wt, int position, std::unordered_set<char> char_set)
{
    std::unordered_map<char, int> return_map;
    int full_length = wt.size();
    for (auto c:char_set){
        int rank_c = wt.rank(position, c), r = wt.rank(full_length, c) - wt.rank(position, c);
        if (r > 0)
            return_map[c] = wt.select(rank_c + 1, c);
    }
    std::map<char, int> return_ordered_map(return_map.begin(), return_map.end());
    return return_ordered_map;
}


