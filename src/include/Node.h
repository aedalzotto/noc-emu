#ifndef _NODE_H_
#define _NODE_H_

class Node {
public:
    Node(int _x) : x(_x) {}
    virtual void run() = 0;

protected:
    int x;

    static void wait_start();

private:

};

#endif /* _NODE_H_ */