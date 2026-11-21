#ifndef REACTOR_H
#define REACTOR_H
#include <iostream>
#include <vector>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <chrono>

#define MAX_EVENTS 1024
#define BUF_SIZE 1024
#define IDLE_TIMEOUT 10 // 空闲超时10s，超时自动断开，解决fd泄漏

struct ConnInfo {
    int fd;
    std::chrono::steady_clock::time_point last_active;
    char buf[BUF_SIZE];
    int buf_len;
};

class Reactor {
public:
    Reactor();
    ~Reactor();
    bool initServer(int port);
    void eventLoop();
private:
    int epoll_fd;
    int listen_fd;
    std::vector<ConnInfo> conns;

    void setNonBlock(int fd);
    void acceptConn();
    void handleRead(int conn_fd);
    void checkTimeout();
};
#endif
