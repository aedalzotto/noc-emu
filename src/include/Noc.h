#ifndef _NOC_H_
#define _NOC_H_

#include <Router.h>
#include <ProcessingElement.h>

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>

#include <boost/property_tree/ptree.hpp>

struct message_t {
    unsigned int src_x;
    unsigned int src_y;
    unsigned int dst_x;
    unsigned int dst_y;
    std::string msg;
    unsigned int end;
};

class PE;
class Router;

enum class Type {
    RING,
    MESH
};

enum class Outfmt {
    JSON,
    XML
};

class NoC {
public:
    static int build_noc(int _xq);
    static int build_noc(int _xq, int _yq);
    static int inject_to_pe(std::vector<message_t> messages);
    static int create_threads();

    static void received(message_t msg);
    static void writing(message_t msg);
    static void sending(message_t msg, unsigned int src, unsigned int dst);
    static void sending(message_t msg, unsigned int src_x, unsigned int src_y,  unsigned int dst_x, unsigned int dst_y);
    static void reading(message_t msg);

    static bool finish();
    static void wait_simulation();

    static unsigned int get_pes_size();
    static unsigned int get_pe_rcvd(unsigned int i);
    static unsigned int get_pe_x(unsigned int i);
    static unsigned int get_pe_y(unsigned int i);

    static unsigned int get_routers_size();
    static unsigned int get_router_x(unsigned int i);
    static unsigned int get_router_y(unsigned int i);
    static unsigned int get_router_msg_cnt(unsigned int i);
    static unsigned int get_router_local_ratio(unsigned int i);
    static unsigned int get_router_left_ratio(unsigned int i);
    static unsigned int get_router_right_ratio(unsigned int i);
    static unsigned int get_router_upper_ratio(unsigned int i);
    static unsigned int get_router_bottom_ratio(unsigned int i);

    static void report(Outfmt outfmt, std::string fname);

private:
    static Type type;
    static int xq;
    static int yq;
    
    static int qtd;
    static std::vector<Router> routers;
    static std::vector<PE> pes;
    static std::vector<std::thread> threads;

    static int rcvd;
    static std::queue<std::string> prints;

    static std::mutex rcvd_mtx;
    static std::mutex print_mtx;

    static void add_noc(boost::property_tree::ptree &project);
    static void add_nodes(boost::property_tree::ptree &noc);
    static void add_router(boost::property_tree::ptree &nodes, unsigned int i, unsigned int j);

    static void add_communication(boost::property_tree::ptree &project);
    static void add_volume(boost::property_tree::ptree &volume, unsigned int i, unsigned int j);

    static void add_execution(boost::property_tree::ptree &project);
    static void add_processor(boost::property_tree::ptree &exec, unsigned int i, unsigned int j);
};

#endif /* _NOC_H_ */