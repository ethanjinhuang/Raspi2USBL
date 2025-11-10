/***
 * @Author: Jin Huang @ jin.huang@zju.edu.cn
 * @Date: 2024-09-08 15:41:33
 * @LastEditors: Jin's Macbook jin.huang@zju.edu.cn
 * @LastEditTime: 2025-11-05 20:40:12
 * @FilePath: /Raspi2USBL/dataio/tcpServer.h
 * @Description:
 * @
 * @Copyright (c) 2025 by Jin Huang @ jin.huang@zju.edu.cn, All Rights Reserved.
 */

#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <mutex>
#include <netinet/in.h>
#include <vector>

class tcpServer {
public:
    tcpServer(int port);
    ~tcpServer();

    bool start(); // start server
    // bool sendVector(const std::vector<double>& data, int timeout_ms = 5000);  // send data, timeout precision is
    // millisecond
    bool sendVector(const std::vector<uint8_t> &data, int timeout_ms);
    bool waitForConnection(int timeout_ms); // encapsulate the call logic of acceptClient
    void closeConnection();                 // close connection manually
    // get client file descriptor
    int  getClientFd() const {
        return client_fd;
    }

private:
    int port;
    int server_fd; // server file descriptor
    int client_fd; // client connection file descriptor

    struct sockaddr_in server_addr;
    std::mutex         conn_mutex;                       // mutex lock to protect connection resource
    bool               bindServer();                     // bind server
    bool               acceptClient(int timeout_ms);     // private, function to wait for connection
    bool               handleError(const char *context); // print error message and return false
    bool               connection_open;                  // used to mark whether the connection is open
};

#endif // TCPSERVER_H
