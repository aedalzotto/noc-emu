#ifndef _NOC_H_
#define _NOC_H_

#include <Router.h>
#include <ProcessingElement.h>

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>

struct message_t {
    unsigned int src;
    unsigned int dst;
    std::string msg;
    unsigned int end;
};

class PE;
class Router;

class NoC {
public:
    static int build_noc(int _xq);
    static int inject_to_pe(std::vector<message_t> messages);
    static int create_threads();

    static void received(message_t msg);
    static void writing(message_t msg);
    static void sending(message_t msg, int src, int dst);
    static void reading(message_t msg);

    static bool finish();
    static void wait_simulation();

private:
    static int xq;
    static int qtd;
    static std::vector<Router> routers;
    static std::vector<PE> pes;
    static std::vector<std::thread> threads;

    static int rcvd;
    static std::queue<std::string> prints;

    static std::mutex rcvd_mtx;
    static std::mutex print_mtx;
};

#endif /* _NOC_H_ */