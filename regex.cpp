#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <map>
#include <algorithm>

std::vector<std::string> split(const std::string& s, char delim) {
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

class lnfa{
public:
    //std::unordered_map<std::string,std::unordered_map<std::string,std::string>> transitions;
    std::unordered_map<std::string,std::unordered_map<std::string,std::vector<std::string>>> transitions;
    std::string initial_state="";
    std::unordered_map<std::string,bool> final_states;
    std::vector<std::string> words;
public:
    lnfa()=default;
    void read(std::string input_file_path){
        std::ifstream file(input_file_path);
        std::string line;
        //stari
        std::getline(file,line);
        //alfabet
        std::getline(file,line);
        //nr tranzitii
        int transition_count=0;
        std::getline(file,line);
        transition_count=std::stoi(line);
        //tranzitii 
        for(int i=0;i<transition_count;i++){
            std::getline(file,line);
            std::vector<std::string> split_line=split(line,' ');
            this->transitions[split_line[0]][split_line[2]].push_back(split_line[1]);
        }
        //stare initiala
        std::getline(file,line);
        this->initial_state=line;
        //stari finale;
        std::getline(file,line);
        for(auto &state:split(line,' ')){
            this->final_states[state]=true;
        }
    }
};

class RegEx{
    std::string internal_string;
public:
    RegEx(std::string regex){
        this->internal_string=regex;
    }
    RegEx operator+(const RegEx& other) const{
        if(this->internal_string=="∅"){
           return other;
        }
        if(other.internal_string=="∅"){
            return *this;
        }
        if(this->internal_string == other.internal_string){
            return *this;
        }
        return {"("+this->internal_string+"+"+other.internal_string+")"};
    }
    RegEx operator*(const RegEx& other) const{
        if(this->internal_string=="∅"){
            return {"∅"};
        }
        if(other.internal_string=="∅"){
            return {"∅"};
        }
        if(this->internal_string=="λ"){
            return other;
        }
        if(other.internal_string=="λ"){
            return *this;
        }
        return {this->internal_string+other.internal_string};
    }
    RegEx operator*() const{
        if(this->internal_string=="∅"){
            return {"λ"};
        }
        if(this->internal_string=="λ"){
            return {"λ"};
        }
        if(this->internal_string.size() == 1){
            return {this->internal_string+"*"};            
        }
        return {"("+this->internal_string+")*"};
    }
    bool operator<(const RegEx& other) const{
        return internal_string<other.internal_string;
    }
    friend std::ostream& operator<<(std::ostream& os,const RegEx& ex){
        os<<ex.internal_string;
        return os;
    }
};

class efa{
    std::unordered_map<std::string,std::map<RegEx,std::vector<std::string>>> transitions;
    std::string initial_state="";
    std::unordered_map<std::string,bool> final_states;
    std::vector<std::string> words;
public:
    efa()=default;
    void initialize(lnfa& lnfa){
        this->initial_state=lnfa.initial_state;
        this->final_states=lnfa.final_states;
        this->words=lnfa.words;
        for(auto& state:lnfa.transitions){
            for(auto& transition:state.second){
                for(auto& final_state:transition.second){
                    this->transitions[state.first][transition.first].push_back(final_state);
                }
            }
        }
    }
    void normalize_final_states(){
        std::string new_final_state="qf";
        for(auto& state:final_states){
             if(state.second){
                transitions[state.first][{"λ"}].push_back(new_final_state); //notez lambda cu _
             }
        }
        final_states.clear();
        final_states[new_final_state]=true;
    }
    void normalize_initial_state(){
        std::string new_initial_state="qi";
        transitions[new_initial_state][{"λ"}].push_back(initial_state);
        initial_state=new_initial_state;
    }
    RegEx find_regex(std::string from,std::string to){
        for(auto& Regex:transitions[from]){
            for(auto& state:Regex.second){
                if(state==to){
                    return Regex.first;
                }
            }
        }
        return {"∅"};
    }
    void supress_state(std::string state){
        std::cout<<"Supressing state: "<<state<<"\n";

        std::vector<std::string> input_states;
        std::vector<std::string> output_states;
        for(auto& transition:transitions){
             for(auto& regex:transition.second){
                for(auto& final_state:regex.second){
                    if(final_state == state && transition.first!=state){
                        input_states.push_back(transition.first);
                    }
                }
            }
        }
        for(auto& transition:transitions[state]){
            for(auto& final_state:transition.second){
                if(final_state!=state){
                    output_states.push_back(final_state);
                }
            }
        }
        for(auto& input_state:input_states){
            for(auto& output_state:output_states){
                std::cout<<"Processing transition: "<<input_state<<" --> "<<output_state<<"\n";
                RegEx initial_regex=find_regex(input_state,output_state);
                RegEx regex1=find_regex(input_state,state);
                RegEx regex2=find_regex(state,output_state);
                RegEx loop_regex=find_regex(state,state);
                transitions[state].erase(find_regex(input_state,output_state));
                RegEx new_regex=initial_regex+(regex1*(*loop_regex)*regex2);
                transitions[input_state][new_regex].push_back(output_state);
                std::cout<<"New Regex: "<<new_regex<<"\n";
            }
        }
        for(auto& input_state:input_states){
            std::vector<std::string>*transition=&(transitions[input_state][find_regex(input_state,state)]);
            auto it = std::find(transition->begin(), transition->end(), state);
            if (it != transition->end()) {
                transition->erase(it);
            }
        }
        for(auto& output_state:output_states){
            std::vector<std::string>*transition=&(transitions[state][find_regex(state,output_state)]);
            auto it = std::find(transition->begin(), transition->end(), output_state);
            if (it != transition->end()) {
                transition->erase(it);
            }
        }
        transitions[state].erase(find_regex(state,state));
    }
    void determine_regex(){
        for(auto& state:transitions){
            if(state.first!=initial_state && !final_states[state.first]){
                this->supress_state(state.first);
            }
        }
    }
    RegEx get_regex(){
        return find_regex("qi","qf");
    }
};

class regex_converter{
    efa aux_efa;
public:
    regex_converter()=default;
    void convert_lnfa_to_regex(lnfa& lnfa,RegEx& ex){
        aux_efa.initialize(lnfa);
        aux_efa.normalize_final_states();
        aux_efa.normalize_initial_state();
        aux_efa.determine_regex();
        ex=aux_efa.get_regex();
    }
};

int main(){
    lnfa automat_input;
    automat_input.read("date_regex.txt");
    regex_converter convertor;
    RegEx output("");
    convertor.convert_lnfa_to_regex(automat_input,output);
    std::cout<<output;
    return 0;
}