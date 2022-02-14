#include "lyndon.h"

using namespace sdsl;
using namespace std;

bool quiet = false;
// Lyndon enumeration
int enumeration(const wt_huff<rrr_vector<63>> wt, pair<int,int> sig_l, vector<vector<pair<int, float>>> & rates,const std::unordered_set<char> & seq_chars)
{
    /*
     * Operate as in 
     */ 
    function<void(int,int,string,vector<string>&,int&)> _enumeration = [wt, seq_chars, &_enumeration]
        (int position, int matched_position, string seq, vector<string> & lyndon_words, int & visited_trie_nodes) 
    {
        char ref_char = seq[matched_position];
        // For each available char we need to know its placement
        //cout << "Wt size: "<<wt.size()<<" Pos: "<<position<<" "<<<<endl;
        map<char,int> remaining_chars = get_remaining_chars(wt,position, seq_chars);
        /*for (auto k_v:remaining_chars)
            cout << "Key: "<<k_v.first<<" "<<k_v.second<<endl;*/
        for (auto k_v:remaining_chars)
        {
            char c = k_v.first;
            int placement = k_v.second;
            string n_seq = seq + c;
            if (c == ref_char)
                _enumeration(placement + 1, matched_position + 1, n_seq, lyndon_words, visited_trie_nodes);
            else if (c > ref_char)
            {
                lyndon_words.push_back(n_seq);
                _enumeration(placement + 1, 0, n_seq, lyndon_words, visited_trie_nodes);
            }
            visited_trie_nodes++;
        }
    };
    vector<string> lyndon_words;
    int visited_trie_nodes = 0;
    _enumeration(0, 0, "", lyndon_words, visited_trie_nodes);
    if (quiet)
        return lyndon_words.size();
    /*
     * Report
     */ 
    int number = 0;
    ofstream lyndon_report;
    string output_name = "output/sim_"+to_string(sig_l.first)+"_"+to_string(sig_l.second)+".txt";
    lyndon_report.open(output_name.c_str());
    for (auto lyndon:lyndon_words){
        lyndon_report << lyndon<<endl;
        number++;
    }
    rates[sig_l.first].push_back({sig_l.second, visited_trie_nodes/(number*1.)});
    return lyndon_words.size();
}

int main(int argc, char *argv[])
{
    cout << "Lyndon words enumeration"<<endl;
    vector<string> files = get_all_files_dir("simulations/");
    sort(files.begin(), files.end());
    for (auto file: files)
        cout << file << " "<<endl;
    // Wavelette tree
    vector<vector<pair<int,float>>> rates(files.size(), vector<pair<int,float>>());
    vector<vector<pair<int,float>>> times(files.size(), vector<pair<int,float>>());
    set<int> idx;
    for (auto file: files)
    {
        if (file[file.size() - 1] == '.')
            continue;
        cout << "File: "<<file<<endl;
        pair<int,int> sig_l = get_sigma_length(file);
        idx.insert(sig_l.first);
        wt_huff<rrr_vector<63>> wt;
        construct(wt,file, 1);
        std::unordered_set<char> seq_chars = get_seq_chars(file);
        auto start = std::chrono::system_clock::now();
        int number = enumeration(wt, sig_l, rates, seq_chars);
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<float,std::milli> duration = end - start;
        times[sig_l.first].push_back({sig_l.second, (duration.count()/number)});
        cout << "Number of lyndon words: "<<number<<endl;
    }
    // Print rates
    for (auto i:idx)
    {
        cout << "Sigma: "<<i<<endl;
        for (auto p: rates[i])
        {
            cout << "Length string: "<<p.first<<" Rate: "<<p.second<<endl;
        }
        for (auto p: times[i])
        {
            cout << "Length string: "<<p.first<<" Time: "<<p.second<<endl;
        }
    }
}