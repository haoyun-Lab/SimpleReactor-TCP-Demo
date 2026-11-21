#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
int main() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv{};
    serv.sin_family = AF_INET;
    serv.sin_port = htons(8888);
    inet_pton(AF_INET, "127.0.0.1", &serv.sin_addr);
    connect(fd, (sockaddr*)&serv, sizeof(serv));
    std::string msg;
    while(true) {
        std::getline(std::cin, msg);
        msg += "\n";
        write(fd, msg.c_str(), msg.size());
        char buf[1024]{};
        read(fd, buf, 1023);
        std::cout << "recv:" << buf << std::endl;
    }
    close(fd);
}
