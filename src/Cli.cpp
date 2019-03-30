#include <Cli.h>
#include <Parser.h>
#include <cstring>
#include <cctype>

Type Cli::type;
Outfmt Cli::outfmt;
std::string Cli::fname;

unsigned int Cli::x_size = 0;
unsigned int Cli::y_size = 0;

int Cli::run(int &argc, char *argv[])
{
    int ret = 0;
    if((ret = parse_args(argc, argv)))
        return ret;

    if((ret = parse_test()))
        return ret;

    if((ret = create_noc()))
        return ret;

    if((ret = inject_msg()))
        return ret;

    if((ret = init_thread()))
        return ret;

    NoC::wait_simulation();

	//report();
	NoC::report(outfmt, fname);

    return 0;
}

int Cli::parse_args(int &argc, char *argv[])
{
    if(argc < 5){
        print_usage();
        return -1;
    }

	if(!strcmp(argv[2], "mesh")){
		type = Type::MESH;
	} else if(!strcmp(argv[2], "ring")){
		type = Type::RING;
	} else {
		std::cout << "Opção inválida!" << std::endl << std::endl;
		print_usage();
		return -2;
	}
    
    if(type == Type::RING){
        if(argc > 5 || !isdigit(argv[3][0])){
            std::cout << "Tamanho inválido" << std::endl << std::endl;
            print_usage();
            return -3;
        } else {
            x_size = atoi(argv[3]);
        }
    } else if(type == Type::MESH){
        if(argc != 6 || !isdigit(argv[3][0]) || !isdigit(argv[4][0])){
            std::cout << "Tamanho inválido" << std::endl << std::endl;
            print_usage();
            return -4;
        } else {
            x_size = atoi(argv[3]);
            y_size = atoi(argv[4]);
        }
    }

	if(!strcmp(argv[1], "xml")){
		outfmt = Outfmt::XML;
	} else if(!strcmp(argv[1], "json")){
		outfmt = Outfmt::JSON;
	} else {
		std::cout << "Formato de saída inválido" << std::endl << std::endl;
		print_usage();
		return -5;
	}

    fname = argv[argc-1];

    return 0;
}

void Cli::print_usage()
{
    std::cout << "Usage: nocemu OUTFMT TYPE SIZE_X [SIZE_Y] FILE " << std::endl;
	std::cout << "\tOUTFMT: xml/json output format" << std::endl;
    std::cout << "\tTYPE:   ring/mesh" << std::endl;
    std::cout << "\tSIZE_X: x axis topology size" << std::endl;
    std::cout << "\tSIZE_Y: y axis topology size. Only for mesh" << std::endl;
	std::cout << "\tFILE:   input message file" << std::endl;
}

int Cli::parse_test()
{
    int ret = Parser::parse(fname, type);
    switch(ret){
    case -1:
        std::cout << "Arquivo não existente" << std::endl;
        break;
    case -2:
        std::cout << "Arquivo incompatível com mesh" << std::endl;
        break;
    default:
        std::cout << "Carregado com sucesso" << std::endl;
        break;
    }    
    
    return ret;
}

int Cli::create_noc()
{
    int ret = 0;
    if((ret = check_size())){
        std::cout << "Tamanho insuficiente para simulação" << std::endl;
        return ret;
    }

    switch(type){
    case Type::RING:
        ret = NoC::build_noc(x_size);
        break;
    case Type::MESH:
        ret = NoC::build_noc(x_size, y_size);
        break;
    }

    if(!ret) {
        std::cout << "Erro ao criar NoC" << std::endl;
        return -2;
    } else {
        std::cout << "Criado " << ret << " nodos" << std::endl;
    }
    return 0;    
}

int Cli::check_size()
{
    if(x_size-1 < Parser::get_max_x())
        return -1;

    if(type == Type::MESH && y_size < Parser::get_max_y())
        return -2;

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

int Cli::report()
{
	


	std::cout << std::endl << "Requests: " << std::endl;
	for(unsigned int i = 0; i < NoC::get_pes_size(); i++){
		std::cout << "PE" << NoC::get_pe_x(i);
		if(type == Type::MESH) std::cout << NoC::get_pe_y(i);
		std::cout << ": " << NoC::get_pe_rcvd(i) << std::endl;
	}

	std::cout << std::endl << "Volume: " << std::endl;
	for(unsigned int i = 0; i < NoC::get_routers_size(); i++){
		std::cout << "R" << NoC::get_router_x(i);
		if(type == Type::MESH) std::cout << NoC::get_router_y(i);
		std::cout << ":" << std::endl;
		std::cout << "\t" << "Messages: " << NoC::get_router_msg_cnt(i) << std::endl;
		std::cout << "\t" << "Local: " << NoC::get_router_local_ratio(i) << "%" << std::endl;
		std::cout << "\t" << "Left: " << NoC::get_router_left_ratio(i) << "%" << std::endl;
		std::cout << "\t" << "Right: " << NoC::get_router_right_ratio(i) << "%" << std::endl;
		std::cout << "\t" << "Top: " << NoC::get_router_upper_ratio(i) << "%" << std::endl;
		std::cout << "\t" << "Bottom: " << NoC::get_router_bottom_ratio(i) << "%" << std::endl;
	}

	return 0;
}
