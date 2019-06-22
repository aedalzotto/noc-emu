#include <Noc.h>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <sstream>

Type NoC::type;

int NoC::xq = 0;
int NoC::yq = 0;
int NoC::qtd = 0;
std::vector<Router> NoC::routers;
std::vector<PE> NoC::pes;
std::vector<std::thread> NoC::threads;

int NoC::rcvd = 0;
std::queue<std::string> NoC::prints;

std::mutex NoC::rcvd_mtx;
std::mutex NoC::print_mtx;

namespace bpt = boost::property_tree;

int NoC::build_noc(int _xq)
{
    xq = _xq;
    yq = 0;
    type = Type::RING;


    for(int i = 0; i < xq; i++){
        routers.push_back(Router(i));
        pes.push_back(PE(i));
    }

    for(int i = 0; i < xq-1; i++){
        routers[i].connect_right(routers[i+1]);
        pes[i].connect_router(routers[i]);
    }
    
    routers[xq-1].connect_right(routers[0]);
    pes[xq-1].connect_router(routers[xq-1]);

    return xq;
}

int NoC::build_noc(int _xq, int _yq)
{
    xq = _xq;
    yq = _yq;
    type = Type::MESH;

    for(int j = 0; j < yq; j++){        // X first, then Y
        for(int i = 0; i < xq; i++){
            routers.push_back(Router(i, j));
            pes.push_back(PE(i, j));
        }
    }

    // Connect PE to Router
    for(int j = 0; j < yq; j++) // X first, then Y
        for(int i = 0; i < xq; i++)
            pes[i + j*xq].connect_router(routers[i + j*xq]);
        
    // Connect Router to it's right one
    // Last one is connected to none on right
    for(int j = 0; j < yq; j++)
        for(int i = 0; i < xq-1; i++)
            routers[i + j*xq].connect_right(routers[i+1 + j*xq]);
    
    // Connect Router to it's left one
    // First one is connected to none
    for(int j = 0; j < yq; j++)
        for(int i = 1; i < xq; i++)
            routers[i + j*xq].connect_left(routers[i-1 + j*xq]);

    // Connect upper Router
    // Last row is not connected to upper
    for(int j = 0; j < yq-1; j++)
        for(int i = 0; i < xq; i++)
            routers[i + j*xq].connect_upper(routers[i + (j+1)*xq]);

    // Connect bottom Router
    // First row is not connected to bottom
    for(int j = 1; j < yq; j++)
        for(int i = 0; i < xq; i++)
            routers[i + j*xq].connect_bottom(routers[i + (j-1)*xq]);


    return xq*yq;
}

int NoC::inject_to_pe(std::vector<message_t> messages)
{
    for(unsigned int i = 0; i < messages.size(); i++)
        pes[messages[i].src_x + messages[i].src_y*xq].add_msg(messages[i]);
    
    qtd = messages.size();

    return qtd;
}

int NoC::create_threads()
{
    int ythreads = yq ? yq : 1;
    for(int i = 0; i < xq; i++){
        for(int j = 0; j < ythreads; j++){
            threads.push_back(std::thread(&PE::run, &pes[i + j*xq]));
            threads.push_back(std::thread(&Router::run, &routers[i + j*xq]));
        }
    }

    return xq*ythreads*2;
}

bool NoC::finish()
{
    rcvd_mtx.lock();
    int _rcvd = rcvd;
    rcvd_mtx.unlock();

    if(_rcvd == qtd)
        return true;
    
    return false;
}

void NoC::received(message_t msg)
{
    std::ostringstream oss;
    switch(type){
    case Type::RING:
        oss << "PE" << msg.dst_x << " recebeu de PE" << msg.src_x << ": " << msg.msg << ". E: " << msg.end;
        break;
    case Type::MESH:
        oss << "PE" << msg.dst_x << msg.dst_y << " recebeu de PE" << msg.src_x << msg.src_y << ": " << msg.msg << ". E: " << msg.end;
        break;
    }

    print_mtx.lock();
    prints.push(oss.str());
    print_mtx.unlock();

    rcvd_mtx.lock();
    rcvd++;
    rcvd_mtx.unlock();
}

void NoC::writing(message_t msg)
{
    std::ostringstream oss;
    switch(type){
    case Type::RING:
        oss << "PE" << msg.src_x << " enviando para NoC: " << msg.msg << ". Com destino PE" << msg.dst_x;
        break;
    case Type::MESH:
        oss << "PE" << msg.src_x << msg.src_y << " enviando para NoC: " << msg.msg << ". Com destino PE" << msg.dst_x << msg.dst_y;
        break;
    }

    std::lock_guard<std::mutex> lock_a(print_mtx);
    prints.push(oss.str());
}

void NoC::sending(message_t msg, unsigned int src, unsigned int dst)
{
    std::ostringstream oss;
    oss << "R" << src << " encaminhando para R" << dst << " mensagem: " << msg.msg << ". Com destino PE" << msg.dst_x;

    std::lock_guard<std::mutex> lock_a(print_mtx);
    prints.push(oss.str());
}

void NoC::sending(message_t msg, unsigned int src_x, unsigned int src_y,  unsigned int dst_x, unsigned int dst_y)
{
    std::ostringstream oss;
    oss << "R" << src_x << src_y << " encaminhando para R" << dst_x << dst_y << " mensagem: " << msg.msg << ". Com destino PE" << msg.dst_x << msg.dst_y;

    std::lock_guard<std::mutex> lock_a(print_mtx);
    prints.push(oss.str());
}

void NoC::reading(message_t msg)
{
    std::ostringstream oss;
    switch(type){
    case Type::RING:
        oss << "R" << msg.dst_x << " encaminhando mensagem para PE: " << msg.msg;
        break;
    case Type::MESH:
        oss << "R" << msg.dst_x << msg.dst_y << " encaminhando mensagem para PE: " << msg.msg;
        break;
    }

    std::lock_guard<std::mutex> lock_a(print_mtx);
    prints.push(oss.str());
}

void NoC::wait_simulation()
{
    while(true){
        print_mtx.lock();
        if(prints.size()){
            std::cout << prints.front() << std::endl;
            prints.pop();
        }
        print_mtx.unlock();

        if(finish())
            break;
    }

    for(unsigned int i = 0; i < threads.size(); i++){
        threads[i].join();
    }

    while(prints.size()){
        std::cout << prints.front() << std::endl;
        prints.pop();
    }

    std::cout << std::endl << "Simulação finalizada" << std::endl;

}

unsigned int NoC::get_pes_size()
{ 
    return pes.size();
}

unsigned int NoC::get_pe_rcvd(unsigned int i)
{
    return pes[i].get_rcvd();
}

unsigned int NoC::get_pe_x(unsigned int i)
{
    return pes[i].get_x();
}

unsigned int NoC::get_pe_y(unsigned int i)
{
    return pes[i].get_y();
}

unsigned int NoC::get_routers_size()
{
    return routers.size();
}

unsigned int NoC::get_router_x(unsigned int i)
{
    return routers[i].get_x();
}

unsigned int NoC::get_router_y(unsigned int i)
{
    return routers[i].get_y();
}

unsigned int NoC::get_router_msg_cnt(unsigned int i)
{
    return routers[i].get_msg_cnt();
}

unsigned int NoC::get_router_local_ratio(unsigned int i)
{
    return routers[i].get_local_ratio();
}

unsigned int NoC::get_router_left_ratio(unsigned int i)
{
    return routers[i].get_left_ratio();
}

unsigned int NoC::get_router_right_ratio(unsigned int i)
{
    return routers[i].get_right_ratio();
}

unsigned int NoC::get_router_upper_ratio(unsigned int i)
{
    return routers[i].get_upper_ratio();
}

unsigned int NoC::get_router_bottom_ratio(unsigned int i)
{
    return routers[i].get_bottom_ratio();
}

void NoC::report(Outfmt outfmt, std::string fname)
{
    bpt::ptree main_tree;

    add_noc(main_tree);
    
    switch(outfmt){
    case Outfmt::JSON:
        bpt::json_parser::write_json(fname.substr(0, fname.length()-4)+".json", main_tree);
        break;
    case Outfmt::XML:
        bpt::xml_writer_settings<std::string> settings('\t', 1);
        bpt::xml_parser::write_xml(fname.substr(0, fname.length()-4)+".xml", main_tree, std::locale(), settings);
        break;
    }
}

void NoC::add_noc(boost::property_tree::ptree &project)
{
    project.put("x", xq);
	if(type == Type::MESH) project.put("y", yq);
	std::string topology;
	switch(type){
	case Type::MESH:
		topology = "Mesh 2D";
		break;
	case Type::RING:
		topology = "Ring";
		break;
	}
	project.put("topology", topology);

    add_nodes(project);

    //project.put_child("noc", noc);
}

void NoC::add_nodes(boost::property_tree::ptree &noc)
{
    bpt::ptree array;
    bpt::ptree nodes;

    unsigned int ysz = yq ? yq : 1;
    for(unsigned int j = 0; j < ysz; j++)
        for(unsigned int i = 0; i < xq; i++)
            add_router(nodes, i, j);

    noc.put_child("nodes", nodes);
}

void NoC::add_router(boost::property_tree::ptree &nodes, unsigned int i, unsigned int j)
{
    bpt::ptree router;

    std::ostringstream rxy, pexy;
    rxy << routers[i + j*xq].get_x();
    if(type == Type::MESH) rxy << routers[i + j*xq].get_y();

    router.put("id", rxy.str());
    if(routers[i + j*xq].get_right_ptr())
        router.put("right", routers[i + j*xq].get_right_xy());
    if(routers[i + j*xq].get_left_ptr())
        router.put("left", routers[i + j*xq].get_left_xy());
    if(routers[i + j*xq].get_upper_ptr())
        router.put("top", routers[i + j*xq].get_upper_xy());
    if(routers[i + j*xq].get_bottom_ptr())
        router.put("bottom", routers[i + j*xq].get_bottom_xy());
    
    pexy << "PE" << rxy.str();
    router.put("local", pexy.str());

    add_communication(router, i, j);

    nodes.push_back(std::make_pair("", router));
}

void NoC::add_communication(boost::property_tree::ptree &router, unsigned int i, unsigned int j)
{
    bpt::ptree communication;
    bpt::ptree ports;

    communication.put("requests", pes[i + j*xq].get_rcvd());
    communication.put("message_count", routers[i + j*xq].get_msg_cnt());

    std::ostringstream oss;
    if(routers[i + j*xq].get_right_ptr()){
        oss << routers[i + j*xq].get_right_ratio() << "%";
        ports.put("right", oss.str());
        oss.str("");
    }
    if(routers[i + j*xq].get_left_ptr()){
        oss << routers[i + j*xq].get_left_ratio() << "%";
        ports.put("left", oss.str());
        oss.str("");
    }
    if(routers[i + j*xq].get_upper_ptr()){
        oss << routers[i + j*xq].get_upper_ratio() << "%";
        ports.put("top", oss.str());
        oss.str("");
    }
    if(routers[i + j*xq].get_bottom_ptr()){
        oss << routers[i + j*xq].get_bottom_ratio() << "%";
        ports.put("bottom", oss.str());
        oss.str("");
    }
    oss << routers[i + j*xq].get_local_ratio() << "%";
    ports.put("local", oss.str());

    communication.put_child("ports", ports);

    router.put_child("communication", communication);
}
