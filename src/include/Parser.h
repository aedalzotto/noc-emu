#ifndef _PARSER_H_
#define _PARSER_H_

#include <Noc.h>

#include <iostream>
#include <vector>

class Parser {
public:
    static int args(int &argc, char *argv[]);
    static std::string get_fname() { return fname; }

    static int parse(std::string &fname);
    static std::vector<message_t> get_messages();

private:
    static std::string fname;
    static std::vector<message_t> messages;

    static const std::vector<std::string> explode(const std::string& s, const char& c);
    static bool is_integer(const std::string &s);

};

#endif /* _PARSER_H_ */