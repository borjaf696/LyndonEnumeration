#include "lyndon.h"

using namespace sdsl;
using namespace std;

// Lyndon enumeration
void enumeration(const wt_huff<rrr_vector<63>> wt)
{
    std::unordered_set<char> seq_chars = get_seq_chars();
    /*
     * Operate as in 
     */ 
    function<void(int,int,string,vector<string>&)> _enumeration = [wt, seq_chars, &_enumeration](int position, int matched_position, string seq, vector<string> & lyndon_words) 
    {
        char ref_char = seq[matched_position];
        // For each available char we need to know its placement
        unordered_map<char,int> remaining_chars = get_remaining_chars(wt,position, seq_chars);
        for (auto k_v:remaining_chars)
        {
            char c = k_v.first;
            int placement = k_v.second;
            string n_seq = seq + c;
            if (c == ref_char)
                _enumeration(placement + 1, matched_position + 1, n_seq, lyndon_words);
            else if (c > ref_char)
            {
                lyndon_words.push_back(n_seq);
                _enumeration(placement + 1, 0, n_seq, lyndon_words);
            }
        }
    };
    vector<string> lyndon_words;
    _enumeration(0, 0, "", lyndon_words);
    int number = 0;
    for (auto lyndon:lyndon_words)
        cout << "Place: "<<number++<<": "<<lyndon<<endl;
}

int main(int argc, char *argv[])
{
    cout << "Lyndon words enumeration"<<endl;
    // Wavelette tree
    wt_huff<rrr_vector<63>> wt;
    construct(wt,"example/simulation.txt", 1);
    enumeration(wt);
}