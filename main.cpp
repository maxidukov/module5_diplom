#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <variant>
#include <exception>
#include <cctype>
#include <algorithm>

using MultiType = std::variant<int, std::string, double>;

using filestruct = std::map< std::string, std::map<std::string,MultiType>>;


class ini_parser{
private:
    std::string filename_;
    filestruct fstruct_;
    //public:
    std::string eraseWS(const std::string& s){
        std::string res = s;
        res.erase(std::remove_if(res.begin(), res.end(), ::isspace),
                res.end());
        return res;
    }
    void split_in_two(const std::string& s, std::string& before, std::string& after, const char& del)
    {
        std::stringstream ss(s);
        before = "";
        after = "";
        bool got_before = false;
        while (!ss.eof()) {
            if(!got_before){
                getline(ss, before, del);
                got_before = true;
            }else
                getline(ss, after);
        }
    }
    void build_fstruct(){
        std::ifstream infile(filename_);
        if(!infile){
            throw "Error opening file\n";
        }
        std::string line;
        std::string before;
        std::string after;
        std::string section;
        std::string bad_syntax_comment = "";
        int count = 1;
        while(getline(infile,line)){
            split_in_two(line,before,after,';');
            if(!eraseWS(before).empty()){
                line = before;
                split_in_two(line,before,after,'[');
                if(eraseWS(before).empty()){
                    line = after;
                    split_in_two(line,before,after,']');
                    if(line != before){ //']' HAS BEEN FOUND
                        section = before;
                    }else bad_syntax_comment = "No closing ']' found\n";
                }else if(line == before){ //'[' HASN'T BBEN FOUND
                    //before = "";
                    split_in_two(line,before,after,'=');
                    if(before != line && !section.empty() && !after.empty()){
                        try{
                            int after_int = stoi(after); //CHOOSE CORRECT VAR TYPE
                            double after_double = stod(after);
                            if(after==std::to_string(after_int)){
                                fstruct_[section][before] = after_int;
                            }else{
                                fstruct_[section][before] = after_double;
                            }
                        }catch(...){
                            fstruct_[section][before] = after;
                        }
                    }else if(before==line){ //NONE OF ; OR [ OR = HAS BEEN FOUND
                        bad_syntax_comment = "None of ';' or '[' or '=' has been found among other symbols";
                    }
                }else bad_syntax_comment = "Symbols before opening '[' found\n";
            }//else std::cout << "Line " << count << " is a comment or empty\n";
            if(bad_syntax_comment != ""){
                std::cout << "Bad syntax in line " << count << ": " << bad_syntax_comment << "\n";
                bad_syntax_comment = "";
            }
            count++;
        }
    }
    /*void add_value(std::string sec, std::string key, std::string val){
      if(fstruct_.find(sec) == fstruct_.end()){
        fstruct_[sec] = std::map<std::string,MultiType>{};
      }
      fstruct_[sec][key] = val;
    }*/
    void print_fstruct(){
        for(const auto& section : fstruct_){
            std::cout << section.first << ":\n";
            for(const auto& record : section.second){
                std::cout <<"\t" <<record.first << " = " << MT_to_string(record.second) << ", type is " << get_MT_type(record.second) <<"\n";
            }
        }
    }

    std::string MT_to_string(MultiType input){
        if(std::holds_alternative<int>(input)){
            return std::to_string(std::get<int>(input));
        }else if(std::holds_alternative<double>(input)){
            return std::to_string(std::get<double>(input));
        }else{
            return std::get<std::string>(input);
        }
    }

    std::string get_MT_type(MultiType input){
        if(std::holds_alternative<int>(input)){
            return std::string{"int"};
        }else if(std::holds_alternative<double>(input)){
            return std::string{"double"};
        }else{
            return std::string{"std::string"};
        }
    }

public:
    template<typename T>
    T& get_value(std::string str){
        std::string section;
        std::string key;
        split_in_two(str, section, key, '.');
        try{
            if(fstruct_.count(section)==0) throw 1;
            else if(fstruct_[section].count(key)==0) throw 2;
            else if(!std::holds_alternative<T>(fstruct_[section][key])) throw 3;
            return std::get<T>(fstruct_[section][key]);
        }catch (int e){
            std::cout << "Error in retrieving value: ";
            //error_handler(e);
            std::string error_str = "";
            switch(e){
            case 1: std::cout << "Section not found\n";
                std::cout << "Perhaps you meant one of the non-empty sections available:\n";
                for(const auto& sec : fstruct_){
                    std::cout << sec.first << "\n";
                }
                break;
            case 2: std::cout << "Variable name not found\n";
                std::cout << "Perhaps you meant one of the variables available in section \"" << section << "\":\n";
                for(const auto& record : fstruct_[section]){
                    std::cout << record.first << "\n";
                }
                break;
            case 3: std::cout << "Wrong type\n";
                break;
            }
            exit(1); //WITHOUT THIS STATEMENT 'NO RETURN IN SOME PATHS' WARNING IS RAISED
            //std::terminate();
        }
    }
    //ini_parser(){}
    //ini_parser(std::string filename):filename_(filename){ build_fstruct();}
    ini_parser(std::string filename):filename_(filename){
        build_fstruct();
        /*try{
          build_fstruct();
        }catch(char const* e) {
          std::cout << e;
          exit(1);
        }*/
    }
};


int main(){
    try{
        ini_parser parser("file.ini");
        auto value = parser.get_value<double>("Section1.var1");
        //  WRONG SECTION NAME
        //auto value = parser.get_value<double>("Section10.var1");
        //  WRONG VALUE NAME
        //auto value = parser.get_value<double>("Section1.var10");
        //  WRONG TYPE
        //auto value = parser.get_value<std::string>("Section1.var1");
        std::cout << "Value requested is " << value << "\n";
    }catch(const char* e){
        std::cout << e;
    }

    return  0;
}
