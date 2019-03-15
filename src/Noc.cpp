#include <Noc.h>

#include <sstream>

int NoC::xq = 0;
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

int NoC::inject_to_pe(std::vector<message_t> messages)
{
    for(int i = 0; i < messages.size(); i++)
        pes[messages[i].src].add_msg(messages[i]);
    
    qtd = messages.size();

    return qtd;
}

int NoC::create_threads()
{
    for(int i = 0; i < xq; i++){
        threads.push_back(std::thread(&PE::run, &pes[i]));
        threads.push_back(std::thread(&Router::run, &routers[i]));
    }

    return xq*2;
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
    oss << "PE" << msg.dst << " recebeu de PE" << msg.src << ": " << msg.msg << ". E: " << msg.end;

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
    oss << "PE" << msg.src << " enviando para NoC: " << msg.msg << ". Com destino PE" << msg.dst;

    std::lock_guard<std::mutex> lock_a(print_mtx);
    prints.push(oss.str());
}

void NoC::sending(message_t msg, int src, int dst)
{
    std::ostringstream oss;
    oss << "R" << src << " encaminhando para R" << dst << " mensagem: " << msg.msg << ". Com destino PE" << msg.dst;

    std::lock_guard<std::mutex> lock_a(print_mtx);
    prints.push(oss.str());
}

void NoC::reading(message_t msg)
{
    std::ostringstream oss;
    oss << "R" << msg.dst << " encaminhando mensagem para PE: " << msg.msg;

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

    for(int i = 0; i < threads.size(); i++){
        threads[i].join();
    }

    while(prints.size()){
        std::cout << prints.front() << std::endl;
        prints.pop();
    }

    std::cout << std::endl << "Simulação finalizada" << std::endl;

}