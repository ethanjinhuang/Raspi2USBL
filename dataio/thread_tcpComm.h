/***
 * @Author: Jin Huang jin.huang@zju.edu.cn
 * @Date: 2024-09-08 16:20:00
 * @LastEditors: Jin Huang jin.huang@zju.edu.cn
 * @LastEditTime: 2024-09-14 22:35:00
 * @FilePath: /RaspiUSBL/dataio/thread_tcpComm.h
 * @Description: TCP Communication Thread
 * @
 * @Copyright (c) 2024 by JinHuang  (jin.huang@zju.edu.cn) / Zhejiang University, All Rights Reserved.
 */
#ifndef _THREAD_TCPCOMMUNICATION_H_
#define _THREAD_TCPCOMMUNICATION_H_

#include "../config/defineconfig.h"
#include "../core/systeminfo.h"
#include "../general/typedef.h"
#include "../tool/ColorParse.h"
#include "../tool/SafeQueue.hpp"
#include "tcpServer.h"
#include <cstdint> // for uint32_t
#include <mutex>
#include <thread>
#include <vector>

typedef struct TcpSignalType TcpSignalType;

class ThreadTcpCommunication {
public:
    ThreadTcpCommunication() = delete;
    ThreadTcpCommunication(SystemInfo &systemInfo, tcpServer &server);
    ~ThreadTcpCommunication();

    void init();

    // add data queue
    void setDataQueue(sfq::Safe_Queue<ChannelSignalVector> *dataQueue);

    // init and start thread
    void startSending();

    // wait for heartbeat response
    bool waitForHeartbeatResponse();

    // wait for thread to finish
    void joinThread();

    // stop thread
    void stopThread();

private:
    // Configuration
    int connectTimeout_ = 0; // [ms]
    int sendTimeout_    = 0; // [ms]

    // system info
    SystemInfo systemInfo_; // system info

    // thread function
    void sendData(); // data sending function in thread

    // data queue
    sfq::Safe_Queue<ChannelSignalVector> *signalQueue_ = nullptr; // data queue
    ChannelSignalVector                   data_;                  // data

    // server reference
    tcpServer  &server_;     // server reference
    std::thread sendThread_; // sending thread
    bool        stopFlag_;   // stop flag
    std::mutex  mtx_;        // mutex to protect stopFlag
};

struct TcpSignalType {
    bool isInit;                     // whether initialized
    int  channelNum;                 // number of channels
    int  packetLength;               // length of the whole packet
    int  signalLength;               // length of each channel signal
    int  signalType;                 // signal type or protocol version

    std::vector<double> channelData; // use vector to manage signal data
    uint32_t            checksum;    // checksum

    // constructor
    TcpSignalType(bool init, int cn, int sl, const std::vector<std::vector<double>> &channels, int type = 1);
    // constructor for heartbeat packet
    // TcpSignalType();

    // copy constructor
    TcpSignalType(const TcpSignalType &other) = default;

    // move constructor
    TcpSignalType(TcpSignalType &&other) noexcept = default;

    // copy assignment operator
    TcpSignalType &operator=(const TcpSignalType &other) = default;

    // move assignment operator
    TcpSignalType &operator=(TcpSignalType &&other) noexcept = default;

    // calculate checksum
    uint32_t calculateChecksum() const;

    // serialize to byte array
    void serialize(char *buffer) const;

    // deserialize from byte array
    static TcpSignalType deserialize(const char *buffer);

    ~TcpSignalType() = default; // use default destructor
};

#endif // _THREAD_TCPCOMMUNICATION_H_