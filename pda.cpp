#include <iostream>
#include <fstream>
#include <string>
#include <stack>
#include <unordered_map>
#include <vector>

std::vector<std::string> split_null(const std::string& s) {
    std::vector<std::string> result;
    if(s == "λ"){
        return {"λ"};
    }
    for (char c : s) {
        result.push_back(std::string(1,c));
    }
    return result;
}

std::vector<std::string> split(const std::string& s, char delim) {
    if(delim == '\0'){
        return split_null(s);
    }
    std::vector<std::string> result;
    size_t start = 0;
    size_t pos;
    while ((pos = s.find(delim, start)) != std::string::npos) {
        result.emplace_back(s.substr(start, pos - start));
        start = pos + 1;
    }
    result.emplace_back(s.substr(start));
    return result;
}

class pda{
    std::unordered_map<std::string/*initial_state*/,std::unordered_map<std::string/*letter*/,std::unordered_map<std::string/*stack_letter*/,std::vector<std::pair<std::string/*final_state*/,std::vector<std::string/*pushed_letter*/>>>>>> transitions;
    std::stack<std::string> stack;
    std::string marker_inceput="$";
    std::string initial_state;
    std::unordered_map<std::string,bool> final_states;
    std::string acceptance_mode;
    std::string word;
    std::unordered_map<std::string,bool> accepted_words;
    std::string accepted="RESPINS";
public:
    pda()=default;
    void read(std::string input_file_path){
        std::ifstream file(input_file_path);
        std::string line;
        //stari
        std::getline(file,line);
        //alfabet
        std::getline(file,line);
        //alfabet stiva
        std::getline(file,line);
        //nr tranzitii
        int transition_count=0;
        std::getline(file,line);
        transition_count=std::stoi(line);
        //tranzitii 
        for(int i=0;i<transition_count;i++){
            std::getline(file,line);
            std::vector<std::string> split_line=split(line,' ');
            this->transitions[split_line[0]][split_line[1]][split_line[2]].push_back({split_line[3],split(split_line[4],'\0')});
        }
        //stare initiala
        std::getline(file,line);
        this->initial_state=line;
        //marker stiva
        std::getline(file,line);
        this->marker_inceput=line;
        //stari finale;
        std::getline(file,line);
        for(auto &state:split(line,' ')){
            this->final_states[state]=true;
        }
        //mod de acceptare
        std::getline(file,line);
        this->acceptance_mode=line;
        //cuvant
        std::getline(file,line);
        this->word=line;
    }
    bool accepted_state(std::string state){
        if(acceptance_mode == "stare_finala"){
            return final_states[state];
        }
        if(acceptance_mode == "stiva_goala"){
            return stack.empty();
        }
        return final_states[state] && stack.empty();
    }
    void process_letter(int letter_idx,std::string state,std::string& word){ //bkt
        if(letter_idx == word.size()-1){ //caz de baza
            if(accepted_state(state)){
                this->accepted="ACCEPTAT";
            }
            return;
        }
        for(auto& letter:transitions[state]){
            if(letter.first == std::string(1,word[letter_idx]) || letter.first == "λ"){
                for(auto& stack_letter:letter.second){
                    if(stack_letter.first == stack.top()){
                        for(auto& final_state:stack_letter.second){
                            stack.pop();
                            if (!(final_state.second.size() == 1 && final_state.second[0] == "λ")) {
                                for (auto it = final_state.second.rbegin(); it != final_state.second.rend(); ++it) {
                                    if (*it != "λ") {
                                        stack.push(*it);
                                    }
                                }
                            }
                            process_letter(letter_idx+((letter.first == "λ")?0:1),final_state.first,word); //bkt(pas+1)
                            if (!(final_state.second.size() == 1 && final_state.second[0] == "λ")) {
                                for(auto& pushed_letter:final_state.second){
                                    stack.pop();
                                }
                            }
                            stack.push(stack_letter.first);
                        }
                    }
                }
            }
        }
    }
    void process_word(){
        while(!stack.empty()){
            stack.pop();
        }
        stack.push(marker_inceput);
        this->process_letter(0,this->initial_state,this->word);
    }
};

int main(){
    pda automat;
    automat.read("date_pda.txt");
    automat.process_word();
}
