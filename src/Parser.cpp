#include <Parser.h>

#include <fstream>
#include <sstream>

std::vector<message_t> Parser::messages;

int Parser::max_x = 0;
int Parser::max_y = 0;

const std::vector<std::string> Parser::explode(const std::string& s, const char& c)
{
    std::string buff{""};
    std::vector<std::string> v;

    for(auto n:s)
    {
        if(n != c) buff+=n; else
        if(n == c && buff != "") { v.push_back(buff); buff = ""; }
    }
    if(buff != "") v.push_back(buff);

    return v;
}

bool Parser::is_integer(const std::string &s)
{
    if(s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false ;

    char * p ;
    strtol(s.c_str(), &p, 10) ;

    return (*p == 0);
}

int Parser::parse(std::string &fname)
{
    std::fstream file;

    file.open(fname, std::fstream::in);
    if(!file.is_open())
        return -1;
    
    std::string buffer;
    std::vector<std::string> buffer_split;

    messages.clear();

    while(!file.eof()){
        std::getline(file, buffer);

        if(!buffer.size())
            continue;

        buffer_split.clear();
        buffer_split = explode(buffer, 0x09); // tab
        if(buffer_split.size() == 1) buffer_split = explode(buffer, ' ');

        if(buffer_split.size() != 4)
            continue;

        unsigned int _src;
        // Skip headers
        if(is_integer(buffer_split[0]))
            _src = std::stoi(buffer_split[0]);
        else
            continue;

        unsigned int _dst = std::stoi(buffer_split[1]);
        // msg = buffer_split[2]
        unsigned int _end = std::stoi(buffer_split[3]);

        if(_src > max_x)
            max_x = _src;

        if(_dst > max_x)
            max_x = _dst;

        message_t aux = {
            .src = _src,
            .dst = _dst,
            .msg = buffer_split[2],
            .end = _end
        };

        messages.push_back(aux);

    }

    buffer_split.clear();
    buffer.clear();
    file.close();

    return 0;
}

std::vector<message_t> Parser::get_messages()
{
    return messages;
}