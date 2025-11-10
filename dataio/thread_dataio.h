/***
 * @Author: Jin Huang @ jin.huang@zju.edu.cn
 * @Date: 2024-08-30 15:32:30
 * @LastEditors: Jin's Macbook jin.huang@zju.edu.cn
 * @LastEditTime: 2025-11-05 20:40:46
 * @FilePath: /Raspi2USBL/dataio/thread_dataio.h
 * @Description: Using thread to handle data io
 * @             1. Nav result sender
 * @
 * @Copyright (c) 2025 by Jin Huang @ jin.huang@zju.edu.cn, All Rights Reserved.
 */

#ifndef _THREAD_DATAIO_H_
#define _THREAD_DATAIO_H_

#include "../config/defineconfig.h"
#include "../core/systeminfo.h"
#include "../tool/ColorParse.h"
#include "../tool/SafeQueue.hpp"
#include "posResSender.h"
#include "serialDriver.h"
#include <mutex>
#include <string>
#include <thread>

class ThreadDataIO {
public:
    ThreadDataIO() = delete;
    ThreadDataIO(SystemInfo &systemInfo);
    ~ThreadDataIO() = default;

    void init();

    // Set Queue
    void setPosResQueue(sfq::Safe_Queue<PositionResult> *posResQueue);

    // Open Serial Port
    void openPosResSerial(const std::string &serial_port_name, const std::string &baudrate);

    // Close Serial Port
    void closePosResSerial();

    // Send Data Function
    void sendPosResult();

    // Send String
    void sendString(SerialDriver *serial, const std::string &str);

    // Thread Function
    // creat thread
    void creatThreadSendPosResult(sfq::Safe_Queue<PositionResult> *posResQueue);

    // join thread
    void joinThreadSendPosResult();

    // close thread
    void closeThreadSendPosResult();

private:
    // System Info
    SystemInfo &systemInfo_;

    // Result Queue
    sfq::Safe_Queue<PositionResult>      *posResQueue_            = nullptr;
    sfq::Safe_Queue<std::vector<double>> *signalTOFQueue_         = nullptr;
    sfq::Safe_Queue<ChannelSignalVector> *signalCorrelationQueue_ = nullptr;
    sfq::Safe_Queue<ChannelSignalVector> *signalSideAmpSpecQueue_ = nullptr;
    sfq::Safe_Queue<Eigen::MatrixXd>     *beamPatternQueue_       = nullptr;

    // Data Queue
    sfq::Safe_Queue<ChannelSignalVector> *dataQueue_ = nullptr;

    // Data
    PositionResult posResData_;

    // Serial Port
    SerialDriver posResSerial_;

    // Thread
    std::thread threadSendPosResult_;

    // Sender
    PosResSender *posResSender_ = nullptr;

    // Temp data
    std::string tempStr_;
};

#endif // _THREAD_DATAIO_H_