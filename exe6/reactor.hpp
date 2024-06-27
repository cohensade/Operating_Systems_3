#ifndef REACTOR_HPP
#define REACTOR_HPP

#include <functional>
#include <unordered_map>
#include <sys/select.h>

typedef std::function<void(int)> reactorFunc;

class Reactor {
public:
    Reactor();
    void* startReactor();
    int addFdToReactor(int fd, reactorFunc func);
    int removeFdFromReactor(int fd);
    int stopReactor();

private:
    std::unordered_map<int, reactorFunc> handlers;
    fd_set master_set;
    int max_fd;
    bool running;
};

#endif // REACTOR_HPP
