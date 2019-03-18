#include <ProcessingElement.h>

#include <iostream>

void PE::add_msg(message_t msg)
{
    out_msg.push(msg);
}

void PE::connect_router(Router &_router)
{
    router = &_router;
}

void PE::run()
{
    while(true){

        if(out_msg.size()){
            if(out_msg.front().dst != x) { // if not addressed to this PE
                NoC::writing(out_msg.front());
                router->write(out_msg.front()); // Writes to router
            } else   // Can happen if message from PE itself
                NoC::received(out_msg.front()); // Writes to Cli
            
            out_msg.pop();    // Removes from PE
        }

        if(router->available()){
            message_t in_msg = router->read();
            NoC::received(in_msg);     // Writes to Cli
        }
        
        if(NoC::finish())
            break;
    }
}
