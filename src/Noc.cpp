#include <Noc.h>

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