#include "lyndon.h"
#include <omp.h>

using namespace sdsl;
using namespace std;

bool quiet = false;
// Lyndon enumeration
int enumeration(const wt_huff<rrr_vector<63>> wt, pair<int,int> sig_l, vector<vector<pair<int, float>>> & rates,std::unordered_set<char> & seq_chars)
{
    unordered_map<char, int> count_chars = get_num_chars(wt, seq_chars);
    /*
     * Operate as in 
     */ 
    function<void(int,int,string,vector<string>&,int&)> _enumeration = [wt, &seq_chars,&count_chars, &_enumeration]
        (int position, int matched_position, string seq, vector<string> & lyndon_words, int & visited_trie_nodes) 
    {
        char ref_char = seq[matched_position];
        // For each available char we need to know its placement
        //cout << "Wt size: "<<wt.size()<<" Pos: "<<position<<" "<<<<endl;
        map<char,int> remaining_chars = get_remaining_chars(wt,position, seq_chars, count_chars);
        /*for (auto k_v:remaining_chars)
            cout << "Key: "<<k_v.first<<" "<<k_v.second<<endl;*/
        for (auto k_v:remaining_chars)
        {
            char c = k_v.first;
            int placement = k_v.second;
            if ((remaining_chars.size() == 1) & (c == ref_char) & (seq.size() > 0))
                return;
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
    if (DEBUG)
    {
        for (auto i: lyndon_words)
            cout << "Lyndon: "<<i<<endl;
    }
    return lyndon_words.size();
}

// Stack enumeration
int stack_enumeration(const vector<map<char,pair<int,int>>> & next_chars, std::unordered_set<char> & seq_chars)
{
    vector<string> lyndon_words;
    for (auto key: seq_chars)
        lyndon_words.push_back(string(1,key));
    stack<string> cur_IL_stack;
    stack<int> placement_SEQ, placement_IL;
    for (auto k_val:next_chars[0])
    {
        cur_IL_stack.push(string(1,k_val.first));
        placement_SEQ.push(k_val.second.second);
        placement_IL.push(0);
    }
    while (!(cur_IL_stack.empty()))
    {
        string cur_IL_word = cur_IL_stack.top();
        int placement = placement_SEQ.top();
        int idx_IL_word = placement_IL.top();
        //cout << "Word: "<<cur_IL_word<<" placement: "<<placement<<" idx: "<<idx_IL_word<<endl;
        cur_IL_stack.pop();
        placement_SEQ.pop();
        placement_IL.pop();
        if (placement >= (int) next_chars.size())
            continue;
        // Check if all remaining characters are smaller or equal than the first one.
        for (auto k_val:next_chars[placement])
        {
            if (k_val.first > cur_IL_word[0])
                break;
        }
        for (auto k_val:next_chars[placement])
        {
            //cout << "Key: "<<k_val.first<<" "<<k_val.second.first<<endl;
            if (k_val.second.first == 0)
                continue;
            if (k_val.first == cur_IL_word[idx_IL_word])
            {
                cur_IL_stack.push(cur_IL_word + k_val.first);
                placement_SEQ.push(k_val.second.second);
                placement_IL.push(idx_IL_word + 1);
            }
            if (k_val.first > cur_IL_word[idx_IL_word])
            {   
                string new_L_word = cur_IL_word + k_val.first;
                cur_IL_stack.push(new_L_word);
                placement_SEQ.push(k_val.second.second);
                placement_IL.push(0);
                lyndon_words.push_back(new_L_word);
            }
        }
    }
    if (DEBUG)
    {
        for (auto i: lyndon_words)
            cout << "Lyndon: "<<i<<endl;
    }
    return lyndon_words.size();
}

int main(int argc, char *argv[])
{
    cout << "Lyndon words enumeration"<<endl;
    //vector<string> files = get_all_files_dir("simulations/");
    vector<string> files = get_all_files_dir("example/");
    sort(files.begin(), files.end());
    int method = atoi(argv[1]);
    cout << "Method: "<<method<<endl;
    /*for (auto file: files)
        cout << file << " "<<endl;*/
    // Wavelette tree
    vector<vector<pair<int,float>>> rates(files.size(), vector<pair<int,float>>());
    vector<vector<pair<int,float>>> times(files.size(), vector<pair<int,float>>());
    set<int> idx;
    omp_set_num_threads(N_THREADS);
    cout << "Number of threads working: "<<N_THREADS<<endl;
    #pragma omp parallel shared(times, rates, files)
    {
            for (uint i = 0; i < ceil(files.size() / N_THREADS); ++i)
            {
                int th_idx = i*N_THREADS + omp_get_thread_num();
                string file = files[th_idx];
                if (file[file.size() - 1] == '.')
                    continue;
                int percentage = ceil(((float) th_idx / (float) files.size() * 100));
                if (omp_get_thread_num() == 0)
                    if ((percentage % 10) == 0)
                        cout << percentage<<"% "<<endl;
                //cout << "File: "<<file<<" Thread working: "<<id<<" of: "<<np<<endl;
                pair<int,int> sig_l = get_sigma_length(file);
                idx.insert(sig_l.first);
                int number = 0;
                std::unordered_set<char> seq_chars = get_seq_chars(file);
                // Wavelette tree version
                if (method == 1){
                    wt_huff<rrr_vector<63>> wt;
                    construct(wt,file, 1);
                    auto start = std::chrono::system_clock::now();
                    number = enumeration(wt, sig_l, rates, seq_chars);
                    auto end = std::chrono::system_clock::now();
                    std::chrono::duration<float,std::milli> duration = end - start;
                    #pragma omp critical
                    {
                        times[sig_l.first].push_back({sig_l.second, (duration.count()/number)});
                    }
                } else if (method == 2)
                {
                    cout << "Method: "<<method<<endl;
                    vector<map<char, pair<int,int>>> forward_count = construct(file);
                    auto start = std::chrono::system_clock::now();
                    number = stack_enumeration(forward_count, seq_chars);
                    auto end = std::chrono::system_clock::now();
                    std::chrono::duration<float,std::milli> duration = end - start;
                    #pragma omp critical
                    {
                        times[sig_l.first].push_back({sig_l.second, (duration.count()/number)});
                    }
                }
                cout << "Number of lyndon words: "<<number<<endl;
                exit(1);
            }
    }
    cout << endl;
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