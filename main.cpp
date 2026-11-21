#include "reactor.h"
int main() {
    Reactor r;
    if(!r.initServer(8888)) {
        std::cerr << "server init failed" << std::endl;
        return -1;
    }
    std::cout << "Reactor server start on port 8888" << std::endl;
    r.eventLoop();
    return 0;
}
