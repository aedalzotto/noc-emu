#include <Router.h>

// https://stackoverflow.com/questions/29986208/how-should-i-deal-with-mutexes-in-movable-types-in-c
// https://stackoverflow.com/questions/4086800/move-constructor-on-derived-object
Router::Router(Router&& r) : Node(std::move(r))
{
    local_r_mtx.lock();
    local_w_mtx.lock();
    left_mtx.lock();

    local_w_buffer = std::move(r.local_w_buffer);
    local_r_buffer = std::move(r.local_r_buffer);
    left_buf = std::move(r.left_buf);
    right = std::move(r.right);


    local_r_mtx.unlock();
    local_w_mtx.unlock();
    left_mtx.unlock();
}

void Router::connect_right(Router &_right)
{
    right = &_right;
}

void Router::write(message_t msg)
{
    local_w_mtx.lock();

    local_w_buffer.push(msg);

    local_w_mtx.unlock();
}

int Router::available()
{
    local_r_mtx.lock();

    int ret = local_r_buffer.size();
    
    local_r_mtx.unlock();

    return ret;
}

message_t Router::read()
{
    local_r_mtx.lock();

    message_t ret = local_r_buffer.front();
    local_r_buffer.pop();

    local_r_mtx.unlock();

    return ret;
}

void Router::send(message_t msg)
{
    left_mtx.lock();

    left_buf.push(msg);

    left_mtx.unlock();
}

void Router::run()
{

    while(true) {

        local_w_mtx.lock();
        if(local_w_buffer.size()){
            NoC::sending(local_w_buffer.front(), x, right->get_x());
            right->send(local_w_buffer.front());
            local_w_buffer.pop();
        }
        local_w_mtx.unlock();

        left_mtx.lock();
        if(left_buf.size()){
            if(left_buf.front().dst != x){
                NoC::sending(left_buf.front(), x, right->get_x());
                right->send(left_buf.front());
            } else {
                local_r_mtx.lock();
                NoC::reading(left_buf.front());
                local_r_buffer.push(left_buf.front());
                local_r_mtx.unlock();
            }

            left_buf.pop();
        }
        left_mtx.unlock();

        if(NoC::finish())
            break;
    }

}