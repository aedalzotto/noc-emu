#ifndef _PROCESSING_ELEMENT_H_
#define _PROCESSING_ELEMENT_H_

#include <Noc.h>

#include <Node.h>
#include <Router.h>

#include <queue>

struct message_t;
class Router;

class PE : public Node {
public:
    PE(int _x) : Node(_x), router(nullptr) { }
    void add_msg(message_t msg);
    void connect_router(Router &_router);
    
    void run();

private:
    std::queue<message_t> out_msg;

    Router *router;

};


#endif /* _PROCESSING_ELEMENT_H_ */