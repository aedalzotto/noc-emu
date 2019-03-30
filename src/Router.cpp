#include <Router.h>

#include <sstream>

Router::Router(unsigned int _x) : x(_x), y(0), type(Type::RING), msg_cnt(0), local_cnt(0),
                                left_cnt(0), right_cnt(0), upper_cnt(0), bottom_cnt(0),
                                right(nullptr), left(nullptr), upper(nullptr), bottom(nullptr)
{

}

Router::Router(unsigned int _x, unsigned int _y) : x(_x), y(_y), type(Type::MESH), msg_cnt(0), local_cnt(0),
                                                left_cnt(0), right_cnt(0), upper_cnt(0), bottom_cnt(0),
                                                right(nullptr), left(nullptr), upper(nullptr), bottom(nullptr)
{

}

// https://stackoverflow.com/questions/29986208/how-should-i-deal-with-mutexes-in-movable-types-in-c
// https://stackoverflow.com/questions/4086800/move-constructor-on-derived-object
Router::Router(Router&& r)
{
    std::lock_guard<std::mutex> locka(left_mtx);
    std::lock_guard<std::mutex> lockb(right_mtx);
    std::lock_guard<std::mutex> lockc(upper_mtx);
    std::lock_guard<std::mutex> lockd(bottom_mtx);
    std::lock_guard<std::mutex> locke(local_w_mtx);
    std::lock_guard<std::mutex> lockf(local_r_mtx);

    msg_cnt = std::move(r.msg_cnt);
    local_cnt = std::move(r.local_cnt);
    left_cnt = std::move(r.left_cnt);
    right_cnt = std::move(r.right_cnt);
    upper_cnt = std::move(r.upper_cnt);
    bottom_cnt = std::move(r.bottom_cnt);

    local_w_buffer = std::move(r.local_w_buffer);
    local_r_buffer = std::move(r.local_r_buffer);
    left_buf = std::move(r.left_buf);
    right_buf = std::move(r.right_buf);
    upper_buf = std::move(r.upper_buf);
    bottom_buf = std::move(r.bottom_buf);
    right = std::move(r.right);
    left = std::move(r.left);
    upper = std::move(r.upper);
    bottom = std::move(r.bottom);

    x = std::move(r.x);
    y = std::move(r.y);
    type = std::move(r.type);
}

void Router::connect_right(Router &_right)
{
    right = &_right;
}

void Router::connect_left(Router &_left)
{
    left = &_left;
}

void Router::connect_upper(Router &_upper)
{
    upper = &_upper;
}

void Router::connect_bottom(Router &_bottom)
{
    bottom = &_bottom;
}

void Router::write(message_t msg)
{
    std::lock_guard<std::mutex> lock(local_w_mtx);

    local_w_buffer.push(msg);
}

int Router::available()
{
    std::lock_guard<std::mutex> lock(local_r_mtx);

    return local_r_buffer.size();
}

message_t Router::read()
{
    std::lock_guard<std::mutex> lock(local_r_mtx);

    message_t ret = local_r_buffer.front();
    local_r_buffer.pop();

    return ret;
}

void Router::run()
{

    while(true) {

        route_from_pe();
        route_from_left();
        route_from_right();
        route_from_upper();
        route_from_bottom();

        if(NoC::finish())
            break;
    }

}

void Router::route_from_pe()
{
    std::lock_guard<std::mutex> lock(local_w_mtx);

    if(local_w_buffer.size()){
        msg_cnt++;
        local_cnt++;
        
        if(local_w_buffer.front().dst_x != x){
            if(local_w_buffer.front().dst_x > x || type == Type::RING){
                if(type == Type::RING){
                    NoC::sending(local_w_buffer.front(), x, right->get_x());
                } else {
                    NoC::sending(local_w_buffer.front(), x, y, right->get_x(), right->get_y());
                }

                right_cnt++;
                right->send2right(local_w_buffer.front());
                
            } else {
                NoC::sending(local_w_buffer.front(), x, y, left->get_x(), left->get_y());

                left_cnt++;
                left->send2left(local_w_buffer.front());
            }
        } else {
            if(local_w_buffer.front().dst_y > y){
                NoC::sending(local_w_buffer.front(), x, y, upper->get_x(), upper->get_y());
                upper_cnt++;
                upper->send2upper(local_w_buffer.front());
                
            } else {
                NoC::sending(local_w_buffer.front(), x, y, bottom->get_x(), bottom->get_y());
                bottom_cnt++;
                bottom->send2bottom(local_w_buffer.front());
                
            }
        }
        
        local_w_buffer.pop();
    }
}

void Router::route_from_left()
{
    std::lock_guard<std::mutex> lock(left_mtx);

    if(left_buf.size()){
        msg_cnt++;
        left_cnt++;

        if(left_buf.front().dst_x != x){
            if(type == Type::RING)
                NoC::sending(left_buf.front(), x, right->get_x());
            else
                NoC::sending(left_buf.front(), x, y, right->get_x(), right->get_y());
		   	
            right_cnt++;
            right->send2right(left_buf.front());
        
        } else if(left_buf.front().dst_y != y){
            if(left_buf.front().dst_y > y){
                NoC::sending(left_buf.front(), x, y, upper->get_x(), upper->get_y());

                upper_cnt++;
                upper->send2upper(left_buf.front());
            } else {
                NoC::sending(left_buf.front(), x, y, bottom->get_x(), bottom->get_y());

                bottom_cnt++;
                bottom->send2bottom(left_buf.front());
            }
        } else {
            NoC::reading(left_buf.front());

            local_cnt++;
            send2pe(left_buf.front());
        }

        left_buf.pop();
    }
}

void Router::route_from_right()
{
    std::lock_guard<std::mutex> lock(right_mtx);

    if(right_buf.size()){
        msg_cnt++;
        right_cnt++;

        if(right_buf.front().dst_x != x){
            NoC::sending(right_buf.front(), x, y, left->get_x(), left->get_y());

            left_cnt++;
            left->send2left(right_buf.front());
        } else if(right_buf.front().dst_y != y){
            if(right_buf.front().dst_y > y){
                NoC::sending(right_buf.front(), x, y, upper->get_x(), upper->get_y());

                upper_cnt++;
                upper->send2upper(right_buf.front());
            } else {
                NoC::sending(right_buf.front(), x, y, bottom->get_x(), bottom->get_y());

                bottom_cnt++;
                bottom->send2bottom(right_buf.front());
            }
        } else {
            NoC::reading(right_buf.front());

            local_cnt++;
            send2pe(right_buf.front());
        }

        right_buf.pop();
    }
}

void Router::route_from_upper()
{
    std::lock_guard<std::mutex> lock(upper_mtx);

    if(upper_buf.size()){
        msg_cnt++;
        upper_cnt++;

        if(upper_buf.front().dst_x != x){
            if(upper_buf.front().dst_x > x){
                NoC::sending(upper_buf.front(), x, y, right->get_x(), right->get_y());

                right_cnt++;
                right->send2right(upper_buf.front());
            } else {
                NoC::sending(upper_buf.front(), x, y, left->get_x(), left->get_y());

                left_cnt++;
                left->send2left(upper_buf.front());
            }            
        
        } else if(upper_buf.front().dst_y != y){
            NoC::sending(upper_buf.front(), x, y, bottom->get_x(), bottom->get_y());

            bottom_cnt++;
            bottom->send2bottom(upper_buf.front());
        } else {
            NoC::reading(upper_buf.front());

            local_cnt++;
            send2pe(upper_buf.front());
        }

        upper_buf.pop();
    }
}

void Router::route_from_bottom()
{
    std::lock_guard<std::mutex> lock(bottom_mtx);

    if(bottom_buf.size()){
        msg_cnt++;
        bottom_cnt++;

        if(bottom_buf.front().dst_x != x){
            if(bottom_buf.front().dst_x > x){
                NoC::sending(bottom_buf.front(), x, y, right->get_x(), right->get_y());

                right_cnt++;
                right->send2right(bottom_buf.front());
            } else {
                NoC::sending(bottom_buf.front(), x, y, left->get_x(), left->get_y());

                left_cnt++;
                left->send2left(bottom_buf.front());
            }            
        
        } else if(bottom_buf.front().dst_y != y){
            NoC::sending(bottom_buf.front(), x, y, upper->get_x(), upper->get_y());

            upper_cnt++;
            upper->send2upper(bottom_buf.front());
        } else {
            NoC::reading(bottom_buf.front());

            local_cnt++;
            send2pe(bottom_buf.front());
        }

        bottom_buf.pop();
    }
}

void Router::send2right(message_t msg)
{
    std::lock_guard<std::mutex> lock(left_mtx);
    left_buf.push(msg);
}

void Router::send2left(message_t msg)
{
    std::lock_guard<std::mutex> lock(right_mtx);
    right_buf.push(msg);
}

void Router::send2upper(message_t msg)
{
    std::lock_guard<std::mutex> lock(bottom_mtx);
    bottom_buf.push(msg);
}

void Router::send2bottom(message_t msg)
{
    std::lock_guard<std::mutex> lock(upper_mtx);
    upper_buf.push(msg);
}

void Router::send2pe(message_t msg)
{
    std::lock_guard<std::mutex> lock(local_r_mtx);
    
    local_r_buffer.push(msg);    
}

std::string Router::get_left_xy()
{
    if(type == Type::MESH)
        return make_xy(left->get_x(), left->get_y());
    else
        return std::to_string(left->get_x());
}

std::string Router::get_right_xy()
{
    if(type == Type::MESH)
        return make_xy(right->get_x(), right->get_y());
    else
        return std::to_string(right->get_x());
}

std::string Router::get_upper_xy()
{
    if(type == Type::MESH)
        return make_xy(upper->get_x(), upper->get_y());
    else
        return std::to_string(upper->get_x());
}

std::string Router::get_bottom_xy()
{
    if(type == Type::MESH)
        return make_xy(bottom->get_x(), bottom->get_y());
    else
        return std::to_string(bottom->get_x());
}

std::string Router::make_xy(unsigned int x, unsigned int y)
{
    std::ostringstream oss;
    oss << x << y;
    return oss.str();
}