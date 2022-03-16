#include "lyndon.h"
#include <omp.h>

using namespace sdsl;
using namespace std;

bool quiet = false;
// Lyndon enumeration
int enumeration(const wt_huff<rrr_vector<63>> wt, pair<int,int> sig_l, vector<vector<pair<int, float>>> & rates,std::set<char> & seq_chars)
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

// Lyndon enumeration
vector<string> enumeration_dominik(const vector<vector<int>> & next_chars, std::set<char> & seq_chars, string seq)
{
    vector<string> lyndon_words;
    map<char,int> char_map;
    int place = 0;
    for (char c: seq_chars){
        char_map[c] = place++;
        lyndon_words.push_back(string(1,c));
    }
    /*
     * Constants
     */
    int sigma = seq_chars.size(), seq_size = seq.size();
    /*
     * Stacks
     */
    vector<int> cur_trav_stack(seq.size()+1,-1);
    stack<int> IL_stack, backtrack_decision_stack;
    // Place we are
    int cur_trav_pos = 1;
    cur_trav_stack[0] = 0;
    cur_trav_stack[1] = next_chars[0][0];
    // Place we compare
    IL_stack.push(1);
    // Decision we made
    backtrack_decision_stack.push(0);
    while (!(backtrack_decision_stack.empty()))
    {
        if (cur_trav_pos > seq_size){
            cout << "That is the case"<<endl;
            exit(1);
        }
        int cur_place = cur_trav_stack[cur_trav_pos],
            IL_val = IL_stack.top();
        int offset = char_map[seq[cur_trav_stack[IL_val]-1]], num_failures = 0;
        //cout << "Cur traversal: "<<cur_trav_pos<<endl;
        //print_stack(cur_trav_stack);
        //cout << "IL_val:" <<IL_val<<" Offset: "<<offset<<" Num_failures: "<<num_failures<<" Cur place: "<<cur_place<<endl;
        while (((offset + num_failures) < sigma) & (next_chars[cur_place][offset + num_failures] > seq_size)){
            //cout << "Next chars: "<<next_chars[cur_place][offset + num_failures]<<endl;
            num_failures++;
        }
        //cout << "IL_val:" <<IL_val<<" Offset: "<<offset<<" Num_failures: "<<num_failures<<" Cur place: "<<cur_place<<endl;
        if ((offset + num_failures) < sigma)
        {
            cur_trav_pos++;
            cur_trav_stack[cur_trav_pos] = next_chars[cur_place][offset + num_failures];
            if (num_failures == 0)
                IL_stack.push(IL_val + 1);
            else {
                lyndon_words.push_back(translate_container(cur_trav_stack,cur_trav_pos+1,seq));
                IL_stack.push(1);
            }
            backtrack_decision_stack.push(offset + num_failures);
        } else {
            // Backtrack
            /*cout << "Backtrack"<<endl;
            cout << "Cur trav pos: "<<cur_trav_pos<<" "<<cur_trav_stack[cur_trav_pos]<<endl;*/
            cur_trav_pos--;
            int backtrack_decision = backtrack_decision_stack.top() + 1;
            //cout << "Iterating: "<<IL_stack.size()<<" "<<backtrack_decision_stack.size()<<endl;
            while ((backtrack_decision >= sigma) || (next_chars[cur_trav_stack[cur_trav_pos]][backtrack_decision] >= seq_size)){
                backtrack_decision_stack.pop();
                IL_stack.pop();
                if (backtrack_decision_stack.empty())
                    break;
                backtrack_decision = backtrack_decision_stack.top() + 1;
                cur_trav_pos--;
            }
            /*cout << "Backtrack: "<<backtrack_decision<<endl;
            cout << "Cur trav pos: "<<cur_trav_pos<<" "<<cur_trav_stack[cur_trav_pos]<<endl;*/
            if (!backtrack_decision_stack.empty()){
                backtrack_decision_stack.pop();
                backtrack_decision_stack.push(backtrack_decision);
                IL_stack.pop();
                IL_stack.push(1);
                int cur_place = cur_trav_stack[cur_trav_pos];
                cur_trav_pos++;
                cur_trav_stack[cur_trav_pos] = next_chars[cur_place][backtrack_decision];
                lyndon_words.push_back(translate_container(cur_trav_stack,cur_trav_pos+1,seq));
            }
        }  
    }
    return lyndon_words;
}

// Stack enumeration
int stack_enumeration(const vector<map<char,pair<int,int>>> & next_chars, std::set<char> & seq_chars)
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
            if ((k_val.first > cur_IL_word[0]) & (k_val.second.first > 0))
                break;
        }
        for (auto k_val:next_chars[placement])
        {
            //cout << "Key: "<<k_val.first<<" "<<k_val.second.first<<endl;
            if (k_val.first < cur_IL_word[idx_IL_word])
                continue;
            if (k_val.second.first == 0)
                continue;
            string new_L_word = cur_IL_word + k_val.first;
            if (k_val.first == cur_IL_word[idx_IL_word])
            {
                placement_IL.push(idx_IL_word + 1);
            }
            if (k_val.first > cur_IL_word[idx_IL_word])
            {   
                placement_IL.push(0);
                lyndon_words.push_back(new_L_word);
            }
            cur_IL_stack.push(new_L_word);
             placement_SEQ.push(k_val.second.second);
        }
    }
    if (DEBUG)
    {
        for (auto i: lyndon_words)
            cout << "Lyndon: "<<i<<endl;
    }
    return lyndon_words.size();
}


// DFS enumeration
vector<string> dfs_enumeration(const vector<vector<int>> & next_chars, std::set<char> & seq_chars, string seq)
{
    int seq_size = int(seq.size()), sigma_size = seq_chars.size();
    int lyndon_w = 0;
    vector<string> lyndon_words;
    for (auto key: seq_chars){
        lyndon_w++;
        lyndon_words.push_back(string(1,key));
    }
    map<char,int> char_map;
    int place = 0;
    for (auto c:seq_chars)
        char_map[c] = place++;
    for (size_t i = 0; i < next_chars.size() - 2; ++i)
    {
        cout << endl;
        cout << "Place in sequence: "<<i<<endl;
        cout << endl;
        stack<int> cur_IL_stack, cur_num_char;
        vector<int>  curr_traversal(5,-1);
        int idx_trav = 0;
        cur_IL_stack.push(0);
        curr_traversal[idx_trav] = i;
        cur_num_char.push(0);
        while (true)
        {
            print_stack(curr_traversal, idx_trav);
            int idx_IL = cur_IL_stack.top();
            int idx_seq = curr_traversal[idx_trav] + 1;
            int cur_char = cur_num_char.top(); 
            int pointed_char = char_map[seq[curr_traversal[idx_IL]]];
            // Plot:
            cout << "Cur idx: "<<idx_trav<<" Cur char: "<<cur_char<<" Pointed char: "<<pointed_char<<" idx_IL: "<<idx_IL<<endl;
            if ((idx_trav == 0) && (cur_char+pointed_char >= sigma_size))
                break;
            if ((pointed_char + cur_char) < sigma_size){
                int next_idx = next_chars[idx_seq][pointed_char + cur_char++] - 1;
                cout << "Next chars " <<endl;
                print_stack(next_chars[idx_seq], idx_trav);
                cout <<"Char comparing to: "<<char_map[seq[curr_traversal[pointed_char]]]<<" cur_char "<<cur_char<<endl;
                cout <<"Next idx: "<<next_idx<<endl;
                while ((next_idx >= seq_size) & ((pointed_char + cur_char) < sigma_size)){
                    next_idx = next_chars[idx_seq][pointed_char + cur_char++] - 1;
                    cout <<"Next idx: "<<next_idx<<" "<<cur_char<<endl;
                }
                if (next_idx < seq_size)
                {
                    idx_trav++;
                    curr_traversal[idx_trav] = next_idx;
                    if (cur_char == 1){
                        cur_IL_stack.push(idx_IL + 1);
                    } else{
                        cout << "Inserto: "<<endl;
                        print_stack(curr_traversal, idx_trav);
                        cur_IL_stack.push(char_map[seq[i]]);
                        lyndon_w++;
                        string cur_word = "";
                        for (int i = 0; i <= idx_trav; i++)
                            cur_word += seq[curr_traversal[i]];
                        lyndon_words.push_back(cur_word);
                    }
                    cur_num_char.pop();
                    // Next time I pass through here I follow by cur_char
                    cur_num_char.push(cur_char);
                    // In the next iteration I just have to assay the first option: offset = 0
                    cur_num_char.push(0);
                } else {
                    cout << "Backtrackeando"<<endl;
                    cout << "Cur trav: "<<endl;
                    print_stack(curr_traversal, idx_trav);
                    cout << "Stack IL size: "<<cur_IL_stack.size()<<endl;
                    cout << "cur_num_char size: "<<cur_num_char.size()<<endl;
                    cout << "Next idx: "<<next_idx<<endl;
                    cout << "Idx trav: "<<idx_trav<<endl;
                    // Backtrack
                    while ((next_idx >= seq_size) && (idx_trav > 0))
                    {
                        cur_IL_stack.pop();
                        cur_num_char.pop();
                        idx_trav--;
                        idx_IL = cur_IL_stack.top();
                        idx_seq = curr_traversal[idx_trav] + 1;
                        cur_char = cur_num_char.top(); 
                        pointed_char = char_map[seq[curr_traversal[idx_IL]]];
                        cout << "Idx trav: "<<idx_trav<<endl;
                        cout<<"Pointed char: "<<pointed_char<<" curchar: "<<cur_char<<endl;
                        while ((next_idx >= seq_size) & ((pointed_char + cur_char) < sigma_size)){
                            next_idx = next_chars[idx_seq][pointed_char + cur_char++] - 1;
                            cout <<"Next idx: "<<next_idx<<" "<<endl;
                        }
                        cout << "Final next idx: "<<next_idx<<endl;
                    }
                    if (next_idx < seq_size)
                    {
                        idx_trav++;
                        curr_traversal[idx_trav] = next_idx;
                        if (cur_char == 1)
                            cur_IL_stack.push(idx_IL+1);
                        else{
                            cur_IL_stack.push(char_map[seq[i]]);
                            lyndon_w++;
                        }
                        cur_num_char.pop();
                        cur_num_char.push(cur_char);
                        cur_num_char.push(0);
                    }
                    cout <<"Idx trav: "<<idx_trav<<" next idx: "<<next_idx<<" cur num char "<<cur_char<<endl;
                    print_stack(curr_traversal, idx_trav);
                }
            }
        }
    }
    return lyndon_words;
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
                string seq = get_seq(file);
                cout << "Sequence: "<<seq<<endl;
                std::set<char> seq_chars = get_seq_chars(file);
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
                    //vector<map<char, pair<int,int>>> forward_count = construct(file);
                    vector<vector<int>> forward_count = construct(file);
                    auto start = std::chrono::system_clock::now();
                    //number = stack_enumeration(forward_count, seq_chars);
                    //vector<string> lyndon_words = dfs_enumeration(forward_count, seq_chars, seq);
                    vector<string> lyndon_words = enumeration_dominik(forward_count, seq_chars, seq);
                    auto end = std::chrono::system_clock::now();
                    std::chrono::duration<float,std::milli> duration = end - start;
                    #pragma omp critical
                    {
                        times[sig_l.first].push_back({sig_l.second, (duration.count()/number)});
                    }
                    print_lyndon(lyndon_words);
                    number = lyndon_words.size();
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