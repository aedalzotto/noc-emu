#ifndef _CLI_H_
#define _CLI_H_

#include <Noc.h>

#include <iostream>
#include <vector>

class Cli {
public:
    static int run(int &argc, char *argv[]);

private:
    static Type type;
    static std::string fname;

    static unsigned int x_size;
    static unsigned int y_size;

    static int parse_args(int &argc, char *argv[]);
    static void print_usage();
    static int parse_test();
    static int create_noc();
    static int check_size();
    static int inject_msg();
    static int init_thread();

};

#endif /* _CLI_H_ */