#include "reactor.hpp"
#include <iostream>
#include <unistd.h>

Reactor::Reactor() : max_fd(0), running(false) {
    FD_ZERO(&master_set);
}

void* Reactor::startReactor() {
    running = true;
    while (running) {
        fd_set read_fds = master_set;
        int activity = select(max_fd + 1, &read_fds, nullptr, nullptr, nullptr);

        if (activity < 0) {
            perror("select");
            return nullptr;
        }

        for (int fd = 0; fd <= max_fd; ++fd) {
            if (FD_ISSET(fd, &read_fds)) {
                auto it = handlers.find(fd);
                if (it != handlers.end()) {
                    it->second(fd);
                }
            }
        }
    }
    return nullptr;
}

int Reactor::addFdToReactor(int fd, reactorFunc func) {
    if (handlers.find(fd) != handlers.end()) {
        return -1; // fd already exists
    }
    handlers[fd] = func;
    FD_SET(fd, &master_set);
    if (fd > max_fd) {
        max_fd = fd;
    }
    return 0;
}

int Reactor::removeFdFromReactor(int fd) {
    if (handlers.find(fd) == handlers.end()) {
        return -1; // fd does not exist
    }
    handlers.erase(fd);
    FD_CLR(fd, &master_set);
    if (fd == max_fd) {
        while (max_fd > 0 && !FD_ISSET(max_fd, &master_set)) {
            --max_fd;
        }
    }
    return 0;
}

int Reactor::stopReactor() {
    running = false;
    return 0;
}
