#ifndef _PROCESSING_ELEMENT_H_
#define _PROCESSING_ELEMENT_H_

#include <Noc.h>

#include <Router.h>

#include <queue>

struct message_t;
class Router;

class PE {
public:
    PE(unsigned int _x) : x(_x), y(0), rcvd(0), router(nullptr) { }
    PE(unsigned int _x, unsigned int _y) : x(_x), y(_y), rcvd(0), router(nullptr) { }
    void add_msg(message_t msg);
    void connect_router(Router &_router);
    
    void run();

    unsigned int get_rcvd() { return rcvd; }
    unsigned int get_x() { return x; }
    unsigned int get_y() { return y; }

private:
    unsigned int x;
    unsigned int y;

    unsigned int rcvd;

    std::queue<message_t> out_msg;

    Router *router;

};


#endif /* _PROCESSING_ELEMENT_H_ */