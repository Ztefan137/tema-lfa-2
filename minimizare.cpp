#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <list>
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

class dfa{
public:
    std::unordered_map<std::string,std::unordered_map<std::string,std::string>> transitions;
    std::string initial_state="";
    std::unordered_map<std::string,bool> final_states;
    std::vector<std::string> words;
    std::vector<std::string> states;
    std::vector<std::string> alphabet;
public:
    int get_partition(std::vector<std::set<std::string>>& partitions,std::string state){
        for(int i=0;i<partitions.size();i++){
            if(partitions[i].count(state) == 1){
                return i;
            }
        }
        throw 69;
    }
    std::vector<std::set<std::string>> determine_eqivalent_states(){
        std::vector<std::set<std::string>> partitions;
        std::set<std::string> non_final_partition;
        std::set<std::string> final_partition;
        for(auto &state:states){
            if(final_states[state]){
                std::cout<<state<<"\n";
                final_partition.insert({state});
            }else{
                non_final_partition.insert({state});
            }
        }
        partitions.push_back(non_final_partition);
        partitions.push_back(final_partition);

        bool partitioned=false;
        do{
            std::vector<std::set<std::string>> new_global_partitions;
            partitioned=false;
            for(auto &partition:partitions){
                bool self_partitioned=true;
                std::vector<std::set<std::string>> new_partitions;
                for(auto &letter:alphabet){
                    new_partitions.clear();
                    new_partitions.resize(this->states.size());
                    for(auto state:partition){
                        new_partitions[get_partition(partitions,transitions[state][letter])].insert(state);
                    }
                    int partition_count=0;
                    for(auto& partition:new_partitions){
                        if(!partition.empty()){
                            partition_count++;
                        }
                    }
                    if(partition_count>1){
                        self_partitioned=false;
                        partitioned=true;
                        break;
                    }
                } 
                if(!self_partitioned){
                    for(auto& new_partition:new_partitions){
                        if(!new_partition.empty()){
                           new_global_partitions.push_back(new_partition);
                        }
                    }
                }else{
                    new_global_partitions.push_back(partition);
                }
            }
            partitions=new_global_partitions;
            for(auto& partition:partitions){
                std::cout<<"{";
                for(auto& state:partition){
                    std::cout<<state<<" ";
                }
                std::cout<<"}\n";
            }
        }while(partitioned == true);
        return partitions;
    }
    void combine_states(std::set<std::string> partition){
        std::string new_state="";
        for(auto& state:partition){
            new_state+=state;
        }
        for(auto& state:partition){
            if(final_states[state]){
                final_states[new_state]=true;
                break;
            }
        }
        for(auto& state:partition){
            for(auto& transition:transitions[state]){
                if(partition.count(transition.second) == 0){
                    transitions[new_state][transition.first]=transition.second;
                }else{
                    transitions[new_state][transition.first]=new_state;
                }
            }
        }
        for(auto& state:states){
            for(auto& transition:transitions[state]){
                if(partition.count(transition.second) == 1){
                    transitions[state][transition.first]=new_state;
                }
            }
        }
        for(auto& state:partition){
            transitions.erase(state);
        }
        states.push_back(new_state);
    }
    void minimize(){
        for(auto& partition:this->determine_eqivalent_states()){
            if(partition.size()>1){
                combine_states(partition);
            }
        }
    }
    friend std::ostream& operator<<(std::ostream& os,const dfa& automat){
            os<<"Stari:\n";
            for(auto& state:automat.states){
                os<<state<<" ";
            }
            os<<"\n";
            os<<"Stare initiala: "<<automat.initial_state<<"\n";
            os<<"Stari finale:\n";
            for(auto& state:automat.final_states){
                if(state.second){
                    os<<state.first<<" ";
                }
            }
            os<<"\n";
            os<<"Tranzitii:\n";
            for(auto& state:automat.transitions){
                for(auto& transition:state.second){
                    os<<state.first<<"->"<<transition.second<<" "<<transition.first<<"\n";
                }
            }
            return os;
    }
};

class lnfa{
public:
    std::unordered_map<std::string,std::unordered_map<std::string,std::vector<std::string>>> transitions;
    std::string initial_state="";
    std::unordered_map<std::string,bool> final_states;
    std::vector<std::string> words;
    std::vector<std::string> alphabet;
    std::vector<std::string> states;
public:
    lnfa()=default;
    void read(std::string input_file_path){
        std::ifstream file(input_file_path);
        std::string line;
        //stari
        std::getline(file,line);
        this->states=split(line,' ');
        //alfabet
        std::getline(file,line);
        this->alphabet=split(line,' ');
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
    void add_transition(std::string input_state,std::string output_state,std::string letter){
        bool found_transition=false;
        for(auto& state:transitions[input_state][letter]){
            if(state == output_state){
                found_transition=true;
                break;
            }
        }
        if(!found_transition){
            transitions[input_state][letter].push_back(output_state);
        }
    }
    void normalize_final_states_lambda_closure(){
        for(auto& state:this->states){
            for(auto& destination_state:transitions[state]["λ"]){
                if(final_states[destination_state]){
                    final_states[state]=true;
                }
            }
        }
    }
    void complete_lambda_chain(std::string root_state,std::string curr_state,std::set<std::string> visited_states){
        if(visited_states.count(curr_state) != 0){
            return;
        }
        visited_states.insert(curr_state);
        if(root_state != curr_state){
            this->add_transition(root_state,curr_state,"λ");
        }
        if(final_states[curr_state]){
            final_states[root_state]=true;
        }
        for(auto& state:transitions[curr_state]["λ"]){
            complete_lambda_chain(root_state,state,visited_states);
        }
    }
    void enhance_lambda_chain(std::string state){
        for(auto& first_state:transitions[state]["λ"]){
            for(auto& letter:this->alphabet){
                for(auto& second_state:transitions[first_state][letter]){
                    this->add_transition(state,second_state,letter);
                }
            }
        }
        for(auto& letter:this->alphabet){
            for(auto& first_state:transitions[state][letter]){
                for(auto& second_state:transitions[first_state]["λ"]){
                    this->add_transition(state,second_state,letter);
                }
            }
        }
    }
    void complete_lambda_chains(){
        for(auto& state:this->states){
            std::set<std::string> empty_set;
            complete_lambda_chain(state,state,empty_set);
        }
        for(auto& state:this->states){
            enhance_lambda_chain(state);
        }
    }
    void eliminate_lambda_transitions(){
        for(auto& state:states){
            if(transitions[state]["λ"].size()>0){
                transitions[state].erase("λ");
            }
        }
        alphabet.erase(std::remove(alphabet.begin(), alphabet.end(), "λ"),alphabet.end());
    }
    void lambda_closure(){
        this->normalize_final_states_lambda_closure();
        std::cout<<*this;
        this->complete_lambda_chains();
        this->eliminate_lambda_transitions();
    }
    void sew_transitions(std::vector<std::string>& input_states,std::string output_state,std::string letter){
        std::set<std::string> reachable_states;
        for(auto& input_state:input_states){
            for(auto& state:transitions[input_state][letter]){ //stari in care ajungem din starile precedente cu litera curenta
                reachable_states.insert({state});
            }
        }
        for(auto& reachable_state:reachable_states){
            transitions[output_state][letter].push_back(reachable_state);
        }
    }
    void subset_construction(dfa& result){
        std::set<std::vector<std::string>> subset_states;
        std::queue<std::string> states;
        states.push(initial_state);
        subset_states.insert({initial_state});
        result.initial_state=this->initial_state;
        result.states.push_back(this->initial_state);
        result.alphabet=this->alphabet;
        while(!states.empty()){
            for(auto& letter:this->alphabet){
                std::string new_state="";
                std::vector<std::string> output_states(0);
                bool found_state=false;
                for(auto state:transitions[states.front()][letter]){
                    new_state+=state; //concatenare
                    output_states.push_back(state);
                    found_state=true;
                }
                if(found_state && subset_states.count({new_state}) == 0){
                    states.push(new_state);
                    subset_states.insert({new_state});
                    for(auto &letter:alphabet){
                        bool initial_state=false;
                        for(auto &state:this->states){
                            if(state == new_state){
                                initial_state=true;
                            }
                        }
                        if(initial_state == false){
                            sew_transitions(output_states,new_state,letter);
                        }
                    }
                    result.states.push_back(new_state);
                }
                if(found_state){
                    bool final_state=false; // determinare daca e stare finala sau nu
                    for(auto& state:output_states){
                        if(final_states[state]){
                            final_state=true;
                        }
                    }
                    result.transitions[states.front()][letter]=new_state;
                    result.final_states[new_state]=final_state;
                }
            }
            states.pop();
        }
    }
    dfa convert_to_dfa(){
        dfa result;
        this->lambda_closure();
        this->subset_construction(result);
        return result;
    }
    friend std::ostream& operator<<(std::ostream& os,const lnfa& automat){
        os<<"Stari:\n";
        for(auto& state:automat.states){
            os<<state<<" ";
        }
        os<<"\n";
        os<<"Stare initiala: "<<automat.initial_state<<"\n";
        os<<"Stari finale:\n";
        for(auto& state:automat.final_states){
            if(state.second){
                os<<state.first<<" ";
            }
        }
        os<<"\n";
        os<<"Tranzitii:\n";
        for(auto& state:automat.transitions){
            for(auto& transition:state.second){
                for(auto& final_state:transition.second){
                    os<<state.first<<" --"<<transition.first<<"--> "<<final_state<<"\n";
                }
            }
        }
        return os;
    }
};
int main(){
    lnfa automat;
    automat.read("date_minimizare.txt");
    std::ofstream file("output_minimizare.txt");
    dfa automat_dfa=automat.convert_to_dfa();
    file<<automat_dfa<<"\n";
    automat_dfa.minimize();
    file<<automat_dfa;
}