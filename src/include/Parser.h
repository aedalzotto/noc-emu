#ifndef _PARSER_H_
#define _PARSER_H_

#include <Noc.h>

#include <iostream>
#include <vector>

class Parser {
public:
    static int parse(std::string &fname);
    static std::vector<message_t> get_messages();

    static int get_max_x() { return max_x; }
    static int get_max_y() { return max_y; }

private:
    static std::vector<message_t> messages;
    static bool is_integer(const std::string &s);

    static int max_x;
    static int max_y;

    static const std::vector<std::string> explode(const std::string& s, const char& c);

};

#endif /* _PARSER_H_ */