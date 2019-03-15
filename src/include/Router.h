#ifndef _ROUTER_H_
#define _ROUTER_H_

#include <Noc.h>
#include <Node.h>

#include <mutex>
#include <queue>

struct message_t;

class Router : public Node {
public:
    Router(int _x) : Node(_x), right(nullptr) { }
    Router(Router&& r);

    void connect_right(Router &_right);

    int get_x() { return x; }
    
    void write(message_t msg);
    int available();
    message_t read();
    void send(message_t msg);

    void run();

private:
    std::mutex local_w_mtx;
    std::mutex local_r_mtx;
    std::mutex left_mtx;

    std::queue<message_t> local_w_buffer;
    std::queue<message_t> local_r_buffer;
    std::queue<message_t> left_buf;

    Router *right;
};

#endif /* _ROUTER_H_ */