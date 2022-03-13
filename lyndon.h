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
#include <stack>
#include <dirent.h>
#include <sstream>
#include <chrono>
#include <filesystem>

#define N_THREADS 1
#define DEBUG true

using namespace std;

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

std::set<char> get_seq_chars(std::string path)
{
    std::set<char> solution;
    std::string seq;
    std::ifstream myfile; myfile.open(path);
    if (myfile.is_open())
        myfile >> seq;
    for (auto c: seq)
        solution.emplace(c);
    return solution;
}

string get_seq(std::string path)
{
    std::set<char> solution;
    std::string seq;
    std::ifstream myfile; myfile.open(path);
    if (myfile.is_open())
        myfile >> seq;
    myfile.close();
    return seq;
}

std::unordered_map<char, int> get_num_chars(const sdsl::wt_huff<sdsl::rrr_vector<63>> & wt, const std::set<char> & seq_chars)
{
    std::unordered_map<char, int> results_map;
    int full_length = wt.size();
    for (auto c: seq_chars){
        results_map[c] = wt.rank(full_length, c);
    }
    return results_map;
}

// Support functions

int num_remaining_chars(sdsl::wt_huff<sdsl::rrr_vector<63>> wt, char character, int position)
{
    int full_length = wt.size();
    return wt.rank(full_length, character) - wt.rank(position, character);
}

std::map<char, int> get_remaining_chars(const sdsl::wt_huff<sdsl::rrr_vector<63>> & wt, int position, 
    std::set<char> & char_set, std::unordered_map<char, int> & count_chars)
{
    std::unordered_map<char, int> return_map;
    for (auto c:char_set){
        int rank_c = wt.rank(position, c), 
            r = count_chars[c] - wt.rank(position, c);
        if (r > 0)
            return_map[c] = wt.select(rank_c + 1, c);
    }
    std::map<char, int> return_ordered_map(return_map.begin(), return_map.end());
    return return_ordered_map;
}

// Construct method
//std::vector<std::map<char, std::pair<int,int>>> construct(std::string file)
vector<vector<int>> construct(string file)
{
    std::string line;
    std::ifstream chain_file;
    chain_file.open (file);
    if (chain_file.is_open())
    {
        while ( getline (chain_file,line) )
        {
            std::cout << line << std::endl;
        }
        chain_file.close();
    }
    chain_file.close();
    std::vector<std::vector<int>> return_vect_int;
    std::vector<std::map<char,std::pair<int,int>>> return_vect_map;
    std::map<char,std::pair<int,int>> local_map;
    for (int i = line.size() - 1; i >= 0; --i){
        if (local_map.find(line[i]) == local_map.end())
            local_map[line[i]] = {0,line.size()};
        return_vect_map.push_back(std::map<char,std::pair<int,int>>());
        return_vect_int.push_back(vector<int>());
    }
    for (int i = line.size() - 1; i >= 0; --i)
    {
        // std::cout << "Index: "<<i<<std::endl;
        local_map[line[i]].first++;
        for (auto k_v: local_map){
            // std::cout <<"Key: "<<k_v.first<<"Indice: "<< i<<" "<< local_map[k_v.first].second<<" " << local_map[k_v.first].first<<" "<<local_map[k_v.first].second<< std::endl;
            if ((line[i] == k_v.first) & (i > 0))
                return_vect_map[i][k_v.first] = {local_map[k_v.first].first, i + 1};
            else
                return_vect_map[i][k_v.first] = {local_map[k_v.first].first, local_map[k_v.first].second + 1};//return_vect_map[local_map[k_v.first].second][k_v.first].second};
            //std::cout << k_v.first<<" "<<(return_vect_map[i][k_v.first]).first<<" "<<(return_vect_map[i][k_v.first]).second<<std::endl;
        }
        local_map[line[i]].second = i;
    }
    // Force last position
    return_vect_int.push_back(vector<int>());
    for (auto k_v:return_vect_map[0])
        return_vect_int[return_vect_map.size()].push_back(return_vect_map.size()+1);
    if (DEBUG){
        for (size_t i = 0; i  < return_vect_map.size() ; ++i)
        {
            std::cout << "Index: "<<i<<std::endl;
            for (auto k_v: return_vect_map[i]){
                std::cout << k_v.first<<" "<<k_v.second.first<<" "<<k_v.second.second<<std::endl;
                return_vect_int[i].push_back(k_v.second.second);
            }
        }
    }
    return return_vect_int;
    //return return_vect_map;
}

void print_stack(vector<int> vect, int idx_len = 0)
{
    cout <<"Size check: "<< idx_len<<endl;
    for (auto i:vect)
        cout << i << " ";
    cout << endl;
}

void print_lyndon(vector<string> lyndon_words)
{
    for (auto lw:lyndon_words)
    {
        cout << lw << endl;
    }
}