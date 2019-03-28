#ifndef _ROUTER_H_
#define _ROUTER_H_

#include <Noc.h>

#include <mutex>
#include <queue>

struct message_t;
enum class Type;

class Router {
public:
    Router(unsigned int _x);
    Router(unsigned int _x, unsigned int _y);
    Router(Router&& r);

    void connect_right(Router &_right);
    void connect_left(Router &_left);
    void connect_upper(Router &_upper);
    void connect_bottom(Router &_bottom);

    unsigned int get_x() { return x; }
    unsigned int get_y() { return y; }
    
    void write(message_t msg);
    int available();
    message_t read();

    void send2right(message_t msg);
    void send2left(message_t msg);
    void send2upper(message_t msg);
    void send2bottom(message_t msg);

    void run();

private:
    unsigned int x;
    unsigned int y;
    Type type;

    std::mutex local_w_mtx;
    std::mutex local_r_mtx;

    std::mutex left_mtx;
    std::mutex right_mtx;
    std::mutex upper_mtx;
    std::mutex bottom_mtx;

    std::queue<message_t> local_w_buffer;
    std::queue<message_t> local_r_buffer;

    std::queue<message_t> left_buf;
    std::queue<message_t> right_buf;
    std::queue<message_t> upper_buf;
    std::queue<message_t> bottom_buf;

    Router *right;
    Router *left;
    Router *upper;
    Router *bottom;

    void route_from_pe();
    void route_from_left();
    void route_from_right();
    void route_from_upper();
    void route_from_bottom();

    void send2pe(message_t msg);
};

#endif /* _ROUTER_H_ */