#include "tcpServer.h"
#include "../config/defineconfig.h"
#include "../tool/ColorParse.h"
#include <arpa/inet.h>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <sys/select.h>
#include <unistd.h>

tcpServer::tcpServer(int port)
    : port(port)
    , server_fd(-1)
    , client_fd(-1)
    , connection_open(false) {
    // create server socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        handleError("socket creation");
        exit(EXIT_FAILURE);
    }

    // set server address
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port        = htons(port);
}

tcpServer::~tcpServer() {
    closeConnection(); // make sure the connection is closed
    if (server_fd != -1) {
        close(server_fd);
    }
}

bool tcpServer::handleError(const char *context) {
    std::cerr << termColor("red") << "Error in " << context << ": " << strerror(errno) << termColor("nocolor")
              << std::endl;
    return false;
}

bool tcpServer::bindServer() {
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        return handleError("setsockopt");
    }

    if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        return handleError("bind");
    }
    return true;
}

bool tcpServer::acceptClient(int timeout_ms) {
    if (listen(server_fd, 1) < 0) {
        return handleError("listen");
    }

    fd_set         set;
    struct timeval timeout;
    FD_ZERO(&set);
    FD_SET(server_fd, &set);

    timeout.tv_sec  = timeout_ms / 1000;
    timeout.tv_usec = (timeout_ms % 1000) * 1000;

    int rv          = select(server_fd + 1, &set, nullptr, nullptr, &timeout);

    if (rv == -1) {
        return handleError("select");
    } else if (rv == 0) {
        std::cerr << termColor("red") << "Timeout: No client connected within " << timeout_ms << " milliseconds."
                  << termColor("nocolor") << std::endl;
        return false;
    }

    client_fd = accept(server_fd, nullptr, nullptr);
    if (client_fd < 0) {
        return handleError("accept");
    }

    connection_open = true; // mark the connection is open
    return true;
}

bool tcpServer::start() {
    if (!bindServer()) {
        return false;
    }
    return true;
}

bool tcpServer::sendVector(const std::vector<uint8_t> &data, int timeout_ms) {
    std::lock_guard<std::mutex> lock(conn_mutex); // protect the connection resource

    if (!connection_open) {
        std::cerr << termColor("red") << "No open connection. Call waitForConnection first." << termColor("nocolor")
                  << std::endl;
        return false;
    }

    size_t      dataSize = data.size();                                 // data size
    const char *buffer   = reinterpret_cast<const char *>(data.data()); // convert data to char buffer

    size_t bytesSent     = 0;

    while (bytesSent < dataSize) {
        size_t remainingBytes = dataSize - bytesSent;

        fd_set         set;
        struct timeval timeout;
        FD_ZERO(&set);
        FD_SET(client_fd, &set);

        timeout.tv_sec  = timeout_ms / 1000;
        timeout.tv_usec = (timeout_ms % 1000) * 1000;

        int rv          = select(client_fd + 1, nullptr, &set, nullptr, &timeout);

        if (rv == -1) {
            return handleError("select on send");
        } else if (rv == 0) {
            std::cerr << termColor("red") << "Timeout: Client did not ready for data transmission within "
                      << termColor("nocolor") << timeout_ms << " milliseconds." << std::endl;
            closeConnection(); // close the connection when timeout
            return false;
        }

        ssize_t sent = send(client_fd, buffer + bytesSent, remainingBytes, 0);
        if (sent == -1) {
            return handleError("send data");
        }

        bytesSent += sent;
    }
#ifdef _THREAD_TCPCLIENT_DEBUG_
    std::cout << termColor("green") << "Data sent successfully, total size: " << dataSize << " bytes."
              << termColor("nocolor") << std::endl;
#endif // _THREAD_TCPCLIENT_DEBUG_
    return true;
}

void tcpServer::closeConnection() {
    std::lock_guard<std::mutex> lock(conn_mutex); // make sure the connection is closed safely
    if (client_fd != -1) {
        close(client_fd);
        client_fd       = -1;
        connection_open = false;
#ifdef _THREAD_TCPCLIENT_DEBUG_
        std::cout << termColor("blue") << "Connection closed." << termColor("nocolor") << std::endl;
#endif // _THREAD_TCPCLIENT_DEBUG_
    }
}

bool tcpServer::waitForConnection(int timeout_ms) {
    return acceptClient(timeout_ms);
}