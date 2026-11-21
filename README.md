# SimpleReactor-TCP-Demo
A lightweight single-Reactor TCP server implemented by C++ & Linux epoll, for learning IO multiplexing and event-driven programming.
## ✨ Features
1. Single Reactor event loop, manage listen fd and connection fd
2. Connection idle timeout auto-recycle, prevent file descriptor leak
3. Simple packet boundary parsing, handle half-packet / illegal data
4. Echo service, receive message and send back to client
## 🛠 Tech Stack
- Language: C++11
- OS: Linux
- API: epoll
## 📝 Build & Run
```bash
make
# start server
./server
# start test client
./client
