#include <bits/stdc++.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <fcntl.h>
#include <sys/poll.h>
#include "SignalHandler.hpp"

using namespace std::literals;

class TcpServer final {

private:

    template<typename T>
    using unique_ptr = std::unique_ptr<T>;

    template<typename T>
    using atomic = std::atomic<T>;

    template<typename T>
    using set = std::set<T>;

    using runtime_error = std::runtime_error;
    using mutex = std::mutex;

    int _listenfd;
    sockaddr_in _server;

    atomic<bool> _killed;
    atomic<bool> _listening;
    
    mutex _connectionsLock;

    set<int> _connections;
    atomic<int> _connectionSequenceId;

    static void ResetError() {
        errno = 0;
    }

    static const char *GetErrorMessage() {
        return errno ? strerror(errno) : "UNKNOWN ERROR";
    }

    void CreateSocket() {
        LOG(INFO, TM("TRY CREATE SOCKET"));
        ResetError();
        _listenfd = socket(AF_INET, SOCK_STREAM, 0);
        if (_listenfd < 0) {
            const auto message = GetErrorMessage();
            LOG(CRITICAL, TM("FATAL ERROR: %hs"), message);
            throw runtime_error("FATAL ERROR: "s + message);
        }
        int _true = 1;
        if (setsockopt(_listenfd, SOL_SOCKET, SO_REUSEADDR, &_true, sizeof(int)) < 0) {
            const auto message = GetErrorMessage();
            LOG(CRITICAL, TM("FATAL ERROR: %hs"), message);
            throw runtime_error("FATAL ERROR: "s + message);
        }
        LOG(INFO, TM("SOCKET CREATED SUCCESSFULLY ON FD [%d]"), _listenfd);
    }

    void BindSocket(int port) {
        memset(&_server, 0, sizeof(_server));
        _server.sin_family = AF_INET;
        _server.sin_addr.s_addr = htonl(INADDR_ANY);
        _server.sin_port = htons(port);
        LOG(INFO, TM("TRY BIND FD [%d] TO PORT [%d]"), _listenfd, port);
        ResetError();
        if (bind(_listenfd, (struct sockaddr *) &_server, sizeof(_server)) < 0) {
            const auto message = GetErrorMessage();
            LOG(CRITICAL, TM("FATAL ERROR: %hs"), message);
            throw runtime_error("FATAL ERROR: "s + message);
        }
        LOG(INFO, TM("BIND FD [%d] TO PORT [%d] SUCCESSFULLY"), _listenfd, port);
    }

    void ListenSocket(int connections) const {
        ResetError();
        LOG(INFO, TM("START LISTEN ON FD [%d]"), _listenfd);
//        fcntl(_listenfd, F_SETFL, O_NONBLOCK);
        if (listen(_listenfd, connections) < 0) {
            const auto message = GetErrorMessage();
            LOG(CRITICAL, TM("FATAL ERROR: %hs"), message);
            throw runtime_error("FATAL ERROR: "s + message);
        }
    }
    
    void CloseConnection(int fd) {
        LOG(INFO, TM("CLOSE CONNECTION ON FD [%d]"), fd);
        shutdown(fd, SHUT_RDWR);
        close(fd);
        _connections.erase(fd);
    }

    bool CheckConnection(int fd) {
        ResetError();
        auto x = recv(fd, NULL, 1, MSG_PEEK | MSG_DONTWAIT);
        return x;
    }

    bool AcceptConnection() {
        ResetError();
        int connfd = accept(_listenfd, nullptr, nullptr);
        if (connfd < 0) {
            int connfd = accept(_listenfd, nullptr, nullptr);
            if (errno == EAGAIN) {
                return false;
            }
            const auto message = GetErrorMessage();
            LOG(ERROR, TM("ACCEPTING ON FD [%d] FAILED: %hs"), _listenfd, message);
            return false;
        }

        LOG(DEBUG, TM("ON FD [%d] ACCEPTING SUCCESSFULLY: CONNECTION CREATED ON FD [%d]"), _listenfd, connfd);

        if (!_connectionsLock.try_lock()) {
            LOG(WARNING, TM("CONNECTIONS POOL ON FD [%d] IS LOCKED: CONNECTION ON FD [%d] CLOSED"), _listenfd, connfd);
            close(connfd);
            return false;
        }

        LOG(DEBUG, TM("TO FD [%d] ADDED CONNECTION #%d WITH FD [%d]"), _listenfd, ++_connectionSequenceId, connfd);

        _connections.insert(connfd);

        _connectionsLock.unlock();

        return true;
    }
    
    void PollOnce() {
        AcceptConnection();
        int temp[_connections.size()], s = 0;
        for (const auto &i : _connections) {
            temp[s++] = i;
        }
        for (const auto &i : temp) {
            if (!CheckConnection(i))
            {
                CloseConnection(i);
            }
        }
    }

public:

    TcpServer(int port) : _killed(false), _listening(false) {
        CreateSocket();
        BindSocket(port);
        SignalHandler::AttachHandler([this](int) {
            Kill();
        });
    }

    void SendToAll(const char *data) {
        for (const auto &i : _connections) {
            auto stopwatch = Stopwatch::Start();
            write(i, data, 32);
            stopwatch.Stop();
            LOG(TRACE, TM("-> %hs | ELAPSED: %ld"), data, stopwatch.Elapsed());
        }
    }

    void Kill() {
        if (atomic_exchange(&_killed, true)) {
            return;
        }
        LOG(INFO, TM("KILL TCP SERVER ON FD [%d]"), _listenfd);
        shutdown(_listenfd, SHUT_RDWR);
        close(_listenfd);
        for (const auto &i : _connections) {
            LOG(DEBUG, TM("CLOSE CONNECTION ON FD [%d]"), i);
            shutdown(i, SHUT_RDWR);
            close(i);
        }
    }

    void Listen(int connections) {

        if (atomic_exchange(&_listening, true)) {
            throw runtime_error("ALREADY LISTENING"); 
        }
        if (_killed) {
            throw runtime_error("ALREADY KILLED");
        }
        
        ListenSocket(connections);

        while (!_killed) {
            PollOnce();
        }
        
        _listening = false;

    }

    ~TcpServer() {
        Kill();
    }

};
