#include <Parser.h>
#include <Cli.h>

int main(int argc, char *argv[])
{    
    if(Parser::args(argc, argv)){
        return -1;
    }

    return Cli::run(Parser::get_fname());
}