#include <Cli.h>
#include <Parser.h>

int Cli::run(std::string fname)
{
    int ret;
    if((ret = parse_test(fname)))
        return ret;

    if((ret = create_noc()))
        return ret;

    if((ret = inject_msg()))
        return ret;

    if((ret = init_thread()))
        return ret;

    NoC::wait_simulation();

    return 0;
}

int Cli::parse_test(std::string &fname)
{
    int ret = Parser::parse(fname);
    if(ret)
        std::cout << "Arquivo não existente" << std::endl;
    else
        std::cout << "Carregado com sucesso" << std::endl;
    
    return ret;
}

int Cli::create_noc()
{
    int ret = NoC::build_noc(2);

    if(!ret) {
        std::cout << "Erro ao criar NoC" << std::endl;
        return -2;
    } else {
        std::cout << "Criado " << ret << " nodos" << std::endl;
    }
    return 0;    
}

int Cli::inject_msg()
{
    int ret = NoC::inject_to_pe(Parser::get_messages());

    if(!ret) {
        std::cout << "Erro ao injetar mensagens" << std::endl;
        return -3;
    } else
        std::cout << "Injetado " << ret << " mensagens" << std::endl;
    
    return 0;
}

int Cli::init_thread()
{
    int ret = NoC::create_threads();

    if(!ret) {
        std::cout << "Erro ao criar threads" << std::endl;
        return -4;
    } else
        std::cout << "Criado " << ret << " threads" << std::endl;
    
    std::cout << "Simulação iniciada" << std::endl << std::endl;
    return 0;
}