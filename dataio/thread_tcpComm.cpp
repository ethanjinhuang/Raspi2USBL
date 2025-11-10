/***
 * @Author: Jin Huang jin.huang@zju.edu.cn
 * @Date: 2024-09-08 16:55:17
 * @LastEditors: Jin Huang jin.huang@zju.edu.cn
 * @LastEditTime: 2024-09-14 22:33:33
 * @FilePath: /RaspiUSBL/dataio/thread_tcpComm.cpp
 * @Description:
 * @
 * @Copyright (c) 2024 by JinHuang  (jin.huang@zju.edu.cn) / Zhejiang University, All Rights Reserved.
 */
#include "thread_tcpComm.h"
#include "../config/defineconfig.h"
#include "../tool/ColorParse.h"
#include <chrono>
#include <cstring>
#include <iostream>
#include <thread>
#include <zlib.h>

ThreadTcpCommunication::ThreadTcpCommunication(SystemInfo &systemInfo, tcpServer &server)
    : systemInfo_(systemInfo)
    , server_(server)
    , stopFlag_(false) {
    init();
}

ThreadTcpCommunication::~ThreadTcpCommunication() {
    stopThread();
    if (sendThread_.joinable()) {
        sendThread_.join();
    }
}

void ThreadTcpCommunication::init() {
    stopFlag_       = false;
    connectTimeout_ = systemInfo_.tcpInfo.connectTimeout;
    sendTimeout_    = systemInfo_.tcpInfo.sendTimeout;
}

void ThreadTcpCommunication::setDataQueue(sfq::Safe_Queue<ChannelSignalVector> *dataQueue) {
    if (signalQueue_ != nullptr) {
        std::cerr << termColor("red") << "Data queue is setted." << termColor("nocolor") << std::endl;
        return;
    }
    signalQueue_ = dataQueue;
}

void ThreadTcpCommunication::startSending() {
    stopFlag_   = false;
    sendThread_ = std::thread(&ThreadTcpCommunication::sendData, this);
}

bool ThreadTcpCommunication::waitForHeartbeatResponse() {
    const int responseTimeout = 300; /// wait for response timeout in ms

    // use select to wait for response from client
    fd_set         set;
    struct timeval timeout;
    FD_ZERO(&set);
    FD_SET(server_.getClientFd(), &set); // use getter function to access client_fd

    timeout.tv_sec  = responseTimeout / 1000;
    timeout.tv_usec = (responseTimeout % 1000) * 1000;

    int rv =
        select(server_.getClientFd() + 1, &set, nullptr, nullptr, &timeout); // use getter function to access client_fd

    if (rv == -1) {
        std::cerr << termColor("red") << "Error while waiting for heartbeat response: " << strerror(errno)
                  << termColor("nocolor") << std::endl;
        return false;
    } else if (rv == 0) {
        std::cerr << termColor("red") << "Heartbeat response timed out." << termColor("nocolor") << std::endl;
        return false;
    }

    // data received, read and check if it is a heartbeat response packet
    char    buffer[1024];                                       // temporarily set to 1024 bytes buffer
    ssize_t bytesRead =
        recv(server_.getClientFd(), buffer, sizeof(buffer), 0); // use getter function to access client_fd

    if (bytesRead <= 0) {
        std::cerr << termColor("red") << "Error reading heartbeat response: " << strerror(errno) << termColor("nocolor")
                  << std::endl;
        return false;
    }

    TcpSignalType response = TcpSignalType::deserialize(buffer);

    // if signalType is 9, it is a heartbeat response
    if (response.signalType == 9) {
#ifdef _THREAD_TCPCLIENT_DEBUG_
        std::cout << termColor("green") << "Received heartbeat response." << termColor("nocolor") << std::endl;
#endif // _THREAD_TCPCLIENT_DEBUG_
        return true;
    }

    std::cerr << termColor("red") << "Received non-heartbeat response." << termColor("nocolor") << std::endl;
    return false;
}

void ThreadTcpCommunication::sendData() {
    int       missedHeartbeats    = 0;
    const int maxMissedHeartbeats = 3;
    const int heartbeatInterval   = 5000; // heartbeat interval, 5 seconds
    auto      lastHeartbeatTime   = std::chrono::steady_clock::now();
    int       currentQueueSize    = 0;

    std::vector<char>    buffer;     // declare buffer for serializing data
    std::vector<uint8_t> byteData;   // declare byte array for sending
    size_t               packetSize; // declare variable for storing packet size

    while (true) {
#ifdef _THREAD_TCPCLIENT_DEBUG_
        std::cout << termColor("yellow") << "Thread started, waiting for TCP connection..." << termColor("nocolor")
                  << std::endl;
#endif // _THREAD_TCPCLIENT_DEBUG_

        // wait for TCP connection
        while (true) {
            {
                std::lock_guard<std::mutex> lock(mtx_);
                if (stopFlag_) {
#ifdef _THREAD_TCPCLIENT_DEBUG_
                    std::cout << termColor("yellow") << "Thread stopped while waiting for connection."
                              << termColor("nocolor") << std::endl;
#endif // _THREAD_TCPCLIENT_DEBUG_
                    return;
                }
            }

            if (server_.waitForConnection(connectTimeout_)) {
#ifdef _THREAD_TCPCLIENT_DEBUG_
                std::cout << termColor("green") << "TCP connection established, starting data transmission."
                          << termColor("nocolor") << std::endl;
#endif // _THREAD_TCPCLIENT_DEBUG_
                break;
            }

            std::cerr << termColor("red") << "Failed to establish connection, retrying..." << termColor("nocolor")
                      << std::endl;
            // clear the data send queue
            currentQueueSize = signalQueue_->size();
            for (int i = 0; i < currentQueueSize; i++) {
                signalQueue_->try_pop(data_);
            }
        }

        // start data transmission
        while (true) {
            {
                std::lock_guard<std::mutex> lock(mtx_);
                if (stopFlag_) {
                    std::cout << termColor("yellow") << "Thread stopped before finishing all data."
                              << termColor("nocolor") << std::endl;
                    return;
                }
            }

            // calculate the time interval between current time and last heartbeat packet sent time
            auto now = std::chrono::steady_clock::now();
            auto timeSinceLastHeartbeat =
                std::chrono::duration_cast<std::chrono::milliseconds>(now - lastHeartbeatTime).count();

            if (timeSinceLastHeartbeat >= heartbeatInterval) {
                // send heartbeat packet
                TcpSignalType heartbeatPacket(false, 0, 0, {}, 0); // create heartbeat packet
                packetSize = heartbeatPacket.packetLength;         // calculate heartbeat packet size
                buffer.resize(packetSize);                         // resize buffer
                heartbeatPacket.serialize(buffer.data());

                // send heartbeat packet through tcpServer
                byteData.assign(buffer.begin(), buffer.end()); // convert buffer to byteData
                if (!server_.sendVector(byteData, sendTimeout_)) {
                    std::cerr << termColor("red") << "Failed to send heartbeat packet." << termColor("nocolor")
                              << std::endl;
                    missedHeartbeats++;
                } else {
                    // wait for heartbeat packet response
                    if (!waitForHeartbeatResponse()) {
                        missedHeartbeats++;
                        std::cerr << termColor("red") << "Missed heartbeat response (" << missedHeartbeats << "/"
                                  << maxMissedHeartbeats << ")" << termColor("nocolor") << std::endl;

                        if (missedHeartbeats >= maxMissedHeartbeats) {
                            std::cerr << termColor("red") << "Too many missed heartbeats. Closing connection..."
                                      << termColor("nocolor") << std::endl;
                            server_.closeConnection(); // close connection
                            break;                     // exit current sending loop, wait for connection again
                        }
                    } else {
                        missedHeartbeats = 0; // reset counter when heartbeat response received
                    }
                }

                lastHeartbeatTime = std::chrono::steady_clock::now(); // update last heartbeat sent time
            }

            // continue sending actual data if heartbeat successful
            data_ = signalQueue_->wait_and_pop();
            TcpSignalType signalPacket(true, data_.channelNum, data_.signalLength, data_.channels);

            packetSize = signalPacket.packetLength; // calculate data packet size
            buffer.resize(packetSize);              // resize buffer
            signalPacket.serialize(buffer.data());

            byteData.assign(buffer.begin(), buffer.end()); // convert buffer to byteData
            if (!server_.sendVector(byteData, sendTimeout_)) {
                std::cerr << termColor("red") << "Failed to send data packet." << termColor("nocolor") << std::endl;
                continue;
            }
#ifdef _THREAD_TCPCLIENT_DEBUG_
            // std::cout << termColor("blue") << "Sent data packet successfully." << termColor("nocolor") << std::endl;
#endif // _THREAD_TCPCLIENT_DEBUG_
       // std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // wait for 1 second
        }

// after connection closed, re-enter waiting for connection state
#ifdef _THREAD_TCPCLIENT_DEBUG_
        std::cout << termColor("yellow") << "Connection closed, waiting for new connection." << termColor("nocolor")
                  << std::endl;
#endif // _THREAD_TCPCLIENT_DEBUG_
    }
}

void ThreadTcpCommunication::joinThread() {
    if (sendThread_.joinable()) {
        sendThread_.join();
    }
}

void ThreadTcpCommunication::stopThread() {
    std::lock_guard<std::mutex> lock(mtx_);
    stopFlag_ = true;
}

TcpSignalType::TcpSignalType(bool init, int cn, int sl, const std::vector<std::vector<double>> &channels, int type)
    : isInit(init)
    , channelNum(cn)
    , signalLength(sl)
    , signalType(type) {

    if (signalType == 0 || signalType == 9) {
        // heartbeat packet does not need to carry signal data
        packetLength = sizeof(packetLength) + sizeof(signalType) + sizeof(isInit) + sizeof(channelNum) +
                       sizeof(signalLength) + sizeof(checksum);
    } else {
        // normal signal packet
        channelData.reserve(channelNum * signalLength);
        for (const auto &channel : channels) {
            channelData.insert(channelData.end(), channel.begin(), channel.end());
        }
        packetLength = sizeof(packetLength) + sizeof(signalType) + sizeof(isInit) + sizeof(channelNum) +
                       sizeof(signalLength) + channelData.size() * sizeof(double) + sizeof(checksum);
    }

    // calculate checksum
    checksum = calculateChecksum();
}

uint32_t TcpSignalType::calculateChecksum() const {
    // calculate checksum of channelData using CRC32
    return crc32(0L, reinterpret_cast<const unsigned char *>(channelData.data()), channelData.size() * sizeof(double));
}

// serialize to byte array
void TcpSignalType::serialize(char *buffer) const {
    int offset = 0;

    memcpy(buffer + offset, &packetLength, sizeof(packetLength));
    offset += sizeof(packetLength);

    memcpy(buffer + offset, &signalType, sizeof(signalType));
    offset += sizeof(signalType);

    memcpy(buffer + offset, &isInit, sizeof(isInit));
    offset += sizeof(isInit);

    memcpy(buffer + offset, &channelNum, sizeof(channelNum));
    offset += sizeof(channelNum);

    memcpy(buffer + offset, &signalLength, sizeof(signalLength));
    offset += sizeof(signalLength);

    memcpy(buffer + offset, channelData.data(), channelData.size() * sizeof(double));
    offset += channelData.size() * sizeof(double);

    memcpy(buffer + offset, &checksum, sizeof(checksum));
}

// deserialize from byte array
TcpSignalType TcpSignalType::deserialize(const char *buffer) {
    int      offset = 0;
    int      packetLength, signalType;
    bool     isInit;
    int      channelNum, signalLength;
    uint32_t checksum;

    memcpy(&packetLength, buffer + offset, sizeof(packetLength));
    offset += sizeof(packetLength);

    memcpy(&signalType, buffer + offset, sizeof(signalType));
    offset += sizeof(signalType);

    memcpy(&isInit, buffer + offset, sizeof(isInit));
    offset += sizeof(isInit);

    memcpy(&channelNum, buffer + offset, sizeof(channelNum));
    offset += sizeof(channelNum);

    memcpy(&signalLength, buffer + offset, sizeof(signalLength));
    offset += sizeof(signalLength);

    std::vector<std::vector<double>> channels(channelNum, std::vector<double>(signalLength));
    for (int i = 0; i < channelNum; ++i) {
        memcpy(channels[i].data(), buffer + offset, signalLength * sizeof(double));
        offset += signalLength * sizeof(double);
    }

    memcpy(&checksum, buffer + offset, sizeof(checksum));

    return TcpSignalType(isInit, channelNum, signalLength, channels, signalType);
}