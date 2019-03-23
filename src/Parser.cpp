#include <Parser.h>

#include <fstream>
#include <sstream>

std::vector<message_t> Parser::messages;

unsigned int Parser::max_x = 0;
unsigned int Parser::max_y = 0;

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

int Parser::parse(std::string &fname, Type &type)
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

        unsigned int _src_x;
        unsigned int _src_y = 0;
        unsigned int _dst_x;
        unsigned int _dst_y = 0;
        // Skip headers
        if(is_integer(buffer_split[0])){
            if(type == Type::MESH){
                if(buffer_split[0].size()%2 || buffer_split[1].size()%2)    // Only pair src and dst
                    return -2;
                
                _src_x = std::stoi(buffer_split[0].substr(0,buffer_split[0].size()/2));
                _src_y = std::stoi(buffer_split[0].substr(buffer_split[0].size()/2,buffer_split[0].size()));

                _dst_x = std::stoi(buffer_split[1].substr(0,buffer_split[1].size()/2));
                _dst_y = std::stoi(buffer_split[1].substr(buffer_split[1].size()/2,buffer_split[1].size()));
            } else {
                _src_x = std::stoi(buffer_split[0]);
                _dst_x = std::stoi(buffer_split[1]);
            }
        } else
            continue;

        // msg = buffer_split[2]
        unsigned int _end = std::stoi(buffer_split[3]);

        if(_src_x > max_x)
            max_x = _src_x;

        if(_dst_x > max_x)
            max_x = _dst_x;

        if(_src_y > max_y)
            max_y = _src_y;

        if(_dst_y > max_y)
            max_y = _dst_y;

        message_t aux = {
            .src_x = _src_x,
            .src_y = _src_y,
            .dst_x = _dst_x,
            .dst_y = _dst_y,
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