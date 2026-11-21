#include "reactor.h"

Reactor::Reactor() : epoll_fd(-1), listen_fd(-1) {}
Reactor::~Reactor() {
    close(epoll_fd);
    close(listen_fd);
}

void Reactor::setNonBlock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

bool Reactor::initServer(int port) {
    epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) return false;

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) return false;
    setNonBlock(listen_fd);

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    bind(listen_fd, (sockaddr*)&addr, sizeof(addr));
    listen(listen_fd, 5);

    epoll_event ev{};
    ev.data.fd = listen_fd;
    ev.events = EPOLLIN;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev);
    return true;
}

void Reactor::acceptConn() {
    sockaddr_in client_addr{};
    socklen_t len = sizeof(client_addr);
    int fd = accept4(listen_fd, (sockaddr*)&client_addr, &len, SOCK_NONBLOCK);
    if (fd <= 0) return;

    ConnInfo new_conn{};
    new_conn.fd = fd;
    new_conn.last_active = std::chrono::steady_clock::now();
    new_conn.buf_len = 0;
    conns.push_back(new_conn);

    epoll_event ev{};
    ev.data.fd = fd;
    ev.events = EPOLLIN;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev);
}

void Reactor::handleRead(int conn_fd) {
    ConnInfo* conn = nullptr;
    for(auto &c : conns) {
        if(c.fd == conn_fd) {
            conn = &c;
            break;
        }
    }
    if(!conn) return;

    conn->last_active = std::chrono::steady_clock::now();
    char tmp[BUF_SIZE];
    int n = read(conn_fd, tmp, BUF_SIZE - 1);
    if(n <= 0) {
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, conn_fd, nullptr);
        close(conn_fd);
        for(auto it = conns.begin(); it != conns.end(); ++it) {
            if(it->fd == conn_fd) {
                conns.erase(it);
                break;
            }
        }
        return;
    }
    memcpy(conn->buf + conn->buf_len, tmp, n);
    conn->buf_len += n;
    while(true) {
        char* p = (char*)memchr(conn->buf, '\n', conn->buf_len);
        if(!p) break;
        int msg_len = p - conn->buf + 1;
        write(conn_fd, conn->buf, msg_len);
        int remain = conn->buf_len - msg_len;
        memmove(conn->buf, conn->buf + msg_len, remain);
        conn->buf_len = remain;
    }
}

void Reactor::checkTimeout() {
    auto now = std::chrono::steady_clock::now();
    for(auto it = conns.begin(); it != conns.end(); ) {
        auto diff = std::chrono::duration_cast<std::chrono::seconds>(now - it->last_active).count();
        if(diff > IDLE_TIMEOUT) {
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, it->fd, nullptr);
            close(it->fd);
            it = conns.erase(it);
        } else {
            ++it;
        }
    }
}

void Reactor::eventLoop() {
    epoll_event events[MAX_EVENTS];
    while(true) {
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, 1000);
        for(int i = 0; i < n; ++i) {
            int fd = events[i].data.fd;
            if(fd == listen_fd) {
                acceptConn();
            } else if(events[i].events & EPOLLIN) {
                handleRead(fd);
            }
        }
        checkTimeout();
    }
}
