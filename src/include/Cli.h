#ifndef _CLI_H_
#define _CLI_H_

#include <iostream>
#include <vector>

class Cli {
public:
    static int run(std::string fname);

private:
    static int parse_test(std::string &fname);
    static int create_noc();
    static int inject_msg();
    static int init_thread();

};

#endif /* _CLI_H_ */