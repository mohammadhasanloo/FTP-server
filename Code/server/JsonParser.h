#ifndef JSON_PARSER_HEADER
#define JSON_PARSER_HEADER

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

using namespace std;

const char COMMA =  ',';
const char QOUTATION =  '\"';
const char COLON =  ':';
const char LBRACK =  '[';
const char RBRACK =  ']';
const char LBRACE =  '{';
const char RBRACE =  '}';

class JsonParser{
    public: 
        string find_val_json(string json, string key);
        vector<string> split_array(string array);
        string read_json_file(string path);
};

#endif